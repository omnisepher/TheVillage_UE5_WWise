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

#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "UObject/Class.h"

struct FWwiseSharedPlatformId;
class FWwiseResourceCooker;

UENUM()
enum class EWwiseExportDebugNameRule
{
	Release,
	Name,
	ObjectPath
};

class IWwiseResourceCookerModule : public IModuleInterface
{
public:
	void WWISERESOURCECOOKER_API StartupModule() override;
	void WWISERESOURCECOOKER_API ShutdownModule() override;

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
#if UE_SERVER
		return false;
#else
		static bool bModuleAvailable = false;
		if (LIKELY(!IsEngineExitRequested()) && LIKELY(bModuleAvailable))
		{
			return true;
		}
		bModuleAvailable = FModuleManager::Get().IsModuleLoaded(GetModuleName());
		return bModuleAvailable;
#endif
	}

	static IWwiseResourceCookerModule* GetModule()
	{
#if UE_SERVER
		return nullptr;
#else
		static IWwiseResourceCookerModule* Module = nullptr;
		if (LIKELY(!IsEngineExitRequested()) && LIKELY(Module))
		{
			return Module;
		}
		
		const auto ModuleName = GetModuleName();
		if (ModuleName.IsNone())
		{
			return nullptr;
		}

		FModuleManager& ModuleManager = FModuleManager::Get();
		Module = ModuleManager.GetModulePtr<IWwiseResourceCookerModule>(ModuleName);
		if (UNLIKELY(!Module))
		{
			if (UNLIKELY(IsEngineExitRequested()))
			{
				UE_LOG(LogLoad, Log, TEXT("Skipping reloading missing WwiseResourceCooker module: Exiting."));
			}
			else if (UNLIKELY(!IsInGameThread()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseResourceCooker module: Not in game thread"));
			}
			else
			{
				UE_LOG(LogLoad, Log, TEXT("Loading WwiseResourceCooker module: %s"), *ModuleName.GetPlainNameString());
				Module = ModuleManager.LoadModulePtr<IWwiseResourceCookerModule>(ModuleName);
				if (UNLIKELY(!Module))
				{
					UE_LOG(LogLoad, Fatal, TEXT("Could not load WwiseResourceCooker module: %s not found"), *ModuleName.GetPlainNameString());
				}
			}
		}

		return Module;
#endif
	}

	virtual FWwiseResourceCooker* GetResourceCooker()
	{
		return nullptr;
	}
	virtual FWwiseResourceCooker* InstantiateResourceCooker()
	{
		return nullptr;
	}

	virtual FWwiseResourceCooker* CreateCookerForPlatform(
		const ITargetPlatform* TargetPlatform,
		const FWwiseSharedPlatformId& InPlatform,
		EWwiseExportDebugNameRule InExportDebugNameRule = EWwiseExportDebugNameRule::Release)
	{
		return nullptr;
	}

	virtual void DestroyCookerForPlatform(const ITargetPlatform* TargetPlatform)
	{
	}

	virtual FWwiseResourceCooker* GetCookerForPlatform(const ITargetPlatform* TargetPlatform)
	{
		return nullptr;
	}

	FWwiseResourceCooker* GetCookerForArchive(const FArchive& InArchive)
	{
		if (!InArchive.IsCooking() || !InArchive.IsSaving())
		{
			return nullptr;
		}

		return GetCookerForPlatform(InArchive.CookingTarget());
	}

	virtual void DestroyAllCookerPlatforms()
	{
	}

protected:
	static WWISERESOURCECOOKER_API FDelegateHandle ModifyCookDelegateHandle;

	virtual void OnModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook)
	{
	}

private:
	static inline FName GetModuleNameFromConfig()
	{
		FString ModuleName = TEXT("WwiseResourceCooker");
		GConfig->GetString(TEXT("Audio"), TEXT("WwiseResourceCookerModuleName"), ModuleName, GEngineIni);
		return FName(ModuleName);
	}
};
