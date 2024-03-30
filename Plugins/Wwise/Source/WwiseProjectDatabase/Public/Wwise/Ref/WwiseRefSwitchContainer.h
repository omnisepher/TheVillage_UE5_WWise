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

#include "Wwise/Ref/WwiseRefEvent.h"

class WWISEPROJECTDATABASE_API FWwiseRefSwitchContainer : public FWwiseRefEvent
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::SwitchContainer;

	TArray<WwiseRefIndexType> ChildrenIndices;

	FWwiseRefSwitchContainer() {}
	FWwiseRefSwitchContainer(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InEventIndex,
		const TArray<WwiseRefIndexType>& InChildrenIndices) :
		FWwiseRefEvent(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId, InEventIndex),
		ChildrenIndices(InChildrenIndices)
	{}
	const FWwiseMetadataSwitchContainer* GetSwitchContainer() const;
	FWwiseAnyRef GetSwitchValue(const WwiseSwitchGlobalIdsMap& SwitchGlobalMap, const WwiseStateGlobalIdsMap& StateGlobalMap) const;
	WwiseMediaIdsMap GetSwitchContainerMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseExternalSourceIdsMap GetSwitchContainerExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetSwitchContainerCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwisePluginShareSetIdsMap GetSwitchContainerPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetSwitchContainerAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;
	TArray<FWwiseAnyRef> GetSwitchValues(const WwiseSwitchGlobalIdsMap& SwitchGlobalMap, const WwiseStateGlobalIdsMap& StateGlobalMap) const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefSwitchContainer& Rhs) const
	{
		return FWwiseRefEvent::operator ==(Rhs)
			&& ChildrenIndices == Rhs.ChildrenIndices;
	}
	bool operator!=(const FWwiseRefSwitchContainer& Rhs) const { return !operator==(Rhs); }
};
