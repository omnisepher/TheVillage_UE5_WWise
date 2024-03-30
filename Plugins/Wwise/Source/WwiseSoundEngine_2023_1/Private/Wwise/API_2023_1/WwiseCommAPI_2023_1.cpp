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

#include "Wwise/API_2023_1/WwiseCommAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

AKRESULT FWwiseCommAPI_2023_1::Init(
	const AkCommSettings& in_settings
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseCommAPI_2023_1::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_ENABLE_COMMUNICATION
	return AK::Comm::Init(in_settings);
#else
	return AK_NotImplemented;
#endif
}

AkInt32 FWwiseCommAPI_2023_1::GetLastError()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return 0;
}

void FWwiseCommAPI_2023_1::GetDefaultInitSettings(
	AkCommSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_ENABLE_COMMUNICATION
	AK::Comm::GetDefaultInitSettings(out_settings);
#else
	return;
#endif
}

void FWwiseCommAPI_2023_1::Term()
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseCommAPI_2023_1::Term"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_ENABLE_COMMUNICATION
	AK::Comm::Term();
#else
	return;
#endif
}

AKRESULT FWwiseCommAPI_2023_1::Reset()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_ENABLE_COMMUNICATION
	return AK::Comm::Reset();
#else
	return AK_NotImplemented;
#endif
}

const AkCommSettings& FWwiseCommAPI_2023_1::GetCurrentSettings()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_ENABLE_COMMUNICATION
	return AK::Comm::GetCurrentSettings();
#else
	static const AkCommSettings StaticSettings;
	return StaticSettings;
#endif
}

AkUInt16 FWwiseCommAPI_2023_1::GetCommandPort()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_ENABLE_COMMUNICATION
	return AK::Comm::GetCommandPort();
#else
	return 0;
#endif
}
