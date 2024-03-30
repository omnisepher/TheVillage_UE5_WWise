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

#include "Wwise/API_2023_1/Platforms/Windows/WindowsAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS && (!defined(PLATFORM_WINGDK) || !PLATFORM_WINGDK)
AkUInt32 FWwisePlatformAPI_2023_1_Windows::GetDeviceID(IMMDevice* in_pDevice)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::GetDeviceID(in_pDevice);
}

AkUInt32 FWwisePlatformAPI_2023_1_Windows::GetDeviceIDFromName(wchar_t* in_szToken)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::GetDeviceIDFromName(in_szToken);
}

const wchar_t* FWwisePlatformAPI_2023_1_Windows::GetWindowsDeviceName(AkInt32 index, AkUInt32& out_uDeviceID,
	AkAudioDeviceState uDeviceStateMask)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::GetWindowsDeviceName(index, out_uDeviceID, uDeviceStateMask);
}

AkUInt32 FWwisePlatformAPI_2023_1_Windows::GetWindowsDeviceCount(AkAudioDeviceState uDeviceStateMask)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::GetWindowsDeviceCount(uDeviceStateMask);
}

bool FWwisePlatformAPI_2023_1_Windows::GetWindowsDevice(AkInt32 in_index, AkUInt32& out_uDeviceID,
	IMMDevice** out_ppDevice, AkAudioDeviceState uDeviceStateMask)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::GetWindowsDevice(in_index, out_uDeviceID, out_ppDevice, uDeviceStateMask);
}
#endif

