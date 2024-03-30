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

#include "Wwise/WwiseProcessingModuleImpl.h"

#include "Wwise/WwiseGlobalCallbacks.h"
#include "Wwise/Stats/Processing.h"

IMPLEMENT_MODULE(FWwiseProcessingModule, WwiseProcessing)

FWwiseProcessingModule::FWwiseProcessingModule()
{
}

void FWwiseProcessingModule::StartupModule()
{
	UE_LOG(LogWwiseProcessing, Display, TEXT("Initializing default Processing."));

	GlobalCallbacksLock.WriteLock();
	if (!GlobalCallbacks)
	{
		InitializeGlobalCallbacks();
	}
	GlobalCallbacksLock.WriteUnlock();

	IWwiseProcessingModule::StartupModule();
}

void FWwiseProcessingModule::ShutdownModule()
{
	UE_LOG(LogWwiseProcessing, Display, TEXT("Shutting down default Processing."));
	GlobalCallbacksLock.WriteLock();
	TerminateGlobalCallbacks();
	GlobalCallbacksLock.WriteUnlock();

	IWwiseProcessingModule::ShutdownModule();
}

FWwiseGlobalCallbacks* FWwiseProcessingModule::GetGlobalCallbacks()
{
	GlobalCallbacksLock.ReadLock();
	if (LIKELY(GlobalCallbacks))
	{
		GlobalCallbacksLock.ReadUnlock();
		return GlobalCallbacks;
	}

	GlobalCallbacksLock.ReadUnlock();
	GlobalCallbacksLock.WriteLock();
	if (UNLIKELY(GlobalCallbacks))
	{
		GlobalCallbacksLock.WriteUnlock();
		return GlobalCallbacks;
	}

	InitializeGlobalCallbacks();
	GlobalCallbacksLock.WriteUnlock();
	return GlobalCallbacks;
}

void FWwiseProcessingModule::InitializeGlobalCallbacks()
{
	GlobalCallbacks = new FWwiseGlobalCallbacks;
}

void FWwiseProcessingModule::TerminateGlobalCallbacks()
{
	if (GlobalCallbacks)
	{
		GlobalCallbacks->Terminate();
		delete GlobalCallbacks;
		GlobalCallbacks = nullptr;
	}
}

