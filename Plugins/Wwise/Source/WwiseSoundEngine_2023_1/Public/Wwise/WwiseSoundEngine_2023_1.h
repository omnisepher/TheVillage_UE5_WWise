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

#include "CoreTypes.h"

#include "Wwise/WwiseSoundEngineVersionModule.h"

class WWISESOUNDENGINE_API FWwiseSoundEngine_2023_1 : public IWwiseSoundEngineVersionModule
{
public:
	virtual IWwiseCommAPI* GetComm() override;
	virtual IWwiseMemoryMgrAPI* GetMemoryMgr() override;
	virtual IWwiseMonitorAPI* GetMonitor() override;
	virtual IWwiseMusicEngineAPI* GetMusicEngine() override;
	virtual IWwiseSoundEngineAPI* GetSoundEngine() override;
	virtual IWwiseSpatialAudioAPI* GetSpatialAudio() override;
	virtual IWwiseStreamMgrAPI* GetStreamMgr() override;
	virtual IWwisePlatformAPI* GetPlatform() override;
	virtual IWAAPI* GetWAAPI() override;
};
