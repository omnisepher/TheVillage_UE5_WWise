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

#include "Wwise/WwiseUnitTests.h"

#if WWISE_UNIT_TESTS
#include "Wwise/WwiseExecutionQueue.h"
#include <atomic>

WWISE_TEST_CASE(Concurrency_ExecutionQueue_Smoke, "Wwise::Concurrency::ExecutionQueue_Smoke", "[ApplicationContextMask][SmokeFilter]")
{
	SECTION("Static")
	{
		static_assert(std::is_constructible<FWwiseExecutionQueue, const TCHAR*>::value, "Can create a named Execution Queue");
		static_assert(std::is_constructible<FWwiseExecutionQueue, const TCHAR*, EWwiseTaskPriority>::value, "Can create a named Execution Queue with its own task priority");
		static_assert(!std::is_copy_constructible<FWwiseExecutionQueue>::value, "Cannot copy an Execution Queue");
		static_assert(!std::is_move_constructible<FWwiseExecutionQueue>::value, "Cannot move-construct an Execution Queue");
		static_assert(!std::is_copy_assignable<FWwiseExecutionQueue>::value, "Cannot assign an Execution Queue");
		static_assert(!std::is_move_assignable<FWwiseExecutionQueue>::value, "Cannot move-assign an Execution Queue");
	}

	SECTION("Instantiation")
	{
		FWwiseExecutionQueue NamedTask(WWISE_EQ_NAME("NamedTask Test"));
		FWwiseExecutionQueue PriorityTask(WWISE_EQ_NAME("PriorityTask Test"), EWwiseTaskPriority::Normal);
	}

	SECTION("Async At Destructor")
	{
		constexpr const int LoopCount = 10;
		std::atomic<int> Value{ 0 };
		{
			FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);
			for (int i = 0; i < LoopCount; ++i)
			{
				ExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value]
				{
					++Value;
				});
			}
		}
		CHECK(Value.load() == LoopCount);
	}

	SECTION("AsyncWait")
	{
		constexpr const int LoopCount = 10;
		std::atomic<int> Value{ 0 };
		{
			const auto CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
			FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);
			for (int i = 0; i < LoopCount; ++i)
			{
				ExecutionQueue.AsyncWait(WWISE_TEST_ASYNC_NAME, [&Value, CurrentThreadId]
				{
					CHECK_FALSE(CurrentThreadId == FPlatformTLS::GetCurrentThreadId());
					++Value;
				});
			}
			CHECK(Value.load() == LoopCount);
		}
	}

	SECTION("Async in order")
	{
		constexpr const int LoopCount = 10;
		std::atomic<int> Value{ 0 };
		{
			FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);
			for (int i = 0; i < LoopCount; ++i)
			{
				ExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value, ShouldBe = i]
				{
					CHECK(Value++ == ShouldBe);
				});
			}
		}
		CHECK(Value.load() == LoopCount);
	}
	
	SECTION("IsRunningInThisThread")
	{
		const auto CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
		FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);
		CHECK_FALSE(ExecutionQueue.IsRunningInThisThread());
		ExecutionQueue.AsyncWait(WWISE_TEST_ASYNC_NAME, [&ExecutionQueue, CurrentThreadId]
		{
			CHECK_FALSE(CurrentThreadId == FPlatformTLS::GetCurrentThreadId());
			CHECK(ExecutionQueue.IsRunningInThisThread());
		});
		CHECK_FALSE(ExecutionQueue.IsRunningInThisThread());
	}
}

WWISE_TEST_CASE(Concurrency_ExecutionQueue_Perf, "Wwise::Concurrency::ExecutionQueue_Perf", "[ApplicationContextMask][PerfFilter]")
{
	SECTION("AsyncAddingOpPerf")
	{
		const bool bReduceLogVerbosity = FWwiseExecutionQueue::Test::bReduceLogVerbosity;
		FWwiseExecutionQueue::Test::bReduceLogVerbosity = true;
		ON_SCOPE_EXIT { FWwiseExecutionQueue::Test::bReduceLogVerbosity = bReduceLogVerbosity; };

		constexpr const int LoopCount = 500000;
		constexpr const int ExpectedUS = 600000;
		std::atomic<int> Value{ 0 };

		{
			FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);

			FDateTime StartTime = FDateTime::UtcNow();
			for (int i = 0; i < LoopCount; ++i)
			{
				ExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value]
				{
					++Value;
				});
			}
			FTimespan Duration = FDateTime::UtcNow() - StartTime;
			WWISE_TEST_LOG("AsyncAddingOpPerf %dus < %dus", (int)Duration.GetTotalMicroseconds(), ExpectedUS);
			CHECK(Duration.GetTotalMicroseconds() < ExpectedUS);
		}
		CHECK(Value.load() == LoopCount);
	}

	SECTION("AsyncExecutionPerf")
	{
		const bool bReduceLogVerbosity = FWwiseExecutionQueue::Test::bReduceLogVerbosity;
		FWwiseExecutionQueue::Test::bReduceLogVerbosity = true;
		ON_SCOPE_EXIT { FWwiseExecutionQueue::Test::bReduceLogVerbosity = bReduceLogVerbosity; };
		
		constexpr const int LoopCount = 250000;
		constexpr const int ExpectedUS = 400000;
		std::atomic<int> Value{ 0 };

		FDateTime StartTime;
		{
			FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);

			ExecutionQueue.AsyncWait(WWISE_TEST_ASYNC_NAME, [&ExecutionQueue, LoopCount, &Value]
			{
				for (int i = 0; i < LoopCount; ++i)
				{
					ExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value]
					{
						++Value;
					});
				}
			});
			StartTime = FDateTime::UtcNow();
		}
		FTimespan Duration = FDateTime::UtcNow() - StartTime;
		WWISE_TEST_LOG("AsyncExecutionPerf %dus < %dus", (int)Duration.GetTotalMicroseconds(), ExpectedUS);
		CHECK(Value.load() == LoopCount);
		CHECK(Duration.GetTotalMicroseconds() < ExpectedUS);
	}
}

WWISE_TEST_CASE(Concurrency_ExecutionQueue, "Wwise::Concurrency::ExecutionQueue", "[ApplicationContextMask][ProductFilter]")
{

	SECTION("Close")
	{
		std::atomic<int> Value{ 0 };
		std::atomic<int> OpenedQueues{ 0 };
		constexpr const int RepeatLoop = 2;
		constexpr const int MainLoopCount = 2;
		constexpr const int SubLoopCount = 2;
		constexpr const int FinalLoopCount = 2;
		constexpr const int LoopCount = RepeatLoop * MainLoopCount * SubLoopCount * FinalLoopCount;

		for (int Repeat = 0; Repeat < RepeatLoop; ++Repeat)
		{
			FWwiseExecutionQueue MainExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue* SubExecutionQueue = new FWwiseExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue DeletionExecutionQueue(WWISE_TEST_ASYNC_NAME);

			for (int i = 0; i < MainLoopCount; ++i)
			{
				MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value, &OpenedQueues, SubExecutionQueue, &DeletionExecutionQueue, SubLoopCount, FinalLoopCount]
				{
					for (int i = 0; i < SubLoopCount; ++i)
					{
						SubExecutionQueue->Async(WWISE_TEST_ASYNC_NAME, [&Value, &OpenedQueues, &DeletionExecutionQueue, FinalLoopCount]
						{
							++OpenedQueues;
							auto ExecutionQueue = new FWwiseExecutionQueue(WWISE_TEST_ASYNC_NAME);
							for (int i = 0; i < FinalLoopCount; ++i)
							{
								ExecutionQueue->Async(WWISE_TEST_ASYNC_NAME, [&Value]
								{
									++Value;
								});
							}
							DeletionExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&OpenedQueues, ExecutionQueue]
							{
								delete ExecutionQueue;
								--OpenedQueues;
							});
						});
					}
				});
			}
			
			MainExecutionQueue.Close();
			SubExecutionQueue->AsyncWait(WWISE_TEST_ASYNC_NAME, []{});
			SubExecutionQueue->CloseAndDelete();
		}
		CHECK(Value.load() == LoopCount);
		CHECK(OpenedQueues.load() == 0);
	}

	SECTION("Sleep on State Update")
	{
		const bool bMockSleepOnStateUpdate = FWwiseExecutionQueue::Test::bMockSleepOnStateUpdate;
		const bool bMockEngineDeletion = FWwiseExecutionQueue::Test::bMockEngineDeletion;
		FWwiseExecutionQueue::Test::bMockSleepOnStateUpdate = true;
		ON_SCOPE_EXIT
		{
			FWwiseExecutionQueue::Test::bMockSleepOnStateUpdate = bMockSleepOnStateUpdate;
			FWwiseExecutionQueue::Test::bMockEngineDeletion = bMockEngineDeletion;
		};
		
		std::atomic<int> Value{ 0 };
		constexpr const int RepeatLoop = 2;
		constexpr const int MainLoopCount = 2;
		constexpr const int SubLoopCount = 2;
		constexpr const int FinalLoopCount = 2;
		constexpr const int LoopCount = RepeatLoop * MainLoopCount * SubLoopCount * FinalLoopCount;

		for (int Repeat = 0; Repeat < RepeatLoop; ++Repeat)
		{
			FWwiseExecutionQueue MainExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue SubExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue DeletionExecutionQueue(WWISE_TEST_ASYNC_NAME);

			for (int i = 0; i < MainLoopCount; ++i)
			{
				if (i == MainLoopCount - 1)
				{
					MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, []
					{
						FWwiseExecutionQueue::Test::bMockEngineDeletion = true;
					});
				}
				
				MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value, &SubExecutionQueue, &DeletionExecutionQueue, SubLoopCount, FinalLoopCount]
				{
					for (int i = 0; i < SubLoopCount; ++i)
					{
						SubExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value, &DeletionExecutionQueue, FinalLoopCount]
						{
							auto ExecutionQueue = new FWwiseExecutionQueue(WWISE_TEST_ASYNC_NAME);
							for (int i = 0; i < FinalLoopCount; ++i)
							{
								ExecutionQueue->Async(WWISE_TEST_ASYNC_NAME, [&Value]
								{
									++Value;
								});
							}
							DeletionExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [ExecutionQueue]
							{
								delete ExecutionQueue;
							});
						});
					}
				});
			}
			
			MainExecutionQueue.Close();
			SubExecutionQueue.Close();
			DeletionExecutionQueue.Close();
			FWwiseExecutionQueue::Test::bMockEngineDeletion = false;
		}
		CHECK(Value.load() == LoopCount);
	}
	
	SECTION("Async at exit")
	{
		const bool bMockEngineDeletion = FWwiseExecutionQueue::Test::bMockEngineDeletion;
		FWwiseExecutionQueue::Test::bMockEngineDeletion = true;
		ON_SCOPE_EXIT { FWwiseExecutionQueue::Test::bMockEngineDeletion = bMockEngineDeletion; };
		
		constexpr const int LoopCount = 10;
		std::atomic<int> Value{ 0 };
		{
			FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);
			for (int i = 0; i < LoopCount; ++i)
			{
				ExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value, ShouldBe = i]
				{
					CHECK(Value++ == ShouldBe);
				});
			}
		}
		CHECK(Value.load() == LoopCount);
	}

	SECTION("Async after exit")
	{
		const bool bMockEngineDeleted = FWwiseExecutionQueue::Test::bMockEngineDeleted;
		FWwiseExecutionQueue::Test::bMockEngineDeleted = true;
		ON_SCOPE_EXIT { FWwiseExecutionQueue::Test::bMockEngineDeleted = bMockEngineDeleted; };
		
		constexpr const int LoopCount = 10;
		std::atomic<int> Value{ 0 };
		{
			const auto CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
			FWwiseExecutionQueue::Test::bMockEngineDeleted = true;
			FWwiseExecutionQueue ExecutionQueue(WWISE_TEST_ASYNC_NAME);
			for (int i = 0; i < LoopCount; ++i)
			{
				ExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&Value, ShouldBe = i, CurrentThreadId]
				{
					CHECK(CurrentThreadId == FPlatformTLS::GetCurrentThreadId());
					CHECK(Value++ == ShouldBe);
				});
			}
		}
		CHECK(Value.load() == LoopCount);
	}
}

WWISE_TEST_CASE(Concurrency_ExecutionQueue_Stress, "Wwise::Concurrency::ExecutionQueue_Stress", "[ApplicationContextMask][StressFilter]")
{
	SECTION("AsyncStress")
	{
		const bool bReduceLogVerbosity = FWwiseExecutionQueue::Test::bReduceLogVerbosity;
		FWwiseExecutionQueue::Test::bReduceLogVerbosity = !FWwiseExecutionQueue::Test::bExtremelyVerbose;
		ON_SCOPE_EXIT { FWwiseExecutionQueue::Test::bReduceLogVerbosity = bReduceLogVerbosity; };
		
		constexpr const int LoopCount = 2000000;
		constexpr const int MainLoopCount = 100;
		constexpr const int SubLoopCount = 100;
		constexpr const int FinalLoopCount = LoopCount / MainLoopCount / SubLoopCount;

		{
			FWwiseExecutionQueue MainExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue SubExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue DeletionExecutionQueue(WWISE_TEST_ASYNC_NAME);

			for (int i = 0; i < MainLoopCount; ++i)
			{
				MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&SubExecutionQueue, &DeletionExecutionQueue, SubLoopCount, FinalLoopCount]
				{
					for (int i = 0; i < SubLoopCount; ++i)
					{
						SubExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&DeletionExecutionQueue, FinalLoopCount]
						{
							auto ExecutionQueue = new FWwiseExecutionQueue(WWISE_TEST_ASYNC_NAME);
							for (int i = 0; i < FinalLoopCount; ++i)
							{
								ExecutionQueue->Async(WWISE_TEST_ASYNC_NAME, []
								{
								});
							}
							DeletionExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [ExecutionQueue]
							{
								delete ExecutionQueue;
							});
						});
					}
				});
			}
			
			MainExecutionQueue.Close();
			SubExecutionQueue.Close();
		}
	}

	SECTION("AsyncStress with Sleep")
	{
		const bool bReduceLogVerbosity = FWwiseExecutionQueue::Test::bReduceLogVerbosity;
		const bool bMockSleepOnStateUpdate = FWwiseExecutionQueue::Test::bMockSleepOnStateUpdate;
		FWwiseExecutionQueue::Test::bReduceLogVerbosity = !FWwiseExecutionQueue::Test::bExtremelyVerbose;
		FWwiseExecutionQueue::Test::bMockSleepOnStateUpdate = true;
		ON_SCOPE_EXIT
		{
			FWwiseExecutionQueue::Test::bReduceLogVerbosity = bReduceLogVerbosity;
			FWwiseExecutionQueue::Test::bMockSleepOnStateUpdate = bMockSleepOnStateUpdate;
		};
		
		constexpr const int LoopCount = 1000;
		constexpr const int MainLoopCount = 10;
		constexpr const int SubLoopCount = 10;
		constexpr const int FinalLoopCount = LoopCount / MainLoopCount / SubLoopCount;

		{
			FWwiseExecutionQueue MainExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue SubExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue DeletionExecutionQueue(WWISE_TEST_ASYNC_NAME);

			for (int i = 0; i < MainLoopCount; ++i)
			{
				MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&SubExecutionQueue, &DeletionExecutionQueue, SubLoopCount, FinalLoopCount]
				{
					for (int i = 0; i < SubLoopCount; ++i)
					{
						SubExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&DeletionExecutionQueue, FinalLoopCount]
						{
							auto ExecutionQueue = new FWwiseExecutionQueue(WWISE_TEST_ASYNC_NAME);
							for (int i = 0; i < FinalLoopCount; ++i)
							{
								ExecutionQueue->Async(WWISE_TEST_ASYNC_NAME, []
								{
								});
							}
							DeletionExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [ExecutionQueue]
							{
								delete ExecutionQueue;
							});
						});
					}
				});
			}
			
			MainExecutionQueue.Close();
			SubExecutionQueue.Close();
		}
	}

	SECTION("AsyncStress at Exit")
	{
		const bool bReduceLogVerbosity = FWwiseExecutionQueue::Test::bReduceLogVerbosity;
		const bool bMockEngineDeletion = FWwiseExecutionQueue::Test::bMockEngineDeletion;
		const bool bMockEngineDeleted = FWwiseExecutionQueue::Test::bMockEngineDeleted;
		FWwiseExecutionQueue::Test::bReduceLogVerbosity = !FWwiseExecutionQueue::Test::bExtremelyVerbose;
		ON_SCOPE_EXIT
		{
			FWwiseExecutionQueue::Test::bReduceLogVerbosity = bReduceLogVerbosity;
			FWwiseExecutionQueue::Test::bMockEngineDeletion = bMockEngineDeletion;
			FWwiseExecutionQueue::Test::bMockEngineDeleted = bMockEngineDeleted;
		};

		
		constexpr const int LoopCount = 100000;
		constexpr const int MainLoopCount = 100;
		constexpr const int SubLoopCount = 100;
		constexpr const int FinalLoopCount = LoopCount / MainLoopCount / SubLoopCount;

		{
			FWwiseExecutionQueue MainExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue SubExecutionQueue(WWISE_TEST_ASYNC_NAME);
			FWwiseExecutionQueue DeletionExecutionQueue(WWISE_TEST_ASYNC_NAME);

			for (int i = 0; i < MainLoopCount; ++i)
			{
				MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&SubExecutionQueue, &DeletionExecutionQueue, SubLoopCount, FinalLoopCount]
				{
					for (int i = 0; i < SubLoopCount; ++i)
					{
						SubExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [&DeletionExecutionQueue, FinalLoopCount]
						{
							auto ExecutionQueue = new FWwiseExecutionQueue(WWISE_TEST_ASYNC_NAME);
							for (int i = 0; i < FinalLoopCount; ++i)
							{
								ExecutionQueue->Async(WWISE_TEST_ASYNC_NAME, []
								{
								});
							}
							DeletionExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, [ExecutionQueue]
							{
								delete ExecutionQueue;
							});
						});
					}
				});

				if (i == MainLoopCount / 3)
				{
					MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, []
					{
						FWwiseExecutionQueue::Test::bMockEngineDeletion = true;
					});
				}
			}
			MainExecutionQueue.Async(WWISE_TEST_ASYNC_NAME, []
			{
				FWwiseExecutionQueue::Test::bMockEngineDeleted = true;
			});
		
			MainExecutionQueue.Close();
			SubExecutionQueue.Close();
		}
	}
}

#endif // WWISE_UNIT_TESTS