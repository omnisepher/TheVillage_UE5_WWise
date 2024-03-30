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

#include "Engine/EngineTypes.h"
#include "AkInclude.h"
#include "InitializationSettings/AkInitializationSettings.h"
#include "InitializationSettings/AkPlatformInitialisationSettingsBase.h"

#include "AkAndroidInitializationSettings.generated.h"

UENUM(Meta = (Bitmask))
enum class EAkAndroidAudioAPI : uint32
{
	AAudio,
	OpenSL_ES
};

USTRUCT()
struct FAkAndroidAdvancedInitializationSettings : public FAkAdvancedInitializationSettingsWithMultiCoreRendering
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkAndroidAudioAPI", ToolTip = "Main audio API to use. Leave set to \"Default\" for the default audio sink."))
	uint32 AudioAPI = (1 << (uint32)EAkAndroidAudioAPI::AAudio) | (1 << (uint32)EAkAndroidAudioAPI::OpenSL_ES);

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Used when hardware-preferred frame size and user-preferred frame size are not compatible. If true (default), the sound engine will initialize to a multiple of the HW setting, close to the user setting. If false, the user setting is used as is, regardless of the HW preference (might incur a performance hit)."))
	bool RoundFrameSizeToHardwareSize = true;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Use the lowest output latency possible for the current hardware. If true (default), the output audio device will be initialized in low-latency operation, allowing for more responsive audio playback on most devices. However, when operating in low-latency mode, some devices may have differences in audio reproduction. If false, the output audio device will be initialized without low-latency operation."))
	bool UseLowLatencyMode = true;	

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


UCLASS(config = Game, defaultconfig)
class AKAUDIO_API UAkAndroidInitializationSettings : public UObject, public IAkPlatformInitialisationSettingsBase
{
	GENERATED_BODY()

public:
	virtual const TCHAR* GetConfigOverridePlatform() const override
	{
		return TEXT("Android");
	}

	UAkAndroidInitializationSettings(const class FObjectInitializer& ObjectInitializer);

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const override;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommonInitializationSettingsWithSampleRate CommonSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommunicationSettingsWithSystemInitialization CommunicationSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", AdvancedDisplay)
	FAkAndroidAdvancedInitializationSettings AdvancedSettings;

	UFUNCTION()
	void MigrateMultiCoreRendering(bool NewValue) 
	{
		AdvancedSettings.EnableMultiCoreRendering = NewValue;
	}
};
