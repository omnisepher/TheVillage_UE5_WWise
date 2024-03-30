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

#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefPluginShareSet : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::PluginShareSet;
	struct FGlobalIdsMap;

	WwiseRefIndexType PluginShareSetIndex;

	FWwiseRefPluginShareSet() {}
	FWwiseRefPluginShareSet(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InPluginShareSetIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		PluginShareSetIndex(InPluginShareSetIndex)
	{}
	const FWwiseMetadataPlugin* GetPlugin() const;
	WwiseMediaIdsMap GetPluginMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetPluginCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwisePluginShareSetIdsMap GetPluginPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetPluginAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;

	uint32 PluginShareSetId() const;
	FGuid PluginShareSetGuid() const;
	FName PluginShareSetName() const;
	FName PluginShareSetObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefPluginShareSet& Rhs) const
	{
		return FWwiseRefSoundBank::operator==(Rhs)
			&& PluginShareSetIndex == Rhs.PluginShareSetIndex;
	}
	bool operator!=(const FWwiseRefPluginShareSet& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefPluginShareSet::FGlobalIdsMap
{
	WwisePluginShareSetGlobalIdsMap GlobalIdsMap;
};
