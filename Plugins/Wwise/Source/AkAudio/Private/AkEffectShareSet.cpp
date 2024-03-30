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

#include "AkEffectShareSet.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/Stats/AkAudio.h"

#if WITH_EDITORONLY_DATA
#include "AkAudioDevice.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCooker.h"
#endif

void UAkEffectShareSet::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
#if !UE_SERVER
#if WITH_EDITORONLY_DATA
	if (Ar.IsCooking() && Ar.IsSaving() && !Ar.CookingTarget()->IsServerOnly())
	{
		FWwiseLocalizedShareSetCookedData CookedDataToArchive;
		if (auto* ResourceCooker = FWwiseResourceCooker::GetForArchive(Ar))
		{
			ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(ShareSetInfo));
		}
		CookedDataToArchive.Serialize(Ar);
	}
#else
	ShareSetCookedData.Serialize(Ar);
#endif
#endif
}

void UAkEffectShareSet::LoadEffectShareSet()
{
	SCOPED_AKAUDIO_EVENT_2(TEXT("LoadEffectShareSet"));
	auto* ResourceLoader = FWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	
	UnloadEffectShareSet(false);

#if WITH_EDITORONLY_DATA
	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (!ProjectDatabase || !ProjectDatabase->IsProjectDatabaseParsed())
	{
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("UAkEffectShareSet::LoadEffectShareSet: Not loading '%s' because project database is not parsed."), *GetName())
		return;
	}
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(ShareSetCookedData, GetValidatedInfo(ShareSetInfo))))
	{
		return;
	}
#endif
	
	const auto NewlyLoadedShareSet = ResourceLoader->LoadShareSet(ShareSetCookedData);
	auto PreviouslyLoadedShareSet = LoadedShareSet.exchange(NewlyLoadedShareSet);
	if (UNLIKELY(PreviouslyLoadedShareSet))
	{
		ResourceLoader->UnloadShareSet(MoveTemp(PreviouslyLoadedShareSet));
	}
}

void UAkEffectShareSet::UnloadEffectShareSet(bool bAsync)
{
	auto PreviouslyLoadedShareSet = LoadedShareSet.exchange(nullptr);
	if (PreviouslyLoadedShareSet)
	{
		auto* ResourceLoader = FWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		if (bAsync)
		{
			FWwiseLoadedShareSetPromise Promise;
			Promise.EmplaceValue(MoveTemp(PreviouslyLoadedShareSet));
			ResourceUnload = ResourceLoader->UnloadShareSetAsync(Promise.GetFuture());
		}
		else
		{
			ResourceLoader->UnloadShareSet(MoveTemp(PreviouslyLoadedShareSet));
		}
	}
}

#if WITH_EDITORONLY_DATA
bool UAkEffectShareSet::ObjectIsInSoundBanks()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkEffectShareSet::GetWwiseRef: ResourceCooker not initialized"));
		return false;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkEffectShareSet::GetWwiseRef: ProjectDatabase not initialized"));
		return false;
	}

	FWwiseObjectInfo* AudioTypeInfo = &ShareSetInfo;
	const FWwiseRefPluginShareSet AudioTypeRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetPluginShareSet(
		GetValidatedInfo(ShareSetInfo));

	return AudioTypeRef.IsValid();
}

void UAkEffectShareSet::CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
	TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	FWwiseResourceCooker* ResourceCooker = FWwiseResourceCooker::GetForPlatform(TargetPlatform);
	if (!ResourceCooker)
	{
		return;
	}
	ResourceCooker->SetSandboxRootPath(PackageFilename);
	ResourceCooker->CookShareSet(GetValidatedInfo(ShareSetInfo), WriteAdditionalFile);
}

void UAkEffectShareSet::FillInfo()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkEffectShareSet::FillInfo: ResourceCooker not initialized"));
		return;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkEffectShareSet::FillInfo: ProjectDatabase not initialized"));
		return;
	}

	FWwiseObjectInfo* AudioTypeInfo = &ShareSetInfo;
	const FWwiseRefPluginShareSet AudioTypeRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetPluginShareSet(
		GetValidatedInfo(ShareSetInfo));

	if (AudioTypeRef.PluginShareSetName().ToString().IsEmpty() || !AudioTypeRef.PluginShareSetGuid().IsValid() || AudioTypeRef.PluginShareSetId() == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkEffectShareSet::FillInfo: Valid object not found in Project Database"));
		return;
	}

	AudioTypeInfo->WwiseName = AudioTypeRef.PluginShareSetName();
	AudioTypeInfo->WwiseGuid = AudioTypeRef.PluginShareSetGuid();
	AudioTypeInfo->WwiseShortId = AudioTypeRef.PluginShareSetId();
}

#endif
