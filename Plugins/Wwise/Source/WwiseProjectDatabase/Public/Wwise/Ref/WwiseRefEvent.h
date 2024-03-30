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

#include "Wwise/Metadata/WwiseMetadataEvent.h"
#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefEvent : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Event;
	struct FGlobalIdsMap;

	WwiseRefIndexType EventIndex;

	FWwiseRefEvent() {}
	FWwiseRefEvent(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InEventIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		EventIndex(InEventIndex)
	{}
	const FWwiseMetadataEvent* GetEvent() const;
	WwiseMediaIdsMap GetEventMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseMediaIdsMap GetAllMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseExternalSourceIdsMap GetEventExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const;
	WwiseExternalSourceIdsMap GetAllExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetEventCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetAllCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwisePluginShareSetIdsMap GetEventPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const;
	WwisePluginShareSetIdsMap GetAllPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetEventAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetAllAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;
	WwiseSwitchContainerArray GetSwitchContainers(const WwiseSwitchContainersByEvent& GlobalMap) const;
	WwiseEventIdsMap GetActionPostEvent(const WwiseEventGlobalIdsMap& GlobalMap) const;
	WwiseStateIdsMap GetActionSetState(const WwiseStateGlobalIdsMap& GlobalMap) const;
	WwiseSwitchIdsMap GetActionSetSwitch(const WwiseSwitchGlobalIdsMap& GlobalMap) const;
	WwiseTriggerIdsMap GetActionTrigger(const WwiseTriggerGlobalIdsMap& GlobalMap) const;
	WwiseAuxBusIdsMap GetEventAuxBusses(const WwiseAuxBusGlobalIdsMap& GlobalMap) const;

	uint32 EventId() const;
	FGuid EventGuid() const;
	FName EventName() const;
	FName EventObjectPath() const;
	float MaxAttenuation() const;
	bool GetDuration(EWwiseMetadataEventDurationType& OutDurationType, float& OutDurationMin, float& OutDurationMax) const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefEvent& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& EventIndex == Rhs.EventIndex;
	}
	bool operator!=(const FWwiseRefEvent& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefEvent::FGlobalIdsMap
{
	WwiseEventGlobalIdsMap GlobalIdsMap;
};
