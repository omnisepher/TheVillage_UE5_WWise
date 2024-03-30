/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/WwiseFileCache.h"

#include "Wwise/WwiseExecutionQueue.h"
#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"
#include "Wwise/WwiseTask.h"
#include "WwiseUnrealDefines.h"

#include "Async/Async.h"
#include "Async/AsyncFileHandle.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif

#include <inttypes.h>

FWwiseFileCache* FWwiseFileCache::Get()
{
	if (auto* Module = IWwiseFileHandlerModule::GetModule())
	{
		if (auto* FileCache = Module->GetFileCache())
		{
			return FileCache;
		}
	}
	return nullptr;
}

FWwiseFileCache::FWwiseFileCache():
	OpenQueue(TEXT("FWwiseFileCache OpenQueue"), EWwiseTaskPriority::BackgroundHigh),
	DeleteRequestQueue(TEXT("FWwiseFileCache DeleteRequestQueue"), EWwiseTaskPriority::BackgroundLow)
{
}

FWwiseFileCache::~FWwiseFileCache()
{
}

void FWwiseFileCache::CreateFileCacheHandle(
	FWwiseFileCacheHandle*& OutHandle,
	const FString& Pathname,
	FWwiseFileOperationDone&& OnDone)
{
	OutHandle = new FWwiseFileCacheHandle(Pathname);
	if (UNLIKELY(!OutHandle))
	{
		OnDone(false);
	}
	OutHandle->Open(MoveTemp(OnDone));
}

FWwiseFileCacheHandle::FWwiseFileCacheHandle(const FString& InPathname) :
	Pathname { InPathname },
	FileHandle { nullptr },
	FileSize { 0 },
	InitializationStat { nullptr }
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::FWwiseFileCacheHandle (%p) Creating %s."), this, *Pathname);
}

FWwiseFileCacheHandle::~FWwiseFileCacheHandle()
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::~FWwiseFileCacheHandle"));

	const auto* FileHandleToDestroy = FileHandle; FileHandle = nullptr;

	const auto NumRequests = RequestsInFlight.load(std::memory_order_seq_cst);
	if (LIKELY(NumRequests == 0))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::~FWwiseFileCacheHandle (%p): Closing %s."), this, *Pathname);
		delete FileHandleToDestroy;
		DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
	}
	else
	{
		UE_CLOG(NumRequests > 0, LogWwiseFileHandler, Log, TEXT("FWwiseFileCacheHandle::~FWwiseFileCacheHandle (%p): Closing %s with %" PRIi32 " operation(s) left to process! Use CloseAndDelete!"), this, *Pathname, NumRequests);
		UE_CLOG(NumRequests < 0, LogWwiseFileHandler, Log, TEXT("FWwiseFileCacheHandle::~FWwiseFileCacheHandle (%p): Closing %s and leaking."), this, *Pathname);
	}
}

void FWwiseFileCacheHandle::CloseAndDelete()
{
	const auto FileCache = FWwiseFileCache::Get();
	if (LIKELY(FileCache))
	{
		FileCache->DeleteRequestQueue.AsyncAlways(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileCacheHandle::DeleteRequest"), [this]() mutable
		{
			OnCloseAndDelete();
		});
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseFileCacheHandle::CloseAndDelete (%p): Closing Request for %s after FileCache module is destroyed. Will leak."), this, *Pathname);
	}
}

void FWwiseFileCacheHandle::Open(FWwiseFileOperationDone&& OnDone)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::Open"));
	check(!InitializationStat);
	check(!FileHandle);

	InitializationStat = new FWwiseAsyncCycleCounter(GET_STATID(STAT_WwiseFileHandlerFileOperationLatency));
	InitializationDone = MoveTemp(OnDone);

	FWwiseAsyncCycleCounter Stat(GET_STATID(STAT_WwiseFileHandlerFileOperationLatency));

	const auto FileCache = FWwiseFileCache::Get();
	if (UNLIKELY(!FileCache))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseFileCacheHandle::Open (%p): FileCache not available while opening %s."), this, *Pathname);
		delete InitializationStat; InitializationStat = nullptr;
		CallDone(false, MoveTemp(InitializationDone));
		return;
	}

	++RequestsInFlight;
	FileCache->OpenQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileCacheHandle::Open async"), [this, OnDone = MoveTemp(OnDone)]() mutable
	{
		check(!FileHandle);

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::Open (%p): Opening %s."), this, *Pathname);
		IAsyncReadFileHandle* CurrentFileHandle;
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
		{
			SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::Open OpenAsyncRead"));
			CurrentFileHandle = FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenAsyncRead(*Pathname);
		}
		if (UNLIKELY(!CurrentFileHandle))
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseFileCacheHandle::Open (%p): OpenAsyncRead %s failed instantiating."), this, *Pathname);
			delete InitializationStat; InitializationStat = nullptr;
			CallDone(false, MoveTemp(InitializationDone));
			RemoveRequestInFlight();
			return;
		}

		FAsyncFileCallBack SizeCallbackFunction = [this](bool bWasCancelled, IAsyncReadRequest* Request) mutable
		{
			OnSizeRequestDone(bWasCancelled, Request);
		};
		IAsyncReadRequest* Request;
		{
			SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::Open SizeRequest"));
			// ++RequestsInFlight; already done
			Request = CurrentFileHandle->SizeRequest(&SizeCallbackFunction);
		}
		if (UNLIKELY(!Request))
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseFileCacheHandle::Open (%p): SizeRequest %s failed instantiating."), this, *Pathname);
			delete InitializationStat; InitializationStat = nullptr;
			CallDone(false, MoveTemp(InitializationDone));
			RemoveRequestInFlight();
		}
	});
}

void FWwiseFileCacheHandle::OnSizeRequestDone(bool bWasCancelled, IAsyncReadRequest* Request)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::OnSizeRequestDone"));
	FileSize = Request->GetSizeResults();

	const bool bSizeOpSuccess = LIKELY(FileSize > 0);

	UE_CLOG(!bSizeOpSuccess, LogWwiseFileHandler, Log, TEXT("FWwiseFileCacheHandle::OnSizeRequestDone (%p): Streamed file \"%s\" could not be opened."), this, *Pathname);
	UE_CLOG(bSizeOpSuccess, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::OnSizeRequestDone (%p): Initializing %s succeeded."), this, *Pathname);
	delete InitializationStat; InitializationStat = nullptr;
	CallDone(bSizeOpSuccess, MoveTemp(InitializationDone));
	DeleteRequest(Request);
}

void FWwiseFileCacheHandle::CallDone(bool bResult, FWwiseFileOperationDone&& OnDone)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::CallDone Callback"));
	OnDone(bResult);
}

void FWwiseFileCacheHandle::DeleteRequest(IAsyncReadRequest* Request)
{
	const auto FileCache = FWwiseFileCache::Get();
	if (LIKELY(FileCache))
	{
		FileCache->DeleteRequestQueue.AsyncAlways(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileCacheHandle::DeleteRequest"), [this, Request]() mutable
		{
			OnDeleteRequest(Request);
		});
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseFileCacheHandle::DeleteRequest (%p): Deleting Request for %s after FileCache module is destroyed. Will leak."), this, *Pathname);
		if (--RequestsInFlight == 0)
		{
			RequestsInFlight = -1;
		}
	}
}

void FWwiseFileCacheHandle::OnDeleteRequest(IAsyncReadRequest* Request)
{
	if (Request && !Request->WaitCompletion(1))
	{
		return DeleteRequest(Request);
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::DeleteRequest (%p req:%p) [%" PRIi32 "]: Deleting request."), this, Request, FPlatformTLS::GetCurrentThreadId());
	delete Request;
	RemoveRequestInFlight();
}

void FWwiseFileCacheHandle::RemoveRequestInFlight()
{
	const auto FileCache = FWwiseFileCache::Get();
	if (LIKELY(FileCache))
	{
		FileCache->DeleteRequestQueue.AsyncAlways(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileCacheHandle::RemoveRequestInFlight"), [this]() mutable
		{
			--RequestsInFlight;
		});
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseFileCacheHandle::RemoveRequestInFlight (%p): Removing Request for %s after FileCache module is destroyed. Will leak."), this, *Pathname);
		if (--RequestsInFlight == 0)
		{
			RequestsInFlight = -1;
		}
	}
}

void FWwiseFileCacheHandle::OnCloseAndDelete()
{
	if (RequestsInFlight > 0)
	{
		return CloseAndDelete();
	}
	delete this;
}

void FWwiseFileCacheHandle::ReadData(uint8* OutBuffer, int64 Offset, int64 BytesToRead,
	EAsyncIOPriorityAndFlags Priority, FWwiseFileOperationDone&& OnDone)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::ReadData"));
	FWwiseAsyncCycleCounter Stat(GET_STATID(STAT_WwiseFileHandlerFileOperationLatency));
	++RequestsInFlight;

	IAsyncReadFileHandle* CurrentFileHandle = FileHandle;
	if (UNLIKELY(!CurrentFileHandle))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileCacheHandle::ReadData (%p): Trying to read in file %s while it was not properly initialized."), this, *Pathname);
		OnReadDataDone(false, MoveTemp(OnDone));
		return;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::ReadData (%p): %" PRIi64 "@%" PRIi64 " in %s"), this, BytesToRead, Offset, *Pathname);
	FAsyncFileCallBack ReadCallbackFunction = [this, OnDone = new FWwiseFileOperationDone(MoveTemp(OnDone)), BytesToRead, Stat = MoveTemp(Stat)](bool bWasCancelled, IAsyncReadRequest* Request) mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::ReadData Callback"));
		if (!bWasCancelled && Request)	// Do not add Request->GetReadResults() since it will break subsequent results retrievals.
		{
			ASYNC_INC_FLOAT_STAT_BY(STAT_WwiseFileHandlerTotalStreamedMB, static_cast<float>(BytesToRead) / 1024 / 1024);
		}
		OnReadDataDone(bWasCancelled, Request, MoveTemp(*OnDone));
		delete OnDone;
		DeleteRequest(Request);
	};
	ASYNC_INC_FLOAT_STAT_BY(STAT_WwiseFileHandlerStreamingKB, static_cast<float>(BytesToRead) / 1024);
	check(BytesToRead > 0);

#if STATS
	switch (Priority & EAsyncIOPriorityAndFlags::AIOP_PRIORITY_MASK)
	{
	case EAsyncIOPriorityAndFlags::AIOP_CriticalPath: INC_DWORD_STAT(STAT_WwiseFileHandlerCriticalPriority); break;
	case EAsyncIOPriorityAndFlags::AIOP_High: INC_DWORD_STAT(STAT_WwiseFileHandlerHighPriority); break;
	case EAsyncIOPriorityAndFlags::AIOP_BelowNormal: INC_DWORD_STAT(STAT_WwiseFileHandlerBelowNormalPriority); break;
	case EAsyncIOPriorityAndFlags::AIOP_Low: INC_DWORD_STAT(STAT_WwiseFileHandlerLowPriority); break;
	case EAsyncIOPriorityAndFlags::AIOP_MIN: INC_DWORD_STAT(STAT_WwiseFileHandlerBackgroundPriority); break;

	default:
	case EAsyncIOPriorityAndFlags::AIOP_Normal: INC_DWORD_STAT(STAT_WwiseFileHandlerNormalPriority); break;
	}
#endif
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileCacheHandle::ReadData Async ReadRequest"));
	const auto* Request = CurrentFileHandle->ReadRequest(Offset, BytesToRead, Priority, &ReadCallbackFunction, OutBuffer);
	if (UNLIKELY(!Request))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseFileCacheHandle::ReadData (%p): ReadRequest %s failed instantiating."), this, *Pathname);
		ReadCallbackFunction(true, nullptr);
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::ReadData (%p req:%p) [%" PRIi32 "]: Created request"), this, Request, FPlatformTLS::GetCurrentThreadId());
	}
}

void FWwiseFileCacheHandle::ReadAkData(uint8* OutBuffer, int64 Offset, int64 BytesToRead, int8 AkPriority, FWwiseFileOperationDone&& OnDone)
{
	// Wwise priority is what we expect our priority to be. Audio will skip if "our normal" is not met.
	constexpr const auto bHigherAudioPriority = true;

	EAsyncIOPriorityAndFlags Priority;
	if (LIKELY(AkPriority == AK_DEFAULT_PRIORITY))
	{
		Priority = bHigherAudioPriority ? AIOP_High : AIOP_Normal;
	}
	else if (AkPriority <= AK_MIN_PRIORITY)
	{
		Priority = bHigherAudioPriority ? AIOP_BelowNormal : AIOP_Low;
	}
	else if (AkPriority >= AK_MAX_PRIORITY)
	{
		Priority = AIOP_CriticalPath;
	}
	else if (AkPriority < AK_DEFAULT_PRIORITY)
	{
		Priority = bHigherAudioPriority ? AIOP_Normal : AIOP_Low;
	}
	else
	{
		Priority = bHigherAudioPriority ? AIOP_CriticalPath : AIOP_High;
	}
	ReadData(OutBuffer, Offset, BytesToRead, Priority, MoveTemp(OnDone));
}

void FWwiseFileCacheHandle::ReadAkData(const AkIoHeuristics& Heuristics, AkAsyncIOTransferInfo& TransferInfo,
	FWwiseAkFileOperationDone&& Callback)
{
	ReadAkData(
		static_cast<uint8*>(TransferInfo.pBuffer),
		static_cast<int64>(TransferInfo.uFilePosition),
		static_cast<int64>(TransferInfo.uRequestedSize),
		Heuristics.priority,
		[TransferInfo = &TransferInfo, FileOpDoneCallback = MoveTemp(Callback)](bool bResult)
		{
			FileOpDoneCallback(TransferInfo, bResult ? AK_Success : AK_UnknownFileError);
		});
}


void FWwiseFileCacheHandle::OnReadDataDone(bool bWasCancelled, IAsyncReadRequest* Request,
	FWwiseFileOperationDone&& OnDone)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileCacheHandle::OnReadDataDone (%p req:%p) [%" PRIi32 "]: Request done."), this, Request, FPlatformTLS::GetCurrentThreadId());
	OnReadDataDone(!bWasCancelled && Request && Request->GetReadResults(), MoveTemp(OnDone));
}

void FWwiseFileCacheHandle::OnReadDataDone(bool bResult, FWwiseFileOperationDone&& OnDone)
{
	CallDone(bResult, MoveTemp(OnDone));
}
