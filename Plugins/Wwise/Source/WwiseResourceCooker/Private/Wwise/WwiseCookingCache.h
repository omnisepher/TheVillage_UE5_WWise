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

#include "Wwise/Info/WwiseEventInfo.h"
#include "Wwise/Info/WwiseGroupValueInfo.h"
#include "Wwise/CookedData/WwiseAcousticTextureCookedData.h"
#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSoundBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedShareSetCookedData.h"
#include "Wwise/CookedData/WwiseGameParameterCookedData.h"
#include "Wwise/CookedData/WwiseTriggerCookedData.h"

#include "Wwise/Info/WwiseObjectInfo.h"

#include "Wwise/WwiseDatabaseIdentifiers.h"

class IWwiseExternalSourceManager;

class WWISERESOURCECOOKER_API FWwiseCookingCache
{
public:
	FWwiseCookingCache() :
		ExternalSourceManager(nullptr)
	{}

	TMap<FString, FString> StagedFiles;
	TMap<FWwiseObjectInfo, FWwiseLocalizedAuxBusCookedData> AuxBusCache;
	TMap<FWwiseObjectInfo, FWwiseLocalizedSoundBankCookedData> SoundBankCache;
	TMap<FWwiseEventInfo, FWwiseLocalizedEventCookedData> EventCache;
	TMap<uint32, FWwiseExternalSourceCookedData> ExternalSourceCache;
	TMap<FWwiseObjectInfo, FWwiseInitBankCookedData> InitBankCache;
	TMap<FWwiseDatabaseMediaIdKey, FWwiseMediaCookedData> MediaCache;
	TMap<FWwiseObjectInfo, FWwiseLocalizedShareSetCookedData> ShareSetCache;
	TMap<FWwiseGroupValueInfo, FWwiseGroupValueCookedData> StateCache;
	TMap<FWwiseGroupValueInfo, FWwiseGroupValueCookedData> SwitchCache;
	TMap<FWwiseObjectInfo, FWwiseGameParameterCookedData> GameParameterCache;
	TMap<FWwiseObjectInfo, FWwiseAcousticTextureCookedData> AcousticTextureCache;
	TMap<FWwiseObjectInfo, FWwiseTriggerCookedData> TriggerCache;

	IWwiseExternalSourceManager* ExternalSourceManager;
};
