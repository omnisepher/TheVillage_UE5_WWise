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

#include "Platforms/AkPlatform_Mac/AkMacInitializationSettings.h"
#include "WwiseDefines.h"
#include "AkAudioDevice.h"

///////////////////////////////////////////////////////////////////////////
// UAkMacInitializationSettings

void FAkMacAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);
	
#if PLATFORM_MAC
#if WWISE_2023_1_OR_LATER
	InitializationStructure.PlatformInitSettings.uNumSpatialAudioPointSources = uNumSpatialAudioPointSources;
	InitializationStructure.PlatformInitSettings.bVerboseSystemOutput = bVerboseSystemOutput;
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////
// UAkMacInitializationSettings

UAkMacInitializationSettings::UAkMacInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAkMacInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	InitializationStructure.SetPluginDllPath("Mac_Xcode1400");

	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_MAC
	InitializationStructure.PlatformInitSettings.uSampleRate = CommonSettings.SampleRate;
	// From FRunnableThreadMac
	InitializationStructure.DeviceSettings.threadProperties.uStackSize = 4 * 1024 * 1024;
#endif
}
