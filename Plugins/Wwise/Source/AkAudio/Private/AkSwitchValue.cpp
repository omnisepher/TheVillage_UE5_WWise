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

#include "AkSwitchValue.h"
#include "Wwise/Stats/AkAudio.h"

#include "Wwise/WwiseResourceLoader.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCooker.h"
#include "AkAudioDevice.h"
#endif

void UAkSwitchValue::LoadGroupValue()
{
	SCOPED_AKAUDIO_EVENT_2(TEXT("UAkSwitchValue::LoadGroupValue"));
	auto* ResourceLoader = FWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	
	UnloadGroupValue(false);

#if WITH_EDITORONLY_DATA
	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (!ProjectDatabase || !ProjectDatabase->IsProjectDatabaseParsed())
	{
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("UAkSwitchValue::LoadGroupValue: Not loading '%s' because project database is not parsed."), *GetName())
		return;
	}
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(GroupValueCookedData, GetValidatedInfo(GroupValueInfo), EWwiseGroupType::Switch)))
	{
		return;
	}
#endif
	
	const auto NewlyLoadedGroupValue = ResourceLoader->LoadGroupValue(GroupValueCookedData);
	auto PreviouslyLoadedGroupValue = LoadedGroupValue.exchange(NewlyLoadedGroupValue);
	if (UNLIKELY(PreviouslyLoadedGroupValue))
	{
		ResourceLoader->UnloadGroupValue(MoveTemp(PreviouslyLoadedGroupValue));
	}
}

void UAkSwitchValue::Serialize(FArchive& Ar)
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
		FWwiseGroupValueCookedData CookedDataToArchive;
		if (auto* ResourceCooker = FWwiseResourceCooker::GetForArchive(Ar))
		{
			ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(GroupValueInfo), EWwiseGroupType::Switch);
		}
		CookedDataToArchive.Serialize(Ar);
	}
#else
	GroupValueCookedData.Serialize(Ar);
#endif
#endif
}

#if WITH_EDITORONLY_DATA
void UAkSwitchValue::FillInfo()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::FillInfo: ResourceCooker not initialized"));
		return;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::FillInfo: ProjectDatabase not initialized"));
		return;
	}

	FWwiseGroupValueInfo* AudioTypeInfo = static_cast<FWwiseGroupValueInfo*>(GetInfoMutable());
	FWwiseRefSwitch RefSwitch = FWwiseDataStructureScopeLock(*ProjectDatabase).GetSwitch(
		GetValidatedInfo(*AudioTypeInfo));

	if (RefSwitch.SwitchName().ToString().IsEmpty() || !RefSwitch.SwitchGuid().IsValid() || RefSwitch.SwitchId() == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkSwitchValue::FillInfo: Valid object not found in Project Database"));
		return;
	}

	FWwiseAnyRef Ref = FWwiseAnyRef::Create(RefSwitch);

	AudioTypeInfo->WwiseName = RefSwitch.SwitchName();
	AudioTypeInfo->WwiseGuid = RefSwitch.SwitchGuid();
	AudioTypeInfo->WwiseShortId = RefSwitch.SwitchId();
	AudioTypeInfo->GroupShortId = RefSwitch.SwitchGroupId();
}

void UAkSwitchValue::FillInfo(const FWwiseAnyRef& CurrentWwiseRef)
{
	FWwiseGroupValueInfo* AudioTypeInfo = static_cast<FWwiseGroupValueInfo*>(GetInfoMutable());

	AudioTypeInfo->WwiseName = CurrentWwiseRef.GetName();
	AudioTypeInfo->WwiseGuid = CurrentWwiseRef.GetGuid();
	AudioTypeInfo->WwiseShortId = CurrentWwiseRef.GetId();
	AudioTypeInfo->GroupShortId = CurrentWwiseRef.GetGroupId();
}

bool UAkSwitchValue::ObjectIsInSoundBanks()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::GetWwiseRef: ResourceCooker not initialized"));
		return false;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();

	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::GetWwiseRef: ProjectDatabase not initialized"));
		return false;
	}

	FWwiseGroupValueInfo* AudioTypeInfo = static_cast<FWwiseGroupValueInfo*>(GetInfoMutable());
	FWwiseRefSwitch RefSwitch = FWwiseDataStructureScopeLock(*ProjectDatabase).GetSwitch(
		GetValidatedInfo(*AudioTypeInfo));

	return RefSwitch.IsValid();
}

FName UAkSwitchValue::GetWwiseGroupName()
{

	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::GetWwiseRef: ResourceCooker not initialized"));
		return {};
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();

	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkSwitchValue::GetWwiseRef: ProjectDatabase not initialized"));
		return {};
	}

	FWwiseGroupValueInfo* AudioTypeInfo = static_cast<FWwiseGroupValueInfo*>(GetInfoMutable());
	FWwiseRefSwitch RefSwitch = FWwiseDataStructureScopeLock(*ProjectDatabase).GetSwitch(
		GetValidatedInfo(*AudioTypeInfo));

	return RefSwitch.SwitchGroupName();

}
#endif

