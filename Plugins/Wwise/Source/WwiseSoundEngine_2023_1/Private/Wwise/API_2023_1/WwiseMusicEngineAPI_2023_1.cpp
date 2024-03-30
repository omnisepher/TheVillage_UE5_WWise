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

#include "Wwise/API_2023_1/WwiseMusicEngineAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

AKRESULT FWwiseMusicEngineAPI_2023_1::Init(
	AkMusicSettings* in_pSettings
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseMusicEngineAPI_2023_1::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::MusicEngine::Init(in_pSettings);
}

void FWwiseMusicEngineAPI_2023_1::GetDefaultInitSettings(
	AkMusicSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::MusicEngine::GetDefaultInitSettings(out_settings);
}

void FWwiseMusicEngineAPI_2023_1::Term(
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseMusicEngineAPI_2023_1::Term"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::MusicEngine::Term();
}

AKRESULT FWwiseMusicEngineAPI_2023_1::GetPlayingSegmentInfo(
	AkPlayingID		in_PlayingID,
	AkSegmentInfo& out_segmentInfo,
	bool			in_bExtrapolate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::MusicEngine::GetPlayingSegmentInfo(in_PlayingID, out_segmentInfo, in_bExtrapolate);
}
