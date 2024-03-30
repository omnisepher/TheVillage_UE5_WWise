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

#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/WwiseIOHook.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"
#include "Wwise/WwiseTask.h"

#include "Wwise/Stats/FileHandler.h"
#include "Wwise/Stats/AsyncStats.h"

#include "Misc/ScopeRWLock.h"

#include <inttypes.h>


FWwiseFileHandlerBase::FWwiseFileHandlerBase() :
	FileHandlerExecutionQueue(WWISE_EQ_NAME("FWwiseFileHandler"), EWwiseTaskPriority::High)
{
}

FWwiseFileHandlerBase::~FWwiseFileHandlerBase()
{
	FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_Write);
	if (UNLIKELY(FileStatesById.Num() > 0))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Closing FileHandler with remaining %" PRIu32 " FileStates. Leaking."), FileStatesById.Num());
		for (auto& State : FileStatesById)
		{
			// Locking in memory the file states
			new FWwiseFileStateSharedPtr(State.Value);
		}
	}
}

void FWwiseFileHandlerBase::OpenStreaming(AkAsyncFileOpenData* io_pOpenData)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));

	IncrementFileStateUseAsync(io_pOpenData->fileID, EWwiseFileStateOperationOrigin::Streaming,
		[ManagingTypeName = GetManagingTypeName(), io_pOpenData]
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Trying to open streaming for unknown %s %" PRIu32), ManagingTypeName, io_pOpenData->fileID);
			return FWwiseFileStateSharedPtr{};
		},
		[this, io_pOpenData, OpCycleCounter = MoveTemp(OpCycleCounter)](const FWwiseFileStateSharedPtr& InFileState, bool bInResult) mutable
		{
			OpCycleCounter.Stop();

			SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileHandlerBase::OpenStreaming %s Async"), GetManagingTypeName());
			AKRESULT Result = GetOpenStreamingResult(io_pOpenData);
			if (Result == AK_Success && UNLIKELY(!bInResult))
			{
				Result = AK_UnknownFileError;
			}

			if (LIKELY(Result == AK_Success))
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Succeeded opening %" PRIu32 " for streaming"), io_pOpenData->fileID);
				io_pOpenData->pCallback(io_pOpenData, Result);
			}
			else
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Failed opening %" PRIu32 " for streaming. Doing callback later."), io_pOpenData->fileID);
				LaunchWwiseTask(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileHandlerBase::OpenStreaming Failure Async"), [io_pOpenData, Result]
				{
					io_pOpenData->pCallback(io_pOpenData, Result);
				});
			}
		});
}

AKRESULT FWwiseFileHandlerBase::GetOpenStreamingResult(AkAsyncFileOpenData* io_pOpenData)
{
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerIORequestLatency));

	FWwiseFileStateSharedPtr State;
	{
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		const auto* StatePtr = FileStatesById.Find(io_pOpenData->fileID);
		if (UNLIKELY(!StatePtr || !StatePtr->IsValid()))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not open file %" PRIu32 " for streaming: File wasn't initialized prior to OpenStreaming."), io_pOpenData->fileID);
			return AK_FileNotFound;
		}
		State = *StatePtr;
	}

	AKRESULT Result = AK_Success;
	if (auto* StreamableFileStateInfo = State->GetStreamableFileStateInfo())
	{
		io_pOpenData->pFileDesc = StreamableFileStateInfo->GetFileDesc();
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not open file %" PRIu32 " for streaming: Could not get AkFileDesc."), io_pOpenData->fileID);
		Result = AK_UnknownFileError;
	}
	return Result;
}

void FWwiseFileHandlerBase::CloseStreaming(uint32 InShortId, FWwiseFileState& InFileState)
{
	return DecrementFileStateUseAsync(InShortId, &InFileState, EWwiseFileStateOperationOrigin::Streaming, []{});
}

void FWwiseFileHandlerBase::IncrementFileStateUseAsync(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin,
	FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback)
{
	FileHandlerExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileHandlerBase::IncrementFileStateUseAsync"), [this, InShortId, InOperationOrigin, InCreate = MoveTemp(InCreate), InCallback = MoveTemp(InCallback)]() mutable
	{
		IncrementFileStateUse(InShortId, InOperationOrigin, MoveTemp(InCreate), MoveTemp(InCallback));
	});
}

void FWwiseFileHandlerBase::DecrementFileStateUseAsync(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback)
{
	FileHandlerExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileHandlerBase::DecrementFileStateUseAsync"), [this, InShortId, InFileState, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
	{
		DecrementFileStateUse(InShortId, InFileState, InOperationOrigin, MoveTemp(InCallback));
	});
}

void FWwiseFileHandlerBase::IncrementFileStateUse(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin, FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileHandlerBase::IncrementFileStateUse %s"), GetManagingTypeName());
	FWwiseFileStateSharedPtr State;
	{
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		if (const auto* StatePtr = FileStatesById.Find(InShortId))
		{
			State = *StatePtr;
		}
	}

	if (!State.IsValid())
	{
		FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_Write);
		if (const auto* StatePtr = FileStatesById.Find(InShortId))
		{
			State = *StatePtr;
		}
		else
		{
			State = InCreate();
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Created new State for %s %" PRIu32 " [%p]"), GetManagingTypeName(), InShortId, State.Get());
			if (LIKELY(State.IsValid()))
			{
				FileStatesById.Add(InShortId, State);
			}
		}
	}

	if (UNLIKELY(!State.IsValid()))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Trying to increment invalid state for %s %" PRIu32), GetManagingTypeName(), InShortId);
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileHandlerBase::IncrementFileStateUse Callback"));
		InCallback(State, false);
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Incrementing State for %s %" PRIu32), GetManagingTypeName(), InShortId);
		State->IncrementCountAsync(InOperationOrigin, [this, InOperationOrigin, State, InCallback = MoveTemp(InCallback)](bool bInResult) mutable
		{
			OnIncrementCountAsyncDone(bInResult, InOperationOrigin, State, MoveTemp(InCallback));
		});
	}
}

void FWwiseFileHandlerBase::DecrementFileStateUse(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileHandlerBase::DecrementFileStateUse %s"), GetManagingTypeName());
	if (!InFileState)
	{
		const FWwiseFileStateSharedPtr* StatePtr;
		{
			FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
			StatePtr = FileStatesById.Find(InShortId);
			if (LIKELY(StatePtr && StatePtr->IsValid()))
			{
				InFileState = StatePtr->Get();
			}
		}
		if (UNLIKELY(!StatePtr || !StatePtr->IsValid()))
		{
			UE_LOG(LogWwiseFileHandler, Log, TEXT("Could not find state for for %s %" PRIu32), GetManagingTypeName(), InShortId);
			SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileHandlerBase::DecrementFileStateUse %s Callback"), GetManagingTypeName());
			InCallback();
			return;
		}
	}

	InFileState->DecrementCountAsync(InOperationOrigin, [this, InShortId, InFileState, InOperationOrigin](FDecrementStateCallback&& InCallback) mutable
	{
		// File state deletion request
		FileHandlerExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileHandlerBase::DecrementFileStateUse delete"), [this, InShortId, InFileState, InOperationOrigin, InCallback = MoveTemp(InCallback)]() mutable
		{
			OnDeleteState(InShortId, *InFileState, InOperationOrigin, MoveTemp(InCallback));
		});
	}, MoveTemp(InCallback));
}

void FWwiseFileHandlerBase::OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileHandlerBase::OnDeleteState %s"), GetManagingTypeName());
	{
		FWwiseFileStateSharedPtr DeletingState;

		{
			FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_Write);
			if (!InFileState.CanDelete())
			{
				UE_LOG(LogWwiseFileHandler, Verbose, TEXT("OnDeleteState %s %" PRIu32 " [%p]: Cannot delete State. Probably re-loaded between deletion request and now."),
					GetManagingTypeName(), InShortId, &InFileState);
			}
			else
			{
				UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("OnDeleteState %s %" PRIu32 " [%p]: Deleting."), GetManagingTypeName(), InShortId, &InFileState);
				DeletingState = FileStatesById[InShortId];		// Will delete at the end of the scope, out of lock's harm way
				const auto RemovalCount = FileStatesById.Remove(InShortId);

				UE_CLOG(RemovalCount != 1, LogWwiseFileHandler, Error, TEXT("Removing a state for %s %" PRIu32 ", ended up deleting %" PRIi32 " states."),
					GetManagingTypeName(), InShortId, RemovalCount);
			}
		}

		// State will be deleted here (if required)
	}

	SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileHandlerBase::OnDeleteState %s Callback"), GetManagingTypeName());
	InCallback();
}

void FWwiseFileHandlerBase::OnIncrementCountAsyncDone(bool bInResult, EWwiseFileStateOperationOrigin InOperationOrigin,
	FWwiseFileStateSharedPtr State, FIncrementStateCallback&& InCallback)
{
	if (UNLIKELY(!bInResult) && LIKELY(State))
	{
		DecrementFileStateUse(State->GetShortId(), State.Get(), InOperationOrigin, [State, Callback = MoveTemp(InCallback)]()
		{
			SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileHandlerBase::IncrementFileStateUse Callback Fail"));
			Callback(State, false);
		});
	}
	else
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileHandlerBase::IncrementFileStateUse Callback"));
		InCallback(State, bInResult);
	}
}
