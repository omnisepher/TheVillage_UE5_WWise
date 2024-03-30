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

#include "Async/TaskGraphInterfaces.h"
#include "Containers/Queue.h"
#include "Misc/DateTime.h"
#include "Misc/QueuedThreadPool.h"
#include "Wwise/WwiseTask.h"

#include <atomic>

/**
 * @brief Asynchronous sequential execution queue.
 * 
 * The goal of the Execution Queue is to call asynchronous operations sequentially. In effect,
 * this creates the equivalent of a GCD's Dispatch Queue, where operations are known to be executed
 * one after the other, but asynchronously.
*/
struct WWISECONCURRENCY_API FWwiseExecutionQueue
{
	const TCHAR* const DebugName;
	const EWwiseTaskPriority TaskPriority;
	using FBasicFunction = TUniqueFunction<void()>;

#define WWISE_EQ_NAME(name) TEXT(name) TEXT(" Execution Queue worker") 

	/**
	 * @brief Starts a new Execution Queue running at a particular thread priority.
	 * @param InDebugName Name for this execution queue's task. Can use WWISE_EQ_NAME to create a standardized name.
	 * @param InTaskPriority The priority for the task.
	*/
	FWwiseExecutionQueue(const TCHAR* InDebugName, EWwiseTaskPriority InTaskPriority = EWwiseTaskPriority::Default);

	/**
	 * @brief Destructor for Execution Queue.
	 * 
	 * This will lock the calling thread until the Execution Queue is actually closed.
	 *
	 * If the Execution Queue can be deleted while executing its own Async operation, you need to call CloseAndDelete() instead
	 * of deleting the Execution Queue yourself.
	*/
	~FWwiseExecutionQueue();

	/**
	 * @brief Calls a function asynchronously in the Execution Queue.
	 * @param InDebugName Name of the async task. Can use UE_SOURCE_LOCATION for a standardized name.
	 * @param InFunction The function to be called.
	 * 
	 * Async usually calls the passed function asynchronously. However, in deletion instances or exiting instances, this
	 * will be called synchronously. If you absolutely need this call to be done asynchronously (it might not be called),
	 * you should call AsyncAlways instead.
	*/
	void Async(const TCHAR* InDebugName, FBasicFunction&& InFunction);
	/**
	 * @brief Calls a function asynchronously in the Execution Queue. If no Execution Queue is available, it will be called
	 *        on any Task Graph thread.
	 * @param InDebugName Name of the async task. Can use UE_SOURCE_LOCATION for a standardized name.
	 * @param InFunction The function to be called.
	 *
	 * Execute the function asynchronously, or on the Task Graph, but never synchronously.
	*/
	void AsyncAlways(const TCHAR* InDebugName, FBasicFunction&& InFunction);

	/**
	 * @brief Calls a function asynchronously in the Execution Queue, and then wait for it to be called.
	 * @param InDebugName Name of the async task. Can use UE_SOURCE_LOCATION for a standardized name.
	 * @param InFunction The function to be called.
	*/
	void AsyncWait(const TCHAR* InDebugName, FBasicFunction&& InFunction);


	/**
	 * @brief Permanently closes the Execution Queue, ensuring every Async operations are done processing before closing.
	 *
	 * This gets called by the destructor. This will lock the calling thread until the Execution Queue is actually closed.
	*/
	void Close();

	/**
	 * @brief Permanently closes the Execution Queue once it's done processing asynchronously. Then, deletes the Execution Queue pointer.
	 * 
	 * This is the way to destroy an Execution Queue whose destruction can be achieved through one of its own function. For example,
	 * an object that async a structure cleanup, and then removes itself once it's possible to delete it.
	*/
	void CloseAndDelete();

	bool IsBeingClosed() const;
	bool IsClosed() const;

	bool IsRunningInThisThread() const;

	struct FOpQueueItem
	{
#if ENABLE_NAMED_EVENTS
		FOpQueueItem() : DebugName(nullptr), Function([]{}) {}
		FOpQueueItem(const TCHAR* InDebugName, FBasicFunction&& InFunction) :
			DebugName(InDebugName),
			Function(MoveTemp(InFunction))
		{}
		FOpQueueItem(FOpQueueItem&& Rhs) :
			DebugName(Rhs.DebugName),
			Function(MoveTemp(Rhs.Function))
		{
		}
#else
		FOpQueueItem() : Function([]{}) {}
		FOpQueueItem(const TCHAR*, FBasicFunction&& InFunction) :
			Function(MoveTemp(InFunction))
		{}
		FOpQueueItem(FOpQueueItem&& Rhs) :
			Function(MoveTemp(Rhs.Function))
		{
		}
#endif		
		FOpQueueItem& operator=(const FOpQueueItem& Rhs)
		{
#if ENABLE_NAMED_EVENTS
			check(Rhs.DebugName == nullptr);
			DebugName = nullptr;
#endif
			Function = []{};
			return *this;
		}
		
#if ENABLE_NAMED_EVENTS
		const TCHAR* DebugName;
#endif
		FBasicFunction Function;
	};

private:
	class ExecutionQueuePoolTask;

	enum class EWorkerState
	{
		Stopped,			///< Idle
		Running,			///< There is a thread owner executing operations
		AddOp,				///< While a thread is running, a producer is adding operations
		Closing,			///< While a thread is running, the last producer is asking to permanently close the Execution Queue
		Closed				///< Execution Queue is permanently closed. It can be deleted.
	};
	std::atomic<EWorkerState> WorkerState{ EWorkerState::Stopped };
	bool bDeleteOnceClosed{ false };

	using FOpQueue = TQueue<FOpQueueItem, EQueueMode::Mpsc>;
	FOpQueue OpQueue;

	struct TLS;

	void StartWorkerIfNeeded();
	void Work();
	bool KeepWorking();
	void ProcessWork();
	bool TrySetStoppedWorkerToRunning();
	bool TrySetRunningWorkerToStopped();
	bool TrySetRunningWorkerToAddOp();
	bool TrySetAddOpWorkerToRunning();
	bool TrySetRunningWorkerToClosing();
	bool TrySetClosingWorkerToClosed();

	static const TCHAR* StateName(EWorkerState State);
	FORCEINLINE bool TryStateUpdate(EWorkerState NeededState, EWorkerState WantedState);

private:
	FWwiseExecutionQueue(const FWwiseExecutionQueue& Rhs) = delete;
	FWwiseExecutionQueue(FWwiseExecutionQueue&& Rhs) = delete;
	FWwiseExecutionQueue& operator=(const FWwiseExecutionQueue& Rhs) = delete;
	FWwiseExecutionQueue& operator=(FWwiseExecutionQueue&& Rhs) = delete;

public:
	struct WWISECONCURRENCY_API Test
	{
#if defined(WITH_LOW_LEVEL_TESTS) && WITH_LOW_LEVEL_TESTS || defined(WITH_AUTOMATION_TESTS) || (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)
#define WWISE_EXECUTIONQUEUE_TEST_CONST
#else
#define WWISE_EXECUTIONQUEUE_TEST_CONST const
#endif
		static const bool bExtremelyVerbose;
		static WWISE_EXECUTIONQUEUE_TEST_CONST bool bMockEngineDeletion;
		static WWISE_EXECUTIONQUEUE_TEST_CONST bool bMockEngineDeleted;
		static WWISE_EXECUTIONQUEUE_TEST_CONST bool bMockSleepOnStateUpdate;
		static WWISE_EXECUTIONQUEUE_TEST_CONST bool bReduceLogVerbosity;
		static bool IsExtremelyVerbose(){ return bExtremelyVerbose && !bReduceLogVerbosity; }
	};
};
