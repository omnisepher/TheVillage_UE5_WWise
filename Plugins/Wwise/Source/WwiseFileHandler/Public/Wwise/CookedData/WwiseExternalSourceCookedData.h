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

#include "WwiseExternalSourceCookedData.generated.h"

USTRUCT(BlueprintType)
struct WWISEFILEHANDLER_API FWwiseExternalSourceCookedData
{
	GENERATED_BODY()

	/**
	 * @brief User-defined Cookie for the External Source
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 Cookie = 0;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName DebugName;

	FWwiseExternalSourceCookedData();

	void Serialize(FArchive& Ar);

	FString GetDebugString() const;
};

inline uint32 GetTypeHash(const FWwiseExternalSourceCookedData& InCookedData)
{
	return GetTypeHash(InCookedData.Cookie);
}
inline bool operator==(const FWwiseExternalSourceCookedData& InLhs, const FWwiseExternalSourceCookedData& InRhs)
{
	return InLhs.Cookie == InRhs.Cookie;
}
inline bool operator!=(const FWwiseExternalSourceCookedData& InLhs, const FWwiseExternalSourceCookedData& InRhs)
{
	return InLhs.Cookie != InRhs.Cookie;
}
