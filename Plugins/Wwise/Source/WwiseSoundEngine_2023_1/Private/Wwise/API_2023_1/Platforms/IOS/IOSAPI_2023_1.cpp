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

#include "Wwise/API_2023_1/Platforms/IOS/IOSAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

#if defined(PLATFORM_IOS) && PLATFORM_IOS && !(defined(PLATFORM_TVOS) && PLATFORM_TVOS)
void FWwisePlatformAPI_2023_1_IOS::ChangeAudioSessionProperties(
	const AkAudioSessionProperties &in_properties
	)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::iOS::ChangeAudioSessionProperties(in_properties);
}

AkDeviceID FWwisePlatformAPI_2023_1_IOS::GetDeviceIDFromPlayerIndex(int playerIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetDeviceIDFromPlayerIndex(playerIndex);
}
#endif
