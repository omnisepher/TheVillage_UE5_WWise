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

#include "CoreMinimal.h"
#include "Wwise/Info/WwiseObjectInfo.h"
#include "WwiseGroupValueInfo.generated.h"

USTRUCT(BlueprintType, Meta = (Category = "Wwise", DisplayName = "GroupValue Info", HasNativeMake = "/Script/WwiseResourceLoader.WwiseGroupValueInfoLibrary:MakeStruct", HasNativeBreak = "/Script/WwiseResourceLoader.WwiseGroupValueInfoLibrary:BreakStruct"))
struct WWISERESOURCELOADER_API FWwiseGroupValueInfo: public FWwiseObjectInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnyWhere, Category = "Info")
	uint32 GroupShortId = 0;
	
	FWwiseGroupValueInfo() :
		FWwiseObjectInfo(),
		GroupShortId(0)
	{}

	FWwiseGroupValueInfo(const FWwiseGroupValueInfo& InGroupValueInfo) :
		FWwiseObjectInfo(InGroupValueInfo),
		GroupShortId(InGroupValueInfo.GroupShortId)
	{}

	FWwiseGroupValueInfo(const FGuid& InWwiseGuid, uint32 InGroupShortId, uint32 InWwiseShortId, const FString& InWwiseName) :
		FWwiseObjectInfo(InWwiseGuid, InGroupShortId, InWwiseName),
		GroupShortId(InGroupShortId)
	{}
	
	FWwiseGroupValueInfo(const FGuid& InWwiseGuid, uint32 InGroupShortId, uint32 InWwiseShortId, const FName& InWwiseName) :
		FWwiseObjectInfo(InWwiseGuid, InGroupShortId, InWwiseName),
		GroupShortId(InGroupShortId)
	{}

	FWwiseGroupValueInfo(uint32 InGroupShortId, uint32 InWwiseShortId, const FString& InWwiseName) :
		FWwiseObjectInfo(InWwiseShortId, InWwiseName),
		GroupShortId(InGroupShortId)
	{}

	FWwiseGroupValueInfo(uint32 InGroupShortId, uint32 InWwiseShortId, const FName& InWwiseName) :
		FWwiseObjectInfo(InWwiseShortId, InWwiseName),
		GroupShortId(InGroupShortId)
	{}

	bool operator==(const FWwiseGroupValueInfo& Rhs) const
	{
		return (!WwiseGuid.IsValid() || !Rhs.WwiseGuid.IsValid() || WwiseGuid == Rhs.WwiseGuid) &&
			((GroupShortId == 0 && WwiseShortId == 0) || (Rhs.GroupShortId == 0 && Rhs.WwiseShortId == 0) || (GroupShortId == Rhs.GroupShortId && WwiseShortId == Rhs.WwiseShortId)) &&
			(WwiseName.IsNone() || Rhs.WwiseName.IsNone() || WwiseName == Rhs.WwiseName);
	}

	bool operator!=(const FWwiseGroupValueInfo& Rhs) const
	{
		return !operator==(Rhs);
	}
};

inline uint32 GetTypeHash(const FWwiseGroupValueInfo& InValue)
{
	return HashCombine(HashCombine(HashCombine(
		GetTypeHash(InValue.WwiseGuid),
		GetTypeHash(InValue.GroupShortId)),
		GetTypeHash(InValue.WwiseShortId)),
		GetTypeHash(InValue.WwiseName));
}
