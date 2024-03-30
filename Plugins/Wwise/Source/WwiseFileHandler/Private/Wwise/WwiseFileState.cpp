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

#include "Wwise/WwiseFileState.h"
#include "Wwise/Stats/AsyncStats.h"

#include <inttypes.h>

#include "Wwise/WwiseGlobalCallbacks.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"

FWwiseFileState::~FWwiseFileState()
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileState::~FWwiseFileState"));
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::~FWwiseFileState %p (dtor)"), this);
	UE_CLOG(FileStateExecutionQueue, LogWwiseFileHandler, Error, TEXT("Dtor FWwiseFileState %p without closing the execution queue!"), this);
	UE_CLOG(LoadCount > 0, LogWwiseFileHandler, Error, TEXT("Dtor FWwiseFileState %p with LoadCount still active!"), this);
	UE_CLOG(State != EState::Closed, LogWwiseFileHandler, Error, TEXT("Dtor FWwiseFileState %p with State %s not closed!"), this, GetStateName());
}

const TCHAR* FWwiseFileState::GetStateNameFor(EState State)
{
	switch (State)
	{
	case EState::Closed: return TEXT("Closed");
	case EState::Opening: return TEXT("Opening");
	case EState::Opened: return TEXT("Opened");
	case EState::Loading: return TEXT("Loading");
	case EState::Loaded: return TEXT("Loaded");
	case EState::Unloading: return TEXT("Unloading");
	case EState::Closing: return TEXT("Closing");
	case EState::WillReload: return TEXT("WillReload");
	case EState::CanReload: return TEXT("CanReload");
	case EState::WillReopen: return TEXT("WillReopen");
	case EState::CanReopen: return TEXT("CanReopen");
	}
	return TEXT("Unknown");
}

const TCHAR* FWwiseFileState::GetStateName() const
{
	return GetStateNameFor(State);
}

void FWwiseFileState::SetState(const TCHAR* const Caller, EState NewState)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("%s %p %s %" PRIu32 ": State %s -> %s"),
		Caller, this, GetManagingTypeName(), GetShortId(),
		GetStateName(), GetStateNameFor(NewState));
	State = NewState;
}

void FWwiseFileState::SetStateFrom(const TCHAR* const Caller, EState ExpectedState, EState NewState)
{
	UE_CLOG(ExpectedState != State, LogWwiseFileHandler, Fatal, TEXT("%s %p %s %" PRIu32 ": State %s -> %s. Expected %s!"),
		Caller, this, GetManagingTypeName(), GetShortId(), GetStateName(),
		GetStateNameFor(NewState), GetStateNameFor(ExpectedState));
	SetState(Caller, NewState);
}

void FWwiseFileState::IncrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin,
                                          FIncrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileState::IncrementCountAsync"));
	if (UNLIKELY(!FileStateExecutionQueue))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::IncrementCountAsync on a termed asset! Failing immediately!"));
		return InCallback(false);
	}

	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerStateOperationLatency));
	
	++OpenedInstances;
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::IncrementCountAsync Async"), [this, InOperationOrigin, OpCycleCounter = MoveTemp(OpCycleCounter), InCallback = MoveTemp(InCallback)]() mutable
	{
		INC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
		UE_CLOG(CreationOpOrder == 0, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountAsync %p %s %" PRIu32 ": Initial loading."), this, GetManagingTypeName(), GetShortId());

		const auto CurrentOpOrder = CreationOpOrder++;
		IncrementCount(InOperationOrigin, CurrentOpOrder, [OpCycleCounter = MoveTemp(OpCycleCounter), InCallback = MoveTemp(InCallback), CurrentOpOrder](bool bInResult) mutable
		{
			IncrementCountAsyncDone(MoveTemp(OpCycleCounter), MoveTemp(InCallback), bInResult);
		});
	});
}

void FWwiseFileState::IncrementCountAsyncDone(FWwiseAsyncCycleCounter&& InOpCycleCounter, FIncrementCountCallback&& InCallback, bool bInResult)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileState::IncrementCountAsync Callback"));
	InOpCycleCounter.Stop();
	DEC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
	InCallback(bInResult);
}

void FWwiseFileState::DecrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileState::DecrementCountAsync"));
	if (UNLIKELY(!FileStateExecutionQueue))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::DecrementCountAsync on a termed asset! Failing immediately!"));
		return InCallback();
	}

	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseFileHandlerStateOperationLatency));
	
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::DecrementCountAsync Async"), [this, InOperationOrigin, OpCycleCounter = MoveTemp(OpCycleCounter), InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
	{
		INC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
		const auto CurrentOpOrder = CreationOpOrder++;
		DecrementCount(InOperationOrigin, CurrentOpOrder, MoveTemp(InDeleteState), [OpCycleCounter = MoveTemp(OpCycleCounter), InCallback = MoveTemp(InCallback), CurrentOpOrder]() mutable
		{
			DecrementCountAsyncDone(MoveTemp(OpCycleCounter), MoveTemp(InCallback));
		});
	});
}

void FWwiseFileState::DecrementCountAsyncDone(FWwiseAsyncCycleCounter&& InOpCycleCounter, FDecrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileState::DecrementCountAsyncDone Callback"));
	InOpCycleCounter.Stop();
	DEC_DWORD_STAT(STAT_WwiseFileHandlerStateOperationsBeingProcessed);
	InCallback();
}

bool FWwiseFileState::CanDelete() const
{
	return OpenedInstances.load() == 0 && State == EState::Closed && LoadCount == 0;
}

FWwiseFileState::FWwiseFileState():
	LoadCount(0),
	StreamingCount(0),
	State(EState::Closed)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::FWwiseFileState %p (ctor)"), this);
}

void FWwiseFileState::Term()
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseFileState::Term"));
	if (UNLIKELY(!FileStateExecutionQueue))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::Term %p %s %" PRIu32 " already Term!"), this, GetManagingTypeName(), GetShortId());
		return;
	}
	while (UNLIKELY(bRecurringCallbackRegistered.load()))
	{
		FileStateExecutionQueue->AsyncWait(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::Term RecurringCallback Wait"), [] {});
	}
	if (UNLIKELY(OpenedInstances.load() > 0))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseFileState::Term %p %s %" PRIu32 ": Terminating with active states. Waiting 10 loops before bailing out."), this, GetManagingTypeName(), GetShortId());
		for (int i = 0; OpenedInstances.load() > 0 && i < 10; ++i)
		{
			FileStateExecutionQueue->AsyncWait(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::Term Wait"), []{});
		}
		UE_CLOG(OpenedInstances.load() > 0, LogWwiseFileHandler, Error, TEXT("FWwiseFileState::Term %p %s %" PRIu32 ": Terminating with active states. This might cause a crash."), this, GetManagingTypeName(), GetShortId());
	}
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::Term %p %s %" PRIu32 ": Terminating."), this, GetManagingTypeName(), GetShortId());
	UE_CLOG(!IsEngineExitRequested() && UNLIKELY(State != EState::Closed), LogWwiseFileHandler, Warning, TEXT("FWwiseFileState::Term %s State: Term unclosed file state %s %" PRIu32 ". Leaking."), GetManagingTypeName(), GetStateName(), GetShortId());
	UE_CLOG(IsEngineExitRequested() && State != EState::Closed, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::Term %s State: Term unclosed file state %s %" PRIu32 " at exit. Leaking."), GetManagingTypeName(), GetStateName(), GetShortId());
	UE_CLOG(LoadCount != 0, LogWwiseFileHandler, Log, TEXT("FWwiseFileState::Term %p %s %" PRIu32 " when there are still %d load count"), this, GetManagingTypeName(), GetShortId(), LoadCount);

	FileStateExecutionQueue->CloseAndDelete(); FileStateExecutionQueue = nullptr;
}

void FWwiseFileState::IncrementCount(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
                                     FIncrementCountCallback&& InCallback)
{
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	IncrementLoadCount(InOperationOrigin);

	IncrementCountOpen(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));
}

void FWwiseFileState::IncrementCountOpen(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
	FIncrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::IncrementCountOpen %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	if (IsBusy())
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountOpen %s %" PRIu32 ": Deferred."),
			GetManagingTypeName(), GetShortId());
		if (State == EState::Closing)
		{
			SetState(TEXT("FWwiseFileState::IncrementCountOpen"), EState::WillReopen);
		}

		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::IncrementCountOpen Busy"), [this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountOpen %s %" PRIu32 ": Retrying open"),
							GetManagingTypeName(), GetShortId());
			IncrementCountOpen(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));			// Call ourselves back
		});
		return;
	}

	if (State == EState::CanReopen)
	{
		SetState(TEXT("FWwiseFileState::IncrementCountOpen"), EState::Closed);
	}

	if (State == EState::Opening)
	{
		// We are currently opening asynchronously. We must wait for that operation to be initially done, so we can keep on processing this.
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountOpen %s %" PRIu32 ": Waiting for deferred Opening file."),
			GetManagingTypeName(), GetShortId());
		AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::IncrementCountOpen Opening"), [this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountOpen %s %" PRIu32 ": Retrying open"),
							GetManagingTypeName(), GetShortId());
			IncrementCountOpen(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));			// Call ourselves back
		});
		return;
	}

	if (!CanOpenFile())
	{
		IncrementCountLoad(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));					// Continue
		return;
	}

	SetStateFrom(TEXT("FWwiseFileState::IncrementCountOpen"), EState::Closed, EState::Opening);

	OpenFile([this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::IncrementCountOpen %s OpenFile"), GetManagingTypeName());
		IncrementCountLoad(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));
	});
}

void FWwiseFileState::IncrementCountLoad(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
	FIncrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::IncrementCountLoad %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	if (IsBusy())
	{
		if (State == EState::Unloading)
		{
			SetState(TEXT("FWwiseFileState::IncrementCountLoad"), EState::WillReload);
		}
		else if (State == EState::Closing)
		{
			SetState(TEXT("FWwiseFileState::IncrementCountLoad"), EState::WillReload);
		}

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountLoad %s %" PRIu32 ": Deferred."),
			GetManagingTypeName(), GetShortId());
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::IncrementCountLoad Busy"), [this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountLoad %s %" PRIu32 ": Retrying open"),
							GetManagingTypeName(), GetShortId());
			IncrementCountOpen(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));			// Restart the op from start
		});
		return;
	}

	if (State == EState::CanReload)
	{
		SetState(TEXT("FWwiseFileState::IncrementCountLoad"), EState::Opened);
	}

	if (State == EState::Loading)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountLoad %s %" PRIu32 ": Waiting for deferred Loading file."),
			GetManagingTypeName(), GetShortId());
		AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::IncrementCountLoad Loading"), [this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountLoad %s %" PRIu32 ": Retrying load"),
				GetManagingTypeName(), GetShortId());
			IncrementCountLoad(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));			// Call ourselves back
		});
		return;
	}

	if (!CanLoadInSoundEngine())
	{
		IncrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));					// Continue
		return;
	}

	SetStateFrom(TEXT("FWwiseFileState::IncrementCountLoad"), EState::Opened, EState::Loading);

	LoadInSoundEngine([this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::IncrementCountLoad %s LoadInSoundEngine"), GetManagingTypeName());
		IncrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));
	});
}

void FWwiseFileState::IncrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
	FIncrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::IncrementCountDone %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	UE_CLOG(UNLIKELY(InCurrentOpOrder < DoneOpOrder), LogWwiseFileHandler, Error, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": CurrentOpOrder %d < DoneOpOrder %d"),
		GetManagingTypeName(), GetShortId(), InCurrentOpOrder, DoneOpOrder);
	
	if (UNLIKELY(IsBusy()))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": Deferred."),
				GetManagingTypeName(), GetShortId());
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::IncrementCountDone Busy"), [this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
		{
			IncrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));
		});
		return;
	}

	ProcessLaterOpQueue();
	
	if (UNLIKELY(InCurrentOpOrder > DoneOpOrder))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": Done incrementing. Out of Order callback. Waiting for our turn (remaining %d)."),
				GetManagingTypeName(), GetShortId(), InCurrentOpOrder-DoneOpOrder);
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::IncrementCountDone Async"), [this, InOperationOrigin, InCurrentOpOrder, InCallback = MoveTemp(InCallback)]() mutable
		{
			IncrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InCallback));
		});
		return;
	}

	++DoneOpOrder;
	bool bResult;
	if (InOperationOrigin == EWwiseFileStateOperationOrigin::Streaming)
	{
		bResult = (State == EState::Loaded);
		if (UNLIKELY(!bResult))
		{
			UE_CLOG(CanLoadInSoundEngine(), LogWwiseFileHandler, Warning, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": Could not load file for streaming."),
				GetManagingTypeName(), GetShortId());
			UE_CLOG(!CanLoadInSoundEngine(), LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": Streaming request aborted before load done."),
				GetManagingTypeName(), GetShortId());
		}
	}
	else
	{
		const auto bCanLoadInSoundEngine = CanLoadInSoundEngine();
		const auto bCanOpenFile = CanOpenFile();
		bResult = (State == EState::Loaded)
			|| (State == EState::Opened && !bCanLoadInSoundEngine)
			|| (State == EState::Closed && !bCanOpenFile);
		if (UNLIKELY(!bResult))
		{
			UE_CLOG((State == EState::CanReload && State == EState::CanReopen),
				LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": File open is now useless [State:%s CanLoad:%s CanOpen:%s LoadCount:%d]"),
				GetManagingTypeName(), GetShortId(),
				GetStateName(), bCanLoadInSoundEngine ? TEXT("true"):TEXT("false"), bCanOpenFile ? TEXT("true"):TEXT("false"), LoadCount);
			UE_CLOG((State != EState::CanReload && State != EState::CanReopen),
				LogWwiseFileHandler, Warning, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": Could not open file for asset loading. [State:%s CanLoad:%s CanOpen:%s LoadCount:%d]"),
				GetManagingTypeName(), GetShortId(),
				GetStateName(), bCanLoadInSoundEngine ? TEXT("true"):TEXT("false"), bCanOpenFile ? TEXT("true"):TEXT("false"), LoadCount);
		}
	}

	UE_CLOG(LIKELY(bResult), LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementCountDone %s %" PRIu32 ": Done incrementing."),
		GetManagingTypeName(), GetShortId());
	{
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::IncrementCountDone %s Callback"), GetManagingTypeName());
		InCallback(bResult);
	}
}

void FWwiseFileState::DecrementCount(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
                                  FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	if (UNLIKELY(LoadCount == 0))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::DecrementCount %s %" PRIu32 ": File State is already closed."), GetManagingTypeName(), GetShortId());
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::DecrementCount %s Callback"), GetManagingTypeName());
		InCallback();
		return;
	}

	DecrementLoadCount(InOperationOrigin);

	DecrementCountUnload(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));
}

void FWwiseFileState::DecrementCountUnload(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::DecrementCountUnload %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	if (IsBusy())
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountUnload %s %" PRIu32 ": UnloadFromSoundEngine deferred."),
			GetManagingTypeName(), GetShortId());
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::DecrementCountUnload Busy"), [this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountUnload %s %" PRIu32 ": Retrying unload"),
							GetManagingTypeName(), GetShortId());
			DecrementCountUnload(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));			// Call ourselves back
		});
		return;
	}

	if (!CanUnloadFromSoundEngine())
	{
		DecrementCountClose(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));					// Continue
		return;
	}

	SetState(TEXT("FWwiseFileState::DecrementCountUnload"), EState::Unloading);

	UnloadFromSoundEngine([this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)](EResult InDefer) mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::DecrementCountUnload %s UnloadFromSoundEngine"), GetManagingTypeName());
		DecrementCountUnloadCallback(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback), InDefer);
	});
}

void FWwiseFileState::DecrementCountUnloadCallback(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback, EResult InDefer)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::DecrementCountUnloadCallback %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	if (LIKELY(InDefer == EResult::Done))
	{
		DecrementCountClose(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Continue
		return;
	}

	if (UNLIKELY(State == EState::WillReload))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountUnloadCallback %s %" PRIu32 ": Another user needs this to be kept loaded."),
				GetManagingTypeName(), GetShortId());
		SetState(TEXT("FWwiseFileState::DecrementCountUnloadCallback"), EState::Loaded);
		DecrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Skip all
	}
	else if (UNLIKELY(State != EState::Unloading))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountUnloadCallback %s %" PRIu32 ": State got changed to %s. Not unloading anymore."),
				GetManagingTypeName(), GetShortId(), GetStateName());
		DecrementCountClose(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Continue
	}
	else
	{
		SetState(TEXT("FWwiseFileState::DecrementCountUnloadCallback"), EState::Loaded);
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::DecrementCountUnloadCallback Deferred"), [this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountUnloadCallback %s %" PRIu32 ": Retrying deferred unload"),
							GetManagingTypeName(), GetShortId());
			DecrementCountUnload(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Call ourselves back
		});
	}
}

void FWwiseFileState::DecrementCountClose(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
                                          FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::DecrementCountClose %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	if (IsBusy())
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountClose %s %" PRIu32 ": CloseFile deferred."),
			GetManagingTypeName(), GetShortId());
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::DecrementCountClose Busy"), [this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountClose %s %" PRIu32 ": Retrying close"),
							GetManagingTypeName(), GetShortId());
			DecrementCountClose(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));			// Call ourselves back
		});
		return;
	}

	if (!CanCloseFile())
	{
		DecrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));					// Continue
		return;
	}

	SetState(TEXT("FWwiseFileState::DecrementCountClose"), EState::Closing);

	CloseFile([this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)](EResult InDefer) mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::DecrementCountClose %s CloseFile"), GetManagingTypeName());
		DecrementCountCloseCallback(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback), InDefer);
	});
}

void FWwiseFileState::DecrementCountCloseCallback(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
	FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback, EResult InDefer)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::DecrementCountCloseCallback %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	if (LIKELY(InDefer == EResult::Done))
	{
		DecrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));				// Continue
		return;
	}

	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseFileState::DecrementCountCloseCallback %s %" PRIu32 ": Processing deferred Close."),
		GetManagingTypeName(), GetShortId());
	
	if (UNLIKELY(State == EState::WillReopen))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountCloseCallback %s %" PRIu32 ": Another user needs this to be kept open."),
				GetManagingTypeName(), GetShortId());
		SetState(TEXT("FWwiseFileState::DecrementCountCloseCallback"), EState::Opened);
		DecrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Skip all
	}
	else if (UNLIKELY(State != EState::Closing))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountCloseCallback %s %" PRIu32 ": State got changed to %s. Not closing anymore."),
				GetManagingTypeName(), GetShortId(), GetStateName());
		DecrementCountClose(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Continue
	}
	else
	{
		SetState(TEXT("FWwiseFileState::DecrementCountCloseCallback"), EState::Opened);
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::DecrementCountUnloadCallback Deferred"), [this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountCloseCallback %s %" PRIu32 ": Retrying deferred close"),
							GetManagingTypeName(), GetShortId());
			DecrementCountClose(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));		// Call ourselves back
		});
	}
}

void FWwiseFileState::DecrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder,
                                         FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_F_3(TEXT("FWwiseFileState::DecrementCountDone %s"), GetManagingTypeName());
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	UE_CLOG(UNLIKELY(InCurrentOpOrder < DoneOpOrder), LogWwiseFileHandler, Error, TEXT("FWwiseFileState::DecrementCountDone %s %" PRIu32 ": CurrentOpOrder %d < DoneOpOrder %d"),
		GetManagingTypeName(), GetShortId(), InCurrentOpOrder, DoneOpOrder);
	
	if (UNLIKELY(IsBusy()))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountDone %s %" PRIu32 ": Deferred."),
				GetManagingTypeName(), GetShortId());
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::DecrementCountDone Busy"), [this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			DecrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));
		});
		return;
	}

	ProcessLaterOpQueue();
	
	if (UNLIKELY(InCurrentOpOrder > DoneOpOrder))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountDone %s %" PRIu32 ": Done decrementing. Out of Order callback. Waiting for our turn (remaining %d)."),
				GetManagingTypeName(), GetShortId(), InCurrentOpOrder-DoneOpOrder);
		AsyncOpLater(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::DecrementCountDone Async"), [this, InOperationOrigin, InCurrentOpOrder, InDeleteState = MoveTemp(InDeleteState), InCallback = MoveTemp(InCallback)]() mutable
		{
			DecrementCountDone(InOperationOrigin, InCurrentOpOrder, MoveTemp(InDeleteState), MoveTemp(InCallback));
		});
		return;
	}

	++DoneOpOrder;
	--OpenedInstances;
	if (CanDelete())
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountDone %s %" PRIu32 ": Done decrementing. Deleting state."),
				GetManagingTypeName(), GetShortId());
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::DecrementCountDone %s Delete"), GetManagingTypeName());
		InDeleteState(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementCountDone %s %" PRIu32 ": Done decrementing."),
				GetManagingTypeName(), GetShortId());
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::DecrementCountDone %s Callback"), GetManagingTypeName());
		InCallback();
	}
}

void FWwiseFileState::IncrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin)
{
	check(FileStateExecutionQueue->IsRunningInThisThread());
	
	const bool bIncrementStreamingCount = (InOperationOrigin == EWwiseFileStateOperationOrigin::Streaming);

	if (bIncrementStreamingCount) ++StreamingCount;
	++LoadCount;

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::IncrementLoadCount %s %" PRIu32 ": ++LoadCount %d %sStreamingCount %d"),
	       GetManagingTypeName(), GetShortId(), LoadCount, bIncrementStreamingCount ? TEXT("++") : TEXT(""), StreamingCount);
}

bool FWwiseFileState::CanOpenFile() const
{
	return State == EState::Closed && LoadCount > 0;
}

void FWwiseFileState::OpenFileSucceeded(FOpenFileCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::OpenFileSucceeded"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		if (UNLIKELY(State != EState::Opening))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::OpenFileSucceeded: Succeeded opening %s %" PRIu32 " while not in Opening state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
		}
		else
		{
			SetState(TEXT("FWwiseFileState::OpenFileSucceeded"), EState::Opened);
		}
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::OpenFileSucceeded %s Callback"), GetManagingTypeName());
		InCallback();
	});
}

void FWwiseFileState::OpenFileFailed(FOpenFileCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::OpenFileFailed"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		if (UNLIKELY(State != EState::Opening))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::OpenFileFailed: Failed opening %s %" PRIu32 " while not in Opening state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
		}
		else
		{
			SetState(TEXT("FWwiseFileState::OpenFileFailed"), EState::Closed);
		}
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::OpenFileFailed %s Callback"), GetManagingTypeName());
		InCallback();
	});
}

bool FWwiseFileState::CanLoadInSoundEngine() const
{
	return State == EState::Opened && (!IsStreamedState() || StreamingCount > 0);
}

void FWwiseFileState::LoadInSoundEngineSucceeded(FLoadInSoundEngineCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::LoadInSoundEngineSucceeded"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		if (UNLIKELY(State != EState::Loading))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::LoadInSoundEngineSucceeded: Succeeded loading %s %" PRIu32 " while not in Loading state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
		}
		else
		{
			SetState(TEXT("FWwiseFileState::LoadInSoundEngineSucceeded"), EState::Loaded);
		}
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::LoadInSoundEngineSucceeded %s Callback"), GetManagingTypeName());
		InCallback();
	});
}

void FWwiseFileState::LoadInSoundEngineFailed(FLoadInSoundEngineCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::LoadInSoundEngineFailed"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		INC_DWORD_STAT(STAT_WwiseFileHandlerTotalErrorCount);
		if (UNLIKELY(State != EState::Loading))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::LoadInSoundEngineFailed: Failed loading %s %" PRIu32 " while not in Loading state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
		}
		else
		{
			SetState(TEXT("FWwiseFileState::LoadInSoundEngineFailed"), EState::Opened);
		}
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::LoadInSoundEngineFailed %s Callback"), GetManagingTypeName());
		InCallback();
	});
}

void FWwiseFileState::DecrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin)
{
	const bool bDecrementStreamingCount = (InOperationOrigin == EWwiseFileStateOperationOrigin::Streaming);

	if (bDecrementStreamingCount) --StreamingCount;
	--LoadCount;

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::DecrementLoadCount %s %" PRIu32 ": --LoadCount %d %sStreamingCount %d"),
	       GetManagingTypeName(), GetShortId(), LoadCount, bDecrementStreamingCount ? TEXT("--") : TEXT(""), StreamingCount);
}

bool FWwiseFileState::CanUnloadFromSoundEngine() const
{
	return State == EState::Loaded && ((IsStreamedState() && StreamingCount == 0) || (!IsStreamedState() && LoadCount == 0));
}

void FWwiseFileState::UnloadFromSoundEngineDone(FUnloadFromSoundEngineCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::UnloadFromSoundEngineDone"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		if (UNLIKELY(State != EState::Unloading && State != EState::WillReload))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::UnloadFromSoundEngineDone: Done unloading %s %" PRIu32 " while not in Unloading state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
		}
		else if (LIKELY(State == EState::Unloading))
		{
			SetState(TEXT("FWwiseFileState::UnloadFromSoundEngineDone"), EState::Opened);
		}
		else
		{
			SetState(TEXT("FWwiseFileState::UnloadFromSoundEngineDone"), EState::CanReload);
			ProcessLaterOpQueue();
		}
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::UnloadFromSoundEngineDone %s Callback"), GetManagingTypeName());
		InCallback(EResult::Done);
	});
}

void FWwiseFileState::UnloadFromSoundEngineToClosedFile(FUnloadFromSoundEngineCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::UnloadFromSoundEngineToClosedFile"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		if (UNLIKELY(State != EState::Unloading && State != EState::WillReload))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::UnloadFromSoundEngineToClosedFile: Done unloading %s %" PRIu32 " while not in Unloading state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
		}
		else if (LIKELY(State == EState::Unloading))
		{
			SetState(TEXT("FWwiseFileState::UnloadFromSoundEngineToClosedFile"), EState::Closed);
		}
		else
		{
			SetState(TEXT("FWwiseFileState::UnloadFromSoundEngineToClosedFile"), EState::CanReopen);
			ProcessLaterOpQueue();
		}
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::UnloadFromSoundEngineToClosedFile %s Callback"), GetManagingTypeName());
		InCallback(EResult::Done);
	});
}

void FWwiseFileState::UnloadFromSoundEngineDefer(FUnloadFromSoundEngineCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::UnloadFromSoundEngineDefer"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		if (UNLIKELY(State != EState::Unloading && State != EState::WillReload))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::UnloadFromSoundEngineDefer %s %" PRIu32 ": Deferring unloading while not in Unloading state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
			SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::UnloadFromSoundEngineDefer %s Callback"), GetManagingTypeName());
			InCallback(EResult::Done);
			return;
		}
		if (UNLIKELY(State == EState::WillReload))
		{
			UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::UnloadFromSoundEngineDefer %s %" PRIu32 ": WillReload -> Loaded"), GetManagingTypeName(), GetShortId());
			SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::UnloadFromSoundEngineDefer %s Callback"), GetManagingTypeName());
			State = EState::Loaded;
			InCallback(EResult::Done);
			return;
		}

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::UnloadFromSoundEngineDefer %s %" PRIu32 ": Deferring Unload"), GetManagingTypeName(), GetShortId());
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::UnloadFromSoundEngineDefer %s Callback"), GetManagingTypeName());
		InCallback(EResult::Deferred);
	});
}

bool FWwiseFileState::CanCloseFile() const
{
	return State == EState::Opened && LoadCount == 0;
}

void FWwiseFileState::CloseFileDone(FCloseFileCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::CloseFileDone"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		if (UNLIKELY(State != EState::Closing && State != EState::WillReopen))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::CloseFileDone %s %" PRIu32 ": Done closing while not in Closing state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
		}
		else if (LIKELY(State == EState::Closing))
		{
			SetState(TEXT("FWwiseFileState::CloseFileDone"), EState::Closed);
		}
		else
		{
			SetState(TEXT("FWwiseFileState::CloseFileDone"), EState::CanReopen);
			ProcessLaterOpQueue();
		}
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::CloseFileDone %s Callback"), GetManagingTypeName());
		InCallback(EResult::Done);
	});
}

void FWwiseFileState::CloseFileDefer(FCloseFileCallback&& InCallback)
{
	AsyncOp(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileState::CloseFileDefer"), [this, InCallback = MoveTemp(InCallback)]() mutable
	{
		if (UNLIKELY(State != EState::Closing && State != EState::WillReopen))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::CloseFileDefer %s %" PRIu32 ": Deferring closing while not in Closing state: %s"), GetManagingTypeName(), GetShortId(), GetStateName());
			SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::CloseFileDefer %s Callback"), GetManagingTypeName());
			InCallback(EResult::Done);
			return;
		}
		if (UNLIKELY(State == EState::WillReopen))
		{
			SetState(TEXT("FWwiseFileState::CloseFileDefer"), EState::Opened);
			SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::CloseFileDefer %s Callback"), GetManagingTypeName());
			InCallback(EResult::Done);
			return;
		}

		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::CloseFileDefer %s %" PRIu32 ": Deferring Close"), GetManagingTypeName(), GetShortId());
		SCOPED_WWISEFILEHANDLER_EVENT_F_4(TEXT("FWwiseFileState::CloseFileDefer %s Callback"), GetManagingTypeName());
		InCallback(EResult::Deferred);
	});
}

bool FWwiseFileState::IsBusy() const
{
	switch (State)
	{
	case EState::Opening:
	case EState::Loading:
	case EState::Unloading:
	case EState::Closing:
	case EState::WillReload:
	case EState::WillReopen:
		return true;
	default:
		return false;
	}
}

void FWwiseFileState::AsyncOp(const TCHAR* InDebugName, FBasicFunction&& Fct)
{
	if (UNLIKELY(!FileStateExecutionQueue))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileState::AsyncOp %s %" PRIu32 ": Doing async op on terminated state"), GetManagingTypeName(), GetShortId());
		return Fct();
	}
	FileStateExecutionQueue->Async(InDebugName, MoveTemp(Fct));
}

void FWwiseFileState::AsyncOpLater(const TCHAR* InDebugName, FBasicFunction&& Fct)
{
	RegisterRecurringCallback();
	LaterOpQueue.Enqueue(FOpQueueItem(InDebugName, MoveTemp(Fct)));
}

void FWwiseFileState::ProcessLaterOpQueue()
{
	check(!FileStateExecutionQueue || FileStateExecutionQueue->IsRunningInThisThread());

	int Count = 0;
	for (FOpQueueItem* Op; (Op = LaterOpQueue.Peek()) != nullptr; LaterOpQueue.Pop())
	{
		++Count;
#if ENABLE_NAMED_EVENTS
		AsyncOp(Op->DebugName, MoveTemp(Op->Function));
#else
		AsyncOp(nullptr, MoveTemp(Op->Function));
#endif
	}
	UE_CLOG(Count > 0, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileState::ProcessLaterOpQueue %s %" PRIu32 ": Added back %d operations to be executed."),  GetManagingTypeName(), GetShortId(), Count);
}

void FWwiseFileState::RegisterRecurringCallback()
{
	auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
	if (GlobalCallbacks && !bRecurringCallbackRegistered.exchange(true))
	{
		GlobalCallbacks->EndAsync([this]() mutable
		{
			AsyncOp(TEXT("FWwiseFileState::RegisterRecurringCallback"), [this]() mutable
			{
				ProcessLaterOpQueue();
				bRecurringCallbackRegistered.store(false);
			});
			return EWwiseDeferredAsyncResult::Done;
		});
	}
}
