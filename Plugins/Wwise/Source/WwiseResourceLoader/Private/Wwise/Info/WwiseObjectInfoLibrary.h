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

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Wwise/Info/WwiseObjectInfo.h"

#include "WwiseObjectInfoLibrary.generated.h"

UCLASS()
class WWISERESOURCELOADER_API UWwiseObjectInfoLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Wwise|WwiseObjectInfo", Meta = (BlueprintThreadSafe, DisplayName = "Make WwiseObjectInfo", AdvancedDisplay = "HardCodedSoundBankShortId"))
	static
	UPARAM(DisplayName="Wwise Object Info") FWwiseObjectInfo
	MakeStruct(
		const FGuid& WwiseGuid,
		int32 WwiseShortId,
		const FString& WwiseName,
		int32 HardCodedSoundBankShortId = 0)
	{
		return FWwiseObjectInfo(WwiseGuid, (uint32)WwiseShortId, WwiseName, (uint32)HardCodedSoundBankShortId);
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|WwiseObjectInfo", Meta = (BlueprintThreadSafe, DisplayName = "Break WwiseObjectInfo", AdvancedDisplay = "OutHardCodedSoundBankShortId"))
	static void
	BreakStruct(
		UPARAM(DisplayName="Wwise Object Info") FWwiseObjectInfo Ref,
		FGuid& OutWwiseGuid,
		int32& OutWwiseShortId,
		FString& OutWwiseName,
		int32& OutHardCodedSoundBankShortId)
	{
		OutWwiseGuid = Ref.WwiseGuid;
		OutWwiseShortId = (int32)Ref.WwiseShortId;
		OutWwiseName = Ref.WwiseName.ToString();
		OutHardCodedSoundBankShortId = (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="GUID") FGuid
	GetWwiseGuid(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref)
	{
		return Ref.WwiseGuid;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetWwiseShortId(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref)
	{
		return (int32)Ref.WwiseShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Name") FString
	GetWwiseName(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref)
	{
		return Ref.WwiseName.ToString();
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref)
	{
		return (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseObjectInfo
	SetWwiseGuid(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref,
		const FGuid& WwiseGuid)
	{
		auto Result = Ref;
		Result.WwiseGuid = WwiseGuid;
		return Result;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseObjectInfo
	SetWwiseShortId(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref,
		int32 WwiseShortId)
	{
		auto Result = Ref;
		Result.WwiseShortId = WwiseShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseObjectInfo
	SetWwiseName(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref,
		const FString& WwiseName)
	{
		auto Result = Ref;
		Result.WwiseName = FName(WwiseName);
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Wwise Object Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseObjectInfo
	SetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Wwise Object Info") const FWwiseObjectInfo& Ref,
		int32 HardCodedSoundBankShortId = 0)
	{
		auto Result = Ref;
		Result.HardCodedSoundBankShortId = (uint32)HardCodedSoundBankShortId;
		return Result;
	}
};
