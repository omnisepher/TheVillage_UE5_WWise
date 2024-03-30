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

#include "Wwise/AudioLink/WwiseAudioLinkSettingsFactory.h"
#include "Wwise/AudioLink/WwiseAudioLinkSettings.h"
#include "AssetTypeCategories.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_WwiseAudioLinkSettings::GetName() const
{
	return LOCTEXT("AssetTypeActions_WwiseAudioLinkSettings", "Wwise AudioLink Settings");
}

FColor FAssetTypeActions_WwiseAudioLinkSettings::GetTypeColor() const
{
	return FColor(100, 100, 100);
}

const TArray<FText>& FAssetTypeActions_WwiseAudioLinkSettings::GetSubMenus() const
{
	static const TArray<FText> SubMenus
	{
		LOCTEXT("AssetAudioLinkSubMenu", "AudioLink")
	};

	return SubMenus;
}

UClass* FAssetTypeActions_WwiseAudioLinkSettings::GetSupportedClass() const
{
	return UWwiseAudioLinkSettings::StaticClass();
}

uint32 FAssetTypeActions_WwiseAudioLinkSettings::GetCategories()
{
	return EAssetTypeCategories::Sounds;
}

UWwiseAudioLinkSettingsFactory::UWwiseAudioLinkSettingsFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UWwiseAudioLinkSettings::StaticClass();

	bCreateNew = true;
	bEditorImport = true;
	bEditAfterNew = true;
}

UObject* UWwiseAudioLinkSettingsFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	return NewObject<UWwiseAudioLinkSettings>(InParent, Name, Flags);
}

uint32 UWwiseAudioLinkSettingsFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Sounds;
}
#undef LOCTEXT_NAMESPACE
