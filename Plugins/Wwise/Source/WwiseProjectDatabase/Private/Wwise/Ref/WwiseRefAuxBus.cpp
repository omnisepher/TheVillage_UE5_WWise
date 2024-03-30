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

#include "Wwise/Ref/WwiseRefAuxBus.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Ref/WwiseRefAudioDevice.h"
#include "Wwise/Ref/WwiseRefCustomPlugin.h"
#include "Wwise/Ref/WwiseRefPluginShareSet.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include <inttypes.h>

const TCHAR* const FWwiseRefAuxBus::NAME = TEXT("AuxBus");

const FWwiseMetadataBus* FWwiseRefAuxBus::GetAuxBus() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& AuxBusses = SoundBank->AuxBusses;
	if (AuxBusses.IsValidIndex(AuxBusIndex))
	{
		return &AuxBusses[AuxBusIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get AuxBus index #%zu"), AuxBusIndex);
		return nullptr;
	}
}

void FWwiseRefAuxBus::GetAllAuxBusRefs(TSet<const FWwiseRefAuxBus*>& OutAuxBusRefs, const WwiseAuxBusGlobalIdsMap& InGlobalMap) const
{
	bool bIsAlreadyInSet = false;
	OutAuxBusRefs.Add(this, &bIsAlreadyInSet);
	if (UNLIKELY(bIsAlreadyInSet))		// Unlikely but can still be done (circular references are possible in Aux Busses)
	{
		return;
	}

	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return;
	}
	for (const auto& SubAuxBus : AuxBus->AuxBusRefs)
	{
		const auto* SubAuxBusRef = InGlobalMap.Find(FWwiseDatabaseLocalizableIdKey(SubAuxBus.Id, LanguageId));
		if (UNLIKELY(!SubAuxBusRef))
		{
			SubAuxBusRef = InGlobalMap.Find(FWwiseDatabaseLocalizableIdKey(SubAuxBus.Id, 0));
		}
		if (UNLIKELY(!SubAuxBusRef))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Aux Bus Id %" PRIu32), SubAuxBus.Id);
			continue;
		}
		SubAuxBusRef->GetAllAuxBusRefs(OutAuxBusRefs, InGlobalMap);
	}
}

WwiseCustomPluginIdsMap FWwiseRefAuxBus::GetAuxBusCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const
{
	const auto* AuxBus = GetAuxBus();
	if (!AuxBus || !AuxBus->PluginRefs)
	{
		return {};
	}
	const auto& Plugins = AuxBus->PluginRefs->Custom;
	WwiseCustomPluginIdsMap Result;
	Result.Empty(Plugins.Num());
	for (const auto& Elem : Plugins)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);
		const auto* GlobalRef = GlobalMap.Find(Id);
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}
	return Result;
}

WwisePluginShareSetIdsMap FWwiseRefAuxBus::GetAuxBusPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const
{
	const auto* AuxBus = GetAuxBus();
	if (!AuxBus || !AuxBus->PluginRefs)
	{
		return {};
	}
	const auto& Plugins = AuxBus->PluginRefs->ShareSets;
	WwisePluginShareSetIdsMap Result;
	Result.Empty(Plugins.Num());
	for (const auto& Elem : Plugins)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);
		const auto* GlobalRef = GlobalMap.Find(Id);
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}
	return Result;
}

WwiseAudioDeviceIdsMap FWwiseRefAuxBus::GetAuxBusAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const
{
	const auto* AuxBus = GetAuxBus();
	if (!AuxBus || !AuxBus->PluginRefs)
	{
		return {};
	}
	const auto& Plugins = AuxBus->PluginRefs->AudioDevices;
	WwiseAudioDeviceIdsMap Result;
	Result.Empty(Plugins.Num());
	for (const auto& Elem : Plugins)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);
		const auto* GlobalRef = GlobalMap.Find(Id);
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}
	return Result;
}

uint32 FWwiseRefAuxBus::AuxBusId() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return 0;
	}
	return AuxBus->Id;
}

FGuid FWwiseRefAuxBus::AuxBusGuid() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return {};
	}
	return AuxBus->GUID;
}

FName FWwiseRefAuxBus::AuxBusName() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return {};
	}
	return AuxBus->Name;
}

FName FWwiseRefAuxBus::AuxBusObjectPath() const
{
	const auto* AuxBus = GetAuxBus();
	if (UNLIKELY(!AuxBus))
	{
		return {};
	}
	return AuxBus->ObjectPath;
}

uint32 FWwiseRefAuxBus::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(AuxBusIndex));
	return Result;
}
