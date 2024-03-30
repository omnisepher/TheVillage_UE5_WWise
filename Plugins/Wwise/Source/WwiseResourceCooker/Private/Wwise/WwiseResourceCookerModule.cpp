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

#include "Wwise/WwiseResourceCookerModule.h"
#include "Wwise/WwiseResourceCookerImpl.h"
#include "GameDelegates.h"

FDelegateHandle IWwiseResourceCookerModule::ModifyCookDelegateHandle;

void IWwiseResourceCookerModule::StartupModule()
{
#if UE_5_0_OR_LATER
	// This StartupModule can be executed multiple times as more than one module can derive from the interface.
	// Use GetModule to load what the user wishes, and ignore the current "this".
	auto* This = GetModule();
	if (UNLIKELY(!This))
	{
		return;
	}

	// The act of GetModule() can StartupModule another one. Make sure we are not recursively set.
	if (ModifyCookDelegateHandle.IsValid())
	{
		return;
	}
	ModifyCookDelegateHandle = FGameDelegates::Get().GetModifyCookDelegate().AddRaw(This, &IWwiseResourceCookerModule::OnModifyCook);
#endif
}

void IWwiseResourceCookerModule::ShutdownModule()
{
#if UE_5_0_OR_LATER
	if (!ModifyCookDelegateHandle.IsValid())
	{
		return;
	}

	FGameDelegates::Get().GetModifyCookDelegate().Remove(ModifyCookDelegateHandle);
	ModifyCookDelegateHandle.Reset();
#endif
}
