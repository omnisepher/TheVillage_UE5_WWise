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
#include "Wwise/WwiseFileStateTools.h"
#include <atomic>

class FWwiseAsyncCycleCounter;
class FWwiseStreamableFileStateInfo;

enum class WWISEFILEHANDLER_API EWwiseFileStateOperationOrigin
{
	Loading,
	Streaming
};

class WWISEFILEHANDLER_API FWwiseFileState : protected FWwiseFileStateTools
{
public:
	~FWwiseFileState() override;

	template <typename RequestedType>
	RequestedType* GetStateAs()
	{
#if defined(WITH_RTTI) || defined(_CPPRTTI) || defined(__GXX_RTTI)
		auto* Result = dynamic_cast<RequestedType*>(this);
		checkf(Result, TEXT("Invalid Type Cast"));
#else
		auto* Result = static_cast<RequestedType*>(this);
#endif
		return Result;
	}

	/// A serial queue of operations being processed for this state
	FWwiseExecutionQueue* FileStateExecutionQueue{ new FWwiseExecutionQueue(WWISE_EQ_NAME("FWwiseFileState")) };

	using FBasicFunction = FWwiseExecutionQueue::FBasicFunction;
	using FOpQueueItem = FWwiseExecutionQueue::FOpQueueItem; 
	using FLaterOpQueue = TQueue<FOpQueueItem, EQueueMode::Spsc>;

	/// Operation queue containing operations waiting to be unclogged (such as a load time)
	FLaterOpQueue LaterOpQueue;

	/// Number of instances opened. Slightly equivalent to LoadCount, but set synchronously and updated at extremities.
	std::atomic<int> OpenedInstances{ 0 };
	
	/// Number of times the Loading operation got requested for this state
	int LoadCount{ 0 };

	/// Number of times the Streaming operation got requested for this state
	int StreamingCount{ 0 };

	/// Current operation serial # being processed. Used to return callbacks in order of calling.
	int CreationOpOrder{ 0 };

	/// Current operation serial # done. Used to return callbacks in order of calling.
	int DoneOpOrder{ 0 };

	enum class WWISEFILEHANDLER_API EState
	{
		Closed,
		Opening,
		Opened,
		Loading,
		Loaded,
		Unloading,
		Closing,

		WillReload,			// While unloading, a task is waiting to load again
		CanReload,			// Equivalent to Opened, but won't Close
		WillReopen,			// While closing, a task is waiting to open again
		CanReopen			// Equivalent to Closed, but won't Delete
	};
	EState State;
	static const TCHAR* GetStateNameFor(EState State);
	const TCHAR* GetStateName() const;
	void SetState(const TCHAR* const Caller, EState NewState);
	void SetStateFrom(const TCHAR* const Caller, EState ExpectedState, EState NewState);

	enum class WWISEFILEHANDLER_API EResult
	{
		/**
		 * @brief The File State operation is completed. It doesn't tell whether the result is successful or not.
		 */
		Done,

		/**
		 * @brief The File State operation got deferred at a later time.
		 */
		Deferred
	};

	using FIncrementCountCallback = TUniqueFunction<void(bool bInResult)>;
	void IncrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin, FIncrementCountCallback&& InCallback);

	using FDecrementCountCallback = TUniqueFunction<void()>;
	using FDeleteFileStateFunction = TUniqueFunction<void(FDecrementCountCallback&& InCallback)>;
	void DecrementCountAsync(EWwiseFileStateOperationOrigin InOperationOrigin, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);

	virtual bool CanDelete() const;
	virtual const TCHAR* GetManagingTypeName() const { return TEXT("Invalid"); }
	virtual uint32 GetShortId() const { return 0; }

	virtual FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() { return nullptr; }
	virtual const FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo()  const { return nullptr; }
	virtual bool IsStreamedState() const { return GetStreamableFileStateInfo() != nullptr; }

	FWwiseFileState(FWwiseFileState const&) = delete;
	FWwiseFileState& operator=(FWwiseFileState const&) = delete;
	FWwiseFileState(FWwiseFileState&&) = delete;
	FWwiseFileState& operator=(FWwiseFileState&&) = delete;

protected:
	std::atomic<bool> bRecurringCallbackRegistered { false };

	FWwiseFileState();
	void Term();

	static void IncrementCountAsyncDone(FWwiseAsyncCycleCounter&& InOpCycleCounter, FIncrementCountCallback&& InCallback, bool bInResult);
	static void DecrementCountAsyncDone(FWwiseAsyncCycleCounter&& InOpCycleCounter, FDecrementCountCallback&& InCallback);

	virtual void IncrementCount(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FIncrementCountCallback&& InCallback);
	virtual void IncrementCountOpen(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FIncrementCountCallback&& InCallback);
	virtual void IncrementCountLoad(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FIncrementCountCallback&& InCallback);
	virtual void IncrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FIncrementCountCallback&& InCallback);

	virtual void DecrementCount(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);
	virtual void DecrementCountUnload(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);
	virtual void DecrementCountUnloadCallback(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback, EResult InDefer);
	virtual void DecrementCountClose(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);
	virtual void DecrementCountCloseCallback(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback, EResult InDefer);
	virtual void DecrementCountDone(EWwiseFileStateOperationOrigin InOperationOrigin, int InCurrentOpOrder, FDeleteFileStateFunction&& InDeleteState, FDecrementCountCallback&& InCallback);

	using FOpenFileCallback = TUniqueFunction<void()>;
	using FLoadInSoundEngineCallback = TUniqueFunction<void()>;
	virtual void IncrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin);

	virtual bool CanOpenFile() const;
	virtual void OpenFile(FOpenFileCallback&& InCallback) { OpenFileFailed(MoveTemp(InCallback)); }
	void OpenFileSucceeded(FOpenFileCallback&& InCallback);
	void OpenFileFailed(FOpenFileCallback&& InCallback);

	virtual bool CanLoadInSoundEngine() const;
	virtual void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) { LoadInSoundEngineFailed(MoveTemp(InCallback)); }
	void LoadInSoundEngineSucceeded(FLoadInSoundEngineCallback&& InCallback);
	void LoadInSoundEngineFailed(FLoadInSoundEngineCallback&& InCallback);

	using FUnloadFromSoundEngineCallback = TUniqueFunction<void(EResult InResult)>;
	using FCloseFileCallback = TUniqueFunction<void(EResult InResult)>;
	virtual void DecrementLoadCount(EWwiseFileStateOperationOrigin InOperationOrigin);

	virtual bool CanUnloadFromSoundEngine() const;
	virtual void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) { UnloadFromSoundEngineDone(MoveTemp(InCallback)); }
	void UnloadFromSoundEngineDone(FUnloadFromSoundEngineCallback&& InCallback);
	void UnloadFromSoundEngineToClosedFile(FUnloadFromSoundEngineCallback&& InCallback);
	void UnloadFromSoundEngineDefer(FUnloadFromSoundEngineCallback&& InCallback);

	virtual bool CanCloseFile() const;
	virtual void CloseFile(FCloseFileCallback&& InCallback) { CloseFileDone(MoveTemp(InCallback)); }
	void CloseFileDone(FCloseFileCallback&& InCallback);
	void CloseFileDefer(FCloseFileCallback&& InCallback);

	bool IsBusy() const;
	
	void AsyncOp(const TCHAR* InDebugName, FBasicFunction&& Fct);
	void AsyncOpLater(const TCHAR* InDebugName, FBasicFunction&& Fct);
	void ProcessLaterOpQueue();
	
	virtual void RegisterRecurringCallback();
};

using FWwiseFileStateSharedPtr = TSharedPtr<FWwiseFileState, ESPMode::ThreadSafe>;
