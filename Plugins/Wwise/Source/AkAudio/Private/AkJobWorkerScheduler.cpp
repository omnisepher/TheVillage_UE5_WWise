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

#include "AkJobWorkerScheduler.h"
#include "AkAudioDevice.h"
#include "Wwise/WwiseTask.h"
#include "Wwise/API/WwiseMemoryMgrAPI.h"

#define AK_DECLARE_JOB_TYPE(__job__, __desc__, __thread__) \
	DECLARE_CYCLE_STAT(TEXT(__desc__), STAT_AkJob##__job__, STATGROUP_Audio); \
	namespace AkJobWorkerSchedulerInternals \
	{ \
		static const TCHAR* const Name##__job__ = TEXT(__desc__); \
		static EWwiseTaskPriority Task##__job__ = __thread__; \
	}

#define AK_DEFINE_JOB_CASE(__job__) \
	case AkJobType_##__job__: \
		Name = AkJobWorkerSchedulerInternals::Name##__job__; \
		StatId = GET_STATID(STAT_AkJob##__job__); \
		TaskPriority = AkJobWorkerSchedulerInternals::Task##__job__

static_assert(AK_NUM_JOB_TYPES == 3, "Update the stat groups and switch cases below for new job types!");
AK_DECLARE_JOB_TYPE(Generic, "Wwise Generic Job", EWwiseTaskPriority::High)
AK_DECLARE_JOB_TYPE(AudioProcessing, "Wwise Audio Processing Job", EWwiseTaskPriority::High)
AK_DECLARE_JOB_TYPE(SpatialAudio, "Wwise Spatial Audio Job", EWwiseTaskPriority::High)

static void OnJobWorkerRequest(AkJobWorkerFunc in_fnJobWorker, AkJobType in_jobType, AkUInt32 in_uNumWorkers, void* in_pUserData)
{
	FAkJobWorkerScheduler* Scheduler = static_cast<FAkJobWorkerScheduler*>(in_pUserData);
	AkUInt32 MaxExecutionTime = Scheduler->uMaxExecutionTime;
	const TCHAR* Name;
	TStatId StatId;
	EWwiseTaskPriority TaskPriority;
	switch (in_jobType)
	{
		AK_DEFINE_JOB_CASE(AudioProcessing); break;
		AK_DEFINE_JOB_CASE(SpatialAudio); break;
	default:
		check(!"Unknown job type.");
		// Fall-through
		AK_DEFINE_JOB_CASE(Generic);
	}
	for (int i=0; i < (int)in_uNumWorkers; i++)
	{ 
		LaunchWwiseTask(Name, TaskPriority, [=]() {
#if STATS
#if UE_5_0_OR_LATER
			FScopeCycleCounter CycleCount(StatId, FStat_STAT_AkJobGeneric::GetFlags());
#else
			FScopeCycleCounter CycleCount(StatId);
#endif
#endif
			in_fnJobWorker(in_jobType, MaxExecutionTime); 
			// After completion of the worker function, release any thread-local memory resources
			if (auto* MemoryManager = IWwiseMemoryMgrAPI::Get())
			{
				MemoryManager->TrimForThread();
			}
		}); 
	} 
}

#undef AK_DECLARE_JOB_TYPE
#undef AK_DEFINE_JOB_TYPE


void FAkJobWorkerScheduler::InstallJobWorkerScheduler(uint32 in_uMaxExecutionTime, uint32 in_uMaxWorkerCount, AkJobMgrSettings & out_settings)
{
	if (!FTaskGraphInterface::Get().IsRunning())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Multi-core rendering was requested, but Task Graph is not running. Multi-core rendering disabled."));
	}
	else if (!FPlatformProcess::SupportsMultithreading())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Multi-core rendering was requested, platform does not support multi-threading. Multi-core rendering disabled."));
	}
	else
	{
		uMaxExecutionTime = in_uMaxExecutionTime;
		AkUInt32 uNumWorkerThreads = FTaskGraphInterface::Get().GetNumWorkerThreads();
		AkUInt32 uMaxActiveWorkers = FMath::Min(uNumWorkerThreads, in_uMaxWorkerCount);
		if (uMaxActiveWorkers > 0)
		{
			out_settings.fnRequestJobWorker = OnJobWorkerRequest;
			out_settings.pClientData = this;
			for (int i = 0; i < AK_NUM_JOB_TYPES; i++)
			{
				out_settings.uMaxActiveWorkers[i] = uMaxActiveWorkers;
			}
		}
		else
		{
			UE_LOG(LogAkAudio, Warning, TEXT("Multi-core rendering was requested, but Max Num Job Workers is set to 0. Multi-core rendering disabled."));
		}
	}
}
