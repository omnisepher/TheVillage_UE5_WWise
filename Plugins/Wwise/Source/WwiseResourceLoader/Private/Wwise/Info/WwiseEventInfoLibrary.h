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
#include "Wwise/Info/WwiseEventInfo.h"

#include "WwiseEventInfoLibrary.generated.h"

UCLASS()
class WWISERESOURCELOADER_API UWwiseEventInfoLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Wwise|EventInfo", Meta = (BlueprintThreadSafe, DisplayName = "Make EventInfo", AdvancedDisplay = "DestroyOptions, HardCodedSoundBankShortId"))
	static
	UPARAM(DisplayName="Event Info") FWwiseEventInfo
	MakeStruct(
		const FGuid& WwiseGuid,
		int32 WwiseShortId,
		const FString& WwiseName,
		EWwiseEventSwitchContainerLoading SwitchContainerLoading,
		EWwiseEventDestroyOptions DestroyOptions,
		int32 HardCodedSoundBankShortId = 0)
	{
		return FWwiseEventInfo(WwiseGuid, (uint32)WwiseShortId, FName(WwiseName), SwitchContainerLoading, DestroyOptions, (uint32)HardCodedSoundBankShortId);
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|EventInfo", Meta = (BlueprintThreadSafe, DisplayName = "Break EventInfo", AdvancedDisplay = "OutDestroyOptions, OutHardCodedSoundBankShortId"))
	static void
	BreakStruct(
		UPARAM(DisplayName="Event Info") FWwiseEventInfo Ref,
		FGuid& OutWwiseGuid,
		int32& OutWwiseShortId,
		FString& OutWwiseName,
		EWwiseEventSwitchContainerLoading& OutSwitchContainerLoading,
		EWwiseEventDestroyOptions& OutDestroyOptions,
		int32& OutHardCodedSoundBankShortId)
	{
		OutWwiseGuid = Ref.WwiseGuid;
		OutWwiseShortId = (int32)Ref.WwiseShortId;
		OutWwiseName = Ref.WwiseName.ToString();
		OutSwitchContainerLoading = Ref.SwitchContainerLoading;
		OutDestroyOptions = Ref.DestroyOptions;
		OutHardCodedSoundBankShortId = (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="GUID") FGuid
	GetWwiseGuid(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.WwiseGuid;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetWwiseShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return (int32)Ref.WwiseShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Name") FString
	GetWwiseName(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.WwiseName.ToString();
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Switch Container Loading") EWwiseEventSwitchContainerLoading
	GetSwitchContainerLoading(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.SwitchContainerLoading;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Destroy Options") EWwiseEventDestroyOptions
	GetDestroyOptions(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return Ref.DestroyOptions;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Short Id") int32
	GetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref)
	{
		return (int32)Ref.HardCodedSoundBankShortId;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetWwiseGuid(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const FGuid& WwiseGuid)
	{
		auto Result = Ref;
		Result.WwiseGuid = WwiseGuid;
		return Result;
	}
	
	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetWwiseShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		int32 WwiseShortId)
	{
		auto Result = Ref;
		Result.WwiseShortId = WwiseShortId;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetWwiseName(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const FString& WwiseName)
	{
		auto Result = Ref;
		Result.WwiseName = FName(WwiseName);
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetSwitchContainerLoading(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const EWwiseEventSwitchContainerLoading& SwitchContainerLoading)
	{
		auto Result = Ref;
		Result.SwitchContainerLoading = SwitchContainerLoading;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetDestroyOptions(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		const EWwiseEventDestroyOptions& DestroyOptions)
	{
		auto Result = Ref;
		Result.DestroyOptions = DestroyOptions;
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "Wwise|Event Info", Meta = (BlueprintThreadSafe))
	static
	UPARAM(DisplayName="Struct Out") FWwiseEventInfo
	SetHardCodedSoundBankShortId(
		UPARAM(DisplayName="Event Info") const FWwiseEventInfo& Ref,
		int32 HardCodedSoundBankShortId = 0)
	{
		auto Result = Ref;
		Result.HardCodedSoundBankShortId = (uint32)HardCodedSoundBankShortId;
		return Result;
	}
};
