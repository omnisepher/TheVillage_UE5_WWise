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

#include "Wwise/WwiseExecutionQueue.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/Event.h"
#include "Misc/IQueuedWork.h"
#include "Wwise/WwiseConcurrencyModule.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/Concurrency.h"

#include <inttypes.h>

const bool FWwiseExecutionQueue::Test::bExtremelyVerbose{ false };
WWISE_EXECUTIONQUEUE_TEST_CONST bool FWwiseExecutionQueue::Test::bMockEngineDeletion{ false };
WWISE_EXECUTIONQUEUE_TEST_CONST bool FWwiseExecutionQueue::Test::bMockEngineDeleted{ false };
WWISE_EXECUTIONQUEUE_TEST_CONST bool FWwiseExecutionQueue::Test::bMockSleepOnStateUpdate{ false };
WWISE_EXECUTIONQUEUE_TEST_CONST bool FWwiseExecutionQueue::Test::bReduceLogVerbosity{ false };

struct FWwiseExecutionQueue::TLS
{
	static thread_local FWwiseExecutionQueue* CurrentExecutionQueue;
};

thread_local FWwiseExecutionQueue* FWwiseExecutionQueue::TLS::CurrentExecutionQueue{ nullptr };

FWwiseExecutionQueue::FWwiseExecutionQueue(const TCHAR* InDebugName, EWwiseTaskPriority InTaskPriority) :
#if ENABLE_NAMED_EVENTS || !NO_LOGGING
	DebugName(InDebugName),
#else
	DebugName(TEXT("")),
#endif
	TaskPriority(InTaskPriority)
{
	ASYNC_INC_DWORD_STAT(STAT_WwiseExecutionQueues);
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue (%p \"%s\") [%" PRIi32 "]: Creating with task priority %d"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (int)InTaskPriority);
}

FWwiseExecutionQueue::~FWwiseExecutionQueue()
{
	UE_CLOG(UNLIKELY(bDeleteOnceClosed && WorkerState.load(std::memory_order_seq_cst) != EWorkerState::Closed), LogWwiseConcurrency, Fatal, TEXT("Deleting FWwiseExectionQueue twice!"));

	Close();
	ASYNC_DEC_DWORD_STAT(STAT_WwiseExecutionQueues);
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::~FWwiseExecutionQueue(%p \"%s\") [%" PRIi32 "]: Deleted Execution Queue"), this, DebugName, FPlatformTLS::GetCurrentThreadId());
}

void FWwiseExecutionQueue::Async(const TCHAR* InDebugName, FBasicFunction&& InFunction)
{
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::Async(%p \"%s\") [%" PRIi32 "]: Enqueuing async function %p"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
	if (UNLIKELY(IsBeingClosed() || !OpQueue.Enqueue(FOpQueueItem(InDebugName, MoveTemp(InFunction)))))
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseExecutionQueueAsyncCalls);
		UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::Async(%p \"%s\") [%" PRIi32 "]: Executing async function %p"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
		InFunction();
		return;
	}
	StartWorkerIfNeeded();
}

void FWwiseExecutionQueue::AsyncAlways(const TCHAR* InDebugName, FBasicFunction&& InFunction)
{
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::AsyncAlways(%p \"%s\") [%" PRIi32 "]: Enqueuing async always function %p"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
	Async(InDebugName, [this, CallerThreadId = IsBeingClosed() ? FPlatformTLS::GetCurrentThreadId() : 0, InFunction = MoveTemp(InFunction)]() mutable
	{
		if (CallerThreadId == FPlatformTLS::GetCurrentThreadId())
		{
			LaunchWwiseTask(WWISECONCURRENCY_ASYNC_NAME("FWwiseExecutionQueue::AsyncAlways"), [this, InFunction = MoveTemp(InFunction)]
			{
				UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::AsyncAlways(%p \"%s\") [%" PRIi32 "]: Executing function %p in TaskGraph"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
				InFunction();
			});
		}
		else
		{
			UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::AsyncAlways(%p \"%s\") [%" PRIi32 "]: Executing function %p in worker thread"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
			InFunction();
		}
	});
}

void FWwiseExecutionQueue::AsyncWait(const TCHAR* InDebugName, FBasicFunction&& InFunction)
{
	SCOPED_WWISECONCURRENCY_EVENT_4(TEXT("FWwiseExecutionQueue::AsyncWait"));
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::AsyncWait(%p \"%s\") [%" PRIi32 "]: Enqueuing async wait function %p"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
	FEventRef Event(EEventMode::ManualReset);
	if (UNLIKELY(IsBeingClosed() || !OpQueue.Enqueue(FOpQueueItem(InDebugName, [this, &Event, &InFunction] {
		ASYNC_INC_DWORD_STAT(STAT_WwiseExecutionQueueAsyncWaitCalls);
		UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::AsyncWait(%p \"%s\") [%" PRIi32 "]: Executing async wait function %p"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
		InFunction();
		Event->Trigger();
	}))))
	{
		UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::AsyncWait(%p \"%s\") [%" PRIi32 "]: Executing async wait function %p synchronously!"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (intptr_t&)InFunction);
		InFunction();
		return;
	}
	StartWorkerIfNeeded();
	Event->Wait();
}

void FWwiseExecutionQueue::Close()
{
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::Close(%p \"%s\") [%" PRIi32 "]: Closing"), this, DebugName, FPlatformTLS::GetCurrentThreadId());
	if (IsBeingClosed())
	{
		return;
	}

	AsyncWait(WWISECONCURRENCY_ASYNC_NAME("FWwiseExecutionQueue::Close"), [this]
	{
		TrySetRunningWorkerToClosing();
	});

	auto State = WorkerState.load(std::memory_order_relaxed);
	if (State != EWorkerState::Closed)
	{
		SCOPED_WWISECONCURRENCY_EVENT_4(TEXT("FWwiseExecutionQueue::Close Waiting"));
		UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::Close(%p \"%s\") [%" PRIi32 "]: Waiting for Closed"), this, DebugName, FPlatformTLS::GetCurrentThreadId());
		while (State != EWorkerState::Closed)
		{
			FPlatformProcess::Yield();
			State = WorkerState.load(std::memory_order_relaxed);
		}
	}
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::Close(%p \"%s\") [%" PRIi32 "]: Done Closing"), this, DebugName, FPlatformTLS::GetCurrentThreadId());
}

void FWwiseExecutionQueue::CloseAndDelete()
{
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::Close(%p \"%s\") [%" PRIi32 "]: Closing and Request Deleting"), this, DebugName, FPlatformTLS::GetCurrentThreadId());
	bDeleteOnceClosed = true;
	FEvent* CloseAndDeleteFunctionReturned = FPlatformProcess::GetSynchEventFromPool();		// We must wait for the Async to be done before the Worker thread can "delete this".
	
	Async(WWISECONCURRENCY_ASYNC_NAME("FWwiseExecutionQueue::CloseAndDelete"), [CloseAndDeleteFunctionReturned, this]
	{
		if (CloseAndDeleteFunctionReturned)
		{
			CloseAndDeleteFunctionReturned->Wait();
			FPlatformProcess::ReturnSynchEventToPool(CloseAndDeleteFunctionReturned);
		}
		else
		{
			FPlatformProcess::Yield();
		}
		TrySetRunningWorkerToClosing();
	});
	if (CloseAndDeleteFunctionReturned)
	{
		CloseAndDeleteFunctionReturned->Trigger();
	}
}

bool FWwiseExecutionQueue::IsBeingClosed() const
{
	const auto State = WorkerState.load(std::memory_order_seq_cst);
	return UNLIKELY(State == EWorkerState::Closed || State == EWorkerState::Closing);
}

bool FWwiseExecutionQueue::IsClosed() const
{
	const auto State = WorkerState.load(std::memory_order_seq_cst);
	return State == EWorkerState::Closed;
}

bool FWwiseExecutionQueue::IsRunningInThisThread() const
{
	return this == TLS::CurrentExecutionQueue;
}

void FWwiseExecutionQueue::StartWorkerIfNeeded()
{
	if (!TrySetRunningWorkerToAddOp() && TrySetStoppedWorkerToRunning())
	{
		if (UNLIKELY(!IWwiseConcurrencyModule::GetModule() || Test::bMockEngineDeletion || Test::bMockEngineDeleted) &&
			UNLIKELY(!FTaskGraphInterface::IsRunning() || Test::bMockEngineDeleted))
		{
			UE_CLOG(!Test::bMockEngineDeleted, LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::StartWorkerIfNeeded(%p \"%s\") [%" PRIi32 "]: No Task Graph. Do tasks now"), this, DebugName, FPlatformTLS::GetCurrentThreadId());
			Work();
		}
		else
		{
			UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::StartWorkerIfNeeded(%p \"%s\") [%" PRIi32 "]: Starting new worker task with priority %d"), this, DebugName, FPlatformTLS::GetCurrentThreadId(), (int)TaskPriority);
			LaunchWwiseTask(WWISECONCURRENCY_ASYNC_NAME("FWwiseExecutionQueue::StartWorkerIfNeeded"), TaskPriority, [this]
			{
				Work();
			});
		}
	}
}

void FWwiseExecutionQueue::Work()
{
	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::Work(%p \"%s\") [%" PRIi32 "]: Started worker."), this, DebugName, FPlatformTLS::GetCurrentThreadId());

	do
	{
		ProcessWork();
	}
	while (KeepWorking());
}

bool FWwiseExecutionQueue::KeepWorking()
{
	const auto CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
	const auto bDeleteOnceClosedCopy = this->bDeleteOnceClosed;

	if (LIKELY(TrySetRunningWorkerToStopped()))
	{
		UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::KeepWorking(%p \"%s\") [%" PRIi32 "]: Stopped worker."), this, DebugName, CurrentThreadId);
		// We don't have any more operations queued. Done.
		// Don't execute operations here, as the Execution Queue might be deleted here.
		return false;
	}
	else if (LIKELY(TrySetClosingWorkerToClosed()))
	{
		// We were exiting and we don't have operations anymore. Immediately return, as our worker is not valid at this point.
		// Don't do any operations here!

		if (bDeleteOnceClosedCopy)		// We use a copy since the deletion might've already occurred
		{
			LaunchWwiseTask(WWISECONCURRENCY_ASYNC_NAME("FWwiseExecutionQueue::KeepWorking delete"), [this]
			{
				UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue::KeepWorking(%p \"%s\") [%" PRIi32 "]: Auto deleting on Any Thread"), this, DebugName, FPlatformTLS::GetCurrentThreadId());
				delete this;
			});
		}
		return false;
	}

	// Error cases
	else if (UNLIKELY(WorkerState.load() == EWorkerState::Closed))
	{
		UE_LOG(LogWwiseConcurrency, Error, TEXT("FWwiseExecutionQueue::KeepWorking(%p \"%s\") [%" PRIi32 "]: Worker is closed, but we are still looping."), this, DebugName, CurrentThreadId);
		return false;
	}
	else if (UNLIKELY(WorkerState.load() == EWorkerState::Stopped))
	{
		UE_LOG(LogWwiseConcurrency, Error, TEXT("FWwiseExecutionQueue::KeepWorking(%p \"%s\") [%" PRIi32 "]: Worker is stopped, but we haven't stopped it ourselves."), this, DebugName, CurrentThreadId);
		return false;
	}

	// Keep running (AddOp)
	return true;
}

void FWwiseExecutionQueue::ProcessWork()
{
	TrySetAddOpWorkerToRunning();

	auto* PreviousExecutionQueue = TLS::CurrentExecutionQueue;
	TLS::CurrentExecutionQueue = this;
	
	for (const FOpQueueItem* Op; (Op = OpQueue.Peek()) != nullptr; OpQueue.Pop())
	{
#if ENABLE_NAMED_EVENTS
		SCOPED_NAMED_EVENT_TCHAR_CONDITIONAL(Op->DebugName, WwiseNamedEvents::Color3, Op->DebugName != nullptr);
#endif
		Op->Function();
	}

	TLS::CurrentExecutionQueue = PreviousExecutionQueue;
}

bool FWwiseExecutionQueue::TrySetStoppedWorkerToRunning()
{
	return TryStateUpdate(EWorkerState::Stopped, EWorkerState::Running);

}

bool FWwiseExecutionQueue::TrySetRunningWorkerToStopped()
{
	return TryStateUpdate(EWorkerState::Running, EWorkerState::Stopped);
}

bool FWwiseExecutionQueue::TrySetRunningWorkerToAddOp()
{
	return TryStateUpdate(EWorkerState::Running, EWorkerState::AddOp);
}

bool FWwiseExecutionQueue::TrySetAddOpWorkerToRunning()
{
	return TryStateUpdate(EWorkerState::AddOp, EWorkerState::Running);
}

bool FWwiseExecutionQueue::TrySetRunningWorkerToClosing()
{
	return TryStateUpdate(EWorkerState::Running, EWorkerState::Closing)
		|| TryStateUpdate(EWorkerState::AddOp, EWorkerState::Closing)
		|| TryStateUpdate(EWorkerState::Stopped, EWorkerState::Closing);
	// Warning: Try not to do operations past this method returning "true". There's a slight chance "this" might be deleted!
}

bool FWwiseExecutionQueue::TrySetClosingWorkerToClosed()
{
	return TryStateUpdate(EWorkerState::Closing, EWorkerState::Closed);
	// Warning: NEVER do operations past this method returning "true". "this" is probably deleted!
}

const TCHAR* FWwiseExecutionQueue::StateName(EWorkerState State)
{
	switch (State)
	{
	case EWorkerState::Stopped: return TEXT("Stopped");
	case EWorkerState::Running: return TEXT("Running");
	case EWorkerState::AddOp: return TEXT("AddOp");
	case EWorkerState::Closing: return TEXT("Closing");
	case EWorkerState::Closed: return TEXT("Closed");
	default: return TEXT("UNKNOWN");
	}
}

bool FWwiseExecutionQueue::TryStateUpdate(EWorkerState NeededState, EWorkerState WantedState)
{
	EWorkerState PreviousState = NeededState;
	bool bResult = WorkerState.compare_exchange_strong(PreviousState, WantedState);
	bResult = bResult && PreviousState == NeededState;

	UE_CLOG(Test::IsExtremelyVerbose(), LogWwiseConcurrency, VeryVerbose, TEXT("FWwiseExecutionQueue(%p \"%s\") [%" PRIi32 "]: %s %s [%s -> %s] %s %s"), this, DebugName, FPlatformTLS::GetCurrentThreadId(),
		StateName(PreviousState),
		bResult ? TEXT("==>") : TEXT("XX>"),
		StateName(NeededState), StateName(WantedState),
		bResult ? TEXT("==>") : TEXT("XX>"),
		bResult ? StateName(WantedState) : StateName(PreviousState));

	if (UNLIKELY(Test::bMockSleepOnStateUpdate) && bResult)
	{
		FPlatformProcess::Sleep(0.0001f);
	}
	return bResult;
}
