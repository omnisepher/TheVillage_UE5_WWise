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

#include "Wwise/WwiseSoundEngine_Null.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/API_Null/WwiseCommAPI_Null.h"
#include "Wwise/API_Null/WwiseMemoryMgrAPI_Null.h"
#include "Wwise/API_Null/WwiseMonitorAPI_Null.h"
#include "Wwise/API_Null/WwiseMusicEngineAPI_Null.h"
#include "Wwise/API_Null/WwiseSoundEngineAPI_Null.h"
#include "Wwise/API_Null/WwiseSpatialAudioAPI_Null.h"
#include "Wwise/API_Null/WwiseStreamMgrAPI_Null.h"
#include "Wwise/API_Null/WwisePlatformAPI_Null.h"
#include "Wwise/API_Null/WAAPI_Null.h"

IWwiseCommAPI* FWwiseSoundEngine_Null::GetComm()
{
	return new FWwiseCommAPI_Null;
}

IWwiseMemoryMgrAPI* FWwiseSoundEngine_Null::GetMemoryMgr()
{
	return new FWwiseMemoryMgrAPI_Null;
}

IWwiseMonitorAPI* FWwiseSoundEngine_Null::GetMonitor()
{
	return new FWwiseMonitorAPI_Null;
}

IWwiseMusicEngineAPI* FWwiseSoundEngine_Null::GetMusicEngine()
{
	return new FWwiseMusicEngineAPI_Null;
}

IWwiseSoundEngineAPI* FWwiseSoundEngine_Null::GetSoundEngine()
{
	return new FWwiseSoundEngineAPI_Null;
}

IWwiseSpatialAudioAPI* FWwiseSoundEngine_Null::GetSpatialAudio()
{
	return new FWwiseSpatialAudioAPI_Null;
}

IWwiseStreamMgrAPI* FWwiseSoundEngine_Null::GetStreamMgr()
{
	return new FWwiseStreamMgrAPI_Null;
}

IWwisePlatformAPI* FWwiseSoundEngine_Null::GetPlatform()
{
	// Nulls are platform-independent.
	// return new FWwisePlatformAPI;
	return nullptr;
}

IWAAPI* FWwiseSoundEngine_Null::GetWAAPI()
{
#if AK_SUPPORT_WAAPI
	return new FWAAPI_Null;
#else
	return nullptr;
#endif
}
