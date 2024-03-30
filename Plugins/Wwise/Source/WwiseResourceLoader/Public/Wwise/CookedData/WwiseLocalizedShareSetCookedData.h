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

#include "Wwise/CookedData/WwiseShareSetCookedData.h"
#include "Wwise/CookedData/WwiseLanguageCookedData.h"

#include "WwiseLocalizedShareSetCookedData.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseLocalizedShareSetCookedData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	TMap<FWwiseLanguageCookedData, FWwiseShareSetCookedData> ShareSetLanguageMap;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName DebugName;

	/**
	* @brief Short ID for the ShareSet.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 ShareSetId = 0;

	FWwiseLocalizedShareSetCookedData();

	void Serialize(FArchive& Ar);
};
