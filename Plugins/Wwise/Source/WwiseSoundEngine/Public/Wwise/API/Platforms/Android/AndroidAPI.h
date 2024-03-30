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

#include "CoreMinimal.h"
#include "AkInclude.h"

#include "Wwise/WwiseSoundEngineModule.h"

#if defined(PLATFORM_ANDROID) && PLATFORM_ANDROID

class IWwisePlatformAPI
{
public:
	inline static IWwisePlatformAPI* Get()
	{
		IWwiseSoundEngineModule::ForceLoadModule();
		return IWwiseSoundEngineModule::Platform;
	}

	UE_NONCOPYABLE(IWwisePlatformAPI);

protected:
	IWwisePlatformAPI() = default;

public:
	virtual ~IWwisePlatformAPI() {}

	/// Get instance of OpenSL created by the sound engine at initialization.
	/// \return NULL if sound engine is not initialized
	virtual SLObjectItf GetWwiseOpenSLInterface() = 0;

	/// Gets specific settings for the fast audio path on Android.  Call this function after AK::SoundEngine::GetDefaultSettings and AK::SoundEngine::GetPlatformDefaultSettings to modify settings for the fast path.
	/// in_pfSettings.pJavaVM and in_pfSettings.jNativeActivity must be filled properly prior to calling GetFastPathSettings.
	/// The fast path constraints are:
	/// -The sample rate must match the hardware native sample rate 
	/// -The number of samples per frame must be a multiple of the hardware buffer size.
	/// Not fulfilling these constraints makes the audio hardware less efficient.
	/// In general, using the fast path means a higher CPU usage.  Complex audio designs may not be feasible while using the fast path.
	virtual AKRESULT GetFastPathSettings(AkInitSettings &in_settings, AkPlatformInitSettings &in_pfSettings) = 0;
};

#endif
