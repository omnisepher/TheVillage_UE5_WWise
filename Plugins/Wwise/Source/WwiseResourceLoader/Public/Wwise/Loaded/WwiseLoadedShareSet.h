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

#include "Wwise/CookedData/WwiseLocalizedShareSetCookedData.h"
#include "CoreMinimal.h"
#include "Wwise/WwiseFuture.h"

struct WWISERESOURCELOADER_API FWwiseLoadedShareSetInfo
{
	FWwiseLoadedShareSetInfo(const FWwiseLocalizedShareSetCookedData& InShareSet, const FWwiseLanguageCookedData& InLanguage);
	FWwiseLoadedShareSetInfo& operator=(const FWwiseLoadedShareSetInfo&) = delete;

	const FWwiseLocalizedShareSetCookedData LocalizedShareSetCookedData;
	FWwiseLanguageCookedData LanguageRef;

	struct WWISERESOURCELOADER_API FLoadedData
	{
		FLoadedData() {}
		FLoadedData(const FLoadedData&) = delete;
		FLoadedData& operator=(const FLoadedData&) = delete;

		TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
		TArray<const FWwiseMediaCookedData*> LoadedMedia;
		int IsProcessing{0};

		bool IsLoaded() const;
	} LoadedData;

	FString GetDebugString() const;

private:
	friend class TDoubleLinkedList<FWwiseLoadedShareSetInfo>::TDoubleLinkedListNode;
	FWwiseLoadedShareSetInfo(const FWwiseLoadedShareSetInfo& InOriginal);
};

using FWwiseLoadedShareSetList = TDoubleLinkedList<FWwiseLoadedShareSetInfo>;
using FWwiseLoadedShareSetListNode = FWwiseLoadedShareSetList::TDoubleLinkedListNode;
using FWwiseLoadedShareSetPtr = FWwiseLoadedShareSetListNode*;
using FWwiseLoadedShareSetPtrAtomic = std::atomic<FWwiseLoadedShareSetPtr>;
using FWwiseLoadedShareSetPromise = TWwisePromise<FWwiseLoadedShareSetPtr>;
using FWwiseLoadedShareSetFuture = TWwiseFuture<FWwiseLoadedShareSetPtr>;
