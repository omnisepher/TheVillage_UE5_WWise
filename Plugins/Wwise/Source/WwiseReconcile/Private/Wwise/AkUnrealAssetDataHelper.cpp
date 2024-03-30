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

#include "AkUnrealAssetDataHelper.h"
#include "Wwise/WwiseReconcile.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkEffectShareSet.h"
#include "AkGroupValue.h"
#include "AkSettings.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "PackageTools.h"
#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"
#include "Wwise/Ref/WwiseAnyRef.h"
#include "Wwise/Ref/WwiseRefType.h"
#include "Wwise/Stats/Reconcile.h"

namespace AkUnrealAssetDataHelper
{
	bool IsSameType(const FAssetData& AssetData, EWwiseItemType::Type ItemType)
	{
		return GetUClassName(ItemType) == GetAssetClassName(AssetData);
	}

	FName GetUClassName(EWwiseItemType::Type ItemType)
	{
		UClass* Class = nullptr;
		switch (ItemType)
		{
		case EWwiseItemType::Event:
			Class = UAkAudioEvent::StaticClass();
			break;
		case EWwiseItemType::AuxBus:
			Class = UAkAuxBus::StaticClass();
			break;
		case EWwiseItemType::AcousticTexture:
			Class = UAkAcousticTexture::StaticClass();
			break;
		case EWwiseItemType::State:
			Class = UAkStateValue::StaticClass();
			break;
		case EWwiseItemType::Switch:
			Class = UAkSwitchValue::StaticClass();
			break;
		case EWwiseItemType::GameParameter:
			Class = UAkRtpc::StaticClass();
			break;
		case EWwiseItemType::Trigger:
			Class = UAkTrigger::StaticClass();
			break;
		case EWwiseItemType::EffectShareSet:
			Class = UAkEffectShareSet::StaticClass();
			break;
		}
		if (Class)
		{
#if UE_5_1_OR_LATER
			return Class->GetClassPathName().GetAssetName();
#else
			return Class->GetFName();
#endif
		}
		return FName();
	}

	FName GetAssetClassName(const FAssetData& AssetData)
	{
#if UE_5_1_OR_LATER
		return AssetData.AssetClassPath.GetAssetName();
#else
		return AssetData.AssetClass;
#endif
	}

	bool IsAssetAkAudioType(const FAssetData& AssetData)
	{
		auto GuidValue = AssetData.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseGuid));
		return GuidValue.IsSet();
	}

	bool IsAssetTransient(const FAssetData& AssetData)
	{
		return AssetData.PackagePath.ToString() == UPackageTools::SanitizePackageName(GetTransientPackage()->GetPathName());
	}

	void SetAssetClassName(FAssetData& AssetData, UClass* Class)
	{
#if UE_5_1_OR_LATER 
		AssetData.AssetClassPath = Class->GetClassPathName();
#else
		AssetData.AssetClass = Class->GetFName();
#endif
	}

	FString GetAssetDefaultPackagePath(const FAssetData& AssetData)
	{
		if (auto AkAudioAsset = Cast<UAkAudioType>(AssetData.GetAsset()))
		{
			return AkAudioAsset->GetAssetDefaultPackagePath().ToString();
		}

		return {};
	}

	FString GetAssetDefaultPackagePath(const FWwiseAnyRef* WwiseRef)
	{
		auto AkSettings = GetMutableDefault<UAkSettings>();
		if (!AkSettings)
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Could not fetch AkSettings"));
			return {};
		}

		FString DefaultPath = AkSettings->DefaultAssetCreationPath + WwiseRef->GetObjectPath().ToString();
		DefaultPath.ReplaceCharInline('\\', '/');
		int32 Index;
		if(DefaultPath.FindLastChar('/', Index))
		{
			DefaultPath = DefaultPath.Left(Index);
		}
		return UPackageTools::SanitizePackageName(DefaultPath);;
	}

	FName GetAssetDefaultName(const FAssetData& AssetData)
	{
		if (auto AkAudioAsset = Cast<UAkAudioType>(AssetData.GetAsset()))
		{
			return AkAudioAsset->GetAssetDefaultName();
		}

		return {};
	}

	FName GetAssetDefaultName(const FWwiseAnyRef* WwiseRef)
	{
		EWwiseRefType WwiseRefType = WwiseRef->GetType();
		FName WwiseName = WwiseRef->GetName();
		FNameBuilder DefaultName;

		switch (WwiseRefType)
		{
		case EWwiseRefType::AcousticTexture:
		case EWwiseRefType::AuxBus:
		case EWwiseRefType::Event:
		case EWwiseRefType::GameParameter:
		case EWwiseRefType::PluginShareSet:
		case EWwiseRefType::Trigger:
			return WwiseName;

		case EWwiseRefType::Switch:
			{
			FString GroupName = WwiseRef->GetSwitchGroup()->Name.ToString();
			DefaultName << GroupName << TEXT("-") << WwiseName;
#if UE_5_0_OR_LATER
			return FName(DefaultName.ToView());
#else
			return FName(DefaultName.ToString());
#endif
			}

		case EWwiseRefType::State:
			{
			FString GroupName = WwiseRef->GetStateGroup()->Name.ToString();
			DefaultName << GroupName << TEXT("-") << WwiseName;
#if UE_5_0_OR_LATER
			return FName(DefaultName.ToView());
#else
			return FName(DefaultName.ToString());
#endif
			}

		default:
			return {};
		}
	}
}
