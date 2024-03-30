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

#include "Wwise/CookedData/WwiseExternalSourceCookedData.h"
#include "CoreMinimal.h"
#include "Wwise/WwiseFuture.h"

struct WWISERESOURCELOADER_API FWwiseLoadedExternalSourceInfo
{
	FWwiseLoadedExternalSourceInfo(const FWwiseExternalSourceCookedData& InExternalSource);
	FWwiseLoadedExternalSourceInfo& operator=(const FWwiseLoadedExternalSourceInfo&) = delete;

	const FWwiseExternalSourceCookedData ExternalSourceCookedData;

	struct WWISERESOURCELOADER_API FLoadedData
	{
		FLoadedData() {}
		FLoadedData(const FLoadedData&) = delete;
		FLoadedData& operator=(const FLoadedData&) = delete;

		bool bLoaded = false;
		int IsProcessing{0};

		bool IsLoaded() const;
	} LoadedData;

	FString GetDebugString() const;

private:
	friend class TDoubleLinkedList<FWwiseLoadedExternalSourceInfo>::TDoubleLinkedListNode;
	FWwiseLoadedExternalSourceInfo(const FWwiseLoadedExternalSourceInfo& InOriginal);
};

using FWwiseLoadedExternalSourceList = TDoubleLinkedList<FWwiseLoadedExternalSourceInfo>;
using FWwiseLoadedExternalSourceListNode = FWwiseLoadedExternalSourceList::TDoubleLinkedListNode;
using FWwiseLoadedExternalSourcePtr = FWwiseLoadedExternalSourceListNode*;
using FWwiseLoadedExternalSourcePtrAtomic = std::atomic<FWwiseLoadedExternalSourcePtr>;
using FWwiseLoadedExternalSourcePromise = TWwisePromise<FWwiseLoadedExternalSourcePtr>;
using FWwiseLoadedExternalSourceFuture = TWwiseFuture<FWwiseLoadedExternalSourcePtr>;
