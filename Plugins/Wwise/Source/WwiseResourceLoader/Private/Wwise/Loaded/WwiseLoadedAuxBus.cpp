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

#include "Wwise/Loaded/WwiseLoadedAuxBus.h"
#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>


FWwiseLoadedAuxBusInfo::FWwiseLoadedAuxBusInfo(const FWwiseLocalizedAuxBusCookedData& InAuxBus,
	const FWwiseLanguageCookedData& InLanguage):
	LocalizedAuxBusCookedData(InAuxBus),
	LanguageRef(InLanguage)
{}

bool FWwiseLoadedAuxBusInfo::FLoadedData::IsLoaded() const
{
	return LoadedSoundBanks.Num() > 0 || LoadedMedia.Num() > 0;
}

FString FWwiseLoadedAuxBusInfo::GetDebugString() const
{
	if (const auto* CookedData = LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LanguageRef))
	{
		return FString::Printf(TEXT("%s in language %s (%" PRIu32 ")"),
			*CookedData->GetDebugString(),
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
	else
	{
		return FString::Printf(TEXT("AuxBus %s (%" PRIu32 ") unset for language %s (%" PRIu32 ")"),
			*LocalizedAuxBusCookedData.DebugName.ToString(), LocalizedAuxBusCookedData.AuxBusId,
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
}

FWwiseLoadedAuxBusInfo::FWwiseLoadedAuxBusInfo(const FWwiseLoadedAuxBusInfo& InOriginal):
	LocalizedAuxBusCookedData(InOriginal.LocalizedAuxBusCookedData),
	LanguageRef(InOriginal.LanguageRef)
{
}
