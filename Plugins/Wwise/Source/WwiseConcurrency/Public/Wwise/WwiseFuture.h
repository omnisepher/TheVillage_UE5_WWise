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

#include "CoreTypes.h"
#include "Misc/AssertionMacros.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/Function.h"
#include "Misc/Timespan.h"
#include "Templates/SharedPointer.h"
#include "Misc/DateTime.h"
#include "HAL/Event.h"

#include "Wwise/Stats/Concurrency.h"
#include "Wwise/Stats/AsyncStats.h"

#include <atomic>

//
// WwiseFuture is a replica of Unreal's Async/Future.h code, with optimizations on the FutureState to reduce the
// amount of FCriticalSection and FEvent to the bare minimum. Most of Wwise Integrations code uses the "Then" paradigm
// of Futures/Promises instead of the Wait paradigm.
//



/**
 * Base class for the internal state of asynchronous return values (futures).
 */
class FWwiseFutureState
{
public:

	/** Default constructor. */
	FWwiseFutureState()
		: CompletionCallback(nullptr)
		, Complete(false)
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseFutures);
	}

	/**
	 * Create and initialize a new instance with a callback.
	 *
	 * @param InCompletionCallback A function that is called when the state is completed.
	 */
	FWwiseFutureState(TUniqueFunction<void()>&& InCompletionCallback)
		: CompletionCallback(InCompletionCallback ? new TUniqueFunction<void()>(MoveTemp(InCompletionCallback)) : nullptr)
		, Complete(false)
	{
		ASYNC_INC_DWORD_STAT(STAT_WwiseFutures);
	}

	/** Destructor. */
	~FWwiseFutureState()
	{
		const auto* Continuation = CompletionCallback.exchange(nullptr);
		check(!Continuation);
		if (UNLIKELY(Continuation))
		{
			delete Continuation;
		}

		ASYNC_DEC_DWORD_STAT(STAT_WwiseFutures);
	}

public:

	/**
	 * Checks whether the asynchronous result has been set.
	 *
	 * @return true if the result has been set, false otherwise.
	 * @see WaitFor
	 */
	bool IsComplete() const
	{
		return Complete.load(std::memory_order_seq_cst);
	}

	/**
	 * Blocks the calling thread until the future result is available.
	 *
	 * Compared to Unreal's native version, this version uses continuation, and assumes you haven't set a continuation
	 * function on this state.
	 *
	 * @param Duration The maximum time span to wait for the future result.
	 * @return true if the result is available, false otherwise.
	 * @see IsComplete
	 */
	bool WaitFor(const FTimespan& Duration)
	{
		check(!CompletionCallback.load(std::memory_order_seq_cst));
		
		if (IsComplete())
		{
			return true;
		}

		auto CompletionEvent = MakeShared<FEventRef, ESPMode::ThreadSafe>();
		SetContinuation([CompletionEvent]
		{
			CompletionEvent.Get()->Trigger();
		});
		return CompletionEvent.Get()->Wait(Duration);
	}

	/**
	 * Set a continuation to be called on completion of the promise
	 * @param Continuation
	 */
	void SetContinuation(TUniqueFunction<void()>&& Continuation)
	{
		if (IsComplete())
		{
			if (Continuation)
			{
				Continuation();
			}
			return;
		}

		// Store the Copy to the CompletionCallback
		auto Copy = Continuation ? new TUniqueFunction<void()>(MoveTemp(Continuation)) : nullptr;
		auto OldCopy = CompletionCallback.exchange(Copy);
		check(!OldCopy);		// We can only execute one continuation per WwiseFuture.

		if (!IsComplete())
		{
			return;
		}

		// We are already complete. See if we need to execute ourselves.
		Copy = CompletionCallback.exchange(nullptr);
		if (Copy)
		{
			(*Copy)();
			delete Copy;
		}
	}

protected:

	/** Notifies any waiting threads that the result is available. */
	void MarkComplete()
	{
		Complete.store(true, std::memory_order_seq_cst);

		auto* Continuation = CompletionCallback.exchange(nullptr);

		if (Continuation)
		{
			(*Continuation)();
			delete Continuation;
		}
	}

private:
	/** An optional callback function that is executed the state is completed. */
	std::atomic< TUniqueFunction<void()>* > CompletionCallback;

	/** Whether the asynchronous result is available. */
	std::atomic<bool> Complete;
};


/**
 * Implements the internal state of asynchronous return values (futures).
 */
template<typename InternalResultType>
class TWwiseFutureState
	: public FWwiseFutureState
{
public:

	/** Default constructor. */
	TWwiseFutureState()
		: FWwiseFutureState()
	{ }

	~TWwiseFutureState()
	{
		if (IsComplete())
		{
			DestructItem(Result.GetTypedPtr());
		}
	}

	/**
	 * Create and initialize a new instance with a callback.
	 *
	 * @param CompletionCallback A function that is called when the state is completed.
	 */
	TWwiseFutureState(TUniqueFunction<void()>&& CompletionCallback)
		: FWwiseFutureState(MoveTemp(CompletionCallback))
	{ }

public:

	/**
	 * Gets the result (will block the calling thread until the result is available).
	 *
	 * @return The result value.
	 * @see EmplaceResult
	 */
	const InternalResultType& GetResult()
	{
		while (!IsComplete())
		{
			WaitFor(FTimespan::MaxValue());
		}

		return *Result.GetTypedPtr();
	}

	/**
	 * Sets the result and notifies any waiting threads.
	 *
	 * @param InResult The result to set.
	 * @see GetResult
	 */
	template<typename... ArgTypes>
	void EmplaceResult(ArgTypes&&... Args)
	{
		check(!IsComplete());
		new(Result.GetTypedPtr()) InternalResultType(Forward<ArgTypes>(Args)...);
		MarkComplete();
	}

private:

	/** Holds the asynchronous result. */
	TTypeCompatibleBytes<InternalResultType> Result;
};

/* TWwiseFuture
*****************************************************************************/

/**
 * Abstract base template for futures and shared futures.
 */
template<typename InternalResultType>
class TWwiseFutureBase
{
public:

	/**
	 * Checks whether this future object has its value set.
	 *
	 * @return true if this future has a shared state and the value has been set, false otherwise.
	 * @see IsValid
	 */
	bool IsReady() const
	{
		return State.IsValid() ? State->IsComplete() : false;
	}

	/**
	 * Checks whether this future object has a valid state.
	 *
	 * @return true if the state is valid, false otherwise.
	 * @see IsReady
	 */
	bool IsValid() const
	{
		return State.IsValid();
	}

	/**
	 * Blocks the calling thread until the future result is available.
	 *
	 * Note that this method may block forever if the result is never set. Use
	 * the WaitFor or WaitUntil methods to specify a maximum timeout for the wait.
	 *
	 * @see WaitFor, WaitUntil
	 */
	void Wait() const
	{
		if (State.IsValid())
		{
			while (!WaitFor(FTimespan::MaxValue()));
		}
	}

	/**
	 * Blocks the calling thread until the future result is available or the specified duration is exceeded.
	 *
	 * @param Duration The maximum time span to wait for the future result.
	 * @return true if the result is available, false otherwise.
	 * @see Wait, WaitUntil
	 */
	bool WaitFor(const FTimespan& Duration) const
	{
		return State.IsValid() ? State->WaitFor(Duration) : false;
	}

	/**
	 * Blocks the calling thread until the future result is available or the specified time is hit.
	 *
	 * @param Time The time until to wait for the future result (in UTC).
	 * @return true if the result is available, false otherwise.
	 * @see Wait, WaitUntil
	 */
	bool WaitUntil(const FDateTime& Time) const
	{
		return WaitFor(Time - FDateTime::UtcNow());
	}

protected:

	typedef TSharedPtr<TWwiseFutureState<InternalResultType>, ESPMode::ThreadSafe> StateType;

	/** Default constructor. */
	TWwiseFutureBase() { }

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InState The shared state to initialize with.
	 */
	TWwiseFutureBase(const StateType& InState)
		: State(InState)
	{ }

	/**
	 * Protected move construction
	 */
	TWwiseFutureBase(TWwiseFutureBase&&) = default;

	/**
	 * Protected move assignment
	 */
	TWwiseFutureBase& operator=(TWwiseFutureBase&&) = default;

	/**
	 * Protected copy construction
	 */
	TWwiseFutureBase(const TWwiseFutureBase&) = default;

	/**
	 * Protected copy assignment
	 */
	TWwiseFutureBase& operator=(const TWwiseFutureBase&) = default;

	/** Protected destructor. */
	~TWwiseFutureBase() { }

protected:

	/**
	 * Gets the shared state object.
	 *
	 * @return The shared state object.
	 */
	const StateType& GetState() const
	{
		// if you hit this assertion then your future has an invalid state.
		// this happens if you have an uninitialized future or if you moved
		// it to another instance.
		check(State.IsValid());

		return State;
	}

	/**
	 * Set a completion callback that will be called once the future completes
	 *	or immediately if already completed
	 *
	 * @param Continuation a continuation taking an argument of type TWwiseFuture<InternalResultType>
	 * @return nothing at the moment but could return another future to allow future chaining
	 */
	template<typename Func>
	auto Then(Func Continuation);

	/**
	 * Convenience wrapper for Then that
	 *	set a completion callback that will be called once the future completes
	 *	or immediately if already completed
	 * @param Continuation a continuation taking an argument of type InternalResultType
	 * @return nothing at the moment but could return another future to allow future chaining
	 */
	template<typename Func>
	auto Next(Func Continuation);

	/**
	 * Reset the future.
	 *	Resetting a future removes any continuation from its shared state and invalidates it.
	 *	Useful for discarding yet to be completed future cleanly.
	 */
	void Reset()
	{
		if (IsValid())
		{
			this->State->SetContinuation(nullptr);
			this->State.Reset();
		}
	}

private:

	/** Holds the future's state. */
	StateType State;
};

/**
 * Template for unshared futures.
 */
template<typename ResultType>
class TWwiseFuture
	: public TWwiseFutureBase<ResultType>
{
	typedef TWwiseFutureBase<ResultType> BaseType;

public:

	/** Default constructor. */
	TWwiseFuture() { }

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InState The shared state to initialize with.
	 */
	TWwiseFuture(const typename BaseType::StateType& InState)
		: BaseType(InState)
	{ }

	/**
	 * Move constructor.
	 */
	TWwiseFuture(TWwiseFuture&&) = default;

	/**
	 * Move assignment operator.
	 */
	TWwiseFuture& operator=(TWwiseFuture&& Other) = default;

	/** Destructor. */
	~TWwiseFuture() { }

public:

	/**
	 * Gets the future's result.
	 *
	 * @return The result.
	 */
	ResultType Get() const
	{
		return this->GetState()->GetResult();
	}

	/**
	 * Expose Then functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Then;

	/**
	 * Expose Next functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Next;

	/**
	 * Expose Reset functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Reset;

private:

	/** Hidden copy constructor (futures cannot be copied). */
	TWwiseFuture(const TWwiseFuture&);

	/** Hidden copy assignment (futures cannot be copied). */
	TWwiseFuture& operator=(const TWwiseFuture&);
};


/**
 * Template for unshared futures (specialization for reference types).
 */
template<typename ResultType>
class TWwiseFuture<ResultType&>
	: public TWwiseFutureBase<ResultType*>
{
	typedef TWwiseFutureBase<ResultType*> BaseType;

public:

	/** Default constructor. */
	TWwiseFuture() { }

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InState The shared state to initialize with.
	 */
	TWwiseFuture(const typename BaseType::StateType& InState)
		: BaseType(InState)
	{ }

	/**
	 * Move constructor.
	 */
	TWwiseFuture(TWwiseFuture&&) = default;

	/**
	 * Move assignment operator.
	 */
	TWwiseFuture& operator=(TWwiseFuture&& Other) = default;

	/** Destructor. */
	~TWwiseFuture() { }

public:

	/**
	 * Gets the future's result.
	 *
	 * @return The result.
	 */
	ResultType& Get() const
	{
		return *this->GetState()->GetResult();
	}

	/**
	 * Expose Then functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Then;

	/**
	 * Expose Next functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Next;

	/**
	 * Expose Reset functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Reset;

private:

	/** Hidden copy constructor (futures cannot be copied). */
	TWwiseFuture(const TWwiseFuture&);

	/** Hidden copy assignment (futures cannot be copied). */
	TWwiseFuture& operator=(const TWwiseFuture&);
};


/**
 * Template for unshared futures (specialization for void).
 */
template<>
class TWwiseFuture<void>
	: public TWwiseFutureBase<int>
{
	typedef TWwiseFutureBase<int> BaseType;

public:

	/** Default constructor. */
	TWwiseFuture() { }

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InState The shared state to initialize with.
	 */
	TWwiseFuture(const BaseType::StateType& InState)
		: BaseType(InState)
	{ }

	/**
	 * Move constructor.
	 */
	TWwiseFuture(TWwiseFuture&&) = default;

	/**
	 * Move assignment operator.
	 */
	TWwiseFuture& operator=(TWwiseFuture&& Other) = default;

	/** Destructor. */
	~TWwiseFuture() { }

public:

	/**
	 * Gets the future's result.
	 *
	 * @return The result.
	 */
	void Get() const
	{
		GetState()->GetResult();
	}

	/**
	 * Expose Then functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Then;

	/**
	 * Expose Next functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Next;

	/**
	 * Expose Reset functionality
	 * @see TWwiseFutureBase
	 */
	using BaseType::Reset;

private:

	/** Hidden copy constructor (futures cannot be copied). */
	TWwiseFuture(const TWwiseFuture&);

	/** Hidden copy assignment (futures cannot be copied). */
	TWwiseFuture& operator=(const TWwiseFuture&);
};


/* TWwisePromise
*****************************************************************************/

template<typename InternalResultType>
class TWwisePromiseBase
	: FNoncopyable
{
	typedef TSharedPtr<TWwiseFutureState<InternalResultType>, ESPMode::ThreadSafe> StateType;

public:

	/** Default constructor. */
	TWwisePromiseBase()
		: State(MakeShared<TWwiseFutureState<InternalResultType>, ESPMode::ThreadSafe>())
	{ }

	/**
	 * Move constructor.
	 *
	 * @param Other The promise holding the shared state to move.
	 */
	TWwisePromiseBase(TWwisePromiseBase&& Other)
		: State(MoveTemp(Other.State))
	{
		Other.State.Reset();
	}

	/**
	 * Create and initialize a new instance with a callback.
	 *
	 * @param CompletionCallback A function that is called when the future state is completed.
	 */
	TWwisePromiseBase(TUniqueFunction<void()>&& CompletionCallback)
		: State(MakeShared<TWwiseFutureState<InternalResultType>, ESPMode::ThreadSafe>(MoveTemp(CompletionCallback)))
	{ }

public:

	/** Move assignment operator. */
	TWwisePromiseBase& operator=(TWwisePromiseBase&& Other)
	{
		State = Other.State;
		Other.State.Reset();
		return *this;
	}

protected:

	/** Destructor. */
	~TWwisePromiseBase()
	{
		if (State.IsValid())
		{
			// if you hit this assertion then your promise never had its result
			// value set. broken promises are considered programming errors.
			check(State->IsComplete());

			State.Reset();
		}
	}

	/**
	 * Gets the shared state object.
	 *
	 * @return The shared state object.
	 */
	const StateType& GetState()
	{
		// if you hit this assertion then your promise has an invalid state.
		// this happens if you move the promise to another instance.
		check(State.IsValid());

		return State;
	}

private:

	/** Holds the shared state object. */
	StateType State;
};


/**
 * Template for promises.
 */
template<typename ResultType>
class TWwisePromise
	: public TWwisePromiseBase<ResultType>
{
public:

	typedef TWwisePromiseBase<ResultType> BaseType;

	/** Default constructor (creates a new shared state). */
	TWwisePromise()
		: BaseType()
		, FutureRetrieved(false)
	{ }

	/**
	 * Move constructor.
	 *
	 * @param Other The promise holding the shared state to move.
	 */
	TWwisePromise(TWwisePromise&& Other)
		: BaseType(MoveTemp(Other))
		, FutureRetrieved(MoveTemp(Other.FutureRetrieved))
	{ }

	/**
	 * Create and initialize a new instance with a callback.
	 *
	 * @param CompletionCallback A function that is called when the future state is completed.
	 */
	TWwisePromise(TUniqueFunction<void()>&& CompletionCallback)
		: BaseType(MoveTemp(CompletionCallback))
		, FutureRetrieved(false)
	{ }

public:

	/**
	 * Move assignment operator.
	 *
	 * @param Other The promise holding the shared state to move.
	 */
	TWwisePromise& operator=(TWwisePromise&& Other)
	{
		BaseType::operator=(MoveTemp(Other));
		FutureRetrieved = MoveTemp(Other.FutureRetrieved);

		return *this;
	}

public:

	/**
	 * Gets a TWwiseFuture object associated with the shared state of this promise.
	 *
	 * @return The TWwiseFuture object.
	 */
	TWwiseFuture<ResultType> GetFuture()
	{
		check(!FutureRetrieved);
		FutureRetrieved = true;

		return TWwiseFuture<ResultType>(this->GetState());
	}

	/**
	 * Sets the promised result.
	 *
	 * The result must be set only once. An assertion will
	 * be triggered if this method is called a second time.
	 *
	 * @param Result The result value to set.
	 */
	FORCEINLINE void SetValue(const ResultType& Result)
	{
		EmplaceValue(Result);
	}

	/**
	 * Sets the promised result (from rvalue).
	 *
	 * The result must be set only once. An assertion will
	 * be triggered if this method is called a second time.
	 *
	 * @param Result The result value to set.
	 */
	FORCEINLINE void SetValue(ResultType&& Result)
	{
		EmplaceValue(MoveTemp(Result));
	}

	/**
	 * Sets the promised result.
	 *
	 * The result must be set only once. An assertion will
	 * be triggered if this method is called a second time.
	 *
	 * @param Result The result value to set.
	 */
	template<typename... ArgTypes>
	void EmplaceValue(ArgTypes&&... Args)
	{
		this->GetState()->EmplaceResult(Forward<ArgTypes>(Args)...);
	}

private:

	/** Whether a future has already been retrieved from this promise. */
	bool FutureRetrieved;
};


/**
 * Template for promises (specialization for reference types).
 */
template<typename ResultType>
class TWwisePromise<ResultType&>
	: public TWwisePromiseBase<ResultType*>
{
	typedef TWwisePromiseBase<ResultType*> BaseType;

public:

	/** Default constructor (creates a new shared state). */
	TWwisePromise()
		: BaseType()
		, FutureRetrieved(false)
	{ }

	/**
	 * Move constructor.
	 *
	 * @param Other The promise holding the shared state to move.
	 */
	TWwisePromise(TWwisePromise&& Other)
		: BaseType(MoveTemp(Other))
		, FutureRetrieved(MoveTemp(Other.FutureRetrieved))
	{ }

	/**
	 * Create and initialize a new instance with a callback.
	 *
	 * @param CompletionCallback A function that is called when the future state is completed.
	 */
	TWwisePromise(TUniqueFunction<void()>&& CompletionCallback)
		: BaseType(MoveTemp(CompletionCallback))
		, FutureRetrieved(false)
	{ }

public:

	/**
	 * Move assignment operator.
	 *
	 * @param Other The promise holding the shared state to move.
	 */
	TWwisePromise& operator=(TWwisePromise&& Other)
	{
		BaseType::operator=(MoveTemp(Other));
		FutureRetrieved = MoveTemp(Other.FutureRetrieved);

		return this;
	}

public:

	/**
	 * Gets a TWwiseFuture object associated with the shared state of this promise.
	 *
	 * @return The TWwiseFuture object.
	 */
	TWwiseFuture<ResultType&> GetFuture()
	{
		check(!FutureRetrieved);
		FutureRetrieved = true;

		return TWwiseFuture<ResultType&>(this->GetState());
	}

	/**
	 * Sets the promised result.
	 *
	 * The result must be set only once. An assertion will
	 * be triggered if this method is called a second time.
	 *
	 * @param Result The result value to set.
	 */
	FORCEINLINE void SetValue(ResultType& Result)
	{
		EmplaceValue(Result);
	}

	/**
	 * Sets the promised result.
	 *
	 * The result must be set only once. An assertion will
	 * be triggered if this method is called a second time.
	 *
	 * @param Result The result value to set.
	 */
	void EmplaceValue(ResultType& Result)
	{
		this->GetState()->EmplaceResult(&Result);
	}

private:

	/** Whether a future has already been retrieved from this promise. */
	bool FutureRetrieved;
};


/**
 * Template for promises (specialization for void results).
 */
template<>
class TWwisePromise<void>
	: public TWwisePromiseBase<int>
{
	typedef TWwisePromiseBase<int> BaseType;

public:

	/** Default constructor (creates a new shared state). */
	TWwisePromise()
		: BaseType()
		, FutureRetrieved(false)
	{ }

	/**
	 * Move constructor.
	 *
	 * @param Other The promise holding the shared state to move.
	 */
	TWwisePromise(TWwisePromise&& Other)
		: BaseType(MoveTemp(Other))
		, FutureRetrieved(false)
	{ }

	/**
	 * Create and initialize a new instance with a callback.
	 *
	 * @param CompletionCallback A function that is called when the future state is completed.
	 */
	TWwisePromise(TUniqueFunction<void()>&& CompletionCallback)
		: BaseType(MoveTemp(CompletionCallback))
		, FutureRetrieved(false)
	{ }

public:

	/**
	 * Move assignment operator.
	 *
	 * @param Other The promise holding the shared state to move.
	 */
	TWwisePromise& operator=(TWwisePromise&& Other)
	{
		BaseType::operator=(MoveTemp(Other));
		FutureRetrieved = MoveTemp(Other.FutureRetrieved);

		return *this;
	}

public:

	/**
	 * Gets a TWwiseFuture object associated with the shared state of this promise.
	 *
	 * @return The TWwiseFuture object.
	 */
	TWwiseFuture<void> GetFuture()
	{
		check(!FutureRetrieved);
		FutureRetrieved = true;

		return TWwiseFuture<void>(GetState());
	}

	/**
	 * Sets the promised result.
	 *
	 * The result must be set only once. An assertion will
	 * be triggered if this method is called a second time.
	 */
	FORCEINLINE void SetValue()
	{
		EmplaceValue();
	}

	/**
	 * Sets the promised result.
	 *
	 * The result must be set only once. An assertion will
	 * be triggered if this method is called a second time.
	 *
	 * @param Result The result value to set.
	 */
	void EmplaceValue()
	{
		this->GetState()->EmplaceResult(0);
	}

private:

	/** Whether a future has already been retrieved from this promise. */
	bool FutureRetrieved;
};

/* TWwiseFuture::Then
*****************************************************************************/

namespace FutureDetail
{
	/**
	* Template for setting a promise value from a continuation.
	*/
	template<typename Func, typename ParamType, typename ResultType>
	inline void SetPromiseValue(TWwisePromise<ResultType>& Promise, Func& Function, TWwiseFuture<ParamType>&& Param)
	{
		Promise.SetValue(Function(MoveTemp(Param)));
	}
	template<typename Func, typename ParamType>
	inline void SetPromiseValue(TWwisePromise<void>& Promise, Func& Function, TWwiseFuture<ParamType>&& Param)
	{
		Function(MoveTemp(Param));
		Promise.SetValue();
	}
}

// Then implementation
template<typename InternalResultType>
template<typename Func>
auto TWwiseFutureBase<InternalResultType>::Then(Func Continuation) //-> TWwiseFuture<decltype(Continuation(MoveTemp(TWwiseFuture<InternalResultType>())))>
{
	check(IsValid());
	using ReturnValue = decltype(Continuation(MoveTemp(TWwiseFuture<InternalResultType>())));

	TWwisePromise<ReturnValue> Promise;
	TWwiseFuture<ReturnValue> FutureResult = Promise.GetFuture();
	TUniqueFunction<void()> Callback = [PromiseCapture = MoveTemp(Promise), ContinuationCapture = MoveTemp(Continuation), StateCapture = this->State]() mutable
	{
		FutureDetail::SetPromiseValue(PromiseCapture, ContinuationCapture, TWwiseFuture<InternalResultType>(MoveTemp(StateCapture)));
	};

	// This invalidate this future.
	StateType MovedState = MoveTemp(this->State);
	MovedState->SetContinuation(MoveTemp(Callback));
	return FutureResult;
}

// Next implementation
template<typename InternalResultType>
template<typename Func>
auto TWwiseFutureBase<InternalResultType>::Next(Func Continuation) //-> TWwiseFuture<decltype(Continuation(Get()))>
{
	return this->Then([Continuation = MoveTemp(Continuation)](TWwiseFuture<InternalResultType> Self) mutable
	{
		return Continuation(Self.Get());
	});
}

/** Helper to create and immediately fulfill a promise */
template<typename ResultType, typename... ArgTypes>
TWwisePromise<ResultType> MakeFulfilledWwisePromise(ArgTypes&&... Args)
{
	TWwisePromise<ResultType> Promise;
	Promise.EmplaceValue(Forward<ArgTypes>(Args)...);
	return Promise;
}
