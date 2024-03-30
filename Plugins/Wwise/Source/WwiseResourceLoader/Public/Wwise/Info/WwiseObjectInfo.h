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

#include "WwiseObjectInfo.generated.h"

USTRUCT(BlueprintType, Meta = (Category = "Wwise", DisplayName = "Wwise Object Info", HasNativeMake = "/Script/WwiseResourceLoader.WwiseObjectInfoLibrary:MakeStruct", HasNativeBreak = "/Script/WwiseResourceLoader.WwiseObjectInfoLibrary:BreakStruct"))
struct WWISERESOURCELOADER_API FWwiseObjectInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Info")
	FGuid WwiseGuid;

	UPROPERTY(EditAnywhere, Category = "Info")
	uint32 WwiseShortId = 0;

	UPROPERTY(EditAnywhere, Category = "Info")
	FName WwiseName;

	UPROPERTY(EditAnywhere, Category = "Info")
	uint32 HardCodedSoundBankShortId = 0;

	FWwiseObjectInfo()
	{}

	FWwiseObjectInfo(const FWwiseObjectInfo& InWwiseObjectInfo) :
		WwiseGuid(InWwiseObjectInfo.WwiseGuid),
		WwiseShortId(InWwiseObjectInfo.WwiseShortId),
		WwiseName(InWwiseObjectInfo.WwiseName),
		HardCodedSoundBankShortId(InWwiseObjectInfo.HardCodedSoundBankShortId)
	{}

	FWwiseObjectInfo(const FGuid& InWwiseGuid, uint32 InWwiseShortId, const FName& InWwiseName, uint32 InHardCodedSoundBankShortId = 0) :
		WwiseGuid(InWwiseGuid),
		WwiseShortId(InWwiseShortId),
		WwiseName(InWwiseName),
		HardCodedSoundBankShortId(InHardCodedSoundBankShortId)
	{}

	FWwiseObjectInfo(const FGuid& InWwiseGuid, uint32 InWwiseShortId, const FString& InWwiseName, uint32 InHardCodedSoundBankShortId = 0) :
		WwiseGuid(InWwiseGuid),
		WwiseShortId(InWwiseShortId),
		WwiseName(FName(InWwiseName)),
		HardCodedSoundBankShortId(InHardCodedSoundBankShortId)
	{}

	FWwiseObjectInfo(uint32 InWwiseShortId, const FName& InWwiseName) :
		WwiseShortId(InWwiseShortId),
		WwiseName(InWwiseName)
	{}

	FWwiseObjectInfo(uint32 InWwiseShortId, const FString& InWwiseName) :
		WwiseShortId(InWwiseShortId),
		WwiseName(FName(InWwiseName))
	{}

	FWwiseObjectInfo(uint32 InWwiseShortId) :
		WwiseShortId(InWwiseShortId)
	{}

	static const FWwiseObjectInfo DefaultInitBank;

	bool operator==(const FWwiseObjectInfo& Rhs) const
	{
		return (!WwiseGuid.IsValid() || !Rhs.WwiseGuid.IsValid() || WwiseGuid == Rhs.WwiseGuid) &&
			(WwiseShortId == 0 || Rhs.WwiseShortId == 0 || WwiseShortId == Rhs.WwiseShortId) &&
			(WwiseName.IsNone() || Rhs.WwiseName.IsNone() || WwiseName == Rhs.WwiseName)
			&& HardCodedSoundBankShortId == Rhs.HardCodedSoundBankShortId;
	}

	bool operator!=(const FWwiseObjectInfo& Rhs) const
	{
		return !operator==(Rhs);
	}
};

inline uint32 GetTypeHash(const FWwiseObjectInfo& InValue)
{
	return HashCombine(HashCombine(HashCombine(
		GetTypeHash(InValue.WwiseGuid),
		GetTypeHash(InValue.WwiseShortId)),
		GetTypeHash(InValue.WwiseName)),
		GetTypeHash(InValue.HardCodedSoundBankShortId));
}
