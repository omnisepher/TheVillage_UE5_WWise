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

#pragma once

#include "AkInclude.h"
#include "InitializationSettings/AkInitializationSettings.h"
#include "InitializationSettings/AkPlatformInitialisationSettingsBase.h"

#include "AkWindowsInitializationSettings.generated.h"

USTRUCT()
struct FAkWindowsAdvancedInitializationSettings : public FAkAdvancedInitializationSettingsWithMultiCoreRendering
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings")
	bool UseHeadMountedDisplayAudioDevice = false;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Maximum number of System Audio Objects to reserve. Other processes will not be able to use them. Default is 128."))
	uint32 MaxSystemAudioObjects = 128;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


UCLASS(config = Game, defaultconfig)
class AKAUDIO_API UAkWindowsInitializationSettings : public UObject, public IAkPlatformInitialisationSettingsBase
{
	GENERATED_BODY()

public:
	virtual const TCHAR* GetConfigOverridePlatform() const override
	{
		return TEXT("Windows");
	}

	UAkWindowsInitializationSettings(const FObjectInitializer& ObjectInitializer);
	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const override;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommonInitializationSettingsWithSampleRate CommonSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommunicationSettingsWithSystemInitialization CommunicationSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", AdvancedDisplay)
	FAkWindowsAdvancedInitializationSettings AdvancedSettings;

	UFUNCTION()
	void MigrateMultiCoreRendering(bool NewValue)
	{
		AdvancedSettings.EnableMultiCoreRendering = NewValue;
	}
};
