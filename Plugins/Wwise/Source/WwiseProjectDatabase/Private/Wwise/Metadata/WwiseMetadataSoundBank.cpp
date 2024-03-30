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

#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataCollections.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

FWwiseMetadataSoundBankReference::FWwiseMetadataSoundBankReference(FWwiseMetadataLoader& Loader) :
	Id(Loader.GetUint32(this, TEXT("Id"))),
	GUID(Loader.GetGuid(this, TEXT("GUID"))),
	Language(Loader.GetString(this, TEXT("Language")))
{
	Loader.LogParsed(TEXT("SoundBankReference"), Id);
}

FWwiseMetadataSoundBankAttributes::FWwiseMetadataSoundBankAttributes(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataSoundBankReference(Loader),
	Align(Loader.GetUint32(this, TEXT("Align"), EWwiseRequiredMetadata::Optional)),
	bDeviceMemory(Loader.GetBool(this, TEXT("DeviceMemory"), EWwiseRequiredMetadata::Optional)),
	Hash(Loader.GetGuid(this, TEXT("Hash"))),
	Type(TypeFromString(Loader.GetString(this, TEXT("Type"))))
{
	IncLoadedSize(sizeof(EMetadataSoundBankType));
	Loader.LogParsed(TEXT("SoundBankAttributes"), Id);
}

EMetadataSoundBankType FWwiseMetadataSoundBankAttributes::TypeFromString(const FName& TypeString)
{
	if (TypeString == "User")
	{
		return EMetadataSoundBankType::User;
	}
	else if (TypeString == "Event")
	{
		return EMetadataSoundBankType::Event;
	}
	else if (TypeString == "Bus")
	{
		return EMetadataSoundBankType::Bus;
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataSoundBankAttributes: Unknown Type: %s"), *TypeString.ToString());
		return EMetadataSoundBankType::Unknown;
	}
}

FWwiseMetadataSoundBank::FWwiseMetadataSoundBank(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataSoundBankAttributes(Loader),
	ObjectPath(Loader.GetString(this, TEXT("ObjectPath"))),
	ShortName(Loader.GetString(this, TEXT("ShortName"))),
	Path(Loader.GetString(this, TEXT("Path"))),
	Media(Loader.GetArray<FWwiseMetadataMedia>(this, TEXT("Media"))),
	Plugins(Loader.GetObjectPtr<FWwiseMetadataPluginGroup>(this, TEXT("Plugins"))),
	Events(Loader.GetArray<FWwiseMetadataEvent>(this, TEXT("Events"))),
	DialogueEvents(Loader.GetArray<FWwiseMetadataDialogueEvent>(this, TEXT("DialogueEvents"))),
	Busses(Loader.GetArray<FWwiseMetadataBus>(this, TEXT("Busses"))),
	AuxBusses(Loader.GetArray<FWwiseMetadataBus>(this, TEXT("AuxBusses"))),
	GameParameters(Loader.GetArray<FWwiseMetadataGameParameter>(this, TEXT("GameParameters"))),
	StateGroups(Loader.GetArray<FWwiseMetadataStateGroup>(this, TEXT("StateGroups"))),
	SwitchGroups(Loader.GetArray<FWwiseMetadataSwitchGroup>(this, TEXT("SwitchGroups"))),
	Triggers(Loader.GetArray<FWwiseMetadataTrigger>(this, TEXT("Triggers"))),
	ExternalSources(Loader.GetArray<FWwiseMetadataExternalSource>(this, TEXT("ExternalSources"))),
	AcousticTextures(Loader.GetArray<FWwiseMetadataAcousticTexture>(this, TEXT("AcousticTextures")))
{
	bIsInitBank = ShortName == TEXT("Init");
	Loader.LogParsed(TEXT("SoundBank"), Id);
}

TSet<FWwiseMetadataDialogueArgument> FWwiseMetadataSoundBank::GetAllDialogueArguments() const
{
	TSet<FWwiseMetadataDialogueArgument> Result;
	for (const auto& DialogueEvent : DialogueEvents)
	{
		Result.Append(DialogueEvent.Arguments);
	}
	return Result;
}

TSet<WwiseMetadataStateWithGroup> FWwiseMetadataSoundBank::GetAllStates() const
{
	TSet<WwiseMetadataStateWithGroup> Result;
	for (const auto& StateGroup : StateGroups)
	{
		for (const auto& State : StateGroup.States)
		{
			Result.Emplace(WwiseMetadataStateWithGroup(StateGroup, State), nullptr);
		}
	}
	return Result;
}

TSet<WwiseMetadataSwitchWithGroup> FWwiseMetadataSoundBank::GetAllSwitches() const
{
	TSet<WwiseMetadataSwitchWithGroup> Result;
	for (const auto& SwitchGroup : SwitchGroups)
	{
		for (const auto& Switch : SwitchGroup.Switches)
		{
			Result.Emplace(WwiseMetadataSwitchWithGroup(SwitchGroup, Switch), nullptr);
		}
	}
	return Result;
}
