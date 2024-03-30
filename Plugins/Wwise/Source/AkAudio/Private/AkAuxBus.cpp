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

#include "AkAuxBus.h"
#include "AkAudioBank.h"
#include "Wwise/WwiseResourceLoader.h"
#include "AkInclude.h"
#include "AkAudioDevice.h"
#include "Wwise/Stats/AkAudio.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCooker.h"
#endif

void UAkAuxBus::Serialize(FArchive& Ar)
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
		FWwiseLocalizedAuxBusCookedData CookedDataToArchive;
		if (auto* ResourceCooker = FWwiseResourceCooker::GetForArchive(Ar))
		{
			ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(AuxBusInfo));
			FillMetadata(ResourceCooker->GetProjectDatabase());
		}
		CookedDataToArchive.Serialize(Ar);
		Ar << MaxAttenuationRadius;
	}
#else
	AuxBusCookedData.Serialize(Ar);
	Ar << MaxAttenuationRadius;
#endif
#endif

}

void UAkAuxBus::LoadAuxBus()
{
	SCOPED_AKAUDIO_EVENT_2(TEXT("LoadAuxBus"));
	auto* ResourceLoader = FWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}

	UnloadAuxBus(false);

#if WITH_EDITORONLY_DATA
	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (!ProjectDatabase || !ProjectDatabase->IsProjectDatabaseParsed())
	{
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("UAkAuxBus::LoadAuxBus: Not loading '%s' because project database is not parsed."), *GetName())
		return;
	}
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(AuxBusCookedData, GetValidatedInfo(AuxBusInfo))))
	{
		return;
	}
	FillMetadata(ResourceCooker->GetProjectDatabase());
#endif

	const auto NewlyLoadedAuxBus = ResourceLoader->LoadAuxBus(AuxBusCookedData);
	auto PreviouslyLoadedAuxBus = LoadedAuxBus.exchange(NewlyLoadedAuxBus);
	if (UNLIKELY(PreviouslyLoadedAuxBus))
	{
		ResourceLoader->UnloadAuxBus(MoveTemp(PreviouslyLoadedAuxBus));
	}
}

void UAkAuxBus::UnloadAuxBus(bool bAsync)
{
	auto PreviouslyLoadedAuxBus = LoadedAuxBus.exchange(nullptr);
	if (PreviouslyLoadedAuxBus)
	{
		auto* ResourceLoader = FWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}

		if (bAsync)
		{
			FWwiseLoadedAuxBusPromise Promise;
			Promise.EmplaceValue(MoveTemp(PreviouslyLoadedAuxBus));
			ResourceUnload = ResourceLoader->UnloadAuxBusAsync(Promise.GetFuture());
		}
		else
		{
			ResourceLoader->UnloadAuxBus(MoveTemp(PreviouslyLoadedAuxBus));
		}
	}
}

#if WITH_EDITORONLY_DATA
void UAkAuxBus::CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
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

	ResourceCooker->CookAuxBus(GetValidatedInfo(AuxBusInfo), WriteAdditionalFile);
}

void UAkAuxBus::FillMetadata(FWwiseProjectDatabase* ProjectDatabase)
{
	Super::FillMetadata(ProjectDatabase);
	
	const auto AuxBusRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetAuxBus(GetValidatedInfo(AuxBusInfo));
	if (UNLIKELY(!AuxBusRef.IsValid()))
	{
		UE_LOG(LogAkAudio, Log, TEXT("UAkAuxBus::FillMetadata (%s): Cannot fill Metadata - Aux Bus not found in Project Database"), *GetName());
		return;
	}

	const FWwiseMetadataBus* AuxBusMetadata = AuxBusRef.GetAuxBus();
	if (AuxBusMetadata->Name.ToString().IsEmpty() || !AuxBusMetadata->GUID.IsValid() || AuxBusMetadata->Id == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkAuxBus::FillMetadata: Valid object not found in Project Database"));
		return;
	}

	MaxAttenuationRadius = AuxBusMetadata->MaxAttenuation;
}

bool UAkAuxBus::ObjectIsInSoundBanks()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAuxBus::GetWwiseRef: ResourceCooker not initialized"));
		return false;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAuxBus::GetWwiseRef: ProjectDatabase not initialized"));
		return false;
	}

	FWwiseObjectInfo* AudioTypeInfo = &AuxBusInfo;
	const FWwiseRefAuxBus AudioTypeRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetAuxBus(
		GetValidatedInfo(AuxBusInfo));

	return AudioTypeRef.IsValid();
}

void UAkAuxBus::FillInfo()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAuxBus::FillInfo: ResourceCooker not initialized"));
		return;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAuxBus::FillInfo: ProjectDatabase not initialized"));
		return;
	}

	FWwiseObjectInfo* AudioTypeInfo = &AuxBusInfo;
	const FWwiseRefAuxBus AudioTypeRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetAuxBus(
		GetValidatedInfo(AuxBusInfo));

	if (AudioTypeRef.AuxBusName().ToString().IsEmpty() || !AudioTypeRef.AuxBusGuid().IsValid() || AudioTypeRef.AuxBusId() == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkAuxBus::FillInfo: Valid object not found in Project Database"));
		return;
	}

	AudioTypeInfo->WwiseName = AudioTypeRef.AuxBusName();
	AudioTypeInfo->WwiseGuid = AudioTypeRef.AuxBusGuid();
	AudioTypeInfo->WwiseShortId = AudioTypeRef.AuxBusId();
}
#endif
