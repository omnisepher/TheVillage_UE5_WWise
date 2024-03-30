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

#include "Wwise/Loaded/WwiseLoadedEvent.h"
#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>


FWwiseLoadedEventInfo::FWwiseLoadedEventInfo(const FWwiseLocalizedEventCookedData& InEvent,
	const FWwiseLanguageCookedData& InLanguage):
	LocalizedEventCookedData(InEvent),
	LanguageRef(InLanguage)
{}

bool FWwiseLoadedEventInfo::FLoadedData::IsLoaded() const
{
	return bLoadedSwitchContainerLeaves || LoadedSoundBanks.Num() > 0 || LoadedExternalSources.Num() > 0 || LoadedMedia.Num() > 0 || LoadedRequiredGroupValues.Num() > 0;
}

FString FWwiseLoadedEventInfo::GetDebugString() const
{
	if (const auto* CookedData = LocalizedEventCookedData.EventLanguageMap.Find(LanguageRef))
	{
		return FString::Printf(TEXT("%s in language %s (%" PRIu32 ")"),
			*CookedData->GetDebugString(),
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
	else
	{
		return FString::Printf(TEXT("Event %s (%" PRIu32 ") unset for language %s (%" PRIu32 ")"),
			*LocalizedEventCookedData.DebugName.ToString(), LocalizedEventCookedData.EventId,
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
}

FWwiseLoadedEventInfo::FWwiseLoadedEventInfo(const FWwiseLoadedEventInfo& InOriginal):
	LocalizedEventCookedData(InOriginal.LocalizedEventCookedData),
	LanguageRef(InOriginal.LanguageRef)
{
}
