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

#include "Wwise/API/Platforms/Android/AndroidAPI.h"

#if defined(PLATFORM_ANDROID) && PLATFORM_ANDROID

class FWwisePlatformAPI_2022_1_Android : public IWwisePlatformAPI
{
public:
	UE_NONCOPYABLE(FWwisePlatformAPI_2022_1_Android);
	FWwisePlatformAPI_2022_1_Android() = default;
	virtual ~FWwisePlatformAPI_2022_1_Android() override {}

	/// Get instance of OpenSL created by the sound engine at initialization.
	/// \return NULL if sound engine is not initialized
	virtual SLObjectItf GetWwiseOpenSLInterface() override;

	/// Gets specific settings for the fast audio path on Android.  Call this function after AK::SoundEngine::GetDefaultSettings and AK::SoundEngine::GetPlatformDefaultSettings to modify settings for the fast path.
	/// in_pfSettings.pJavaVM and in_pfSettings.jNativeActivity must be filled properly prior to calling GetFastPathSettings.
	/// The fast path constraints are:
	/// -The sample rate must match the hardware native sample rate 
	/// -The number of samples per frame must be a multiple of the hardware buffer size.
	/// Not fulfilling these constraints makes the audio hardware less efficient.
	/// In general, using the fast path means a higher CPU usage.  Complex audio designs may not be feasible while using the fast path.
	virtual AKRESULT GetFastPathSettings(AkInitSettings &in_settings, AkPlatformInitSettings &in_pfSettings) override;
};

using FWwisePlatformAPI = FWwisePlatformAPI_2022_1_Android;

#endif
