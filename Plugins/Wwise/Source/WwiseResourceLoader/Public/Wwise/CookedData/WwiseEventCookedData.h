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

#include "Wwise/CookedData/WwiseSwitchContainerLeafCookedData.h"

#include "WwiseEventCookedData.generated.h"


UENUM(BlueprintType)
enum class EWwiseEventDestroyOptions : uint8
{
	StopEventOnDestroy,
	WaitForEventEnd
};

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseEventCookedData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 EventId = 0;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	TArray<FWwiseSoundBankCookedData> SoundBanks;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	TArray<FWwiseMediaCookedData> Media;
	
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	TArray<FWwiseExternalSourceCookedData> ExternalSources;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	TArray<FWwiseSwitchContainerLeafCookedData> SwitchContainerLeaves;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	TSet<FWwiseGroupValueCookedData> RequiredGroupValueSet;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	EWwiseEventDestroyOptions DestroyOptions = EWwiseEventDestroyOptions::StopEventOnDestroy;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName DebugName;

	FWwiseEventCookedData();

	void Serialize(FArchive& Ar);

	FString GetDebugString() const;
};
