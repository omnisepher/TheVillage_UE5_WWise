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

#include "WwiseMediaCookedData.generated.h"

USTRUCT(BlueprintType)
struct WWISEFILEHANDLER_API FWwiseMediaCookedData
{
	GENERATED_BODY()

	/**
	 * @brief Short ID for the Media
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 MediaId = 0;

	/**
	 * @brief Path name relative to the platform's root.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName MediaPathName;

	/**
	 * @brief How many bytes need to be retrieved at load-time. Only set if streaming.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 PrefetchSize = 0;

	/**
	 * @brief Alignment required to load the asset on device. Can be 0 if no particular requirements.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 MemoryAlignment = 0;

	/**
	 * @brief True if the asset needs to be loaded in a special memory zone on the device.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	bool bDeviceMemory = false;

	/**
	 * @brief True if the asset should not be fully loaded in memory at load time.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	bool bStreaming = false;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName DebugName;

	FWwiseMediaCookedData();

	void Serialize(FArchive& Ar);

	FString GetDebugString() const;
};

inline uint32 GetTypeHash(const FWwiseMediaCookedData& InCookedData)
{
	return HashCombine(GetTypeHash(InCookedData.MediaId), GetTypeHash(InCookedData.MediaPathName));
}
inline bool operator==(const FWwiseMediaCookedData& InLhs, const FWwiseMediaCookedData& InRhs)
{
	return InLhs.MediaId == InRhs.MediaId && InLhs.MediaPathName == InRhs.MediaPathName;
}
inline bool operator!=(const FWwiseMediaCookedData& InLhs, const FWwiseMediaCookedData& InRhs)
{
	return InLhs.MediaId != InRhs.MediaId || InLhs.MediaPathName != InRhs.MediaPathName;
}
