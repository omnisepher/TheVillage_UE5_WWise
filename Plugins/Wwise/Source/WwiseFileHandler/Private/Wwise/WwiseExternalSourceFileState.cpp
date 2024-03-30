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

#include "Wwise/WwiseExternalSourceFileState.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseFileCache.h"
#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/WwiseTask.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/FileHandlerMemory.h"

#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseExternalSourceFileState::FWwiseExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory,
                                                             uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId) :
	AkExternalSourceInfo(),
	MemoryAlignment(InMemoryAlignment),
	bDeviceMemory(bInDeviceMemory),
	MediaId(InMediaId),
	MediaPathName(InMediaPathName),
	RootPath(InRootPath),
	PlayCount(0)
{
	idCodec = InCodecId;
	INC_DWORD_STAT(STAT_WwiseFileHandlerKnownExternalSourceMedia);
}

FWwiseExternalSourceFileState::~FWwiseExternalSourceFileState()
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerKnownExternalSourceMedia);
}

bool FWwiseExternalSourceFileState::GetExternalSourceInfo(AkExternalSourceInfo& OutInfo)
{
	OutInfo = static_cast<AkExternalSourceInfo>(*this);
	return szFile != nullptr || pInMemory != nullptr || idFile != 0;
}

void FWwiseExternalSourceFileState::IncrementPlayCount()
{
	++PlayCount;
}


bool FWwiseExternalSourceFileState::DecrementPlayCount()
{
	const auto NewPlayCount = PlayCount.DecrementExchange() - 1;
	if (PlayCount < 0)
	{
		PlayCount.Store(0);
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseExternalSourceFileState: Play count went below zero for media %" PRIu32 " (%s)"),
		 MediaId, *MediaPathName.ToString());
	}
	return NewPlayCount == 0;
}

FWwiseInMemoryExternalSourceFileState::FWwiseInMemoryExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory,
		uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId) :
	FWwiseExternalSourceFileState(InMemoryAlignment, bInDeviceMemory, InMediaId, InMediaPathName, InRootPath, InCodecId),
	Ptr(nullptr),
	MappedHandle(nullptr),
	MappedRegion(nullptr)
{
#if WITH_EDITOR
	 if (bDeviceMemory)
	 {
	 	UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseExternalSourceFileState: Loading External Source Media with DeviceMemory=true while in in editor. Expect to see \"No Device Memory\" errors in the log."));
	 }
#endif
}

void FWwiseInMemoryExternalSourceFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryExternalSourceFileState::OpenFile"));
	if (UNLIKELY(uiMemorySize || pInMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseInMemoryExternalSourceFileState::OpenFile %" PRIu32 " (%s): Seems to be already opened."), MediaId, *MediaPathName.ToString());
		return OpenFileFailed(MoveTemp(InCallback));
	}

	const auto FullPathName = RootPath.ToString() / MediaPathName.ToString();
	GetFileToPtr([this, FullPathName, InCallback = MoveTemp(InCallback)](bool bInResult, const uint8* InPtr, int64 InSize) mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryExternalSourceFileState::OpenFile Callback"));
		if (LIKELY(bInResult))
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseInMemoryExternalSourceFileState::OpenFile %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());
			pInMemory = const_cast<uint8*>(InPtr);
			uiMemorySize = InSize;
			return OpenFileSucceeded(MoveTemp(InCallback));
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseInMemoryExternalSourceFileState::OpenFile %" PRIu32 ": Failed to open In-Memory External Source (%s)."), MediaId, *FullPathName);
			pInMemory = nullptr;
			uiMemorySize = 0;
			return OpenFileFailed(MoveTemp(InCallback));
		}
	},
		FullPathName, bDeviceMemory, MemoryAlignment, true,
		STAT_WwiseMemoryExtSrc_FName, STAT_WwiseMemoryExtSrcDevice_FName);
}

void FWwiseInMemoryExternalSourceFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryExternalSourceFileState::LoadInSoundEngine"));
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseInMemoryExternalSourceFileState::LoadInSoundEngine %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());
	INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
	LoadInSoundEngineSucceeded(MoveTemp(InCallback));
}

void FWwiseInMemoryExternalSourceFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryExternalSourceFileState::UnloadFromSoundEngine"));
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseInMemoryExternalSourceFileState::UnloadFromSoundEngine %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());
	DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
	UnloadFromSoundEngineDone(MoveTemp(InCallback));
}

void FWwiseInMemoryExternalSourceFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryExternalSourceFileState::CloseFile"));
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseInMemoryExternalSourceFileState::CloseFile %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());
	DeallocateMemory(const_cast<const uint8*&>(reinterpret_cast<uint8*&>(pInMemory)), uiMemorySize, bDeviceMemory, MemoryAlignment, true, STAT_WwiseMemoryExtSrc_FName, STAT_WwiseMemoryExtSrcDevice_FName);
	pInMemory = nullptr;
	uiMemorySize = 0;
	CloseFileDone(MoveTemp(InCallback));
}

FWwiseStreamedExternalSourceFileState::FWwiseStreamedExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory,
	uint32 InPrefetchSize, uint32 InStreamingGranularity,
	uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId) :
	FWwiseExternalSourceFileState(InMemoryAlignment, bInDeviceMemory, InMediaId, InMediaPathName, InRootPath, InCodecId),
	PrefetchSize(InPrefetchSize),
	StreamingGranularity(InStreamingGranularity),
	StreamedFile(nullptr)
{
	sourceID = InMediaId;
	pMediaMemory = nullptr;
	uMediaSize = 0;

	idFile = InMediaId;
}

void FWwiseStreamedExternalSourceFileState::CloseStreaming()
{
	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseStreamedExternalSourceFileState::CloseStreaming %" PRIu32 " (%s): Closing without an ExternalSourceManager."), MediaId, *MediaPathName.ToString());
		return;
	}
	ExternalSourceManager->GetStreamingHooks().CloseStreaming(MediaId, *this);
}

void FWwiseStreamedExternalSourceFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedExternalSourceFileState::OpenFile"));
	if (UNLIKELY(iFileSize != 0 || StreamedFile))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedExternalSourceFileState::OpenFile %" PRIu32 " (%s): Stream seems to be already opened."), MediaId, *MediaPathName.ToString());
		return OpenFileFailed(MoveTemp(InCallback));
	}

	if (PrefetchSize == 0)
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::OpenFile %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());
		return OpenFileSucceeded(MoveTemp(InCallback));
	}

	// Process PrefetchSize and send as SetMedia
	const auto FullPathName = RootPath.ToString() / MediaPathName.ToString();

	auto PrefetchWithGranularity = PrefetchSize;
	if (StreamingGranularity > 1)
	{
		auto PrefetchChunks = PrefetchSize / StreamingGranularity;
		if (PrefetchSize % StreamingGranularity > 0)
		{
			PrefetchChunks += 1;
		}
		PrefetchWithGranularity = PrefetchChunks * StreamingGranularity;
	}

	GetFileToPtr([this, FullPathName, PrefetchWithGranularity, InCallback = MoveTemp(InCallback)](bool bResult, const uint8* InPtr, int64 InSize) mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedExternalSourceFileState::OpenFile Callback"));
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedExternalSourceFileState::OpenFile %" PRIu32 " (%s): Failed to Read prefetch ExternalSource (%s)."), MediaId, *MediaPathName.ToString(), *FullPathName);
			pMediaMemory = nullptr;
			return OpenFileFailed(MoveTemp(InCallback));
		}
		INC_DWORD_STAT(STAT_WwiseFileHandlerPrefetchedExternalSourceMedia);
		uMediaSize = InSize;

		UE_CLOG(InSize == PrefetchSize, LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::OpenFile %" PRIu32 " (%s): Prefetched %" PRIu32 " bytes."), MediaId, *MediaPathName.ToString(), uMediaSize);
		UE_CLOG(PrefetchSize != PrefetchWithGranularity && PrefetchWithGranularity == InSize, LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::OpenFile %" PRIu32 " (%s): Prefetched (%" PRIu32 ") -> %" PRIu32 " bytes."), MediaId, *MediaPathName.ToString(), PrefetchSize, PrefetchWithGranularity);
		UE_CLOG(PrefetchWithGranularity != InSize, LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::OpenFile %" PRIu32 " (%s): Prefetched (%" PRIu32 " -> %" PRIu32 ") -> %" PRIu32 " bytes."), MediaId, *MediaPathName.ToString(), PrefetchSize, PrefetchWithGranularity, uMediaSize);
		return OpenFileSucceeded(MoveTemp(InCallback));
	},
		FullPathName, false, 0, false,
		STAT_WwiseMemoryExtSrcPrefetch_FName, STAT_WwiseMemoryExtSrcPrefetchDevice_FName,
		AIOP_Normal, PrefetchWithGranularity);
}

void FWwiseStreamedExternalSourceFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedExternalSourceFileState::LoadInSoundEngine"));
	if (UNLIKELY(iFileSize != 0 || StreamedFile))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedExternalSourceFileState::LoadInSoundEngine %" PRIu32 " (%s): Stream seems to be already loaded."), MediaId, *MediaPathName.ToString());
		return LoadInSoundEngineFailed(MoveTemp(InCallback));
	}

	FWwiseFileCache* FileCache = FWwiseFileCache::Get();
	if (UNLIKELY(!FileCache))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedExternalSourceFileState::LoadInSoundEngine %" PRIu32 " (%s): WwiseFileCache not available."), MediaId, *MediaPathName.ToString());
		return LoadInSoundEngineFailed(MoveTemp(InCallback));
	}

	const auto FullPathName = RootPath.ToString() / MediaPathName.ToString();

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamedExternalSourceFileState::LoadInSoundEngine %" PRIu32 " (%s): Opening file"), MediaId, *MediaPathName.ToString());
	FileCache->CreateFileCacheHandle(StreamedFile, FullPathName, [this, Callback = MoveTemp(InCallback)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedExternalSourceFileState::LoadInSoundEngine %" PRIu32 ": Failed to load Streaming ExternalSource (%s)."), MediaId, *MediaPathName.ToString());
			LaunchWwiseTask(WWISEFILEHANDLER_ASYNC_NAME("FWwiseStreamedExternalSourceFileState::LoadInSoundEngine delete"), [StreamedFile = StreamedFile]
			{
				delete StreamedFile;
			});
			StreamedFile = nullptr;
			return LoadInSoundEngineFailed(MoveTemp(Callback));
		}

		iFileSize = StreamedFile->GetFileSize();
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::LoadInSoundEngine %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());
		INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
		return LoadInSoundEngineSucceeded(MoveTemp(Callback));
	});
}

void FWwiseStreamedExternalSourceFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedExternalSourceFileState::UnloadFromSoundEngine"));
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::UnloadFromSoundEngine %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());

	const auto* StreamedFileToDelete = StreamedFile;
	StreamedFile = nullptr;
	iFileSize = 0;

	delete StreamedFileToDelete;

	DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedExternalSourceMedia);
	UnloadFromSoundEngineDone(MoveTemp(InCallback));
}

void FWwiseStreamedExternalSourceFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedExternalSourceFileState::CloseFile"));
	if (pMediaMemory == nullptr)
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::CloseFile %" PRIu32 " (%s)"), MediaId, *MediaPathName.ToString());
		return CloseFileDone(MoveTemp(InCallback));
	}

	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedExternalSourceFileState::CloseFile %" PRIu32 " (%s): Unloaded prefetch."), MediaId, *MediaPathName.ToString());
	DeallocateMemory(pMediaMemory, uMediaSize, bDeviceMemory, MemoryAlignment, true, STAT_WwiseMemoryExtSrcPrefetch_FName, STAT_WwiseMemoryExtSrcPrefetchDevice_FName);
	pMediaMemory = nullptr;
	uMediaSize = 0;
	DEC_DWORD_STAT(STAT_WwiseFileHandlerPrefetchedExternalSourceMedia);
	return CloseFileDone(MoveTemp(InCallback));
}

bool FWwiseStreamedExternalSourceFileState::CanProcessFileOp() const
{
	if (UNLIKELY(State != EState::Loaded))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedExternalSourceFileState::CanProcessFileOp %" PRIu32 " (%s): IO Hook asked for a file operation, but state is not ready."), MediaId, *MediaPathName.ToString());
		return false;
	}
	return true;
}

AKRESULT FWwiseStreamedExternalSourceFileState::ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseAkFileOperationDone&& InFileOpDoneCallback)
{
	if (pMediaMemory && OutTransferInfo.uFilePosition + OutTransferInfo.uRequestedSize <= uMediaSize)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamedExternalSourceFileState::ProcessRead: Reading prefetch %" PRIu32 " bytes @ %" PRIu64 " in file %" PRIu32 " (%s)"),
			OutTransferInfo.uRequestedSize, OutTransferInfo.uFilePosition, MediaId, *MediaPathName.ToString());
		FMemory::Memcpy(OutTransferInfo.pBuffer, pMediaMemory + OutTransferInfo.uFilePosition, OutTransferInfo.uRequestedSize);
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseStreamedExternalSourceFileState::ProcessRead Callback"));
		InFileOpDoneCallback(&OutTransferInfo, AK_Success);
		return AK_Success;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamedExternalSourceFileState::ProcessRead: Reading %" PRIu32 " bytes @ %" PRIu64 " in file %" PRIu32 " (%s)"),
		OutTransferInfo.uRequestedSize, OutTransferInfo.uFilePosition, MediaId, *MediaPathName.ToString());

	StreamedFile->ReadAkData(InHeuristics, OutTransferInfo, MoveTemp(InFileOpDoneCallback));
	return AK_Success;
}
