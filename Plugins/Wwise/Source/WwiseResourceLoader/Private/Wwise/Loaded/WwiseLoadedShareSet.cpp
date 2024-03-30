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

#include "Wwise/Loaded/WwiseLoadedShareSet.h"
#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>


FWwiseLoadedShareSetInfo::FWwiseLoadedShareSetInfo(const FWwiseLocalizedShareSetCookedData& InShareSet,
	const FWwiseLanguageCookedData& InLanguage):
	LocalizedShareSetCookedData(InShareSet),
	LanguageRef(InLanguage)
{}

bool FWwiseLoadedShareSetInfo::FLoadedData::IsLoaded() const
{
	return LoadedSoundBanks.Num() > 0 || LoadedMedia.Num() > 0;
}

FString FWwiseLoadedShareSetInfo::GetDebugString() const
{
	if (const auto* CookedData = LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LanguageRef))
	{
		return FString::Printf(TEXT("%s in language %s (%" PRIu32 ")"),
			*CookedData->GetDebugString(),
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
	else
	{
		return FString::Printf(TEXT("ShareSet %s (%" PRIu32 ") unset for language %s (%" PRIu32 ")"),
			*LocalizedShareSetCookedData.DebugName.ToString(), LocalizedShareSetCookedData.ShareSetId,
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
}

FWwiseLoadedShareSetInfo::FWwiseLoadedShareSetInfo(const FWwiseLoadedShareSetInfo& InOriginal):
	LocalizedShareSetCookedData(InOriginal.LocalizedShareSetCookedData),
	LanguageRef(InOriginal.LanguageRef)
{
}
