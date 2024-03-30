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

#include "AkAssetFactories.h"

#include "AkAcousticTexture.h"
#include "AkAudioBank.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkRtpc.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkEffectShareSet.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "AssetToolsModule.h"

struct AkAssetFactory_Helper
{
	template<typename AkAssetType = UAkAudioType>
	static UObject* FactoryCreateNew(UClass* Class, UObject* InParent, const FName& Name, EObjectFlags Flags, FGuid AssetID = FGuid{}, uint32 ShortID = AK_INVALID_UNIQUE_ID, FString WwiseObjectName = "")
	{
		auto ContainingPath = InParent->GetName();

		auto NewWwiseObject = NewObject<AkAssetType>(InParent, Name, Flags);
		FWwiseObjectInfo* Info = NewWwiseObject->GetInfoMutable();
		Info->WwiseGuid = AssetID;
		if (WwiseObjectName.IsEmpty())
		{
			Info->WwiseName = Name;
		}
		else
		{
			Info->WwiseName = FName(WwiseObjectName);
		}
		if (ShortID == AK_INVALID_UNIQUE_ID)
		{
			Info->WwiseShortId = FAkAudioDevice::GetShortID(nullptr, Name.ToString());
		}
		else
		{
			Info->WwiseShortId = ShortID;
		}
		NewWwiseObject->MarkPackageDirty();
		if(NewWwiseObject->bAutoLoad)
		{
			NewWwiseObject->LoadData();
		}
		return NewWwiseObject;
	}

	template<typename AkAssetType = UAkGroupValue>
	static UObject* FactoryCreateNewGroupValue(UClass* Class, UObject* InParent, const FName& Name, EObjectFlags Flags, FGuid AssetID = FGuid{}, uint32 ShortID = AK_INVALID_UNIQUE_ID, FString WwiseObjectName = "")
	{
		auto ContainingPath = InParent->GetName();

		AkAssetType* NewStateValue = NewObject<AkAssetType>(InParent, Name, Flags);
		FWwiseGroupValueInfo* Info = static_cast<FWwiseGroupValueInfo*>(NewStateValue->GetInfoMutable());
		Info->WwiseGuid = AssetID;

		FString StringName = Name.ToString();
		FString ValueName = StringName;
		FString GroupName;
		if (StringName.Contains(TEXT("-")))
		{
			StringName.Split(TEXT("-"), &GroupName, &ValueName);
		}

		if (WwiseObjectName.IsEmpty() && ValueName.IsEmpty())
		{
			Info->WwiseName = FName(StringName);
		}
		else if (WwiseObjectName.IsEmpty())
		{
			Info->WwiseName = FName(ValueName);
		}
		else
		{
			Info->WwiseName = FName(WwiseObjectName);
		}


		if (ShortID == AK_INVALID_UNIQUE_ID)
		{
			Info->WwiseShortId = FAkAudioDevice::GetShortID(nullptr, Info->WwiseName.ToString());
		}
		else
		{
			Info->WwiseShortId = ShortID;
		}
		if (!GroupName.IsEmpty())
		{
			Info->GroupShortId = FAkAudioDevice::GetShortID(nullptr, GroupName);
		}
		else
		{
			Info->GroupShortId = AK_INVALID_UNIQUE_ID;
			UE_LOG(LogAkAudio, Warning, TEXT("FactoryCreateNewGroupValue: New Group Value asset '%s' in '%s' will have an invalid group ID, please set the group ID manually."), *StringName, *InParent->GetPathName());
		}
		NewStateValue->MarkPackageDirty();
		if(NewStateValue->bAutoLoad)
		{
			NewStateValue->LoadData();
		}
		return NewStateValue;
	}

	template<typename AkAssetType>
	static bool CanCreateNew()
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings)
		{
			return true;
		}
		return false;
	}

private:
	static FString ConvertAssetPathToWwisePath(FString ContainingPath, const FString& AssetName, const FString& BasePath)
	{
		ContainingPath.RemoveFromStart(BasePath, ESearchCase::IgnoreCase);
		ContainingPath.RemoveFromEnd(FString("/") + AssetName);
		return ContainingPath.Replace(TEXT("/"), TEXT("\\")).Replace(TEXT("_"), TEXT(" "));
	}
};

//////////////////////////////////////////////////////////////////////////
// UAkAcousticTextureFactory

UAkAcousticTextureFactory::UAkAcousticTextureFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAcousticTexture::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkAcousticTextureFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkAcousticTexture>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}

bool UAkAcousticTextureFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkAcousticTexture>();
}

//////////////////////////////////////////////////////////////////////////
// UAkAudioEventFactory

UAkAudioEventFactory::UAkAudioEventFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAudioEvent::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkAudioEventFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkAudioEvent>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}

bool UAkAudioEventFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkAudioEvent>();
}

//////////////////////////////////////////////////////////////////////////
// UAkAuxBusFactory

UAkAuxBusFactory::UAkAuxBusFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAuxBus::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkAuxBusFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkAuxBus>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}

bool UAkAuxBusFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkAuxBus>();
}

//////////////////////////////////////////////////////////////////////////
// UAkRtpcFactory

UAkRtpcFactory::UAkRtpcFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkRtpc::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkRtpcFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkRtpc>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}

bool UAkRtpcFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkRtpc>();
}

//////////////////////////////////////////////////////////////////////////
// UAkTriggerFactory

UAkTriggerFactory::UAkTriggerFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkTrigger::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkTriggerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkTrigger>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}

bool UAkTriggerFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkTrigger>();
}

//////////////////////////////////////////////////////////////////////////
// UAkStateValueFactory

UAkStateValueFactory::UAkStateValueFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkStateValue::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkStateValueFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNewGroupValue<UAkStateValue>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}

//////////////////////////////////////////////////////////////////////////
// UAkSwitchValueFactory

UAkSwitchValueFactory::UAkSwitchValueFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkSwitchValue::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkSwitchValueFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNewGroupValue<UAkSwitchValue>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}

//////////////////////////////////////////////////////////////////////////
// UAkEffectShareSetFactory

UAkEffectShareSetFactory::UAkEffectShareSetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkEffectShareSet::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkEffectShareSetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkEffectShareSet>(Class, InParent, Name, Flags, AssetID, ShortID, WwiseObjectName);
}