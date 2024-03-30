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

#include "Stats/Stats.h"
#include "Wwise/Stats/NamedEvents.h"

DECLARE_STATS_GROUP(TEXT("Concurrency"), STATGROUP_WwiseConcurrency, STATCAT_Wwise);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Waiting for completion in Game Thread"), STAT_WwiseConcurrencyGameThreadWait, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Waiting for completion"), STAT_WwiseConcurrencyWait, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Executing asynchronous op"), STAT_WwiseConcurrencyAsync, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Executing blocking synchronous op"), STAT_WwiseConcurrencySync, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Executing op in Game Thread"), STAT_WwiseConcurrencyGameThread, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Number of ExecutionQueues"), STAT_WwiseExecutionQueues, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("ExecutionQueue Async calls"), STAT_WwiseExecutionQueueAsyncCalls, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("ExecutionQueue AsyncWait calls"), STAT_WwiseExecutionQueueAsyncWaitCalls, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Number of Futures"), STAT_WwiseFutures, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Number of Futures with Events"), STAT_WwiseFuturesWithEvent, STATGROUP_WwiseConcurrency, WWISECONCURRENCY_API);

WWISECONCURRENCY_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseConcurrency, Log, All);

#define SCOPED_WWISECONCURRENCY_EVENT(Text) SCOPED_WWISE_NAMED_EVENT(TEXT("WwiseConcurrency"), Text)
#define SCOPED_WWISECONCURRENCY_EVENT_4(Text) SCOPED_WWISE_NAMED_EVENT_4(TEXT("WwiseConcurrency"), Text)
#define SCOPED_WWISECONCURRENCY_EVENT_F(Format, ...) SCOPED_WWISE_NAMED_EVENT_F(TEXT("WwiseConcurrency"), Format, __VA_ARGS__)
#define SCOPED_WWISECONCURRENCY_EVENT_F_4(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_4(TEXT("WwiseConcurrency"), Format, __VA_ARGS__)

#define WWISECONCURRENCY_ASYNC_NAME(Text) TEXT("WwiseConcurrency ") TEXT(Text)
