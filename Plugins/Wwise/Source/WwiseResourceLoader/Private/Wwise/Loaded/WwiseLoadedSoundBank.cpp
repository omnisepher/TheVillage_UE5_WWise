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

#include "Wwise/Loaded/WwiseLoadedSoundBank.h"
#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>


FWwiseLoadedSoundBankInfo::FWwiseLoadedSoundBankInfo(const FWwiseLocalizedSoundBankCookedData& InSoundBank,
	const FWwiseLanguageCookedData& InLanguage):
	LocalizedSoundBankCookedData(InSoundBank),
	LanguageRef(InLanguage)
{}

bool FWwiseLoadedSoundBankInfo::FLoadedData::IsLoaded() const
{
	return bLoaded;
}

FString FWwiseLoadedSoundBankInfo::GetDebugString() const
{
	if (const auto* CookedData = LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LanguageRef))
	{
		return FString::Printf(TEXT("%s in language %s (%" PRIu32 ")"),
			*CookedData->GetDebugString(),
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
	else
	{
		return FString::Printf(TEXT("SoundBank %s (%" PRIu32 ") unset for language %s (%" PRIu32 ")"),
			*LocalizedSoundBankCookedData.DebugName.ToString(), LocalizedSoundBankCookedData.SoundBankId,
			*LanguageRef.LanguageName.ToString(), LanguageRef.LanguageId);
	}
}

FWwiseLoadedSoundBankInfo::FWwiseLoadedSoundBankInfo(const FWwiseLoadedSoundBankInfo& InOriginal):
	LocalizedSoundBankCookedData(InOriginal.LocalizedSoundBankCookedData),
	LanguageRef(InOriginal.LanguageRef)
{
}
