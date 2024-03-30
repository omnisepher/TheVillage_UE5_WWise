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

#include "Platforms/AkPlatform_Windows/AkWindowsInitializationSettings.h"
#include "AkAudioDevice.h"
#include "IHeadMountedDisplayModule.h"

#include "Wwise/API/WwisePlatformAPI.h"

//////////////////////////////////////////////////////////////////////////
// FAkWindowsAdvancedInitializationSettings

void FAkWindowsAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if PLATFORM_WINDOWS
	auto Platform = IWwisePlatformAPI::Get();
	if (UNLIKELY(!Platform))
	{
		return;
	}

	if (UseHeadMountedDisplayAudioDevice && IHeadMountedDisplayModule::IsAvailable())
	{
		FString AudioOutputDevice = IHeadMountedDisplayModule::Get().GetAudioOutputDevice();
		if (!AudioOutputDevice.IsEmpty())
			InitializationStructure.InitSettings.settingsMainOutput.idDevice = Platform->GetDeviceIDFromName((wchar_t*)*AudioOutputDevice);
	}
	InitializationStructure.PlatformInitSettings.uMaxSystemAudioObjects = MaxSystemAudioObjects;
#endif // PLATFORM_WINDOWS
}


//////////////////////////////////////////////////////////////////////////
// UAkWindowsInitializationSettings

UAkWindowsInitializationSettings::UAkWindowsInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAkWindowsInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
#if PLATFORM_64BITS
	#define AK_WINDOWS_ARCHITECTURE "x64_"
#else
	#define AK_WINDOWS_ARCHITECTURE "Win32_"
#endif

#ifdef AK_WINDOWS_VS_VERSION
	constexpr auto PlatformArchitecture = AK_WINDOWS_ARCHITECTURE AK_WINDOWS_VS_VERSION;
#else
	constexpr auto PlatformArchitecture = AK_WINDOWS_ARCHITECTURE "vc160";
#endif

#undef AK_WINDOWS_ARCHITECTURE

	InitializationStructure.SetPluginDllPath(PlatformArchitecture);

	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_WINDOWS
	InitializationStructure.PlatformInitSettings.uSampleRate = CommonSettings.SampleRate;
#endif
}
