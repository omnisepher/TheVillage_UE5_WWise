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

class IWwiseReconcile;

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"

class IWwiseReconcileModule : public IModuleInterface
{
public:
	static FName GetModuleName()
	{
		static const FName ModuleName = GetModuleNameFromConfig();
		return ModuleName;
	}

	/**
	 * Checks to see if this module is loaded and ready.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(GetModuleName());
	}

	static IWwiseReconcileModule* GetModule()
	{
		const auto ModuleName = GetModuleName();
		if (ModuleName.IsNone())
		{
			return nullptr;
		}
#if UE_SERVER
		return nullptr;
#else
		FModuleManager& ModuleManager = FModuleManager::Get();
		IWwiseReconcileModule* Result = ModuleManager.GetModulePtr<IWwiseReconcileModule>(ModuleName);
		if (UNLIKELY(!Result))
		{
			if (UNLIKELY(IsEngineExitRequested()))
			{
				UE_LOG(LogLoad, Verbose, TEXT("Skipping reloading missing WwiseReconcileImpl module: Exiting."));
			}
			else if (UNLIKELY(!IsInGameThread()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseReconcileImpl module: Not in game thread"));
			}
			else
			{
				UE_LOG(LogLoad, Log, TEXT("Loading WwiseReconcileImpl module: %s"), *ModuleName.GetPlainNameString());
				Result = ModuleManager.LoadModulePtr<IWwiseReconcileModule>(ModuleName);
				if (UNLIKELY(!Result))
				{
					UE_LOG(LogLoad, Fatal, TEXT("Could not load WwiseReconcileImpl module: %s not found"), *ModuleName.GetPlainNameString());
				}
			}
		}

		return Result;
#endif
	}

	virtual IWwiseReconcile* GetReconcile() { return nullptr; }
	virtual IWwiseReconcile* InstantiateReconcile() { return nullptr; }

private:
	static inline FName GetModuleNameFromConfig()
	{
		FString ModuleName = TEXT("WwiseReconcile");
		GConfig->GetString(TEXT("Audio"), TEXT("WwiseReconcileModuleName"), ModuleName, GEngineIni);
		return FName(ModuleName);
	}
};