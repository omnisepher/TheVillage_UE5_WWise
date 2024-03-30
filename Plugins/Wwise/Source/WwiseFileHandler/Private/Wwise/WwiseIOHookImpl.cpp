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

#include "Wwise/WwiseIOHookImpl.h"

#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/WwiseWriteFileState.h"
#include "Wwise/WwiseSoundBankManager.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseStreamableFileHandler.h"

#include "Wwise/Stats/AsyncStats.h"

#include "WwiseDefines.h"
#include "WwiseUnrealDefines.h"

#include "Async/Async.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif

#include <inttypes.h>

FWwiseIOHookImpl::FWwiseIOHookImpl() :
	BatchExecutionQueue(TEXT("Wwise IO Hook Batch"), EWwiseTaskPriority::High)
#ifndef AK_OPTIMIZED
	,
	CurrentDeviceData(0),
	MaxDeviceData(0)
#endif
{
}

bool FWwiseIOHookImpl::Init(const AkDeviceSettings& InDeviceSettings)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::Init"));
	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (LIKELY(ExternalSourceManager))
	{
		ExternalSourceManager->SetGranularity(InDeviceSettings.uGranularity);
	}
	auto* MediaManager = IWwiseMediaManager::Get();
	if (LIKELY(MediaManager))
	{
		MediaManager->SetGranularity(InDeviceSettings.uGranularity);
	}
	auto* SoundBankManager = IWwiseSoundBankManager::Get();
	if (LIKELY(SoundBankManager))
	{
		SoundBankManager->SetGranularity(InDeviceSettings.uGranularity);
	}
	return FWwiseDefaultIOHook::Init(InDeviceSettings);
}

void FWwiseIOHookImpl::BatchOpen(
	AkUInt32				in_uNumFiles,
	AkAsyncFileOpenData**	in_ppItems)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::BatchOpen"));
	for (AkUInt32 i = 0; i < in_uNumFiles; i++)
	{		
		AkAsyncFileOpenData* pOpenData = in_ppItems[i];

		if (pOpenData)
		{
			Open(pOpenData);
		}
	}	
}


AKRESULT FWwiseIOHookImpl::Open(AkAsyncFileOpenData* io_pOpenData)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::Open"));

	AKRESULT AkResult;

	const FString Filename(io_pOpenData->pszFileName);

	if (UNLIKELY(IsEngineExitRequested()))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseIOHookImpl::Open: Ignoring opening file at Engine exit for File ID %" PRIu32), io_pOpenData->fileID);

		// Notify error to callback
		if (io_pOpenData->pCallback)
			io_pOpenData->pCallback(io_pOpenData, AK_NotInitialized);

		return AK_Success;
	}

	if (io_pOpenData->eOpenMode == AK_OpenModeWrite || io_pOpenData->eOpenMode == AK_OpenModeWriteOvrwr)
	{
		AkResult = OpenFileForWrite(io_pOpenData);
		// Notify result to callback
		if (io_pOpenData->pCallback)
			io_pOpenData->pCallback(io_pOpenData, AkResult);

		return AK_Success;
	}

	if (io_pOpenData->eOpenMode != AK_OpenModeRead || !io_pOpenData->pFlags)
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseIOHookImpl::Open: Unsupported Open Mode for File ID %" PRIu32), io_pOpenData->fileID);
		// Notify error to callback
		if (io_pOpenData->pCallback)
			io_pOpenData->pCallback(io_pOpenData, AK_NotImplemented);

		return AK_Success;
	}

	IWwiseStreamingManagerHooks* StreamingHooks = GetStreamingHooks(*io_pOpenData->pFlags);
	if (UNLIKELY(!StreamingHooks))
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseIOHookImpl::Open: Unsupported Streaming for File ID %" PRIu32), io_pOpenData->fileID);
		// Notify error to callback
		if (io_pOpenData->pCallback)
			io_pOpenData->pCallback(io_pOpenData, AK_NotInitialized);

		return AK_Success;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseIOHookImpl::Open: Opening file for streaming: File ID %" PRIu32), io_pOpenData->fileID);
	StreamingHooks->OpenStreaming(io_pOpenData);

	return AK_Success;
}

AKRESULT FWwiseIOHookImpl::Read(
	AkFileDesc& in_fileDesc,
	const AkIoHeuristics& in_heuristics,
	AkAsyncIOTransferInfo& io_transferInfo
)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::Read"));
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalRequests);
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);

#ifndef AK_OPTIMIZED
	++CurrentDeviceData;
	if (CurrentDeviceData > MaxDeviceData)
	{
		MaxDeviceData.Store(CurrentDeviceData);
	}
#endif

	auto* FileState = FWwiseStreamableFileStateInfo::GetFromFileDesc(in_fileDesc);
	if (UNLIKELY(!FileState))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseIOHookImpl::Read [%p]: Could not find File Descriptor"), AK_FILEHANDLE_TO_UINTPTR(in_fileDesc.hFile));
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);
#ifndef AK_OPTIMIZED
		--CurrentDeviceData;
#endif
		return AK_IDNotFound;
	}

	if (UNLIKELY(!FileState->CanProcessFileOp()))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseIOHookImpl::Read [%p]: FileState is not properly initialized for reading"), AK_FILEHANDLE_TO_UINTPTR(in_fileDesc.hFile));
		
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);
#ifndef AK_OPTIMIZED
		--CurrentDeviceData;
#endif
		return AK_UnknownFileError;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseIOHookImpl::Read [%p]: Reading %" PRIu32 " bytes @ %" PRIu64 " - Priority %" PRIi8 " Deadline %f"),
		AK_FILEHANDLE_TO_UINTPTR(in_fileDesc.hFile), io_transferInfo.uRequestedSize, io_transferInfo.uFilePosition, in_heuristics.priority, (double)in_heuristics.fDeadline);
	const auto Result = FileState->ProcessRead(in_fileDesc, in_heuristics, io_transferInfo,
		[this, OpCycleCounter = MoveTemp(OpCycleCounter), hFile = in_fileDesc.hFile]
			(AkAsyncIOTransferInfo* InTransferInfo, AKRESULT InResult) mutable
		{
			ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);
			if (UNLIKELY(InResult != AK_Success))
			{
				ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
			}

#ifndef AK_OPTIMIZED
			--CurrentDeviceData;
#endif
			OpCycleCounter.Stop();

			if (UNLIKELY(!InTransferInfo->pCallback))
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseIOHookImpl::Read [%p]: No callback reading data"), AK_FILEHANDLE_TO_UINTPTR(hFile));
			}
			else
			{
				SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseIOHookImpl::Read SoundEngine Callback"));
				FWwiseAsyncCycleCounter CallbackCycleCounter(GET_STATID(STAT_WwiseFileHandlerSoundEngineCallbackLatency));
				InTransferInfo->pCallback(InTransferInfo, InResult);
			}
		});
	return Result;
}

void FWwiseIOHookImpl::BatchRead(
	AkUInt32 in_uNumTransfers,
	BatchIoTransferItem* in_pTransferItems
)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::BatchRead"));
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerBatchedRequests);

	BatchExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseIOHookImpl::BatchRead Async"), [this, TransferItems = TArray<BatchIoTransferItem>(in_pTransferItems, in_uNumTransfers)]() mutable
	{
		for (auto& TransferItem : TransferItems)
		{
			auto& FileDesc = *TransferItem.pFileDesc;
			const auto& Heuristics = TransferItem.ioHeuristics;
			auto& TransferInfo = *TransferItem.pTransferInfo;

			AKRESULT Result = Read(FileDesc, Heuristics, TransferInfo);
			if (Result != AK_Success)
			{
				TransferInfo.pCallback(&TransferInfo, Result);
			}
		}
	});
}

AKRESULT FWwiseIOHookImpl::Write(
	AkFileDesc& in_fileDesc,
	const AkIoHeuristics& in_heuristics,
	AkAsyncIOTransferInfo& io_transferInfo
)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::Write"));
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalRequests);
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);

#ifndef AK_OPTIMIZED
	++CurrentDeviceData;
	if (CurrentDeviceData > MaxDeviceData)
	{
		MaxDeviceData.Store(CurrentDeviceData);
	}
#endif

	auto* FileState = FWwiseStreamableFileStateInfo::GetFromFileDesc(in_fileDesc);
	if (!FileState)
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseIOHookImpl::Write [%p]: Could not find File Descriptor"), AK_FILEHANDLE_TO_UINTPTR(in_fileDesc.hFile));
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);

#ifndef AK_OPTIMIZED
		--CurrentDeviceData;
#endif
		return AK_IDNotFound;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseIOHookImpl::Write [%p]: Writing %" PRIu32 " bytes @ %" PRIu64),
		AK_FILEHANDLE_TO_UINTPTR(in_fileDesc.hFile), io_transferInfo.uBufferSize, io_transferInfo.uFilePosition);
	const auto Result = FileState->ProcessWrite(in_fileDesc, in_heuristics, io_transferInfo,
		[this, OpCycleCounter = MoveTemp(OpCycleCounter), hFile = in_fileDesc.hFile]
			(AkAsyncIOTransferInfo* InTransferInfo, AKRESULT InResult) mutable
		{
			ASYNC_DEC_DWORD_STAT(STAT_WwiseFileHandlerPendingRequests);
			if (UNLIKELY(InResult != AK_Success))
			{
				ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
			}

#ifndef AK_OPTIMIZED
			--CurrentDeviceData;
#endif
			OpCycleCounter.Stop();

			if (UNLIKELY(!InTransferInfo->pCallback))
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseIOHookImpl::Write [%p]: No callback reading data"), AK_FILEHANDLE_TO_UINTPTR(hFile));
			}
			else
			{
				SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseIOHookImpl::Write SoundEngine Callback"));
				FWwiseAsyncCycleCounter CallbackCycleCounter(GET_STATID(STAT_WwiseFileHandlerSoundEngineCallbackLatency));
				InTransferInfo->pCallback(InTransferInfo, InResult);
			}

		});
	return Result;
}

void FWwiseIOHookImpl::BatchWrite(
	AkUInt32 in_uNumTransfers,
	BatchIoTransferItem* in_pTransferItems
)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::BatchWrite"));
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerBatchedRequests);

	BatchExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseIOHookImpl::BatchWrite Async"), [this, TransferItems = TArray<BatchIoTransferItem>(in_pTransferItems, in_uNumTransfers)]() mutable
	{
		for (auto& TransferItem : TransferItems)
		{
			auto& FileDesc = *TransferItem.pFileDesc;
			const auto& Heuristics = TransferItem.ioHeuristics;
			auto& TransferInfo = *TransferItem.pTransferInfo;

			const auto Result = Write(FileDesc, Heuristics, TransferInfo);
			if (Result != AK_Success)
			{
				TransferInfo.pCallback(&TransferInfo, Result);
			}
		}
	});
}

void FWwiseIOHookImpl::BatchCancel(
	AkUInt32 in_uNumTransfers,
	BatchIoTransferItem* in_pTransferItems,
	bool** io_ppbCancelAllTransfersForThisFile
)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::BatchCancel"));
	if (UNLIKELY(in_uNumTransfers == 0 || !in_pTransferItems))
	{
		return;
	}
	for (AkUInt32 i = 0; i < in_uNumTransfers; ++i)
	{
		const auto& TransferItem = in_pTransferItems[i];
		UE_CLOG(TransferItem.pFileDesc != nullptr,
			LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseIOHookImpl::BatchCancel [%p]: Cancelling transfer unsupported"), AK_FILEHANDLE_TO_UINTPTR(TransferItem.pFileDesc->hFile));
	}
}

AKRESULT FWwiseIOHookImpl::Close(AkFileDesc* in_pFileDesc)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHookImpl::Close"));
	if (UNLIKELY(!in_pFileDesc))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseIOHookImpl::Close: Closing null file."));
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		return AK_IDNotFound;
	}

	if (UNLIKELY(!IWwiseFileHandlerModule::IsAvailable()))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseIOHookImpl::Close [%p]: Not closing file while engine is exiting."), AK_FILEHANDLE_TO_UINTPTR(in_pFileDesc->hFile));
		return AK_Success;
	}

	auto* FileState = FWwiseStreamableFileStateInfo::GetFromFileDesc(*in_pFileDesc);
	if (!FileState)
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseIOHookImpl::Close [%p]: Could not find File Descriptor"), AK_FILEHANDLE_TO_UINTPTR(in_pFileDesc->hFile));
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		return AK_IDNotFound;
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseIOHookImpl::Close [%p]: Closing streaming file"), AK_FILEHANDLE_TO_UINTPTR(in_pFileDesc->hFile));
	FileState->CloseStreaming();
	return AK_Success;
}

// Returns the block size for the file or its storage device.
AkUInt32 FWwiseIOHookImpl::GetBlockSize(AkFileDesc& in_fileDesc)
{
	return 1;
}

// Returns a description for the streaming device above this low-level hook.
void FWwiseIOHookImpl::GetDeviceDesc(AkDeviceDesc& out_deviceDesc)
{
#if !defined(AK_OPTIMIZED)
	// Deferred scheduler.
	out_deviceDesc.deviceID = m_deviceID;
	out_deviceDesc.bCanRead = true;
	out_deviceDesc.bCanWrite = true;
	AK_CHAR_TO_UTF16(out_deviceDesc.szDeviceName, "UnrealIODevice", AK_MONITOR_DEVICENAME_MAXLENGTH);
	out_deviceDesc.szDeviceName[AK_MONITOR_DEVICENAME_MAXLENGTH - 1] = '\0';
	out_deviceDesc.uStringSize = (AkUInt32)AKPLATFORM::AkUtf16StrLen(out_deviceDesc.szDeviceName) + 1;
#endif
}

// Returns custom profiling data: Current number of pending streaming requests
AkUInt32 FWwiseIOHookImpl::GetDeviceData()
{
#ifndef AK_OPTIMIZED
	AkUInt32 Result = MaxDeviceData;
	MaxDeviceData.Store(CurrentDeviceData);
#else
	AkUInt32 Result = 0;
#endif
	return Result;
}

IWwiseStreamingManagerHooks* FWwiseIOHookImpl::GetStreamingHooks(const AkFileSystemFlags& InFileSystemFlag)
{
	IWwiseStreamableFileHandler* WwiseStreamableFileHandler = nullptr;
	if (InFileSystemFlag.uCompanyID == AKCOMPANYID_AUDIOKINETIC_EXTERNAL)
	{
		WwiseStreamableFileHandler = IWwiseExternalSourceManager::Get();
	}
	else if (InFileSystemFlag.uCodecID == AKCODECID_BANK || InFileSystemFlag.uCodecID == AKCODECID_BANK_EVENT || InFileSystemFlag.uCodecID == AKCODECID_BANK_BUS)
	{
		WwiseStreamableFileHandler = IWwiseSoundBankManager::Get();
	}
	else
	{
		WwiseStreamableFileHandler = IWwiseMediaManager::Get();
	}

	if (UNLIKELY(!WwiseStreamableFileHandler))
	{
		return nullptr;
	}
	return &WwiseStreamableFileHandler->GetStreamingHooks();
}

AKRESULT FWwiseIOHookImpl::OpenFileForWrite(AkAsyncFileOpenData* io_pOpenData)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseIOHookImpl::OpenFileForWrite"));
	const auto TargetDirectory = FPaths::ProjectSavedDir() / TEXT("Wwise");
	static bool TargetDirectoryExists = false;
	if (!TargetDirectoryExists && !FPaths::DirectoryExists(TargetDirectory))
	{
		TargetDirectoryExists = true;
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*TargetDirectory))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseIOHookImpl::OpenFileForWrite: Cannot create writable directory at %s"), *TargetDirectory);
			ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
			return AK_NotImplemented;
		}
	}
	const auto FullPath = FPaths::Combine(TargetDirectory, FString(io_pOpenData->pszFileName));

	UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseIOHookImpl::OpenFileForWrite: Opening file for writing: %s"), *FullPath);
	const auto FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FullPath);
	if (UNLIKELY(!FileHandle))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseIOHookImpl::OpenFileForWrite: Cannot open file %s for write."), *FullPath);
		ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		return AK_UnknownFileError;
	}

	auto* WriteState = new FWwiseWriteFileState(FileHandle, FullPath);
	if (WriteState)
	{
		io_pOpenData->pFileDesc = WriteState->GetFileDesc();
		return AK_Success;
	}

	return AK_InsufficientMemory;
}
