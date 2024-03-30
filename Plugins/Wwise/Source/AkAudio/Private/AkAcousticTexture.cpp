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

#include "AkAcousticTexture.h"
#include "Wwise/Stats/AkAudio.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCooker.h"
#include "AkAudioDevice.h"
#endif

void UAkAcousticTexture::Serialize(FArchive& Ar)
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
		FWwiseAcousticTextureCookedData CookedDataToArchive;
		if (auto* ResourceCooker = FWwiseResourceCooker::GetForArchive(Ar))
		{
			ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(AcousticTextureInfo));
		}
		CookedDataToArchive.Serialize(Ar);
	}
#else
	AcousticTextureCookedData.Serialize(Ar);
#endif
#endif

}

#if WITH_EDITORONLY_DATA
bool UAkAcousticTexture::ObjectIsInSoundBanks()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAcousticTexture::GetWwiseRef: ResourceCooker not initialized"));
		return false;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAcousticTexture::GetWwiseRef: ProjectDatabase not initialized"));
		return false;
	}

	FWwiseObjectInfo* AudioTypeInfo = &AcousticTextureInfo;
	const FWwiseRefAcousticTexture AcousticTextureRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetAcousticTexture(
		GetValidatedInfo(AcousticTextureInfo));

	return AcousticTextureRef.IsValid();
}

void UAkAcousticTexture::FillInfo()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAcousticTexture::FillInfo: ResourceCooker not initialized"));
		return;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAcousticTexture::FillInfo: ProjectDatabase not initialized"));
		return;
	}

	FWwiseObjectInfo* AudioTypeInfo = &AcousticTextureInfo;
	const FWwiseRefAcousticTexture AcousticTextureRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetAcousticTexture(
		GetValidatedInfo(AcousticTextureInfo));

	if (AcousticTextureRef.AcousticTextureName().ToString().IsEmpty() || !AcousticTextureRef.AcousticTextureGuid().IsValid() || AcousticTextureRef.AcousticTextureId() == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkAcousticTexture::FillInfo: Valid object not found in Project Database"));
		return;
	}

	AudioTypeInfo->WwiseName = AcousticTextureRef.AcousticTextureName();
	AudioTypeInfo->WwiseGuid = AcousticTextureRef.AcousticTextureGuid();
	AudioTypeInfo->WwiseShortId = AcousticTextureRef.AcousticTextureId();
}

void UAkAcousticTexture::GetAcousticTextureCookedData()
{
	SCOPED_AKAUDIO_EVENT_2(TEXT("GetAcousticTextureCookedData"));
	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (!ProjectDatabase || !ProjectDatabase->IsProjectDatabaseParsed())
	{
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("UAkAcousticTexture::GetAcousticTextureCookedData: Not loading '%s' because project database is not parsed."), *GetName())
		return;
	}
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}

	ResourceCooker->PrepareCookedData(AcousticTextureCookedData, GetValidatedInfo(AcousticTextureInfo));
}
#endif
