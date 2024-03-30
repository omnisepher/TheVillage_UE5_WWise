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

#include "AkGroupValue.h"
#include "AkAudioDevice.h"

#include "Wwise/WwiseResourceLoader.h"

#include <inttypes.h>

void UAkGroupValue::UnloadGroupValue(bool bAsync)
{
	auto PreviouslyLoadedGroupValue = LoadedGroupValue.exchange(nullptr);
	if (PreviouslyLoadedGroupValue)
	{
		auto* ResourceLoader = FWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		if (bAsync)
		{
			FWwiseLoadedGroupValuePromise Promise;
			Promise.EmplaceValue(MoveTemp(PreviouslyLoadedGroupValue));
			ResourceUnload = ResourceLoader->UnloadGroupValueAsync(Promise.GetFuture());
		}
		else
		{
			ResourceLoader->UnloadGroupValue(MoveTemp(PreviouslyLoadedGroupValue));
		}
	}
}

#if WITH_EDITORONLY_DATA
void UAkGroupValue::MigrateWwiseObjectInfo()
{
	FString GroupName;
	FString ValueName;
	SplitAssetName( GroupName, ValueName);
	if (GroupShortID_DEPRECATED != AK_INVALID_UNIQUE_ID )
	{
		GroupValueInfo.GroupShortId = GroupShortID_DEPRECATED;
	}
	else
	{
		GroupValueInfo.GroupShortId = FAkAudioDevice::GetShortIDFromString(GroupName);
	}

	if (ShortID_DEPRECATED != AK_INVALID_UNIQUE_ID)
	{
		GroupValueInfo.WwiseShortId = ShortID_DEPRECATED;
	}
	else
	{
		GroupValueInfo.WwiseShortId = FAkAudioDevice::GetShortIDFromString(ValueName);
	}
	if (ID_DEPRECATED.IsValid())
	{
		GroupValueInfo.WwiseGuid = ID_DEPRECATED;
	}
	GroupValueInfo.WwiseName = FName(ValueName);
}

void UAkGroupValue::ValidateShortID(FWwiseObjectInfo& WwiseInfo) const
{
	if (WwiseInfo.WwiseShortId != AK_INVALID_UNIQUE_ID  && WwiseInfo.WwiseShortId != FAkAudioDevice::GetShortIDFromString(WwiseInfo.WwiseName.ToString()))
	{
		UE_LOG(LogAkAudio, Log, TEXT("UAkGroupValue::ValidateShortID: WwiseShortId does not match ShortID derived from WwiseName. Asset: %s - WwiseName %s - WwiseShortID: %" PRIu32 " instead of %" PRIu32 "."), *GetName(), *WwiseInfo.WwiseName.ToString(), WwiseInfo.WwiseShortId, FAkAudioDevice::GetShortIDFromString(WwiseInfo.WwiseName.ToString()));
	}

	if (WwiseInfo.WwiseShortId == AK_INVALID_UNIQUE_ID)
	{
		if (!WwiseInfo.WwiseName.ToString().IsEmpty())
		{
			WwiseInfo.WwiseShortId = FAkAudioDevice::GetShortIDFromString(WwiseInfo.WwiseName.ToString());
		}
		else
		{
			FString GroupName;
			FString ValueName;
			SplitAssetName( GroupName, ValueName);
			UE_LOG(LogAkAudio, Warning, TEXT("UAkGroupValue::ValidateShortID: Using ShortID based on asset name '%s'. Assumed value name is '%s'."), *GetName(), *ValueName);
			WwiseInfo.WwiseShortId = FAkAudioDevice::GetShortIDFromString(ValueName);
		}
	}

	if ( auto WwiseGroupInfo = static_cast<FWwiseGroupValueInfo*>(&WwiseInfo))
	{
		if (WwiseGroupInfo->GroupShortId == AK_INVALID_UNIQUE_ID)
		{
			FString GroupName;
			FString ValueName;
			SplitAssetName(GroupName, ValueName);
			UE_LOG(LogAkAudio, Warning, TEXT("UAkGroupValue::ValidateShortID: Using ShortID for group based on asset name '%s'. Assumed group name is '%s'."), *GetName(), *GroupName);
			WwiseGroupInfo->GroupShortId = FAkAudioDevice::GetShortIDFromString(GroupName);
		}
	}
}

bool UAkGroupValue::SplitAssetName(FString& OutGroupName, FString& OutValueName) const
{
	auto AssetName = GetName();
	if (AssetName.Contains(TEXT("-")))
	{
		AssetName.Split(TEXT("-"), &OutGroupName, &OutValueName);
		return true;
	}
	UE_LOG(LogAkAudio, Warning, TEXT("UAkAudioType::GetAssetSplitNames: Couldn't parse group and value names from asset named '%s'."), *GetName());
	return false;
}

#endif