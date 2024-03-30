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

#include "Wwise/API_Null/WwiseCommAPI_Null.h"
#include "Wwise/Stats/SoundEngine_Null.h"

AKRESULT FWwiseCommAPI_Null::Init(
	const AkCommSettings& in_settings
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseCommAPI_Null::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AkInt32 FWwiseCommAPI_Null::GetLastError()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return 0;
}

void FWwiseCommAPI_Null::GetDefaultInitSettings(
	AkCommSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseCommAPI_Null::Term()
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseCommAPI_Null::Term"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

AKRESULT FWwiseCommAPI_Null::Reset()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

const AkCommSettings& FWwiseCommAPI_Null::GetCurrentSettings()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	static const AkCommSettings StaticSettings;
	return StaticSettings;
}

AkUInt16 FWwiseCommAPI_Null::GetCommandPort()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return 0;
}
