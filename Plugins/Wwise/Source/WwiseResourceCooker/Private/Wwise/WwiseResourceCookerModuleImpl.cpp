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

#include "Wwise/WwiseResourceCookerModuleImpl.h"
#include "Interfaces/ITargetPlatform.h"
#include "Wwise/WwiseCookingCache.h"
#include "Wwise/WwiseResourceCookerImpl.h"

#include "Wwise/WwiseResourceLoader.h"

#include "CookOnTheSide/CookLog.h"
#include "Wwise/Stats/ResourceCooker.h"

#include "Misc/CommandLine.h"

IMPLEMENT_MODULE(FWwiseResourceCookerModule, WwiseResourceCooker)

FWwiseResourceCooker* FWwiseResourceCookerModule::GetResourceCooker()
{
	Lock.ReadLock();
	if (LIKELY(ResourceCooker))
	{
		Lock.ReadUnlock();
	}
	else
	{
		Lock.ReadUnlock();
		Lock.WriteLock();
		if (LIKELY(!ResourceCooker))
		{
			UE_LOG(LogWwiseResourceCooker, Display, TEXT("Initializing default Resource Cooker."));
			ResourceCooker.Reset(InstantiateResourceCooker());
		}
		Lock.WriteUnlock();
	}
	return ResourceCooker.Get();
}

FWwiseResourceCooker* FWwiseResourceCookerModule::InstantiateResourceCooker()
{
	return new FWwiseResourceCookerImpl;
}

FWwiseResourceCooker* FWwiseResourceCookerModule::CreateCookerForPlatform(const ITargetPlatform* TargetPlatform,
	const FWwiseSharedPlatformId& InPlatform, EWwiseExportDebugNameRule InExportDebugNameRule)
{
	if (TargetPlatform && TargetPlatform->IsServerOnly())
	{
		static FString bOnce;
		UE_CLOG(bOnce != TargetPlatform->PlatformName(), LogWwiseResourceCooker, Display, TEXT("CreateCookerForPlatform: Disabling cooker on cooking for server platform %s (UE: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"));
		bOnce = TargetPlatform->PlatformName();
		return nullptr;
	}

	{
		const auto* CookingPlatform = CookingPlatforms.Find(TargetPlatform);

		if (UNLIKELY(!CookingPlatform))
		{
#if UE_5_0_OR_LATER
			if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase() && !FParse::Param(FCommandLine::Get(), TEXT("CookOnTheFly")))		// By The Book cooking needs to predefine the requested platforms. InEditor and OnTheFly should create them all the time.
			{
				UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: Not cooking for platform %s (UE: %s, Wwise: %s)"),
					TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
					TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
					*InPlatform.GetPlatformName().ToString());
				return nullptr;
			}
#endif
		}
		else if (*CookingPlatform)
		{
			return CookingPlatform->Get();
		}
	}

	const auto* DefaultResourceCooker = GetResourceCooker();
	if (UNLIKELY(!DefaultResourceCooker))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: No Default Cooker available creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			*InPlatform.GetPlatformName().ToString());
		return nullptr;
	}

	const auto* DefaultProjectDatabase = DefaultResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!DefaultProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: No Default ProjectDatabase available creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			*InPlatform.GetPlatformName().ToString());
		return nullptr;
	}

	const auto* DefaultResourceLoader = DefaultProjectDatabase->GetResourceLoader();
	if (UNLIKELY(!DefaultResourceLoader))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: No ResourceLoader available creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			*InPlatform.GetPlatformName().ToString());
		return nullptr;
	}

	const auto* DefaultResourceLoaderImpl = DefaultResourceLoader->ResourceLoaderImpl.Get();
	if (UNLIKELY(!DefaultResourceLoader))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("CreateCookerForPlatform: No ResourceLoaderImpl available creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			*InPlatform.GetPlatformName().ToString());
		return nullptr;
	}

	UE_LOG(LogWwiseResourceCooker, Display, TEXT("Starting cooking process for platform %s (UE: %s, Wwise: %s)"),
		TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
		TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
		*InPlatform.GetPlatformName().ToString());

	auto* NewResourceLoaderImpl = FWwiseResourceLoaderImpl::Instantiate();
	if (UNLIKELY(!NewResourceLoaderImpl))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CreateCookerForPlatform: Could not instantiate ResourceLoaderImpl creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			*InPlatform.GetPlatformName().ToString());
		return nullptr;
	}

	NewResourceLoaderImpl->CurrentPlatform = InPlatform;
	NewResourceLoaderImpl->StagePath = DefaultResourceLoaderImpl->StagePath;
#if WITH_EDITORONLY_DATA
	NewResourceLoaderImpl->GeneratedSoundBanksPath = DefaultResourceLoaderImpl->GeneratedSoundBanksPath;
#endif

	auto* NewResourceLoader = FWwiseResourceLoader::Instantiate();
	if (UNLIKELY(!NewResourceLoader))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CreateCookerForPlatform: Could not instantiate ResourceLoader creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			*InPlatform.GetPlatformName().ToString());
		delete NewResourceLoaderImpl;
		return nullptr;
	}
	NewResourceLoader->ResourceLoaderImpl.Reset(NewResourceLoaderImpl);
	NewResourceLoaderImpl->CurrentLanguage = NewResourceLoader->SystemLanguage();

	auto* NewProjectDatabase = FWwiseProjectDatabase::Instantiate();
	if (UNLIKELY(!NewProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CreateCookerForPlatform: Could not instantiate ProjectDatabase creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			*InPlatform.GetPlatformName().ToString());
		delete NewResourceLoader;
		return nullptr;
	}
	NewProjectDatabase->PrepareProjectDatabaseForPlatform(MoveTemp(NewResourceLoader));
	NewProjectDatabase->UpdateDataStructure(nullptr, nullptr);

	auto* NewResourceCooker = FWwiseResourceCooker::Instantiate();
	if (UNLIKELY(!NewResourceCooker))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("CreateCookerForPlatform: Could not instantiate ResourceCooker creating platform %s (UE: %s, Wwise: %s)"),
			TargetPlatform ? *TargetPlatform->PlatformName() : TEXT("[nullptr]"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("[nullptr]"),
			* InPlatform.GetPlatformName().ToString());
		delete NewProjectDatabase;
		return nullptr;
	}
	NewResourceCooker->PrepareResourceCookerForPlatform(MoveTemp(NewProjectDatabase), InExportDebugNameRule);

	CookingPlatforms.Add(TargetPlatform, TUniquePtr<FWwiseResourceCooker>(NewResourceCooker));
	return NewResourceCooker;
}

void FWwiseResourceCookerModule::DestroyCookerForPlatform(const ITargetPlatform* TargetPlatform)
{
	if (UNLIKELY(!CookingPlatforms.Find(TargetPlatform)))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("DestroyCookerForPlatform: Target %s not created"),
			TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("null"));
		return;
	}

	UE_LOG(LogWwiseResourceCooker, Log, TEXT("DestroyCookerForPlatform for target: %s"),
		TargetPlatform ? *TargetPlatform->IniPlatformName() : TEXT("null"));

	CookingPlatforms.Remove(TargetPlatform);
}

FWwiseResourceCooker* FWwiseResourceCookerModule::GetCookerForPlatform(const ITargetPlatform* TargetPlatform)
{
	if (TargetPlatform->IsServerOnly())
	{
		UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("GetCookerForPlatform: Target %s is server-only."), *TargetPlatform->IniPlatformName());
		return nullptr;
	}
	auto* Result = CookingPlatforms.Find(TargetPlatform);
	if (UNLIKELY(!Result))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetCookerForPlatform: Target %s not created"), *TargetPlatform->IniPlatformName());
		return nullptr;
	}
	return Result->Get();
}

void FWwiseResourceCookerModule::DestroyAllCookerPlatforms()
{
	TArray<const ITargetPlatform*> PreviousTargets;
	CookingPlatforms.GetKeys(PreviousTargets);

	for (const auto* PreviousTarget : PreviousTargets)
	{
		DestroyCookerForPlatform(PreviousTarget);
	}
}

void FWwiseResourceCookerModule::ShutdownModule()
{
	DestroyAllCookerPlatforms();
	Lock.WriteLock();
	if (ResourceCooker.IsValid())
	{
		UE_LOG(LogWwiseResourceCooker, Display, TEXT("Shutting down default Resource Cooker."));
		ResourceCooker.Reset();
	}
	Lock.WriteUnlock();
	IWwiseResourceCookerModule::ShutdownModule();
}

void FWwiseResourceCookerModule::OnModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook)
{
	DestroyAllCookerPlatforms();

	for (const auto* TargetPlatform : InTargetPlatforms)
	{
		UE_LOG(LogWwiseResourceCooker, Log, TEXT("Starting cooking process for platform %s"), *TargetPlatform->DisplayName().ToString());
		CookingPlatforms.Add(TargetPlatform);
	}
}
