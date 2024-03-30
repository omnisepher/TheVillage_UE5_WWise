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

#include "Wwise/WwiseDeferredQueue.h"

#include "WwiseUnrealHelper.h"


struct WWISECONCURRENCY_API FWwiseRetriggerableAsyncTask
{
	using FFunction = TUniqueFunction<EWwiseDeferredAsyncResult()>;
	DECLARE_MULTICAST_DELEGATE(FGameThreadDelegate);

	FWwiseRetriggerableAsyncTask(ENamedThreads::Type DesiredThread, FFunction&& InFunction);
	~FWwiseRetriggerableAsyncTask();

	/**
	 * @brief Send the task to the task graph. It will be rescheduled on the given thread until the function returns EWwiseDeferredAsyncResult::Done.
	 */
	void ScheduleTask();

private:

	ENamedThreads::Type NamedThread;
	FFunction Task;

};
