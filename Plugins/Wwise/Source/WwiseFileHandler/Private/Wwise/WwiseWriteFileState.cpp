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

#include "Wwise/WwiseWriteFileState.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"

#include <inttypes.h>

FWwiseWriteFileState::FWwiseWriteFileState(IFileHandle* InFileHandle, const FString& InFilePathName):
	FileHandle(InFileHandle),
	FilePathName(InFilePathName)
{
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Creating writable file %s"), *FilePathName);
	ASYNC_INC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
}

FWwiseWriteFileState::~FWwiseWriteFileState()
{
	UE_CLOG(UNLIKELY(FileHandle), LogWwiseFileHandler, Error, TEXT("FWwiseWriteFileState::~FWwiseWriteFileState: Closing stream with a FileHandle still existing."));
	DEC_DWORD_STAT(STAT_WwiseFileHandlerOpenedStreams);
}

void FWwiseWriteFileState::CloseStreaming()
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseWriteFileState::CloseStreaming"));
	if (UNLIKELY(!FileStateExecutionQueue))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseWriteFileState::CloseStreaming: %s file state %" PRIu32" already Term!"), GetManagingTypeName(), GetShortId());
	}
	else
	{
		FileStateExecutionQueue->AsyncWait(WWISEFILEHANDLER_ASYNC_NAME("FWwiseWriteFileState::CloseStreaming"), [this]
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("ProcessWrite: Closing file %s"), *FilePathName);
			if (FileHandle)
			{
				delete FileHandle;
				FileHandle = nullptr;
			}
		});
	}
	delete this;
}

bool FWwiseWriteFileState::CanProcessFileOp() const
{
	if (UNLIKELY(State != EState::Loaded))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("WriteFileState %s: IO Hook asked for a file operation, but state is not ready."), *FilePathName);
		return false;
	}
	return true;
}

AKRESULT FWwiseWriteFileState::ProcessWrite(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseAkFileOperationDone&& InFileOpDoneCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseWriteFileState::ProcessWrite"));
	if (UNLIKELY(!FileStateExecutionQueue))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseWriteFileState::ProcessWrite: %s file state %" PRIu32" already Term!"), GetManagingTypeName(), GetShortId());
		InFileOpDoneCallback(&OutTransferInfo, AK_NotInitialized);
		return AK_NotInitialized;
	}
	FileStateExecutionQueue->Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseWriteFileState::ProcessWrite Async"), [this, InFileDesc, InHeuristics, OutTransferInfo, InFileOpDoneCallback = MoveTemp(InFileOpDoneCallback)]() mutable
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ProcessWrite: Writing %" PRIu32 " bytes @ %" PRIu64 " in file %s"),
			OutTransferInfo.uBufferSize, OutTransferInfo.uFilePosition, *FilePathName);

		FileHandle->Seek(OutTransferInfo.uFilePosition);

		AKRESULT Result;
		if (LIKELY(FileHandle->Write(static_cast<uint8*>(OutTransferInfo.pBuffer), OutTransferInfo.uBufferSize)))
		{
			Result = AK_Success;
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("ProcessWrite: Failed writing %" PRIu32 " bytes @ %" PRIu64 " in file %s"),
				OutTransferInfo.uBufferSize, OutTransferInfo.uFilePosition, *FilePathName);

			Result = AK_UnknownFileError;
		}
		InFileOpDoneCallback(&OutTransferInfo, Result);
		return Result;
	});

	return AK_Success;
}
