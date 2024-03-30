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
#include "HAL/ThreadManager.h"
#include "Stats/Stats.h"

// Unreal Stat system assumes it runs on an Unreal thread. A lot of low-level I/O doesn't follow that assumption.

#if STATS

FORCEINLINE static void ASYNC_LAMBDA_STAT(TUniqueFunction<void()>&& Lambda)
{
	DECLARE_CYCLE_STAT(TEXT("Wwise Async Stat"), STAT_WwiseAsyncStat, STATGROUP_StatSystem);

	if (FThreadManager::GetThreadName(FPlatformTLS::GetCurrentThreadId()).IsEmpty())
	{
		FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(Lambda), GET_STATID(STAT_WwiseAsyncStat));
	}
	else
	{
		Lambda();
	}
}
#define ASYNC_OP_STAT(...) ASYNC_LAMBDA_STAT([]__VA_ARGS__)

class FWwiseAsyncCycleCounter
{
public:
	FName StatName;
	FStatMessage StartStatMessage;

	FORCEINLINE_STATS static FName GetName(TStatId InStatId)
	{
		FMinimalName StatMinimalName = InStatId.GetMinimalName(EMemoryOrder::Relaxed);
		if (UNLIKELY(StatMinimalName.IsNone()))
		{
			return {};
		}
		return MinimalNameToName(StatMinimalName);
	}

	FORCEINLINE_STATS static FStatMessage GetStartMessage(FName InStatName)
	{
		if (UNLIKELY(InStatName.IsNone()))
		{
			return {};
		}
		return FStatMessage(InStatName, EStatOperation::CycleScopeStart);
	}

	void Stop()
	{
		if (StatName.IsNone())
		{
			return;
		}

		FName Name;
		Swap(Name, StatName);

		FStatMessage EndStatMessage(Name, EStatOperation::CycleScopeEnd);
		ASYNC_LAMBDA_STAT([
			Name = MoveTemp(Name),
			StartStatMessage = MoveTemp(StartStatMessage),
			EndStatMessage = MoveTemp(EndStatMessage)]
		{
			FThreadStats* Stats = FThreadStats::GetThreadStats();
			Stats->AddStatMessage(StartStatMessage);
			Stats->AddStatMessage(EndStatMessage);
			Stats->Flush();
		});
	}

	FWwiseAsyncCycleCounter(TStatId InStatId) :
		StatName(GetName(InStatId)),
		StartStatMessage(GetStartMessage(StatName))
	{
	}

	~FWwiseAsyncCycleCounter()
	{
		Stop();
	}
};
#else
#define ASYNC_LAMBDA_STAT(...) (void)0
#define ASYNC_OP_STAT(...) (void)0


class FWwiseAsyncCycleCounter
{
public:
	void Stop()
	{
	}

	FWwiseAsyncCycleCounter(TStatId InStatId)
	{
	}

	~FWwiseAsyncCycleCounter()
	{
	}
};
#endif

#define ASYNC_INC_DWORD_STAT(x) ASYNC_OP_STAT({ INC_DWORD_STAT(x); })
#define ASYNC_DEC_DWORD_STAT(x) ASYNC_OP_STAT({ DEC_DWORD_STAT(x); })
#define ASYNC_INC_FLOAT_STAT_BY(x, y) ASYNC_LAMBDA_STAT([Value = y]{ INC_FLOAT_STAT_BY(x, Value); })
#define ASYNC_INC_MEMORY_STAT_BY(x, y) ASYNC_LAMBDA_STAT([Value = y]{ INC_MEMORY_STAT_BY(x, Value); })
#define ASYNC_INC_MEMORY_STAT_BY_FName(x, y) if (x == NAME_None) {} else ASYNC_LAMBDA_STAT([Name = x, Value = y]{ INC_MEMORY_STAT_BY_FName(Name, Value); })
#define ASYNC_DEC_MEMORY_STAT_BY(x, y) ASYNC_LAMBDA_STAT([Value = y]{ DEC_MEMORY_STAT_BY(x, Value); })
#define ASYNC_DEC_MEMORY_STAT_BY_FName(x, y) if (x == NAME_None) {} else ASYNC_LAMBDA_STAT([Name = x, Value = y]{ DEC_MEMORY_STAT_BY_FName(Name, Value); })