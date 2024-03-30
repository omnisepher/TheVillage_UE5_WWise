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

#include "Wwise/Metadata/WwiseMetadataAcousticTexture.h"
#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataCollections.h"
#include "Wwise/Metadata/WwiseMetadataDialogue.h"
#include "Wwise/Metadata/WwiseMetadataEvent.h"
#include "Wwise/Metadata/WwiseMetadataExternalSource.h"
#include "Wwise/Metadata/WwiseMetadataGameParameter.h"
#include "Wwise/Metadata/WwiseMetadataMedia.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"
#include "Wwise/Metadata/WwiseMetadataTrigger.h"

struct WWISEPROJECTDATABASE_API FWwiseMetadataSoundBankReference : public FWwiseMetadataLoadable
{
	uint32 Id;
	FGuid GUID;
	FName Language;

	FWwiseMetadataSoundBankReference(FWwiseMetadataLoader& Loader);
};

enum class EMetadataSoundBankType : uint32
{
	User = 0,
	Event = 30,
	Bus = 31,
	Unknown = (uint32)-1
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataSoundBankAttributes : public FWwiseMetadataSoundBankReference
{
	uint32 Align;
	bool bDeviceMemory;
	FGuid Hash;
	EMetadataSoundBankType Type;

	FWwiseMetadataSoundBankAttributes(FWwiseMetadataLoader& Loader);

private:
	static EMetadataSoundBankType TypeFromString(const FName& TypeString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataSoundBank : public FWwiseMetadataSoundBankAttributes
{
	FName ObjectPath;
	FName ShortName;
	FName Path;

	TArray<FWwiseMetadataMedia> Media;
	FWwiseMetadataPluginGroup* Plugins;
	TArray<FWwiseMetadataEvent> Events;
	TArray<FWwiseMetadataDialogueEvent> DialogueEvents;
	TArray<FWwiseMetadataBus> Busses;
	TArray<FWwiseMetadataBus> AuxBusses;
	TArray<FWwiseMetadataGameParameter> GameParameters;
	TArray<FWwiseMetadataStateGroup> StateGroups;
	TArray<FWwiseMetadataSwitchGroup> SwitchGroups;
	TArray<FWwiseMetadataTrigger> Triggers;
	TArray<FWwiseMetadataExternalSource> ExternalSources;
	TArray<FWwiseMetadataAcousticTexture> AcousticTextures;

	FWwiseMetadataSoundBank(FWwiseMetadataLoader& Loader);
	TSet<FWwiseMetadataDialogueArgument> GetAllDialogueArguments() const;
	TSet<WwiseMetadataStateWithGroup> GetAllStates() const;
	TSet<WwiseMetadataSwitchWithGroup> GetAllSwitches() const;
	bool IsInitBank() const
	{
		return bIsInitBank;
	}
	bool ContainsMedia() const
	{
		return Media.ContainsByPredicate([](const FWwiseMetadataMedia& MediaToTest)
		{
			return MediaToTest.Location == EWwiseMetadataMediaLocation::Memory;
		});
	}

protected:
	bool bIsInitBank;

};
