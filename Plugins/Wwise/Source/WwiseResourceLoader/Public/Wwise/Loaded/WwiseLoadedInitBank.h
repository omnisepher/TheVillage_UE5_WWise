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

#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "CoreMinimal.h"
#include "Wwise/WwiseFuture.h"

struct WWISERESOURCELOADER_API FWwiseLoadedInitBankInfo
{
	FWwiseLoadedInitBankInfo(const FWwiseInitBankCookedData& InInitBank);
	FWwiseLoadedInitBankInfo& operator=(const FWwiseLoadedInitBankInfo&) = delete;

	const FWwiseInitBankCookedData InitBankCookedData;

	struct WWISERESOURCELOADER_API FLoadedData
	{
		FLoadedData() {}
		FLoadedData(const FLoadedData&) = delete;
		FLoadedData& operator=(const FLoadedData&) = delete;

		bool bLoaded = false;
		TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
		TArray<const FWwiseMediaCookedData*> LoadedMedia;
		int IsProcessing{0};

		bool IsLoaded() const;
	} LoadedData;

	FString GetDebugString() const;

private:
	friend class TDoubleLinkedList<FWwiseLoadedInitBankInfo>::TDoubleLinkedListNode;
	FWwiseLoadedInitBankInfo(const FWwiseLoadedInitBankInfo& InOriginal);
};

using FWwiseLoadedInitBankList = TDoubleLinkedList<FWwiseLoadedInitBankInfo>;
using FWwiseLoadedInitBankListNode = FWwiseLoadedInitBankList::TDoubleLinkedListNode;
using FWwiseLoadedInitBankPtr = FWwiseLoadedInitBankListNode*;
using FWwiseLoadedInitBankPtrAtomic = std::atomic<FWwiseLoadedInitBankPtr>;
using FWwiseLoadedInitBankPromise = TWwisePromise<FWwiseLoadedInitBankPtr>;
using FWwiseLoadedInitBankFuture = TWwiseFuture<FWwiseLoadedInitBankPtr>;
