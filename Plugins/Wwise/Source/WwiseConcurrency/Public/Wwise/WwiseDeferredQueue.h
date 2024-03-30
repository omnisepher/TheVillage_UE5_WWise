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

#pragma once

#include "Wwise/WwiseExecutionQueue.h"

#include "WwiseUnrealDefines.h"

namespace AK
{
	class IAkGlobalPluginContext;
}

enum class WWISECONCURRENCY_API EWwiseDeferredAsyncResult
{
	/**
	 * @brief The Deferred Queue task is now done executing. Stop executing it.
	 */
	Done,

	/**
	 * @brief The Deferred Queue task is recurrent. Execute it again at next request.
	*/
	KeepRunning
};

struct WWISECONCURRENCY_API FWwiseDeferredQueue
{
	using FFunction = TUniqueFunction<EWwiseDeferredAsyncResult()>;
	using FSyncFunction = TUniqueFunction<EWwiseDeferredAsyncResult (AK::IAkGlobalPluginContext*)>;
#if UE_5_1_OR_LATER
	DECLARE_TS_MULTICAST_DELEGATE_OneParam(FThreadSafeDelegate, AK::IAkGlobalPluginContext*);
#else
	DECLARE_MULTICAST_DELEGATE_OneParam(FThreadSafeDelegate, AK::IAkGlobalPluginContext*);
#endif
	DECLARE_MULTICAST_DELEGATE(FGameThreadDelegate);

#define WWISE_DQ_NAME(name) TEXT(name ## " Deferred Queue worker") 

	FWwiseDeferredQueue(const TCHAR* InDebugName);
	~FWwiseDeferredQueue();

	/**
	 * @brief Defer execution of an operation on the next Run() or Wait() call, where a task is started, and the function is run asynchronously.
	 * @param InFunction The function to execute
	 *
	 * For most uses, there is no time sensitivity to a deferred operation. Every single deferred Async operation are run sequentially, however,
	 * they might be executed at the same time than the Sync and Game operations. Also, these will probably be called in a non-game thread, so you are
	 * responsible to properly bound your code if this causes issues.
	 *
	 * The return value of the passed function should be "Done" if it was properly executed, whether there was an error or not. The return value
	 * should be "KeepRunning" if it should be deferred for the next callback loop.
	 */
	void AsyncDefer(FFunction&& InFunction);

	/**
	 * @brief Defer execution of an operation on the next Run() or Wait() call, where the function is immediately called in the Run() or Wait() thread.
	 * @param InFunction The function to execute, synchronous to the Run() thread.
	 *
	 * @warning Since this function will be called synchronously to the Run() call, if the running thread is time-sensitive, it can cause glitches
	 * or hitches. In most cases, you should use DeferAsync or DeferGame.
	 * 
	 * This is required for time-sensitive issues, such as waiting for an operation that absolutely needs to be done before the Run() thread gains
	 * control back.
	 */
	void SyncDefer(FSyncFunction&& InFunction);

	/**
	 * @brief Defer execution of an operation on the next Run() or Wait() call, where the function is subsequently run on the Game Thread.
	 * @param InFunction The function to execute
	 *
	 * For most game uses, there is no time sensitivity to a deferred operation, but you might want to notify game objects when an event occurs.
	 *
	 * Every single deferred Game operation are run sequentially, however, they might be executed at the same time than the other Async and Sync
	 * operations.
	 */
	void GameDefer(FFunction&& InFunction);


	void Run(AK::IAkGlobalPluginContext* InContext);
	void Wait();
	bool IsEmpty() const { return AsyncOpQueue.IsEmpty() && SyncOpQueue.IsEmpty() && GameOpQueue.IsEmpty(); }

	FGameThreadDelegate OnGameRun;
	FThreadSafeDelegate OnSyncRunTS;

protected:
	FWwiseExecutionQueue AsyncExecutionQueue;

	using FOps = TQueue<FFunction, EQueueMode::Mpsc>;
	using FSyncOps = TQueue<FSyncFunction, EQueueMode::Mpsc>;
	FOps AsyncOpQueue;
	FSyncOps SyncOpQueue;
	FOps GameOpQueue;
	TAtomic<int> GameThreadExecuting {0};
	bool bSyncThreadDone = false;
	bool bClosing = false;
	AK::IAkGlobalPluginContext* Context { nullptr };

private:
	void AsyncExec();
	void SyncExec();
	void SyncExecLoop();
	void GameThreadExec();
	void GameThreadExecLoop();
};
