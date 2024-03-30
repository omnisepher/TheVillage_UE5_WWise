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

#include "Wwise/WwiseDeferredQueue.h"

#include "Async/Async.h"

#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/Concurrency.h"

FWwiseDeferredQueue::FWwiseDeferredQueue(const TCHAR* InDebugName) :
	AsyncExecutionQueue(InDebugName)
{
}

FWwiseDeferredQueue::~FWwiseDeferredQueue()
{
	bClosing = true;
	if (!IsEmpty())
	{
		Wait();

		UE_CLOG(UNLIKELY(!IsEmpty()), LogWwiseConcurrency, Error, TEXT("Still operations in queue while deleting Deferred Queue"));
	}
}

void FWwiseDeferredQueue::AsyncDefer(FFunction&& InFunction)
{
	if (!bClosing)
	{
		AsyncOpQueue.Enqueue(MoveTemp(InFunction));
	}
}

void FWwiseDeferredQueue::SyncDefer(FSyncFunction&& InFunction)
{
	if (!bClosing)
	{
		SyncOpQueue.Enqueue(MoveTemp(InFunction));
	}
}

void FWwiseDeferredQueue::GameDefer(FFunction&& InFunction)
{
	if (!bClosing)
	{
		GameOpQueue.Enqueue(MoveTemp(InFunction));
	}
}

void FWwiseDeferredQueue::Run(AK::IAkGlobalPluginContext* InContext)
{
	SCOPED_WWISECONCURRENCY_EVENT_4(TEXT("FWwiseDeferredQueue::Run"));
	FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseConcurrencySync));

	UE_CLOG(UNLIKELY(Context), LogWwiseConcurrency, Error, TEXT("Executing two Run() at the same time."));
	Context = InContext;

	if (!AsyncOpQueue.IsEmpty())
	{
		AsyncExecutionQueue.Async(WWISECONCURRENCY_ASYNC_NAME("FWwiseDeferredQueue::Run"), [this]() mutable
		{
			AsyncExec();
		});
	}

	if (!GameOpQueue.IsEmpty() || OnGameRun.IsBound())
	{
		GameThreadExec();
	}

	if (!SyncOpQueue.IsEmpty() || OnSyncRunTS.IsBound())
	{
		SyncExec();
	}
	OnSyncRunTS.Broadcast(Context);

	Context = nullptr;
}

void FWwiseDeferredQueue::Wait()
{
	const bool bIsInGameThread = IsInGameThread();
	SCOPED_WWISECONCURRENCY_EVENT_4(bIsInGameThread ? TEXT("FWwiseDeferredQueue::Wait GameThread") : TEXT("FWwiseDeferredQueue::Wait"));
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_WwiseConcurrencyGameThreadWait, bIsInGameThread);
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_WwiseConcurrencyWait, !bIsInGameThread);

	if (!AsyncOpQueue.IsEmpty())
	{
		AsyncExecutionQueue.AsyncWait(WWISECONCURRENCY_ASYNC_NAME("FWwiseDeferredQueue::Wait"), [this]() mutable
		{
			AsyncExec();
		});
	}
	if (!GameOpQueue.IsEmpty())
	{
		FEventRef Done;
		if (bIsInGameThread)
		{
			const bool bNeedToStartLoop = GameThreadExecuting.IncrementExchange() == 0;

			GameOpQueue.Enqueue([this, &Done]() mutable
			{
				Done->Trigger();
				GameThreadExecuting.DecrementExchange();
				return EWwiseDeferredAsyncResult::Done;
			});

			if (bNeedToStartLoop)
			{
				FFunction Func;
				while (GameThreadExecuting.Load() > 0 && GameOpQueue.Dequeue(Func))
				{
					if (Func() == EWwiseDeferredAsyncResult::KeepRunning)
					{
						GameDefer(MoveTemp(Func));
					}
				}
			}
		}
		else
		{
			GameOpQueue.Enqueue([&Done]() mutable
			{
				Done->Trigger();
				return EWwiseDeferredAsyncResult::Done;
			});
			GameThreadExec();
		}
		Done->Wait();
	}
	if (!SyncOpQueue.IsEmpty())
	{
		FWwiseAsyncCycleCounter OpCycleCounter(GET_STATID(STAT_WwiseConcurrencySync));

		SyncExec();
	}
}

void FWwiseDeferredQueue::AsyncExec()
{
	SCOPED_WWISECONCURRENCY_EVENT_4(TEXT("FWwiseDeferredQueue::AsyncExec"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseConcurrencyAsync);

	bool bDone = false;
	AsyncOpQueue.Enqueue([&bDone]() mutable
	{
		bDone = true;
		return EWwiseDeferredAsyncResult::Done;
	});

	while (!bDone)
	{
		FFunction Func;
		const bool bResult = AsyncOpQueue.Dequeue(Func);
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseConcurrency, Error, TEXT("FWwiseDeferredQueue: No Result dequeuing Async Deferred Queue"));
			break;
		}
		if (Func() == EWwiseDeferredAsyncResult::KeepRunning)
		{
			AsyncDefer(MoveTemp(Func));
		}
	}
}

void FWwiseDeferredQueue::SyncExec()
{
	SyncOpQueue.Enqueue([this](AK::IAkGlobalPluginContext*) mutable
	{
		bSyncThreadDone = true;
		return EWwiseDeferredAsyncResult::Done;
	});

	SyncExecLoop();
}

void FWwiseDeferredQueue::SyncExecLoop()
{
	FSyncFunction Func;
	while (!bSyncThreadDone && SyncOpQueue.Dequeue(Func))
	{
		if (Func(Context) == EWwiseDeferredAsyncResult::KeepRunning)
		{
			SyncDefer(MoveTemp(Func));
		}
	}
	OnSyncRunTS.Broadcast(Context);
	bSyncThreadDone = false;
}

void FWwiseDeferredQueue::GameThreadExec()
{
	const bool bNeedToStartLoop = GameThreadExecuting.IncrementExchange() == 0;

	GameOpQueue.Enqueue([this]() mutable
	{
		GameThreadExecuting.DecrementExchange();
		return EWwiseDeferredAsyncResult::Done;
	});

	if (bNeedToStartLoop)
	{
		GameThreadExecLoop();
	}
}

void FWwiseDeferredQueue::GameThreadExecLoop()
{
	AsyncTask(ENamedThreads::GameThread, [this]() mutable
	{
		SCOPED_WWISECONCURRENCY_EVENT_4(TEXT("FWwiseDeferredQueue::GameThreadExecLoop"));
		SCOPE_CYCLE_COUNTER(STAT_WwiseConcurrencyGameThread);

		FFunction Func;
		while (GameThreadExecuting.Load() > 0 && GameOpQueue.Dequeue(Func))
		{
			if (Func() == EWwiseDeferredAsyncResult::KeepRunning)
			{
				GameDefer(MoveTemp(Func));
			}
		}

		OnGameRun.Broadcast();
	});
}
