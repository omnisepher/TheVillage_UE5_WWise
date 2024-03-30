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
#include "Wwise/Info/WwiseGroupValueInfo.h"

#include "WwiseGroupValueInfoLibrary.generated.h"

UCLASS()
class WWISERESOURCELOADER_API UWwiseGroupValueInfoLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValueInfo", Meta = (BlueprintThreadSafe, DisplayName = "Make GroupValueInfo"))
	static
	UPARAM(DisplayName="GroupValue Info") FWwiseGroupValueInfo
	MakeStruct(
		const FGuid& AssetGuid,
		int32 GroupShortId,
		int32 WwiseShortId,
		const FString& WwiseName)
	{
		return FWwiseGroupValueInfo(AssetGuid, (uint32)GroupShortId, (uint32)WwiseShortId, FName(WwiseName));
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValueInfo", Meta = (BlueprintThreadSafe, DisplayName = "Break GroupValueInfo"))
	static void
	BreakStruct(
		UPARAM(DisplayName="GroupValue Info") FWwiseGroupValueInfo Ref,
		FGuid& OutAssetGuid,
		int32& OutGroupShortId,
		int32& OutWwiseShortId,
		FString& OutWwiseName)
	{
		OutAssetGuid = Ref.WwiseGuid;
		OutGroupShortId = (int32)Ref.GroupShortId;
		OutWwiseShortId = (int32)Ref.WwiseShortId;
		OutWwiseName = Ref.WwiseName.ToString();
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="GUID") FGuid
	GetAssetGuid(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return Ref.WwiseGuid;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Group Short Id") int32
	GetGroupShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return (int32)Ref.GroupShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetWwiseShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return (int32)Ref.WwiseShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Name") FString
	GetWwiseName(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref)
	{
		return Ref.WwiseName.ToString();
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetAssetGuid(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		const FGuid& AssetGuid)
	{
		auto Result = Ref;
		Result.WwiseGuid = AssetGuid;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetGroupShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		int32 GroupShortId)
	{
		auto Result = Ref;
		Result.GroupShortId = (uint32)GroupShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetWwiseShortId(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		int32 WwiseShortId)
	{
		auto Result = Ref;
		Result.WwiseShortId = WwiseShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|GroupValue Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseGroupValueInfo
	SetWwiseName(
		UPARAM(DisplayName="GroupValue Info") const FWwiseGroupValueInfo& Ref,
		const FString& WwiseName)
	{
		auto Result = Ref;
		Result.WwiseName = FName(WwiseName);
		return Result;
	}
};
