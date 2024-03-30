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

#include "Wwise/WwiseSoundEngine_2023_1.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/API_2023_1/WwiseCommAPI_2023_1.h"
#include "Wwise/API_2023_1/WwiseMemoryMgrAPI_2023_1.h"
#include "Wwise/API_2023_1/WwiseMonitorAPI_2023_1.h"
#include "Wwise/API_2023_1/WwiseMusicEngineAPI_2023_1.h"
#include "Wwise/API_2023_1/WwiseSoundEngineAPI_2023_1.h"
#include "Wwise/API_2023_1/WwiseSpatialAudioAPI_2023_1.h"
#include "Wwise/API_2023_1/WwiseStreamMgrAPI_2023_1.h"
#include "Wwise/API_2023_1/WwisePlatformAPI_2023_1.h"
#include "Wwise/API_2023_1/WAAPI_2023_1.h"

IWwiseCommAPI* FWwiseSoundEngine_2023_1::GetComm()
{
	return new FWwiseCommAPI_2023_1;
}

IWwiseMemoryMgrAPI* FWwiseSoundEngine_2023_1::GetMemoryMgr()
{
	return new FWwiseMemoryMgrAPI_2023_1;
}

IWwiseMonitorAPI* FWwiseSoundEngine_2023_1::GetMonitor()
{
	return new FWwiseMonitorAPI_2023_1;
}

IWwiseMusicEngineAPI* FWwiseSoundEngine_2023_1::GetMusicEngine()
{
	return new FWwiseMusicEngineAPI_2023_1;
}

IWwiseSoundEngineAPI* FWwiseSoundEngine_2023_1::GetSoundEngine()
{
	return new FWwiseSoundEngineAPI_2023_1;
}

IWwiseSpatialAudioAPI* FWwiseSoundEngine_2023_1::GetSpatialAudio()
{
	return new FWwiseSpatialAudioAPI_2023_1;
}

IWwiseStreamMgrAPI* FWwiseSoundEngine_2023_1::GetStreamMgr()
{
	return new FWwiseStreamMgrAPI_2023_1;
}

IWwisePlatformAPI* FWwiseSoundEngine_2023_1::GetPlatform()
{
	return new FWwisePlatformAPI;
}

IWAAPI* FWwiseSoundEngine_2023_1::GetWAAPI()
{
#if AK_SUPPORT_WAAPI
	return new FWAAPI_2023_1;
#else
	return nullptr;
#endif
}
