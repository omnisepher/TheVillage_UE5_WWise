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

#include "Wwise/Metadata/WwiseMetadataActionEntries.h"
#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataSwitchContainer.h"

enum class EWwiseMetadataEventDurationType : uint32
{
	OneShot = 0,
	Unknown = 1,
	Infinite = 2,
	Mixed = 3
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataEventReference : public FWwiseMetadataBasicReference
{
	float MaxAttenuation;
	EWwiseMetadataEventDurationType DurationType;
	float DurationMin;
	float DurationMax;

	FWwiseMetadataEventReference(FWwiseMetadataLoader& Loader);

private:
	static EWwiseMetadataEventDurationType DurationTypeFromString(const FName& TypeString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataEvent : public FWwiseMetadataEventReference
{
	TArray<FWwiseMetadataMediaReference> MediaRefs;
	TArray<FWwiseMetadataExternalSourceReference> ExternalSourceRefs;
	FWwiseMetadataPluginReferenceGroup* PluginRefs;
	TArray<FWwiseMetadataBusReference> AuxBusRefs;
	TArray<FWwiseMetadataSwitchContainer> SwitchContainers;
	TArray<FWwiseMetadataActionPostEventEntry> ActionPostEvent;
	TArray<FWwiseMetadataActionSetStateEntry> ActionSetState;
	TArray<FWwiseMetadataActionSetSwitchEntry> ActionSetSwitch;
	TArray<FWwiseMetadataActionTriggerEntry> ActionTrigger;
	TArray<FWwiseMetadataActionSetFXEntry> ActionSetFX; 

	FWwiseMetadataEvent(FWwiseMetadataLoader& Loader);

	bool IsMandatory() const;
};
