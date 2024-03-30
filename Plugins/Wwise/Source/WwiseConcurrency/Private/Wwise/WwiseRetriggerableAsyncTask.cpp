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

#include "Wwise/WwiseRetriggerableAsyncTask.h"
#include "Async/TaskGraphInterfaces.h"
#include "Wwise/WwiseConcurrencyModule.h"
#include "Wwise/Stats/Concurrency.h"
#include "Async/Async.h"

#include <inttypes.h>

static constexpr const auto DEBUG_WWISERETRIGGERABLEASYNCTASK = false;


FWwiseRetriggerableAsyncTask::FWwiseRetriggerableAsyncTask(ENamedThreads::Type DesiredThread, FFunction&& InFunction)
	: NamedThread(DesiredThread)
	, Task(MoveTemp(InFunction))
{}

FWwiseRetriggerableAsyncTask::~FWwiseRetriggerableAsyncTask()
{
	UE_CLOG(DEBUG_WWISERETRIGGERABLEASYNCTASK, LogWwiseConcurrency, VeryVerbose, TEXT("Destroying FWwiseRetriggerableAsyncTask"));
}

void FWwiseRetriggerableAsyncTask::ScheduleTask()
{
	UE_CLOG(DEBUG_WWISERETRIGGERABLEASYNCTASK, LogWwiseConcurrency, Verbose, TEXT("FWwiseRetriggerableAsyncTask: Scheduling task. Ticks = %llu"), FDateTime::Now().GetTicks());
	AsyncTask(NamedThread, [this]() mutable
	{
		UE_CLOG(DEBUG_WWISERETRIGGERABLEASYNCTASK, LogWwiseConcurrency, Verbose, TEXT("FWwiseRetriggerableAsyncTask: running task. Ticks = %llu"), FDateTime::Now().GetTicks());
		if (Task() == EWwiseDeferredAsyncResult::KeepRunning)
		{
			UE_CLOG(DEBUG_WWISERETRIGGERABLEASYNCTASK, LogWwiseConcurrency, Verbose, TEXT("FWwiseRetriggerableAsyncTask is not done, rescheduling"));
			ScheduleTask();
		}
		else
		{
			UE_CLOG(DEBUG_WWISERETRIGGERABLEASYNCTASK, LogWwiseConcurrency, Verbose, TEXT("FWwiseRetriggerableAsyncTask finished successfully!"));
			delete this;
		}
	});
}

