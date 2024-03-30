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
#include "Wwise/Mock/WwiseMockFileState.h"
#include <atomic>

WWISE_TEST_CASE(FileHandler_FileState_Smoke, "Wwise::FileHandler::FileState_Smoke", "[ApplicationContextMask][SmokeFilter]")
{
	SECTION("Static")
	{
		static_assert(!std::is_constructible<FWwiseFileState>::value, "File State cannot be constructed through a default parameter");
		static_assert(!std::is_copy_constructible<FWwiseFileState>::value, "Cannot copy a File State");
		static_assert(!std::is_copy_assignable<FWwiseFileState>::value, "Cannot assign to a File State");
		static_assert(!std::is_move_constructible<FWwiseFileState>::value, "Cannot move-construct a File State");
	}

	SECTION("Instantiation")
	{
		FWwiseMockFileState(0);
		FWwiseMockFileState(1);
		FWwiseMockFileState(2);
		FWwiseMockFileState(3);
	}

	SECTION("Loading Streaming File")
	{
		FEventRef Done;
		FWwiseMockFileState File(10);
		File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;

		bool bDeleted{ false };
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [&File, &Done, &bDeleted](bool bResult) mutable
		{
			CHECK(bResult);
			CHECK(File.State == FWwiseFileState::EState::Opened);
			File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
				[&File, &bDeleted](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
				{
					CHECK(File.State == FWwiseFileState::EState::Closed);
					bDeleted = true;
					Callback();
				},
				[&Done, &bDeleted]() mutable
				{
					CHECK(bDeleted);
					Done->Trigger();
				});
		});
		CHECK(Done->Wait(1000));
	}

	SECTION("Streaming File")
	{
		FEventRef Done;
		FWwiseMockFileState File(20);
		File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;
		
		bool bDeleted{ false };
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Streaming, [&File, &Done, &bDeleted](bool bResult) mutable
		{
			CHECK(bResult);
			CHECK(File.State == FWwiseFileState::EState::Loaded);
			File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Streaming,
				[&File, &bDeleted](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
				{
					CHECK(File.State == FWwiseFileState::EState::Closed);
					bDeleted = true;
					Callback();
				},
				[&Done, &bDeleted]() mutable
				{
					CHECK(bDeleted);
					Done->Trigger();
				});
		});
		CHECK(Done->Wait(1000));
	}

	SECTION("Delete in Decrement")
	{
		FEventRef Done;
		auto* File = new FWwiseMockFileState(30);

		bool bDeleted{ false };
		File->IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [File, &Done, &bDeleted](bool bResult) mutable
		{
			CHECK(bResult);
			CHECK(File->State == FWwiseFileState::EState::Loaded);
			File->DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
				[File, &bDeleted](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
				{
					CHECK(File->State == FWwiseFileState::EState::Closed);
					delete File;
					bDeleted = true;
					Callback();
				},
				[&Done, &bDeleted]() mutable
				{
					CHECK(bDeleted);
					Done->Trigger();
				});
		});
		CHECK(Done->Wait(1000));
	}

	SECTION("Ordered callbacks")
	{
		FEventRef Done;
		FWwiseMockFileState File(40);
		File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;

		int Order = 0;
		constexpr const int Count = 10;

		for (int NumOp = 0; NumOp < Count; ++NumOp)
		{
			File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [NumOp, &Order](bool bResult) mutable
			{
				CHECK(NumOp*4+0 == Order);
				Order++;
			});
			File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
				[](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
				{
					Callback();
				},
				[NumOp, &Order]() mutable
				{
					CHECK(NumOp*4+1 == Order);
					Order++;
				});
			File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [NumOp, &Order](bool bResult) mutable
			{
				CHECK(NumOp*4+2 == Order);
				Order++;
			});
			File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
				[&Done](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
				{
					Callback();
					Done->Trigger();
				},
				[NumOp, &Order]() mutable
				{
					CHECK(NumOp*4+3 == Order);
					Order++;
				});
		}
		CHECK(Done->Wait(10000));
	}
}

WWISE_TEST_CASE(FileHandler_FileState, "Wwise::FileHandler::FileState", "[ApplicationContextMask][ProductFilter]")
{
	SECTION("Reloading Streaming File")
	{
		FEventRef Done;
		FWwiseMockFileState File(1000);
		File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;

		bool bDeleted{ false };
		bool bInitialDecrementDone{ false };
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [&File, &bDeleted](bool bResult) mutable
		{
			CHECK(bResult);
			CHECK(File.State == FWwiseFileState::EState::Opened);
		});
		File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
			[](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
			{
				CHECK(false);
				Callback();
			},
			[&File, &bDeleted, &bInitialDecrementDone]() mutable
			{
				bInitialDecrementDone = true;
			});
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [&File, &bDeleted](bool bResult) mutable
		{
			CHECK(bResult);
		});
		File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
			[&File, &bDeleted](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
			{
				CHECK(File.State == FWwiseFileState::EState::Closed);
				bDeleted = true;
				Callback();
			},
			[&Done, &bDeleted]() mutable
			{
				CHECK(bDeleted);
				Done->Trigger();
			});
		CHECK(Done->Wait(1000));
		CHECK(bInitialDecrementDone);
	}

	SECTION("Restreaming File")
	{
		FEventRef Done;
		FWwiseMockFileState File(1010);
		File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;

		bool bDeleted{ false };
		bool bInitialDecrementDone{ false };
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Streaming, [&File, &bDeleted](bool bResult) mutable
		{
		});
		File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Streaming,
			[](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
			{
				// Delete State should never be called, since the last one should delete our object 
				CHECK(false);
				Callback();
			},
			[&File, &bDeleted, &bInitialDecrementDone]() mutable
			{
				bInitialDecrementDone = true;
			});
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Streaming, [&File, &bDeleted](bool bResult) mutable
		{
		});
		File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Streaming,
			[&File, &bDeleted](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
			{
				CHECK(File.State == FWwiseFileState::EState::Closed);
				bDeleted = true;
				Callback();
			},
			[&Done, &bDeleted]() mutable
			{
				CHECK(bDeleted);
				Done->Trigger();
			});
		CHECK(Done->Wait(1000));
		CHECK(bInitialDecrementDone);
	}

	SECTION("Deferring Unload")
	{
		FEventRef Done;
		FWwiseMockFileState File(10);
		File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;
		File.UnloadFromSoundEngineDeferCount = 1;

		bool bDeleted{ false };
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [&File, &Done, &bDeleted](bool bResult) mutable
		{
			CHECK(bResult);
			CHECK(File.State == FWwiseFileState::EState::Opened);
			File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
				[&File, &bDeleted](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
				{
					CHECK(File.State == FWwiseFileState::EState::Closed);
					bDeleted = true;
					Callback();
				},
				[&Done, &bDeleted]() mutable
				{
					CHECK(bDeleted);
					Done->Trigger();
				});
		});
		CHECK(Done->Wait(1000));
	}

	SECTION("Deferring Close")
	{
		FEventRef Done;
		FWwiseMockFileState File(10);
		File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;
		File.CloseFileDeferCount = 1;

		bool bDeleted{ false };
		File.IncrementCountAsync(EWwiseFileStateOperationOrigin::Loading, [&File, &Done, &bDeleted](bool bResult) mutable
		{
			CHECK(bResult);
			CHECK(File.State == FWwiseFileState::EState::Opened);
			File.DecrementCountAsync(EWwiseFileStateOperationOrigin::Loading,
				[&File, &bDeleted](FWwiseFileState::FDecrementCountCallback&& Callback) mutable
				{
					CHECK(File.State == FWwiseFileState::EState::Closed);
					bDeleted = true;
					Callback();
				},
				[&Done, &bDeleted]() mutable
				{
					CHECK(bDeleted);
					Done->Trigger();
				});
		});
		CHECK(Done->Wait(1000));
	}
}

/*
WWISE_TEST_CASE(FileHandler_FileState_Perf, "Wwise::FileHandler::FileState_Perf", "[ApplicationContextMask][PerfFilter]")
{
}
*/

WWISE_TEST_CASE(FileHandler_FileState_Stress, "Wwise::FileHandler::FileState_Stress", "[ApplicationContextMask][StressFilter]")
{
	SECTION("Stress Open and Streams")
	{
		constexpr const int StateCount = 10;
		constexpr const int LoadCount = 10;
		constexpr const int WiggleCount = 2;

		FEventRef Dones[StateCount];
		FWwiseMockFileState* Files[StateCount];
		for (int StateIter = 0; StateIter < StateCount; ++StateIter)
		{
			FEventRef& Done(Dones[StateIter]);
			Files[StateIter] = new FWwiseMockFileState(10000 + StateIter);
			FWwiseMockFileState& File = *Files[StateIter];
			File.bIsStreamedState = FWwiseMockFileState::OptionalBool::True;

			for (int LoadIter = 0; LoadIter < LoadCount; ++LoadIter)
			{
				const EWwiseFileStateOperationOrigin FirstOp = (StateIter&1)==0 ? EWwiseFileStateOperationOrigin::Loading : EWwiseFileStateOperationOrigin::Streaming;
				const EWwiseFileStateOperationOrigin SecondOp = (StateIter&1)==1 ? EWwiseFileStateOperationOrigin::Loading : EWwiseFileStateOperationOrigin::Streaming;
				FFunctionGraphTask::CreateAndDispatchWhenReady([Op = FirstOp, &Done, &File, WiggleCount]() mutable
				{
					for (int WiggleIter = 0; WiggleIter < WiggleCount; ++WiggleIter)
					{
						File.IncrementCountAsync(Op, [](bool){});
						File.DecrementCountAsync(Op, [](FWwiseFileState::FDecrementCountCallback&& InCallback){ InCallback(); }, []{});
					}
					File.IncrementCountAsync(Op, [Op, &Done, &File, WiggleCount](bool)
					{
						for (int WiggleIter = 0; WiggleIter < WiggleCount; ++WiggleIter)
						{
							File.DecrementCountAsync(Op, [](FWwiseFileState::FDecrementCountCallback&& InCallback){ InCallback(); }, []{});
							File.IncrementCountAsync(Op, [](bool){});
						}
						File.DecrementCountAsync(Op, [&Done](FWwiseFileState::FDecrementCountCallback&& InCallback)
						{
							Done->Trigger();
							InCallback();
						}, []{});
					});
				});
				FFunctionGraphTask::CreateAndDispatchWhenReady([Op = SecondOp, &Done, &File, WiggleCount]() mutable
				{
					for (int WiggleIter = 0; WiggleIter < WiggleCount; ++WiggleIter)
					{
						File.IncrementCountAsync(Op, [](bool){});
						File.DecrementCountAsync(Op, [](FWwiseFileState::FDecrementCountCallback&& InCallback){ InCallback(); }, []{});
					}
					File.IncrementCountAsync(Op, [Op, &Done, &File, WiggleCount](bool)
					{
						for (int WiggleIter = 0; WiggleIter < WiggleCount; ++WiggleIter)
						{
							File.DecrementCountAsync(Op, [](FWwiseFileState::FDecrementCountCallback&& InCallback){ InCallback(); }, []{});
							File.IncrementCountAsync(Op, [](bool){});
						}
						File.DecrementCountAsync(Op, [&Done](FWwiseFileState::FDecrementCountCallback&& InCallback)
						{
							Done->Trigger();
							InCallback();
						}, []{});
					});
				});
			}
		}

		for (int StateIter = 0; StateIter < StateCount; ++StateIter)
		{
			FEventRef& Done(Dones[StateIter]);
			FWwiseMockFileState* File = Files[StateIter];
		
			CHECK(Done->Wait(100000));
			delete File;
		}
	}	
}
#endif // WWISE_UNIT_TESTS