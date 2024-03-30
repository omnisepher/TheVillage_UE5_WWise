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

class IWwiseStreamMgrAPI;
class IWwiseSpatialAudioAPI;
class IWwiseSoundEngineAPI;
class IWwiseMusicEngineAPI;
class IWwiseMonitorAPI;
class IWwiseMemoryMgrAPI;
class IWwiseCommAPI;
class IWwisePlatformAPI;
class IWAAPI;

class IWwiseSoundEngineVersionModule
{
public:
	virtual ~IWwiseSoundEngineVersionModule() {}

	virtual IWwiseCommAPI* GetComm() = 0;
	virtual IWwiseMemoryMgrAPI* GetMemoryMgr() = 0;
	virtual IWwiseMonitorAPI* GetMonitor() = 0;
	virtual IWwiseMusicEngineAPI* GetMusicEngine() = 0;
	virtual IWwiseSoundEngineAPI* GetSoundEngine() = 0;
	virtual IWwiseSpatialAudioAPI* GetSpatialAudio() = 0;
	virtual IWwiseStreamMgrAPI* GetStreamMgr() = 0;

	virtual IWwisePlatformAPI* GetPlatform() = 0;
	virtual IWAAPI* GetWAAPI() = 0;
};
