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

#include "Wwise/Ref/WwiseRefSoundBanksInfo.h"

class WWISEPROJECTDATABASE_API FWwiseRefSoundBank : public FWwiseRefSoundBanksInfo
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::SoundBank;
	struct WWISEPROJECTDATABASE_API FGlobalIdsMap;

	WwiseRefIndexType SoundBankIndex;
	uint32 LanguageId;

	FWwiseRefSoundBank() {}
	FWwiseRefSoundBank(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId) :
		FWwiseRefSoundBanksInfo(InRootMediaRef, InJsonFilePath),
		SoundBankIndex(InSoundBankIndex),
		LanguageId(InLanguageId)
	{}
	const FWwiseMetadataSoundBank* GetSoundBank() const;
	WwiseMediaIdsMap GetSoundBankMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const;
	WwiseCustomPluginIdsMap GetSoundBankCustomPlugins(const WwiseCustomPluginGlobalIdsMap& GlobalMap) const;
	WwisePluginShareSetIdsMap GetSoundBankPluginShareSets(const WwisePluginShareSetGlobalIdsMap& GlobalMap) const;
	WwiseAudioDeviceIdsMap GetSoundBankAudioDevices(const WwiseAudioDeviceGlobalIdsMap& GlobalMap) const;
	WwiseEventIdsMap GetSoundBankEvents(const WwiseEventGlobalIdsMap& GlobalMap) const;
	WwiseDialogueEventIdsMap GetSoundBankDialogueEvents(const WwiseDialogueEventGlobalIdsMap& GlobalMap) const;
	WwiseDialogueArgumentIdsMap GetAllSoundBankDialogueArguments(const WwiseDialogueArgumentGlobalIdsMap& GlobalMap) const;
	WwiseBusIdsMap GetSoundBankBusses(const WwiseBusGlobalIdsMap& GlobalMap) const;
	WwiseAuxBusIdsMap GetSoundBankAuxBusses(const WwiseAuxBusGlobalIdsMap& GlobalMap) const;
	WwiseGameParameterIdsMap GetSoundBankGameParameters(const WwiseGameParameterGlobalIdsMap& GlobalMap) const;
	WwiseStateGroupIdsMap GetSoundBankStateGroups(const WwiseStateGroupGlobalIdsMap& GlobalMap) const;
	WwiseStateIdsMap GetAllSoundBankStates(const WwiseStateGlobalIdsMap& GlobalMap) const;
	WwiseSwitchGroupIdsMap GetSoundBankSwitchGroups(const WwiseSwitchGroupGlobalIdsMap& GlobalMap) const;
	WwiseSwitchIdsMap GetAllSoundBankSwitches(const WwiseSwitchGlobalIdsMap& GlobalMap) const;
	WwiseTriggerIdsMap GetSoundBankTriggers(const WwiseTriggerGlobalIdsMap& GlobalMap) const;
	WwiseExternalSourceIdsMap GetSoundBankExternalSources(const WwiseExternalSourceGlobalIdsMap& GlobalMap) const;
	WwiseAcousticTextureIdsMap GetSoundBankAcousticTextures(const WwiseAcousticTextureGlobalIdsMap& GlobalMap) const;
	bool IsUserBank() const;
	bool IsInitBank() const;

	uint32 SoundBankId() const;
	FGuid SoundBankGuid() const;
	FName SoundBankShortName() const;
	FName SoundBankObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefSoundBank& Rhs) const
	{
		return FWwiseRefSoundBanksInfo::operator ==(Rhs)
			&& SoundBankIndex == Rhs.SoundBankIndex;
	}
	bool operator!=(const FWwiseRefSoundBank& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefSoundBank::FGlobalIdsMap
{
	WwiseSoundBankGlobalIdsMap GlobalIdsMap;
};
