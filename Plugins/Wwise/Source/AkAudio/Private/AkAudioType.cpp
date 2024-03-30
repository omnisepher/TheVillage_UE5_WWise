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

#include "AkAudioType.h"

#include "AkAudioModule.h"
#include "AkAudioDevice.h"
#include "AkSettings.h"
#include "AkCustomVersion.h"
#include "Platforms/AkPlatformInfo.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseResourceCooker.h"
#endif

UAkAudioType::~UAkAudioType()
{
	SCOPED_AKAUDIO_EVENT_3(TEXT("UAkAudioType Dtor"));
	ResourceUnload.Wait();
}

void UAkAudioType::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	Ar.UsingCustomVersion(FAkCustomVersion::GUID);

#if WITH_EDITORONLY_DATA
	CheckWwiseObjectInfo();
#endif
	LogSerializationState(Ar);
}

void UAkAudioType::PostLoad()
{
	Super::PostLoad();

	if (LIKELY(bAutoLoad))
	{
		if (LIKELY(CanLoadObjects()))
		{
			LoadData();
		}
		else
		{
			LoadOnceSoundEngineReady();
		}
	}
}

void UAkAudioType::BeginDestroy()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return Super::BeginDestroy();
	}

	{
		SCOPED_AKAUDIO_EVENT_F_2(TEXT("UAkAudioType::BeginDestroy %s"), *GetClass()->GetName());
		UE_LOG(LogAkAudio, Verbose, TEXT("UAkAudioType::BeginDestroy[%p] %s %s"), this, *GetClass()->GetName(), *GetName());

		UnloadData(true);
	}
	Super::BeginDestroy();
}

void UAkAudioType::FinishDestroy()
{
	{
		SCOPED_AKAUDIO_EVENT_2(TEXT("UAkAudioType::FinishDestroy"));
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("UAkAudioType::FinishDestroy[%p]"), this);

		ResourceUnload.Wait();
	}
	Super::FinishDestroy();
}

void UAkAudioType::LogSerializationState(const FArchive& Ar)
{
	FString SerializationState = TEXT("");
	if (Ar.IsLoading())
	{
		SerializationState += TEXT("Loading");
	}

	if (Ar.IsSaving())
	{
		SerializationState += TEXT("Saving");
	}

	if (Ar.IsCooking())
	{
		SerializationState += TEXT("Cooking");
	}

	UE_LOG(LogAkAudio, VeryVerbose, TEXT("%s - Serialization - %s"), *GetName(), *SerializationState);

}

AkUInt32 UAkAudioType::GetShortID() const
{
	UE_LOG(LogAkAudio, Error, TEXT("Trying to GetShortID without an overridden Short ID for %s."), *GetName());
	return AK_INVALID_UNIQUE_ID;
}

#if WITH_EDITORONLY_DATA
void UAkAudioType::MigrateWwiseObjectInfo()
{
	if (FWwiseObjectInfo* Info = GetInfoMutable())
	{
		Info->WwiseName = FName(GetName());

		if ( ID_DEPRECATED.IsValid())
		{
			Info->WwiseGuid = ID_DEPRECATED;
		}

		if (ShortID_DEPRECATED != 0)
		{
			Info->WwiseShortId = ShortID_DEPRECATED;
		}
		else
		{
			Info->WwiseShortId = FAkAudioDevice::GetShortIDFromString(GetName());
		}
	}
}

void UAkAudioType::WaitForResourceUnloaded()
{
	SCOPED_AKAUDIO_EVENT_3(TEXT("UAkAudioType::WaitForResourceUnloaded"));
	ResourceUnload.Wait();
	ResourceUnload.Reset();
}

void UAkAudioType::CheckWwiseObjectInfo()
{
	if (FWwiseObjectInfo* WwiseInfo = GetInfoMutable())
	{
		if (!WwiseInfo->WwiseGuid.IsValid() || WwiseInfo->WwiseShortId == 0 || WwiseInfo->WwiseName.ToString().IsEmpty())
		{
			UE_LOG(LogAkAudio, Log, TEXT("CheckWwiseObjectInfo: Wwise Asset %s has empty WwiseObjectInfo fields. WwiseName: '%s' - WwiseShortId: '%d' - WwiseGuid: '%s'"),
				*GetName(), *WwiseInfo->WwiseName.ToString(), WwiseInfo->WwiseShortId, *WwiseInfo->WwiseGuid.ToString());
		}
	}
}

FWwiseObjectInfo* UAkAudioType::GetInfoMutable()
{
	UE_LOG(LogAkAudio, Error, TEXT("GetInfoMutable not implemented"));
	check(false);
	return nullptr;
}

void UAkAudioType::ValidateShortID(FWwiseObjectInfo& WwiseInfo) const
{
	if (WwiseInfo.WwiseShortId == AK_INVALID_UNIQUE_ID)
	{
		if (!WwiseInfo.WwiseName.ToString().IsEmpty())
		{
			WwiseInfo.WwiseShortId = FAkAudioDevice::GetShortIDFromString(WwiseInfo.WwiseName.ToString());
		}
		else
		{
			WwiseInfo.WwiseShortId = FAkAudioDevice::GetShortIDFromString(GetName());
			UE_LOG(LogAkAudio, Warning, TEXT("UAkAudioType::ValidateShortID : Using ShortID based on asset name for %s."), *GetName());
		}
	}
}

#if WITH_EDITOR
void UAkAudioType::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FWwiseObjectInfo* AudioTypeInfo = GetInfoMutable();

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
	{
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseName))
		{
			AudioTypeInfo->WwiseGuid = {};
			AudioTypeInfo->WwiseShortId = 0;
		}
		else if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FWwiseEventInfo, WwiseShortId))
		{
			AudioTypeInfo->WwiseGuid= {};
			AudioTypeInfo->WwiseName = "";
		}
		// The first check should be sufficient, but the property's FName for GUIDs is A/B/C/D,
		// depending on which part of the GUID was modified
		else if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FWwiseEventInfo, WwiseGuid) ||
			PropertyChangedEvent.Property->Owner.GetFName() == "Guid")
		{
			AudioTypeInfo->WwiseName = "";
			AudioTypeInfo->WwiseShortId = {};
		}
		else if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAkAudioType, bAutoLoad))
		{
			if(!bAutoLoad)
			{
				UnloadData();
			}
			// Else not necessary, we will load the data just before exiting the function
		}
		FillInfo();
	}

	if (bAutoLoad)
	{
		LoadData();
	}
}

void UAkAudioType::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	UObject::GetAssetRegistryTags(OutTags);

	auto WwiseInfo = GetInfo();
	//This seems to be more reliable than putting the AssetRegistrySearchable tag on FWwiseObjectInfo::WwiseGuid
	OutTags.Add(FAssetRegistryTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseGuid), WwiseInfo.WwiseGuid.ToString(), FAssetRegistryTag::ETagType::TT_Hidden));
	OutTags.Add(FAssetRegistryTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseShortId), FString::FromInt(WwiseInfo.WwiseShortId), FAssetRegistryTag::ETagType::TT_Hidden));
	OutTags.Add(FAssetRegistryTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseName), WwiseInfo.WwiseName.ToString(), FAssetRegistryTag::ETagType::TT_Hidden));
}
#endif

void UAkAudioType::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	if (auto* AkSettings = GetDefault<UAkSettings>())
	{
		if (AkSettings->AreSoundBanksGenerated())
		{
			UObject::BeginCacheForCookedPlatformData(TargetPlatform);
			if (HasAnyFlags(RF_ClassDefaultObject))
			{
				return;
			}
			auto PlatformID = UAkPlatformInfo::GetSharedPlatformInfo(TargetPlatform->IniPlatformName());
			FWwiseResourceCooker::CreateForPlatform(TargetPlatform, PlatformID, EWwiseExportDebugNameRule::Name);
		}
	}
}

bool UAkAudioType::IsAssetOutOfDate(const FWwiseAnyRef& CurrentWwiseRef)
{
	FWwiseObjectInfo* ObjectInfo = GetInfoMutable();
	if (CurrentWwiseRef.GetGuid() != ObjectInfo->WwiseGuid
		|| CurrentWwiseRef.GetName() != ObjectInfo->WwiseName
		|| CurrentWwiseRef.GetId() != ObjectInfo->WwiseShortId) 
	{
		return true;
	}

	return false;
}

void UAkAudioType::FillInfo(const FWwiseAnyRef& CurrentWwiseRef)
{
	FWwiseObjectInfo* ObjectInfo = GetInfoMutable();

	ObjectInfo->WwiseGuid = CurrentWwiseRef.GetGuid();
	ObjectInfo->WwiseShortId = CurrentWwiseRef.GetId();
	ObjectInfo->WwiseName = CurrentWwiseRef.GetName();
}

FName UAkAudioType::GetAssetDefaultName()
{
	const FName WwiseGroupName = GetWwiseGroupName();
	const FName WwiseAssetName = GetWwiseName();

	if (!WwiseGroupName.IsNone())
	{
		FNameBuilder DefaultName;
		DefaultName << WwiseGroupName << TEXT("-") << WwiseAssetName;
#if UE_5_0_OR_LATER
		return FName(DefaultName.ToView());
#else
		return FName(DefaultName.ToString());
#endif
	}

	return WwiseAssetName;
}

FName UAkAudioType::GetAssetDefaultPackagePath()
{
	auto AkSettings = GetMutableDefault<UAkSettings>();
	if (!AkSettings)
	{
		UE_LOG(LogAkAudio, Error, TEXT("Could not fetch AkSettings"));
		return {};
	}

	FName GroupName = GetWwiseGroupName();

	if (GroupName.ToString().IsEmpty())
	{
		return FName(AkSettings->DefaultAssetCreationPath);
	}

	return FName(AkSettings->DefaultAssetCreationPath / GroupName.ToString());
}

#endif

bool UAkAudioType::CanLoadObjects()
{
	if (UNLIKELY(!FAkAudioModule::AkAudioModuleInstance))
	{
		return false;
	}

	auto AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		return false;
	}

	return LIKELY(!IsEngineExitRequested()) && LIKELY(AudioDevice->IsInitialized());
}

void UAkAudioType::LoadOnceSoundEngineReady()
{
	UE_LOG(LogAkAudio, Verbose, TEXT("UAkAudioType::LoadOnceSoundEngineReady: Delay loading %s once SoundEngine is initialized."), *GetName());
	FAkAudioDevice::LoadAudioObjectsAfterInitialization(MakeWeakObjectPtr(this));
}
