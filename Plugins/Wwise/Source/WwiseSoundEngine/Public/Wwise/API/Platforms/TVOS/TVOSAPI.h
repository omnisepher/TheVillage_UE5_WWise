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

#if defined(PLATFORM_TVOS) && PLATFORM_TVOS

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

	/// Change the category and options of the app's AVAudioSession without restarting the entire Sound Engine.
	/// \remark
	/// As per Apple recommendations, the AVAudioSession will be deactivated and then reactivated. Therefore, 
	/// the primary output device must be reinitialized, which causes all audio to stop for a short period of time.
	/// For a seamless transition, call this API during moments of complete silence in your game.
	///
	/// \sa
	/// - \ref AkAudioSessionProperties
	virtual void ChangeAudioSessionProperties(
		const AkAudioSessionProperties &in_properties                 ///< New properties to apply to the app's AVAudioSession shared instance.
		) = 0;

	/// Get the motion device ID corresponding to a GCController player index. This device ID can be used to add/remove motion output for that gamepad.
	/// The player index is 0-based, and corresponds to a value of type GCControllerPlayerIndex in the Core.Haptics framework.
	/// \note The ID returned is unique to Wwise and does not correspond to any sensible value outside of Wwise.
	/// \return Unique device ID
	virtual AkDeviceID GetDeviceIDFromPlayerIndex(int playerIndex) = 0;
};

#endif
