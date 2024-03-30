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

#include "WwiseAcousticTextureCookedData.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseAcousticTextureCookedData
{
	GENERATED_BODY()

	// The Acoustic Texture's lower Absorption value. The percentage by which sound within a low frequency range is dampened.
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "AkTexture")
	float AbsorptionLow{ 0 };

	// The Acoustic Texture's mid-low Absorption value. The percentage by which sound within a mid-low frequency range is dampened.
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "AkTexture")
	float AbsorptionMidLow{ 0 };

	// The Acoustic Texture's mid-high Absorption value. The percentage by which sound within a mid-high frequency range is dampened.
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "AkTexture")
	float AbsorptionMidHigh{ 0 };

	// The Acoustic Texture's high Absorption value. The percentage by which sound within a high frequency range is dampened.
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "AkTexture")
	float AbsorptionHigh{ 0 };

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 ShortId{ 0 };

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName DebugName;

	FWwiseAcousticTextureCookedData();

	void Serialize(FArchive& Ar);

	FString GetDebugString() const;
};
