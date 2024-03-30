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

#include "Wwise/WwiseTask.h"
#include "Wwise/WwiseUnitTests.h"

#if WWISE_UNIT_TESTS
#include "Wwise/WwiseFuture.h"
#include <atomic>

WWISE_TEST_CASE(Concurrency_Future_Smoke, "Wwise::Concurrency::Future_Smoke", "[ApplicationContextMask][SmokeFilter]")
{
	SECTION("Static")
	{
		static_assert(std::is_constructible<TWwisePromise<void>>::value, "Can create a void promise");
		static_assert(std::is_constructible<TWwiseFuture<void>>::value, "Can create a void future");
		static_assert(std::is_constructible<TWwisePromise<int>>::value, "Can create an int promise");
		static_assert(std::is_constructible<TWwiseFuture<int>>::value, "Can create an int future");
		static_assert(std::is_constructible<TWwisePromise<int&>>::value, "Can create a reference promise");
		static_assert(std::is_constructible<TWwiseFuture<int&>>::value, "Can create a reference future");

		static_assert(!std::is_copy_constructible<TWwisePromise<void>>::value, "A promise cannot be copied");
		static_assert(std::is_move_constructible<TWwisePromise<void>>::value, "A promise can be moved");
		static_assert(!std::is_copy_constructible<TWwiseFuture<void>>::value, "A future cannot be copied");
		static_assert(std::is_move_constructible<TWwiseFuture<void>>::value, "A future can be moved");
	}

	SECTION("Basic Operations")
	{
		TWwisePromise<void> VoidPromise;
		TWwisePromise<int> IntPromise;
		TWwisePromise<int&> IntRefPromise;

		TWwiseFuture<void> UninitVoidFuture;
		TWwiseFuture<int> UninitIntFuture;
		TWwiseFuture<int&> UninitIntRefFuture;

		TWwiseFuture<void> VoidFuture( VoidPromise.GetFuture() );
		TWwiseFuture<int> IntFuture( IntPromise.GetFuture() );
		TWwiseFuture<int&> IntRefFuture( IntRefPromise.GetFuture() );

		CHECK(!UninitVoidFuture.IsValid());
		CHECK(!UninitIntFuture.IsValid());
		CHECK(!UninitIntRefFuture.IsValid());
		CHECK(VoidFuture.IsValid());
		CHECK(IntFuture.IsValid());
		CHECK(IntRefFuture.IsValid());

		CHECK(!VoidFuture.IsReady());
		CHECK(!IntFuture.IsReady());
		CHECK(!IntRefFuture.IsReady());

		int Value = 1;
		VoidPromise.SetValue();
		IntPromise.SetValue(1);
		IntRefPromise.SetValue(Value);

		CHECK(VoidFuture.IsReady());
		CHECK(IntFuture.IsReady());
		CHECK(IntRefFuture.IsReady());
	}

	SECTION("Next")
	{
		bool bDone = false;
		TWwisePromise<void> VoidPromise;
		auto VoidFuture( VoidPromise.GetFuture() );
		auto NewFuture( VoidFuture.Next([&bDone](int)
		{
			bDone = true;
		}));
		CHECK(!VoidFuture.IsValid());
		CHECK(NewFuture.IsValid());
	
		VoidPromise.EmplaceValue();
		CHECK(bDone);
		CHECK(NewFuture.IsReady());
	}

	SECTION("Next Already Emplaced")
	{
		bool bDone = false;
		TWwisePromise<void> VoidPromise;
		VoidPromise.EmplaceValue();
		
		auto VoidFuture( VoidPromise.GetFuture() );
		auto NewFuture( VoidFuture.Next([&bDone](int)
		{
			bDone = true;
		}));
		CHECK(!VoidFuture.IsValid());
		CHECK(NewFuture.IsValid());
	
		CHECK(bDone);
		CHECK(NewFuture.IsReady());
	}

	SECTION("Then")
	{
		bool bDone = false;
		TWwisePromise<void> VoidPromise;
		auto VoidFuture( VoidPromise.GetFuture() );
		auto NewFuture( VoidFuture.Then([&bDone](TWwiseFuture<int> Future) mutable
		{
			Future.Get();
			CHECK(Future.IsReady());
			bDone = true;
		}));
		CHECK(!VoidFuture.IsValid());
		CHECK(NewFuture.IsValid());

		VoidPromise.EmplaceValue();
		CHECK(bDone);
		CHECK(NewFuture.IsReady());
	}

	SECTION("Then Already Emplaced")
	{
		bool bDone = false;
		TWwisePromise<void> VoidPromise;
		VoidPromise.EmplaceValue();

		auto VoidFuture( VoidPromise.GetFuture() );
		auto NewFuture( VoidFuture.Then([&bDone](TWwiseFuture<int> Future) mutable
		{
			Future.Get();
			CHECK(Future.IsReady());
			bDone = true;
		}));
		CHECK(!VoidFuture.IsValid());
		CHECK(NewFuture.IsValid());

		CHECK(bDone);
		CHECK(NewFuture.IsReady());
	}

	SECTION("Wait Fulfilled")
	{
		TWwisePromise<void> VoidPromise;
		auto VoidFuture( VoidPromise.GetFuture() );
		VoidPromise.EmplaceValue();
		CHECK(VoidFuture.WaitFor(0));
	}

	SECTION("Wait Unfulfilled")
	{
		TWwisePromise<void> VoidPromise;
		auto VoidFuture( VoidPromise.GetFuture() );
		FFunctionGraphTask::CreateAndDispatchWhenReady([VoidPromise = MoveTemp(VoidPromise)]() mutable
		{
			VoidPromise.EmplaceValue();
		});
		CHECK(VoidFuture.WaitFor(FTimespan::FromMilliseconds(10)));
	}

	SECTION("Wait Not Ready")
	{
		TWwisePromise<void> VoidPromise;
		auto VoidFuture( VoidPromise.GetFuture() );
		CHECK(!VoidFuture.WaitFor(0));
		VoidPromise.EmplaceValue();
	}
}

/*
WWISE_TEST_CASE(Concurrency_Future, "Wwise::Concurrency::Future", "[ApplicationContextMask][ProductFilter]")
{
}
*/

/*
WWISE_TEST_CASE(Concurrency_Future_Perf, "Wwise::Concurrency::Future_Perf", "[ApplicationContextMask][PerfFilter]")
{
}
*/

void Multithreaded_Then_Future_Op(TWwiseFuture<int>&& Future, FEventRef& Event, int Num)
{
	LaunchWwiseTask(WWISE_TEST_ASYNC_NAME, [Future = MoveTemp(Future), &Event, Num]() mutable
	{
		Future.Get();

		if (Num <= 0)
		{
			Event->Trigger();
			return;
		}
		
		Future.Then([&Event, Num](TWwiseFuture<int> Future) mutable
		{
			Multithreaded_Then_Future_Op(MoveTemp(Future), Event, Num - 1);
		});
	});
};


WWISE_TEST_CASE(Concurrency_Future_Stress, "Wwise::Concurrency::Future_Stress", "[ApplicationContextMask][StressFilter]")
{
	SECTION("Multithreaded Then Future")
	{
		static constexpr const auto Count = 50;

		FEventRef Events[Count];
		
		for (int Num = 0; Num < Count; ++Num)
		{
			TWwisePromise<int> Promise;
			auto Future( Promise.GetFuture() );
			auto& Event(Events[Num]);

			if ( Num & 1 )
			{
				Promise.EmplaceValue(1);
			}

			Multithreaded_Then_Future_Op(MoveTemp(Future), Event, Num);
			
			if (!( Num & 1 ))
			{
				CHECK(! Events[Num]->Wait(1) );
				Promise.EmplaceValue(1);
			}
		}

		for (int Num = 0; Num < Count; ++Num)
		{
			CHECK(Events[Num]->Wait(100));
		}
	}

	SECTION("Multithreaded Promise Wait")
	{
		static constexpr const auto Count = 2000;

		TWwiseFuture<int> Futures[Count];
		FEventRef Events[Count];
		
		for (int Num = 0; Num < Count; ++Num)
		{
			TWwisePromise<int> Promise;
			Futures[Num] = Promise.GetFuture();

			LaunchWwiseTask(WWISE_TEST_ASYNC_NAME, [Future = &Futures[Num], Event = &Events[Num]]() mutable
			{
				CHECK(Future->WaitFor(FTimespan::FromMilliseconds(100)));
				if (Future->IsReady())
				{
					CHECK(Future->Get());
				}
				(*Event)->Trigger();
			});			

			LaunchWwiseTask(WWISE_TEST_ASYNC_NAME, [Promise = MoveTemp(Promise)]() mutable
			{
				Promise.EmplaceValue(1);
			});
		}

		for (int Num = 0; Num < Count; ++Num)
		{
			CHECK(Events[Num]->Wait(FTimespan::FromMilliseconds(200)));
		}
	}
}

#endif // WWISE_UNIT_TESTS