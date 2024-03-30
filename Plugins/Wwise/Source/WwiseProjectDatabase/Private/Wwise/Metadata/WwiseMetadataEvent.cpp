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

#include "Wwise/Metadata/WwiseMetadataEvent.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataEventReference::FWwiseMetadataEventReference(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader),
	MaxAttenuation(Loader.GetFloat(this, TEXT("MaxAttenuation"), EWwiseRequiredMetadata::Optional)),
	DurationType(DurationTypeFromString(Loader.GetString(this, TEXT("DurationType")))),
	DurationMin(Loader.GetFloat(this, TEXT("DurationMin"), EWwiseRequiredMetadata::Optional)),
	DurationMax(Loader.GetFloat(this, TEXT("DurationMax"), EWwiseRequiredMetadata::Optional))
{
	IncLoadedSize(sizeof(EWwiseMetadataEventDurationType));
	Loader.LogParsed(TEXT("EventReference"), Id, Name);
}

EWwiseMetadataEventDurationType FWwiseMetadataEventReference::DurationTypeFromString(const FName& TypeString)
{
	if (TypeString == "OneShot")
	{
		return EWwiseMetadataEventDurationType::OneShot;
	}
	else if (TypeString == "Infinite")
	{
		return EWwiseMetadataEventDurationType::Infinite;
	}
	else if (TypeString == "Mixed")
	{
		return EWwiseMetadataEventDurationType::Mixed;
	}
	else if (!(TypeString == "Unknown"))
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataEventReference: Unknown DurationType: %s"), *TypeString.ToString());
	}
	return EWwiseMetadataEventDurationType::Unknown;
}

FWwiseMetadataEvent::FWwiseMetadataEvent(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataEventReference(Loader),
	MediaRefs(Loader.GetArray<FWwiseMetadataMediaReference>(this, TEXT("MediaRefs"))),
	ExternalSourceRefs(Loader.GetArray<FWwiseMetadataExternalSourceReference>(this, TEXT("ExternalSourceRefs"))),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs"))),
	AuxBusRefs(Loader.GetArray<FWwiseMetadataBusReference>(this, TEXT("AuxBusRefs"))),
	SwitchContainers(Loader.GetArray<FWwiseMetadataSwitchContainer>(this, TEXT("SwitchContainers"))),
	ActionPostEvent(Loader.GetArray<FWwiseMetadataActionPostEventEntry>(this, TEXT("ActionPostEvent"))),
	ActionSetState(Loader.GetArray<FWwiseMetadataActionSetStateEntry>(this, TEXT("ActionSetState"))),
	ActionSetSwitch(Loader.GetArray<FWwiseMetadataActionSetSwitchEntry>(this, TEXT("ActionSetSwitch"))),
	ActionTrigger(Loader.GetArray<FWwiseMetadataActionTriggerEntry>(this, TEXT("ActionTrigger"))),
	ActionSetFX(Loader.GetArray<FWwiseMetadataActionSetFXEntry>(this, TEXT("ActionSetFX")))
{
	Loader.LogParsed(TEXT("Event"), Id, Name);
}

bool FWwiseMetadataEvent::IsMandatory() const
{
	return
		(ActionPostEvent.Num() > 0)
		|| (ActionSetState.Num() > 0)
		|| (ActionSetSwitch.Num() > 0)
		|| (ActionTrigger.Num() > 0)
		|| (ActionSetFX.Num() > 0)
		|| (AuxBusRefs.Num() > 0)
		|| (ExternalSourceRefs.Num() > 0)
		|| (MediaRefs.Num() > 0)
		|| (PluginRefs && (
			(PluginRefs->Custom.Num() > 0)
			|| (PluginRefs->ShareSets.Num() > 0)
			|| (PluginRefs->AudioDevices.Num() > 0)))
		|| (SwitchContainers.Num() == 0);
}