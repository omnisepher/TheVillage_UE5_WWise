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

#include "Platforms/AkPlatform_LinuxArm64/AkLinuxArm64InitializationSettings.h"
#include "AkAudioDevice.h"

//////////////////////////////////////////////////////////////////////////
// UAkLinuxInitializationSettings

UAkLinuxArm64InitializationSettings::UAkLinuxArm64InitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAkLinuxArm64InitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	InitializationStructure.SetPluginDllPath("Linux_aarch64");

	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if (defined(PLATFORM_LINUXARM64) && PLATFORM_LINUXARM64) || (defined(PLATFORM_LINUXAARCH64) && PLATFORM_LINUXAARCH64)
	InitializationStructure.PlatformInitSettings.uSampleRate = CommonSettings.SampleRate;
#endif
}
