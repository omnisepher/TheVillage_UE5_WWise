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

#include "Platforms/AkPlatform_tvOS/AkTVOSInitializationSettings.h"
#include "AkAudioDevice.h"
#include "InitializationSettings/AkAudioSession.h"
#include "WwiseDefines.h"

///////////////////////////////////////////////////////////////////////////
// UAkTVOSInitializationSettings

void FAkTVOSAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);
	
#if PLATFORM_TVOS
#if WWISE_2023_1_OR_LATER
	InitializationStructure.PlatformInitSettings.uNumSpatialAudioPointSources = uNumSpatialAudioPointSources;
	InitializationStructure.PlatformInitSettings.bVerboseSystemOutput = bVerboseSystemOutput;
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////
// UAkTVOSInitializationSettings

UAkTVOSInitializationSettings::UAkTVOSInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CommonSettings.MainOutputSettings.PanningRule = EAkPanningRule::Headphones;
	CommonSettings.MainOutputSettings.ChannelConfigType = EAkChannelConfigType::Standard;
	CommonSettings.MainOutputSettings.ChannelMask = AK_SPEAKER_SETUP_STEREO;
}

void UAkTVOSInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	AudioSession.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_TVOS
	InitializationStructure.PlatformInitSettings.uSampleRate = CommonSettings.SampleRate;
	// From FRunnableThreadApple
	InitializationStructure.DeviceSettings.threadProperties.uStackSize = 256 * 1024;
#endif
}
