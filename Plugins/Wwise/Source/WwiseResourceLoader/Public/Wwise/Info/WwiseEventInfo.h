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

#include "WwiseObjectInfo.h"
#include "Wwise/CookedData/WwiseEventCookedData.h"

#include "WwiseEventInfo.generated.h"

UENUM(BlueprintType)
enum class EWwiseEventSwitchContainerLoading : uint8
{
	AlwaysLoad UMETA(DisplayName = "Always Load Media"),
	LoadOnReference UMETA(DisplayName = "Load Media Only When Referenced")
};

USTRUCT(BlueprintType, Meta = (Category = "Wwise", DisplayName = "Event Info", HasNativeMake = "/Script/WwiseResourceLoader.WwiseEventInfoLibrary:MakeStruct", HasNativeBreak = "/Script/WwiseResourceLoader.WwiseEventInfoLibrary:BreakStruct"))
struct WWISERESOURCELOADER_API FWwiseEventInfo: public FWwiseObjectInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Info")
	EWwiseEventSwitchContainerLoading SwitchContainerLoading = EWwiseEventSwitchContainerLoading::AlwaysLoad;

	UPROPERTY(EditAnywhere, Category = "Info")
	EWwiseEventDestroyOptions DestroyOptions = EWwiseEventDestroyOptions::StopEventOnDestroy;

	FWwiseEventInfo() :
		FWwiseObjectInfo(),
		SwitchContainerLoading(EWwiseEventSwitchContainerLoading::AlwaysLoad),
		DestroyOptions(EWwiseEventDestroyOptions::StopEventOnDestroy)
	{}

	FWwiseEventInfo(const FWwiseEventInfo& InEventInfo):
		FWwiseObjectInfo(InEventInfo),
		SwitchContainerLoading(InEventInfo.SwitchContainerLoading),
		DestroyOptions(InEventInfo.DestroyOptions)
	{}

	FWwiseEventInfo(
		const FGuid& InWwiseGuid,
		int32 InWwiseShortId,
		const FString& InWwiseName,
		EWwiseEventSwitchContainerLoading InSwitchContainerLoading = EWwiseEventSwitchContainerLoading::AlwaysLoad,
		EWwiseEventDestroyOptions InDestroyOptions = EWwiseEventDestroyOptions::StopEventOnDestroy,
		uint32 InHardCodedSoundBankShortId = 0) :
		FWwiseObjectInfo(InWwiseGuid, InWwiseShortId, InWwiseName),
		SwitchContainerLoading(InSwitchContainerLoading),
		DestroyOptions(InDestroyOptions)
	{}

	FWwiseEventInfo(
		const FGuid& InWwiseGuid,
		int32 InWwiseShortId,
		const FName& InWwiseName,
		EWwiseEventSwitchContainerLoading InSwitchContainerLoading = EWwiseEventSwitchContainerLoading::AlwaysLoad,
		EWwiseEventDestroyOptions InDestroyOptions = EWwiseEventDestroyOptions::StopEventOnDestroy,
		uint32 InHardCodedSoundBankShortId = 0) :
		FWwiseObjectInfo(InWwiseGuid, InWwiseShortId, InWwiseName),
		SwitchContainerLoading(InSwitchContainerLoading),
		DestroyOptions(InDestroyOptions)
	{}

	FWwiseEventInfo(uint32 InWwiseShortId, const FString& InWwiseName) :
		FWwiseObjectInfo(InWwiseShortId, InWwiseName),
		SwitchContainerLoading(EWwiseEventSwitchContainerLoading::AlwaysLoad),
		DestroyOptions(EWwiseEventDestroyOptions::StopEventOnDestroy)
	{}

	FWwiseEventInfo(uint32 InWwiseShortId, const FName& InWwiseName) :
		FWwiseObjectInfo(InWwiseShortId, InWwiseName),
		SwitchContainerLoading(EWwiseEventSwitchContainerLoading::AlwaysLoad),
		DestroyOptions(EWwiseEventDestroyOptions::StopEventOnDestroy)
	{}

	FWwiseEventInfo(uint32 InWwiseShortId) :
		FWwiseObjectInfo(InWwiseShortId),
		SwitchContainerLoading(EWwiseEventSwitchContainerLoading::AlwaysLoad),
		DestroyOptions(EWwiseEventDestroyOptions::StopEventOnDestroy)
	{}
};

inline uint32 GetTypeHash(const FWwiseEventInfo& InValue)
{
	return HashCombine(HashCombine(HashCombine(
		GetTypeHash(InValue.WwiseGuid),
		GetTypeHash(InValue.WwiseShortId)),
		GetTypeHash(InValue.WwiseName)),
		GetTypeHash(InValue.HardCodedSoundBankShortId));
}