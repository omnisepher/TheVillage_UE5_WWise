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
#include "InitializationSettings/AkAudioSession.h"
#include "InitializationSettings/AkPlatformInitialisationSettingsBase.h"

#include "AkTVOSInitializationSettings.generated.h"

USTRUCT()
struct FAkTVOSAdvancedInitializationSettings : public FAkAdvancedInitializationSettingsWithMultiCoreRendering
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Number of Apple Spatial Audio point sources to allocate for 3D audio use (each point source is a system audio object)."))
	uint32 uNumSpatialAudioPointSources = 128;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Print detailed system output information to the system log."))
	bool bVerboseSystemOutput = false;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};

UCLASS(config = Game, defaultconfig)
class AKAUDIO_API UAkTVOSInitializationSettings : public UObject, public IAkPlatformInitialisationSettingsBase
{
	GENERATED_BODY()

public:
	virtual const TCHAR* GetConfigOverridePlatform() const override
	{
		return TEXT("TVOS");
	}

	UAkTVOSInitializationSettings(const FObjectInitializer& ObjectInitializer);
	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const override;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommonInitializationSettingsWithSampleRate CommonSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkAudioSession AudioSession;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommunicationSettingsWithSystemInitialization CommunicationSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", AdvancedDisplay)
	FAkTVOSAdvancedInitializationSettings AdvancedSettings;
};
