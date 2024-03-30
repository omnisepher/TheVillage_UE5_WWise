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

#include "Wwise/Ref/WwiseRefEvent.h"

#include "Wwise/Ref/WwiseRefAudioDevice.h"
#include "Wwise/Ref/WwiseRefAuxBus.h"
#include "Wwise/Ref/WwiseRefCustomPlugin.h"
#include "Wwise/Ref/WwiseRefExternalSource.h"
#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/Ref/WwiseRefPluginShareSet.h"
#include "Wwise/Ref/WwiseRefState.h"
#include "Wwise/Ref/WwiseRefSwitch.h"
#include "Wwise/Ref/WwiseRefSwitchContainer.h"
#include "Wwise/Ref/WwiseRefTrigger.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataEvent.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"

#include "Wwise/Stats/ProjectDatabase.h"

#include <inttypes.h>

const TCHAR* const FWwiseRefEvent::NAME = TEXT("Event");

const FWwiseMetadataEvent* FWwiseRefEvent::GetEvent() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& Events = SoundBank->Events;
	if (Events.IsValidIndex(EventIndex))
	{
		return &Events[EventIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Event index #%zu"), EventIndex);
		return nullptr;
	}
}

WwiseMediaIdsMap FWwiseRefEvent::GetEventMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	const auto* Event = GetEvent();
	if (!Event || !SoundBank)
	{
		return {};
	}
	const auto& MediaRefs = Event->MediaRefs;
	WwiseMediaIdsMap Result;
	Result.Empty(MediaRefs.Num());
	for (const auto& Elem : MediaRefs)
	{
		FWwiseDatabaseMediaIdKey Id(Elem.Id, SoundBank->Id);
		const auto* GlobalRef = GlobalMap.Find(Id);
		if (GlobalRef)
		{
			Result.Add(Elem.Id, *GlobalRef);
		}
	}
	return Result;
}

WwiseMediaIdsMap FWwiseRefEvent::GetAllMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const
{
	const auto* SoundBank = GetSoundBank();
	const auto* Event = GetEvent();
	if (!Event || !SoundBank)
	{
		return {};
	}
	WwiseMediaIdsMap Result = GetEventMedia(GlobalMap);

	const auto& SwitchContainers = Event->SwitchContainers;
	for (const auto& SwitchContainer : SwitchContainers)
	{
		for (const auto& Elem : SwitchContainer.GetAllMedia())
		{
			FWwiseDatabaseMediaIdKey Id(Elem.Id, SoundBank->Id);
			const auto* GlobalRef = GlobalMap.Find(Id);
			if (GlobalRef)
			{
				Result.Add(Elem.Id, *GlobalRef);
			}
		}
	}

	if (Event->PluginRefs && SoundBank->Plugins)
	{
		const auto& CustomPlugins = Event->PluginRefs->ShareSets;
		for (const auto& CustomPlugin : CustomPlugins)
		{
			const auto PluginId = CustomPlugin.Id;
			const auto* Plugin = SoundBank->Plugins->Custom.FindByPredicate([PluginId](const FWwiseMetadataPlugin& RhsValue)
			{
				return RhsValue.Id == PluginId;
			});
			if (LIKELY(Plugin))
			{
				for (const auto& Elem : Plugin->MediaRefs)
				{
					FWwiseDatabaseMediaIdKey Id(Elem.Id, SoundBank->Id);
					const auto* GlobalRef = GlobalMap.Find(Id);
					if (GlobalRef)
					{
						Result.Add(Elem.Id, *GlobalRef);
					}
				}
			}
			else
			{
				UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Cannot find Plugin %" PRIu32), PluginId);
			}
		}

		const auto& PluginShareSets = Event->PluginRefs->ShareSets;
		for (const auto& PluginShareSet : PluginShareSets)
		{
			const auto PluginId = PluginShareSet.Id;
			const auto* Plugin = SoundBank->Plugins->ShareSets.FindByPredicate([PluginId](const FWwiseMetadataPlugin& RhsValue)
			{
				return RhsValue.Id == PluginId;
			});
			if (LIKELY(Plugin))
			{
				for (const auto& Elem : Plugin->MediaRefs)
				{
					FWwiseDatabaseMediaIdKey Id(Elem.Id, SoundBank->Id);
					const auto* GlobalRef = GlobalMap.Find(Id);
					if (GlobalRef)
					{
						Result.Add(Elem.Id, *GlobalRef);
					}
				}
			}
			else
			{
				UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Cannot find Plugin %" PRIu32), PluginId);
			}
		}

		const auto& AudioDevices = Event->PluginRefs->AudioDevices;
		for (const auto& AudioDevice : AudioDevices)
		{
			const auto PluginId = AudioDevice.Id;
			const auto* Plugin = SoundBank->Plugins->AudioDevices.FindByPredicate([PluginId](const FWwiseMetadataPlugin& RhsValue)
			{
				return RhsValue.Id == PluginId;
			});
			if (LIKELY(Plugin))
			{
				for (const auto& Elem : Plugin->MediaRefs)
				{
					FWwiseDatabaseMediaIdKey Id(Elem.Id, SoundBank->Id);
					const auto* GlobalRef = GlobalMap.Find(Id);
					if (GlobalRef)
					{
						Result.Add(Elem.Id, *GlobalRef);
					}
				}
			}
			else
			{
				UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Cannot find Plugin %" PRIu32), PluginId);
			}
		}
	}
	return Result;
}

WwiseExternalSourceIdsMap FWwiseRefEvent::GetEventExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	const auto& ExternalSourceRefs = Event->ExternalSourceRefs;
	WwiseExternalSourceIdsMap Result;
	Result.Empty(ExternalSourceRefs.Num());
	for (const auto& Elem : ExternalSourceRefs)
	{
		FWwiseDatabaseLocalizableIdKey Id(Elem.Cookie, LanguageId);
		const auto* GlobalRef = GlobalMap.Find(Id);
		if (GlobalRef)
		{
			Result.Add(Elem.Cookie, *GlobalRef);
		}
	}
	return Result;
}

WwiseExternalSourceIdsMap FWwiseRefEvent::GetAllExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	WwiseExternalSourceIdsMap Result = GetEventExternalSources(GlobalMap);

	const auto& SwitchContainers = Event->SwitchContainers;
	for (const auto& SwitchContainer : SwitchContainers)
	{
		for (const auto& Elem : SwitchContainer.GetAllExternalSources())
		{
			FWwiseDatabaseLocalizableIdKey Id(Elem.Cookie, LanguageId);
			const auto* GlobalRef = GlobalMap.Find(Id);
			if (GlobalRef)
			{
				Result.Add(Elem.Cookie, *GlobalRef);
			}
		}
	}
	return Result;
}

WwiseCustomPluginIdsMap FWwiseRefEvent::GetEventCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event || !Event->PluginRefs)
	{
		return {};
	}
	const auto& Plugins = Event->PluginRefs->Custom;
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

WwiseCustomPluginIdsMap FWwiseRefEvent::GetAllCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event || !Event->PluginRefs)
	{
		return {};
	}
	WwiseCustomPluginIdsMap Result = GetEventCustomPlugins(GlobalMap);

	const auto& SwitchContainers = Event->SwitchContainers;
	for (const auto& SwitchContainer : SwitchContainers)
	{
		for (const auto& Elem : SwitchContainer.GetAllCustomPlugins())
		{
			FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);
			const auto* GlobalRef = GlobalMap.Find(Id);
			if (GlobalRef)
			{
				Result.Add(Elem.Id, *GlobalRef);
			}
		}
	}
	return Result;
}

WwisePluginShareSetIdsMap FWwiseRefEvent::GetEventPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event || !Event->PluginRefs)
	{
		return {};
	}
	const auto& Plugins = Event->PluginRefs->ShareSets;
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

WwisePluginShareSetIdsMap FWwiseRefEvent::GetAllPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event || !Event->PluginRefs)
	{
		return {};
	}
	WwisePluginShareSetIdsMap Result = GetEventPluginShareSets(GlobalMap);

	const auto& SwitchContainers = Event->SwitchContainers;
	for (const auto& SwitchContainer : SwitchContainers)
	{
		for (const auto& Elem : SwitchContainer.GetAllPluginShareSets())
		{
			FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);
			const auto* GlobalRef = GlobalMap.Find(Id);
			if (GlobalRef)
			{
				Result.Add(Elem.Id, *GlobalRef);
			}
		}
	}
	return Result;
}

WwiseAudioDeviceIdsMap FWwiseRefEvent::GetEventAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event || !Event->PluginRefs)
	{
		return {};
	}
	const auto& Plugins = Event->PluginRefs->AudioDevices;
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

WwiseAudioDeviceIdsMap FWwiseRefEvent::GetAllAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event || !Event->PluginRefs)
	{
		return {};
	}
	WwiseAudioDeviceIdsMap Result = GetEventAudioDevices(GlobalMap);

	const auto& SwitchContainers = Event->SwitchContainers;
	for (const auto& SwitchContainer : SwitchContainers)
	{
		for (const auto& Elem : SwitchContainer.GetAllPluginShareSets())
		{
			FWwiseDatabaseLocalizableIdKey Id(Elem.Id, LanguageId);
			const auto* GlobalRef = GlobalMap.Find(Id);
			if (GlobalRef)
			{
				Result.Add(Elem.Id, *GlobalRef);
			}
		}
	}
	return Result;
}

WwiseSwitchContainerArray FWwiseRefEvent::GetSwitchContainers(const WwiseSwitchContainersByEvent& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	FWwiseDatabaseLocalizableIdKey LocId(Event->Id, LanguageId);

	WwiseSwitchContainerArray Result;
	GlobalMap.MultiFind(LocId, Result);
	return Result;
}

WwiseEventIdsMap FWwiseRefEvent::GetActionPostEvent(const WwiseEventGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	const auto& PostEvents = Event->ActionPostEvent;
	WwiseEventIdsMap Result;
	Result.Empty(PostEvents.Num());
	for (const auto& PostEvent : PostEvents)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(PostEvent.Id, LanguageId, SoundBankId()));
		if (GlobalRef)
		{
			Result.Add(PostEvent.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseStateIdsMap FWwiseRefEvent::GetActionSetState(const WwiseStateGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	const auto& SetStates = Event->ActionSetState;
	WwiseStateIdsMap Result;
	Result.Empty(SetStates.Num());
	for (const auto& SetState : SetStates)
	{
		const auto* StateRef = GlobalMap.Find(FWwiseDatabaseLocalizableGroupValueKey(SetState.GroupId, SetState.Id, LanguageId));
		if (StateRef)
		{
			const auto* State = StateRef->GetState();
			const auto* StateGroup = StateRef->GetStateGroup();
			if (State && StateGroup)
			{
				Result.Add(FWwiseDatabaseGroupValueKey(StateGroup->Id, State->Id), *StateRef);
			}
		}
	}

	return Result;
}

WwiseSwitchIdsMap FWwiseRefEvent::GetActionSetSwitch(const WwiseSwitchGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	const auto& SetSwitches = Event->ActionSetSwitch;
	WwiseSwitchIdsMap Result;
	Result.Empty(SetSwitches.Num());
	for (const auto& SetSwitch : SetSwitches)
	{
		const auto* SwitchRef = GlobalMap.Find(FWwiseDatabaseLocalizableGroupValueKey(SetSwitch.GroupId, SetSwitch.Id, LanguageId));
		if (SwitchRef)
		{
			const auto* Switch = SwitchRef->GetSwitch();
			const auto* SwitchGroup = SwitchRef->GetSwitchGroup();
			if (Switch && SwitchGroup)
			{
				Result.Add(FWwiseDatabaseGroupValueKey(SwitchGroup->Id, Switch->Id), *SwitchRef);
			}
		}
	}

	return Result;
}

WwiseTriggerIdsMap FWwiseRefEvent::GetActionTrigger(const WwiseTriggerGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	const auto& Triggers = Event->ActionTrigger;
	WwiseTriggerIdsMap Result;
	Result.Empty(Triggers.Num());
	for (const auto& Trigger : Triggers)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(Trigger.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(Trigger.Id, *GlobalRef);
		}
	}

	return Result;
}

WwiseAuxBusIdsMap FWwiseRefEvent::GetEventAuxBusses(const WwiseAuxBusGlobalIdsMap& GlobalMap) const
{
	const auto* Event = GetEvent();
	if (!Event)
	{
		return {};
	}
	const auto& AuxBusRefs = Event->AuxBusRefs;
	WwiseAuxBusIdsMap Result;
	Result.Empty(AuxBusRefs.Num());
	for (const auto& AuxBusRef : AuxBusRefs)
	{
		const auto* GlobalRef = GlobalMap.Find(FWwiseDatabaseLocalizableIdKey(AuxBusRef.Id, LanguageId));
		if (GlobalRef)
		{
			Result.Add(AuxBusRef.Id, *GlobalRef);
		}
	}

	return Result;
}

uint32 FWwiseRefEvent::EventId() const
{
	const auto* Event = GetEvent();
	if (UNLIKELY(!Event))
	{
		return {};
	}
	return Event->Id;
}

FGuid FWwiseRefEvent::EventGuid() const
{
	const auto* Event = GetEvent();
	if (UNLIKELY(!Event))
	{
		return {};
	}
	return Event->GUID;
}

FName FWwiseRefEvent::EventName() const
{
	const auto* Event = GetEvent();
	if (UNLIKELY(!Event))
	{
		return {};
	}
	return Event->Name;
}

FName FWwiseRefEvent::EventObjectPath() const
{
	const auto* Event = GetEvent();
	if (UNLIKELY(!Event))
	{
		return {};
	}
	return Event->ObjectPath;
}

float FWwiseRefEvent::MaxAttenuation() const
{
	const auto* Event = GetEvent();
	if (UNLIKELY(!Event))
	{
		return 0.f;
	}
	return Event->MaxAttenuation;
}

bool FWwiseRefEvent::GetDuration(EWwiseMetadataEventDurationType& OutDurationType, float& OutDurationMin, float& OutDurationMax) const
{
	const auto* Event = GetEvent();
	if (UNLIKELY(!Event))
	{
		return false;
	}

	OutDurationType = Event->DurationType;
	OutDurationMin = Event->DurationMin;
	OutDurationMax = Event->DurationMax;
	return true;
}

uint32 FWwiseRefEvent::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(EventIndex));
	return Result;
}
