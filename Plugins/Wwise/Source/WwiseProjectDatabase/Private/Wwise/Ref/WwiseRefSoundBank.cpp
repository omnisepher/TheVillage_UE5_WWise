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

#include "Wwise/Ref/WwiseRefSoundBank.h"

#include "Wwise/Ref/WwiseRefAcousticTexture.h"
#include "Wwise/Ref/WwiseRefAudioDevice.h"
#include "Wwise/Ref/WwiseRefAuxBus.h"
#include "Wwise/Ref/WwiseRefBus.h"
#include "Wwise/Ref/WwiseRefCustomPlugin.h"
#include "Wwise/Ref/WwiseRefDialogueArgument.h"
#include "Wwise/Ref/WwiseRefEvent.h"
#include "Wwise/Ref/WwiseRefExternalSource.h"
#include "Wwise/Ref/WwiseRefGameParameter.h"
#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/Ref/WwiseRefPluginShareSet.h"
#include "Wwise/Ref/WwiseRefState.h"
#include "Wwise/Ref/WwiseRefStateGroup.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Ref/WwiseRefSwitch.h"
#include "Wwise/Ref/WwiseRefSwitchGroup.h"
#include "Wwise/Ref/WwiseRefTrigger.h"

#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Metadata/WwiseMetadataSoundBanksInfo.h"
#include "Wwise/Stats/ProjectDatabase.h"


const TCHAR* const FWwiseRefSoundBank::NAME = TEXT("SoundBank");

const FWwiseMetadataSoundBank* FWwiseRefSoundBank::GetSoundBank() const
{
	const auto* SoundBanksInfo = GetSoundBanksInfo();
	if (UNLIKELY(!SoundBanksInfo))
	{
		return nullptr;
	}
	const auto& SoundBanks = SoundBanksInfo->SoundBanks;
	if (SoundBanks.IsValidIndex(SoundBankIndex))
	{
		return &SoundBanks[SoundBankIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get SoundBank index #%zu"), SoundBankIndex);
		return nullptr;
	}
}

WwiseMediaIdsMap FWwiseRefSoundBank::GetSoundBankMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& Media = SoundBank->Media;

	WwiseMediaIdsMap Result;
	Result.Empty(Media.Num());
	for (const auto& Elem : Media)
	{
		FWwiseDatabaseMediaIdKey Id(Elem.Id, SoundBank->Id);

		const auto* MediaInGlobalMap = GlobalMap.Find(Id);
		if (MediaInGlobalMap)
		{
			Result.Add(Elem.Id, *MediaInGlobalMap);
		}
	}
	return Result;
}

WwiseCustomPluginIdsMap FWwiseRefSoundBank::GetSoundBankCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank || !SoundBank->Plugins)
	{
		return {};
	}
	const auto& CustomPlugins = SoundBank->Plugins->Custom;

	WwiseCustomPluginIdsMap Result;
	Result.Empty(CustomPlugins.Num());
	for (const auto& Elem : CustomPlugins)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);

		const auto* InGlobalMap = GlobalMap.Find(Id);
		if (InGlobalMap)
		{
			Result.Add(Elem.Id, *InGlobalMap);
		}
	}
	return Result;
}

WwisePluginShareSetIdsMap FWwiseRefSoundBank::GetSoundBankPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank || !SoundBank->Plugins)
	{
		return {};
	}
	const auto& PluginShareSets = SoundBank->Plugins->ShareSets;

	WwisePluginShareSetIdsMap Result;
	Result.Empty(PluginShareSets.Num());
	for (const auto& Elem : PluginShareSets)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);

		const auto* InGlobalMap = GlobalMap.Find(Id);
		if (InGlobalMap)
		{
			Result.Add(Elem.Id, *InGlobalMap);
		}
	}
	return Result;
}

WwiseAudioDeviceIdsMap FWwiseRefSoundBank::GetSoundBankAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank || !SoundBank->Plugins)
	{
		return {};
	}
	const auto& AudioDevices = SoundBank->Plugins->AudioDevices;

	WwiseAudioDeviceIdsMap Result;
	Result.Empty(AudioDevices.Num());
	for (const auto& Elem : AudioDevices)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);

		const auto* InGlobalMap = GlobalMap.Find(Id);
		if (InGlobalMap)
		{
			Result.Add(Elem.Id, *InGlobalMap);
		}
	}
	return Result;
}

WwiseEventIdsMap FWwiseRefSoundBank::GetSoundBankEvents(const WwiseEventGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& Events = SoundBank->Events;

	WwiseEventIdsMap Result;
	Result.Empty(Events.Num());
	for (const auto& Elem : Events)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId, SoundBankId());

		const auto* InGlobalMap = GlobalMap.Find(Id);
		if (InGlobalMap)
		{
			Result.Add(Elem.Id, *InGlobalMap);
		}
	}
	return Result;
}

WwiseDialogueEventIdsMap FWwiseRefSoundBank::GetSoundBankDialogueEvents(const WwiseDialogueEventGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& DialogueEvents = SoundBank->DialogueEvents;
	WwiseDialogueEventIdsMap Result;
	Result.Empty(DialogueEvents.Num());
	for (const auto& Elem : DialogueEvents)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseDialogueArgumentIdsMap FWwiseRefSoundBank::GetAllSoundBankDialogueArguments(const WwiseDialogueArgumentGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto DialogueArguments = SoundBank->GetAllDialogueArguments();
	WwiseDialogueArgumentIdsMap Result;
	Result.Empty(DialogueArguments.Num());
	for (const auto& Elem : DialogueArguments)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseBusIdsMap FWwiseRefSoundBank::GetSoundBankBusses(const WwiseBusGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& Busses = SoundBank->Busses;
	WwiseBusIdsMap Result;
	Result.Empty(Busses.Num());
	for (const auto& Elem : Busses)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseAuxBusIdsMap FWwiseRefSoundBank::GetSoundBankAuxBusses(const WwiseAuxBusGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& AuxBusses = SoundBank->AuxBusses;
	WwiseAuxBusIdsMap Result;
	Result.Empty(AuxBusses.Num());
	for (const auto& Elem : AuxBusses)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseGameParameterIdsMap FWwiseRefSoundBank::GetSoundBankGameParameters(const WwiseGameParameterGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& GameParameters = SoundBank->GameParameters;
	WwiseGameParameterIdsMap Result;
	Result.Empty(GameParameters.Num());
	for (const auto& Elem : GameParameters)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseStateGroupIdsMap FWwiseRefSoundBank::GetSoundBankStateGroups(const WwiseStateGroupGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& StateGroups = SoundBank->StateGroups;
	WwiseStateGroupIdsMap Result;
	Result.Empty(StateGroups.Num());
	for (const auto& Elem : StateGroups)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseStateIdsMap FWwiseRefSoundBank::GetAllSoundBankStates(const WwiseStateGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto States = SoundBank->GetAllStates();
	WwiseStateIdsMap Result;
	Result.Empty(States.Num());
	for (const auto& Elem : States)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableGroupValueKey(Elem.Get<0>().Id, Elem.Get<1>().Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(FWwiseDatabaseGroupValueKey(Elem.Get<0>().Id, Elem.Get<1>().Id), *GlobalRef);
		}
	}

	return Result;
}

WwiseSwitchGroupIdsMap FWwiseRefSoundBank::GetSoundBankSwitchGroups(const WwiseSwitchGroupGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& SwitchGroups = SoundBank->SwitchGroups;
	WwiseSwitchGroupIdsMap Result;
	Result.Empty(SwitchGroups.Num());
	for (const auto& Elem : SwitchGroups)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseSwitchIdsMap FWwiseRefSoundBank::GetAllSoundBankSwitches(const WwiseSwitchGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto Switches = SoundBank->GetAllSwitches();
	WwiseSwitchIdsMap Result;
	Result.Empty(Switches.Num());
	for (const auto& Elem : Switches)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableGroupValueKey(Elem.Get<0>().Id, Elem.Get<1>().Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(FWwiseDatabaseGroupValueKey(Elem.Get<0>().Id, Elem.Get<1>().Id), *GlobalRef);
		}
	}

	return Result;
}

WwiseTriggerIdsMap FWwiseRefSoundBank::GetSoundBankTriggers(const WwiseTriggerGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& Triggers = SoundBank->Triggers;
	WwiseTriggerIdsMap Result;
	Result.Empty(Triggers.Num());
	for (const auto& Elem : Triggers)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseExternalSourceIdsMap FWwiseRefSoundBank::GetSoundBankExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& ExternalSources = SoundBank->ExternalSources;
	WwiseExternalSourceIdsMap Result;
	Result.Empty(ExternalSources.Num());
	for (const auto& Elem : ExternalSources)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Cookie, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Cookie, *GlobalRef);
		}
	}

	return Result;
}

WwiseAcousticTextureIdsMap FWwiseRefSoundBank::GetSoundBankAcousticTextures(const WwiseAcousticTextureGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return {};
	}
	const auto& AcousticTextures = SoundBank->AcousticTextures;
	WwiseAcousticTextureIdsMap Result;
	Result.Empty(AcousticTextures.Num());
	for (const auto& Elem : AcousticTextures)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Elem.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}

	return Result;
}

bool FWwiseRefSoundBank::IsUserBank() const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return false;
	}
	return SoundBank->Type == EMetadataSoundBankType::User;
}

bool FWwiseRefSoundBank::IsInitBank() const
{
	const auto* SoundBank = GetSoundBank();
	if (!SoundBank)
	{
		return false;
	}
	return SoundBank->IsInitBank();
}


uint32 FWwiseRefSoundBank::SoundBankId() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return 0;
	}
	return SoundBank->Id;
}

FGuid FWwiseRefSoundBank::SoundBankGuid() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return {};
	}
	return SoundBank->GUID;
}

FName FWwiseRefSoundBank::SoundBankShortName() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return {};
	}
	return SoundBank->ShortName;
}

FName FWwiseRefSoundBank::SoundBankObjectPath() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return {};
	}
	return SoundBank->ObjectPath;
}

uint32 FWwiseRefSoundBank::Hash() const
{
	auto Result = FWwiseRefSoundBanksInfo::Hash();
	Result = HashCombine(Result, GetTypeHash(SoundBankIndex));
	return Result;
}
