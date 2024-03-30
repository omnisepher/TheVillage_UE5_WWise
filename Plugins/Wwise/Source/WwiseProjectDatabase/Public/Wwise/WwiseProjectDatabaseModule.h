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

#include "Modules/ModuleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "WwiseUnrealDefines.h"
#if !UE_5_0_OR_LATER
#include "Misc/CommandLine.h"
#endif

class FWwiseProjectDatabase;
class FWwiseProjectDatabaseDelegates;

class IWwiseProjectDatabaseModule : public IModuleInterface
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
		static bool bModuleAvailable = false;
		if (LIKELY(!IsEngineExitRequested()) && LIKELY(bModuleAvailable))
		{
			return true;
		}
		bModuleAvailable = FModuleManager::Get().IsModuleLoaded(GetModuleName());
		return bModuleAvailable;
	}

	static IWwiseProjectDatabaseModule* GetModule()
	{
		static IWwiseProjectDatabaseModule* Module = nullptr;
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
		Module = ModuleManager.GetModulePtr<IWwiseProjectDatabaseModule>(ModuleName);
		if (UNLIKELY(!Module))
		{
			if (UNLIKELY(IsEngineExitRequested()))
			{
				UE_LOG(LogLoad, Log, TEXT("Skipping reloading missing WwiseProjectDatabase module: Exiting."));
			}
			else if (UNLIKELY(!IsInGameThread()))
			{
				UE_LOG(LogLoad, Warning, TEXT("Skipping loading missing WwiseProjectDatabase module: Not in game thread"));
			}
			else
			{
				UE_LOG(LogLoad, Log, TEXT("Loading WwiseProjectDatabase module: %s"), *ModuleName.GetPlainNameString());
				Module = ModuleManager.LoadModulePtr<IWwiseProjectDatabaseModule>(ModuleName);
				if (UNLIKELY(!Module))
				{
					UE_LOG(LogLoad, Fatal, TEXT("Could not load WwiseProjectDatabase module: %s not found"), *ModuleName.GetPlainNameString());
				}
			}
		}

		return Module;
	}

	static bool ShouldInitializeProjectDatabase()
	{
#if UE_5_0_OR_LATER
		return !IsRunningCookCommandlet();
#else
		if(IsRunningCommandlet())
		{
			TArray<FString> Switches;
			TArray<FString> Tokens;
			FCommandLine::Parse(FCommandLine::Get(), Tokens, Switches);
			for(auto& Token : Tokens)
			{
				//Only in the WwiseReconcile commandlet that the Project Database should be initialized
				if(Token.Contains(TEXT("run=WwiseReconcile")))
				{
					return true;
				}
			}
			return false;
		}
		return true;
#endif
	}

	virtual FWwiseProjectDatabase* GetProjectDatabase() { return nullptr; }
	virtual FWwiseProjectDatabase* InstantiateProjectDatabase() { return nullptr; }
	virtual bool CanHaveDefaultInstance() {return false;}
	virtual FWwiseProjectDatabaseDelegates* GetProjectDatabaseDelegates() = 0;
	virtual FWwiseProjectDatabaseDelegates* InstantiateProjectDatabaseDelegates() = 0;


private:
	static FName GetModuleNameFromConfig()
	{
		FString ModuleName = TEXT("WwiseProjectDatabase");
		GConfig->GetString(TEXT("Audio"), TEXT("WwiseProjectDatabaseModuleName"), ModuleName, GEngineIni);
		return FName(ModuleName);
	}
};
