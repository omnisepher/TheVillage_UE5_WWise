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

#include "Platforms/AkPlatform_Linux/AkLinuxInitializationSettings.h"
#include "AkAudioDevice.h"

//////////////////////////////////////////////////////////////////////////
// UAkLinuxInitializationSettings

UAkLinuxInitializationSettings::UAkLinuxInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAkLinuxInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
#if PLATFORM_64BITS
	InitializationStructure.SetPluginDllPath("Linux_x64");
#else
	ensure(!"The Wwise Unreal Engine integration does not support 32-bit Linux distributions.");
#endif

	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_LINUX && (!defined(PLATFORM_LINUXARM64) || !PLATFORM_LINUXARM64) && (!defined(PLATFORM_LINUXAARCH64) || !PLATFORM_LINUXAARCH64)
	InitializationStructure.PlatformInitSettings.uSampleRate = CommonSettings.SampleRate;
#endif
}
