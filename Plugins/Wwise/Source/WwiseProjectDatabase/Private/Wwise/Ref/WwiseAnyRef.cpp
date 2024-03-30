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

#include "Wwise/Ref/WwiseAnyRef.h"

#include "Wwise/Metadata/WwiseMetadataLanguage.h"
#include "Wwise/Metadata/WwiseMetadataPlatform.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Ref/WwiseRefAcousticTexture.h"
#include "Wwise/Ref/WwiseRefAudioDevice.h"
#include "Wwise/Ref/WwiseRefAuxBus.h"
#include "Wwise/Ref/WwiseRefBus.h"
#include "Wwise/Ref/WwiseRefCustomPlugin.h"
#include "Wwise/Ref/WwiseRefDialogueArgument.h"
#include "Wwise/Ref/WwiseRefExternalSource.h"
#include "Wwise/Ref/WwiseRefGameParameter.h"
#include "Wwise/Ref/WwiseRefLanguage.h"
#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/Ref/WwiseRefPlatform.h"
#include "Wwise/Ref/WwiseRefPluginLib.h"
#include "Wwise/Ref/WwiseRefPluginShareSet.h"
#include "Wwise/Ref/WwiseRefSoundBank.h"
#include "Wwise/Ref/WwiseRefState.h"
#include "Wwise/Ref/WwiseRefSwitch.h"
#include "Wwise/Ref/WwiseRefSwitchContainer.h"
#include "Wwise/Ref/WwiseRefTrigger.h"

const FWwiseRefLanguage* FWwiseAnyRef::GetLanguageRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::Language))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefLanguage*>(Ref.Get());
}

const FWwiseRefPlatform* FWwiseAnyRef::GetPlatformRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::Platform))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefPlatform*>(Ref.Get());
}

const FWwiseRefPluginLib* FWwiseAnyRef::GetPluginLibRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::PluginLib))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefPluginLib*>(Ref.Get());
}

const FWwiseRefSoundBank* FWwiseAnyRef::GetSoundBankRef() const
{
	if (UNLIKELY(GetType() < EWwiseRefType::SoundBank || GetType() > EWwiseRefType::AcousticTexture))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefSoundBank*>(Ref.Get());
}

const FWwiseRefMedia* FWwiseAnyRef::GetMediaRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::Media))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefMedia*>(Ref.Get());
}

const FWwiseRefCustomPlugin* FWwiseAnyRef::GetCustomPluginRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::CustomPlugin))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefCustomPlugin*>(Ref.Get());
}

const FWwiseRefPluginShareSet* FWwiseAnyRef::GetPluginShareSetRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::PluginShareSet))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefPluginShareSet*>(Ref.Get());
}

const FWwiseRefAudioDevice* FWwiseAnyRef::GetAudioDeviceRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::AudioDevice))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefAudioDevice*>(Ref.Get());
}

const FWwiseRefEvent* FWwiseAnyRef::GetEventRef() const
{
	if (UNLIKELY(GetType() < EWwiseRefType::Event || GetType() > EWwiseRefType::SwitchContainer))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefEvent*>(Ref.Get());
}

const FWwiseRefSwitchContainer* FWwiseAnyRef::GetSwitchContainerRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::SwitchContainer))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefSwitchContainer*>(Ref.Get());
}

const FWwiseRefDialogueEvent* FWwiseAnyRef::GetDialogueEventRef() const
{
	if (UNLIKELY(GetType() < EWwiseRefType::DialogueEvent || GetType() > EWwiseRefType::DialogueArgument))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefDialogueEvent*>(Ref.Get());
}

const FWwiseRefDialogueArgument* FWwiseAnyRef::GetDialogueArgumentRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::DialogueArgument))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefDialogueArgument*>(Ref.Get());
}

const FWwiseRefBus* FWwiseAnyRef::GetBusRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::Bus))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefBus*>(Ref.Get());
}

const FWwiseRefAuxBus* FWwiseAnyRef::GetAuxBusRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::AuxBus))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefAuxBus*>(Ref.Get());
}

const FWwiseRefGameParameter* FWwiseAnyRef::GetGameParameterRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::GameParameter))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefGameParameter*>(Ref.Get());
}

const FWwiseRefStateGroup* FWwiseAnyRef::GetStateGroupRef() const
{
	if (UNLIKELY(GetType() < EWwiseRefType::StateGroup || GetType() > EWwiseRefType::State))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefStateGroup*>(Ref.Get());
}

const FWwiseRefState* FWwiseAnyRef::GetStateRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::State))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefState*>(Ref.Get());
}

const FWwiseRefSwitchGroup* FWwiseAnyRef::GetSwitchGroupRef() const
{
	if (UNLIKELY(GetType() < EWwiseRefType::SwitchGroup || GetType() > EWwiseRefType::Switch))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefSwitchGroup*>(Ref.Get());
}

const FWwiseRefSwitch* FWwiseAnyRef::GetSwitchRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::Switch))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefSwitch*>(Ref.Get());
}

const FWwiseRefTrigger* FWwiseAnyRef::GetTriggerRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::Trigger))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefTrigger*>(Ref.Get());
}

const FWwiseRefExternalSource* FWwiseAnyRef::GetExternalSourceRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::ExternalSource))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefExternalSource*>(Ref.Get());
}

const FWwiseRefAcousticTexture* FWwiseAnyRef::GetAcousticTextureRef() const
{
	if (UNLIKELY(GetType() != EWwiseRefType::AcousticTexture))
	{
		return nullptr;
	}
	return static_cast<const FWwiseRefAcousticTexture*>(Ref.Get());
}

const FWwiseMetadataLanguage* FWwiseAnyRef::GetLanguage() const
{
	const auto* LanguageRef = GetLanguageRef();
	if (UNLIKELY(!LanguageRef))
	{
		return nullptr;
	}
	return LanguageRef->GetLanguage();
}

const FWwiseMetadataPlatform* FWwiseAnyRef::GetPlatform() const
{
	const auto* PlatformRef = GetPlatformRef();
	if (UNLIKELY(!PlatformRef))
	{
		return nullptr;
	}
	return PlatformRef->GetPlatform();
}

const FWwiseMetadataPlatformReference* FWwiseAnyRef::GetPlatformReference() const
{
	const auto* PlatformRef = GetPlatformRef();
	if (UNLIKELY(!PlatformRef))
	{
		return nullptr;
	}
	return PlatformRef->GetPlatformReference();
}

const FWwiseMetadataPluginLib* FWwiseAnyRef::GetPluginLib() const
{
	const auto* PluginLibRef = GetPluginLibRef();
	if (UNLIKELY(!PluginLibRef))
	{
		return nullptr;
	}
	return PluginLibRef->GetPluginLib();
}

const FWwiseMetadataSoundBank* FWwiseAnyRef::GetSoundBank() const
{
	const auto* SoundBankRef = GetSoundBankRef();
	if (UNLIKELY(!SoundBankRef))
	{
		return nullptr;
	}
	return SoundBankRef->GetSoundBank();
}

const FWwiseMetadataMedia* FWwiseAnyRef::GetMedia() const
{
	const auto* MediaRef = GetMediaRef();
	if (UNLIKELY(!MediaRef))
	{
		return nullptr;
	}
	return MediaRef->GetMedia();
}

const FWwiseMetadataPlugin* FWwiseAnyRef::GetCustomPlugin() const
{
	const auto* CustomPluginRef = GetCustomPluginRef();
	if (UNLIKELY(!CustomPluginRef))
	{
		return nullptr;
	}
	return CustomPluginRef->GetPlugin();
}

const FWwiseMetadataPlugin* FWwiseAnyRef::GetPluginShareSet() const
{
	const auto* PluginShareSetRef = GetPluginShareSetRef();
	if (UNLIKELY(!PluginShareSetRef))
	{
		return nullptr;
	}
	return PluginShareSetRef->GetPlugin();
}

const FWwiseMetadataPlugin* FWwiseAnyRef::GetAudioDevice() const
{
	const auto* AudioDeviceRef = GetAudioDeviceRef();
	if (UNLIKELY(!AudioDeviceRef))
	{
		return nullptr;
	}
	return AudioDeviceRef->GetPlugin();
}

const FWwiseMetadataEvent* FWwiseAnyRef::GetEvent() const
{
	const auto* EventRef = GetEventRef();
	if (UNLIKELY(!EventRef))
	{
		return nullptr;
	}
	return EventRef->GetEvent();
}

const FWwiseMetadataSwitchContainer* FWwiseAnyRef::GetSwitchContainer() const
{
	const auto* SwitchContainerRef = GetSwitchContainerRef();
	if (UNLIKELY(!SwitchContainerRef))
	{
		return nullptr;
	}
	return SwitchContainerRef->GetSwitchContainer();
}

const FWwiseMetadataDialogueEvent* FWwiseAnyRef::GetDialogueEvent() const
{
	const auto* DialogueEventRef = GetDialogueEventRef();
	if (UNLIKELY(!DialogueEventRef))
	{
		return nullptr;
	}
	return DialogueEventRef->GetDialogueEvent();
}

const FWwiseMetadataDialogueArgument* FWwiseAnyRef::GetDialogueArgument() const
{
	const auto* DialogueArgumentRef = GetDialogueArgumentRef();
	if (UNLIKELY(!DialogueArgumentRef))
	{
		return nullptr;
	}
	return DialogueArgumentRef->GetDialogueArgument();
}

const FWwiseMetadataBus* FWwiseAnyRef::GetBus() const
{
	const auto* BusRef = GetBusRef();
	if (UNLIKELY(!BusRef))
	{
		return nullptr;
	}
	return BusRef->GetBus();
}

const FWwiseMetadataBus* FWwiseAnyRef::GetAuxBus() const
{
	const auto* AuxBusRef = GetAuxBusRef();
	if (UNLIKELY(!AuxBusRef))
	{
		return nullptr;
	}
	return AuxBusRef->GetAuxBus();
}

const FWwiseMetadataGameParameter* FWwiseAnyRef::GetGameParameter() const
{
	const auto* GameParameterRef = GetGameParameterRef();
	if (UNLIKELY(!GameParameterRef))
	{
		return nullptr;
	}
	return GameParameterRef->GetGameParameter();
}

const FWwiseMetadataStateGroup* FWwiseAnyRef::GetStateGroup() const
{
	const auto* StateGroupRef = GetStateGroupRef();
	if (UNLIKELY(!StateGroupRef))
	{
		return nullptr;
	}
	return StateGroupRef->GetStateGroup();
}

const FWwiseMetadataState* FWwiseAnyRef::GetState() const
{
	const auto* StateRef = GetStateRef();
	if (UNLIKELY(!StateRef))
	{
		return nullptr;
	}
	return StateRef->GetState();
}

const FWwiseMetadataSwitchGroup* FWwiseAnyRef::GetSwitchGroup() const
{
	const auto* SwitchGroupRef = GetSwitchGroupRef();
	if (UNLIKELY(!SwitchGroupRef))
	{
		return nullptr;
	}
	return SwitchGroupRef->GetSwitchGroup();
}

const FWwiseMetadataSwitch* FWwiseAnyRef::GetSwitch() const
{
	const auto* SwitchRef = GetSwitchRef();
	if (UNLIKELY(!SwitchRef))
	{
		return nullptr;
	}
	return SwitchRef->GetSwitch();
}

const FWwiseMetadataTrigger* FWwiseAnyRef::GetTrigger() const
{
	const auto* TriggerRef = GetTriggerRef();
	if (UNLIKELY(!TriggerRef))
	{
		return nullptr;
	}
	return TriggerRef->GetTrigger();
}

const FWwiseMetadataExternalSource* FWwiseAnyRef::GetExternalSource() const
{
	const auto* ExternalSourceRef = GetExternalSourceRef();
	if (UNLIKELY(!ExternalSourceRef))
	{
		return nullptr;
	}
	return ExternalSourceRef->GetExternalSource();
}

const FWwiseMetadataAcousticTexture* FWwiseAnyRef::GetAcousticTexture() const
{
	const auto* AcousticTextureRef = GetAcousticTextureRef();
	if (UNLIKELY(!AcousticTextureRef))
	{
		return nullptr;
	}
	return AcousticTextureRef->GetAcousticTexture();
}

bool FWwiseAnyRef::GetRef(FWwiseRefLanguage& OutRef) const
{
	const auto* Result = GetLanguageRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefPlatform& OutRef) const
{
	const auto* Result = GetPlatformRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefPluginLib& OutRef) const
{
	const auto* Result = GetPluginLibRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefSoundBank& OutRef) const
{
	const auto* Result = GetSoundBankRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefMedia& OutRef) const
{
	const auto* Result = GetMediaRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefCustomPlugin& OutRef) const
{
	const auto* Result = GetCustomPluginRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefPluginShareSet& OutRef) const
{
	const auto* Result = GetPluginShareSetRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefAudioDevice& OutRef) const
{
	const auto* Result = GetAudioDeviceRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefEvent& OutRef) const
{
	const auto* Result = GetEventRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefSwitchContainer& OutRef) const
{
	const auto* Result = GetSwitchContainerRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefDialogueEvent& OutRef) const
{
	const auto* Result = GetDialogueEventRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefDialogueArgument& OutRef) const
{
	const auto* Result = GetDialogueArgumentRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefBus& OutRef) const
{
	const auto* Result = GetBusRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefAuxBus& OutRef) const
{
	const auto* Result = GetAuxBusRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefGameParameter& OutRef) const
{
	const auto* Result = GetGameParameterRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefStateGroup& OutRef) const
{
	const auto* Result = GetStateGroupRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefState& OutRef) const
{
	const auto* Result = GetStateRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefSwitchGroup& OutRef) const
{
	const auto* Result = GetSwitchGroupRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefSwitch& OutRef) const
{
	const auto* Result = GetSwitchRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefTrigger& OutRef) const
{
	const auto* Result = GetTriggerRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefExternalSource& OutRef) const
{
	const auto* Result = GetExternalSourceRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

bool FWwiseAnyRef::GetRef(FWwiseRefAcousticTexture& OutRef) const
{
	const auto* Result = GetAcousticTextureRef();
	if (UNLIKELY(!Result))
	{
		return false;
	}
	OutRef = *Result;
	return true;
}

FGuid FWwiseAnyRef::GetGuid(const EWwiseRefType* TypeOverride) const
{
	const auto Type = TypeOverride ? *TypeOverride : GetType();
	switch (Type)
	{
	case EWwiseRefType::RootFile: return {};
	case EWwiseRefType::ProjectInfo: return {};
	case EWwiseRefType::Language: return GetLanguage()->GUID;
	case EWwiseRefType::PlatformInfo: return {};
	case EWwiseRefType::Platform: return GetPlatformReference()->GUID;
	case EWwiseRefType::PluginInfo: return {};
	case EWwiseRefType::PluginLib: return {};
	case EWwiseRefType::SoundBanksInfo: return {};
	case EWwiseRefType::SoundBank: return GetSoundBank()->GUID;
	case EWwiseRefType::Media: return {};
	case EWwiseRefType::CustomPlugin: return GetCustomPlugin()->GUID;
	case EWwiseRefType::PluginShareSet: return GetPluginShareSet()->GUID;
	case EWwiseRefType::AudioDevice: return GetAudioDevice()->GUID;
	case EWwiseRefType::Event: return GetEvent()->GUID;
	case EWwiseRefType::SwitchContainer: return {};
	case EWwiseRefType::DialogueEvent: return GetDialogueEvent()->GUID;
	case EWwiseRefType::DialogueArgument: return GetDialogueArgument()->GUID;
	case EWwiseRefType::Bus: return GetBus()->GUID;
	case EWwiseRefType::AuxBus: return GetAuxBus()->GUID;
	case EWwiseRefType::GameParameter: return GetGameParameter()->GUID;
	case EWwiseRefType::StateGroup: return GetStateGroup()->GUID;
	case EWwiseRefType::State: return GetState()->GUID;
	case EWwiseRefType::SwitchGroup: return GetSwitchGroup()->GUID;
	case EWwiseRefType::Switch: return GetSwitch()->GUID;
	case EWwiseRefType::Trigger: return GetTrigger()->GUID;
	case EWwiseRefType::ExternalSource: return GetExternalSource()->GUID;
	case EWwiseRefType::AcousticTexture: return GetAcousticTexture()->GUID;
	case EWwiseRefType::None:
	default:
		return {};
	}
}

uint32 FWwiseAnyRef::GetGroupId(const EWwiseRefType* TypeOverride) const
{
	const auto Type = TypeOverride ? *TypeOverride : GetType();
	switch (Type)
	{
	case EWwiseRefType::RootFile: return 0;
	case EWwiseRefType::ProjectInfo: return 0;
	case EWwiseRefType::Language: return 0;
	case EWwiseRefType::PlatformInfo: return 0;
	case EWwiseRefType::Platform: return 0;
	case EWwiseRefType::PluginInfo: return 0;
	case EWwiseRefType::PluginLib: return 0;
	case EWwiseRefType::SoundBanksInfo: return 0;
	case EWwiseRefType::SoundBank: return 0;
	case EWwiseRefType::Media: return 0;
	case EWwiseRefType::CustomPlugin: return 0;
	case EWwiseRefType::PluginShareSet: return 0;
	case EWwiseRefType::AudioDevice: return 0;
	case EWwiseRefType::Event: return 0;
	case EWwiseRefType::SwitchContainer: return 0;
	case EWwiseRefType::DialogueEvent: return 0;
	case EWwiseRefType::DialogueArgument: return 0;
	case EWwiseRefType::Bus: return 0;
	case EWwiseRefType::AuxBus: return 0;
	case EWwiseRefType::GameParameter: return 0;
	case EWwiseRefType::StateGroup: return 0;
	case EWwiseRefType::State: return GetStateGroup()->Id;
	case EWwiseRefType::SwitchGroup: return 0;
	case EWwiseRefType::Switch: return GetSwitchGroup()->Id;
	case EWwiseRefType::Trigger: return 0;
	case EWwiseRefType::ExternalSource: return 0;
	case EWwiseRefType::AcousticTexture: return 0;
	case EWwiseRefType::None:
	default:
		return 0;
	}
}

uint32 FWwiseAnyRef::GetId(const EWwiseRefType* TypeOverride) const
{
	const auto Type = TypeOverride ? *TypeOverride : GetType();
	switch (Type)
	{
	case EWwiseRefType::RootFile: return 0;
	case EWwiseRefType::ProjectInfo: return 0;
	case EWwiseRefType::Language: return GetLanguage()->Id;
	case EWwiseRefType::PlatformInfo: return 0;
	case EWwiseRefType::Platform: return 0;
	case EWwiseRefType::PluginInfo: return 0;
	case EWwiseRefType::PluginLib: return 0;
	case EWwiseRefType::SoundBanksInfo: return 0;
	case EWwiseRefType::SoundBank: return GetSoundBank()->Id;
	case EWwiseRefType::Media: return GetMedia()->Id;
	case EWwiseRefType::CustomPlugin: return GetCustomPlugin()->Id;
	case EWwiseRefType::PluginShareSet: return GetPluginShareSet()->Id;
	case EWwiseRefType::AudioDevice: return GetAudioDevice()->Id;
	case EWwiseRefType::Event: return GetEvent()->Id;
	case EWwiseRefType::SwitchContainer: return 0;
	case EWwiseRefType::DialogueEvent: return GetDialogueEvent()->Id;
	case EWwiseRefType::DialogueArgument: return GetDialogueArgument()->Id;
	case EWwiseRefType::Bus: return GetBus()->Id;
	case EWwiseRefType::AuxBus: return GetAuxBus()->Id;
	case EWwiseRefType::GameParameter: return GetGameParameter()->Id;
	case EWwiseRefType::StateGroup: return GetStateGroup()->Id;
	case EWwiseRefType::State: return GetState()->Id;
	case EWwiseRefType::SwitchGroup: return GetSwitchGroup()->Id;
	case EWwiseRefType::Switch: return GetSwitch()->Id;
	case EWwiseRefType::Trigger: return GetTrigger()->Id;
	case EWwiseRefType::ExternalSource: return GetExternalSource()->Cookie;
	case EWwiseRefType::AcousticTexture: return GetAcousticTexture()->Id;
	case EWwiseRefType::None:
	default:
		return 0;
	}
}

FName FWwiseAnyRef::GetName(const EWwiseRefType* TypeOverride) const
{
	const auto Type = TypeOverride ? *TypeOverride : GetType();
	switch (Type)
	{
	case EWwiseRefType::RootFile: return {};
	case EWwiseRefType::ProjectInfo: return {};
	case EWwiseRefType::Language: return GetLanguage()->Name;
	case EWwiseRefType::PlatformInfo: return {};
	case EWwiseRefType::Platform: return GetPlatformReference()->Name;
	case EWwiseRefType::PluginInfo: return {};
	case EWwiseRefType::PluginLib: return {};
	case EWwiseRefType::SoundBanksInfo: return {};
	case EWwiseRefType::SoundBank: return GetSoundBank()->ShortName;
	case EWwiseRefType::Media: return GetMedia()->ShortName;
	case EWwiseRefType::CustomPlugin: return GetCustomPlugin()->Name;
	case EWwiseRefType::PluginShareSet: return GetPluginShareSet()->Name;
	case EWwiseRefType::AudioDevice: return GetAudioDevice()->Name;
	case EWwiseRefType::Event: return GetEvent()->Name;
	case EWwiseRefType::SwitchContainer: return {};
	case EWwiseRefType::DialogueEvent: return GetDialogueEvent()->Name;
	case EWwiseRefType::DialogueArgument: return GetDialogueArgument()->Name;
	case EWwiseRefType::Bus: return GetBus()->Name;
	case EWwiseRefType::AuxBus: return GetAuxBus()->Name;
	case EWwiseRefType::GameParameter: return GetGameParameter()->Name;
	case EWwiseRefType::StateGroup: return GetStateGroup()->Name;
	case EWwiseRefType::State: return GetState()->Name;
	case EWwiseRefType::SwitchGroup: return GetSwitchGroup()->Name;
	case EWwiseRefType::Switch: return GetSwitch()->Name;
	case EWwiseRefType::Trigger: return GetTrigger()->Name;
	case EWwiseRefType::ExternalSource: return GetExternalSource()->Name;
	case EWwiseRefType::AcousticTexture: return GetAcousticTexture()->Name;
	case EWwiseRefType::None:
	default:
		return {};
	}
}

FName FWwiseAnyRef::GetObjectPath(const EWwiseRefType* TypeOverride) const
{
	const auto Type = TypeOverride ? *TypeOverride : GetType();
	switch (Type)
	{
	case EWwiseRefType::RootFile: return {};
	case EWwiseRefType::ProjectInfo: return {};
	case EWwiseRefType::Language: return {};
	case EWwiseRefType::PlatformInfo: return {};
	case EWwiseRefType::Platform: return {};
	case EWwiseRefType::PluginInfo: return {};
	case EWwiseRefType::PluginLib: return {};
	case EWwiseRefType::SoundBanksInfo: return {};
	case EWwiseRefType::SoundBank: return GetSoundBank()->ObjectPath;
	case EWwiseRefType::Media: return {};
	case EWwiseRefType::CustomPlugin: return GetCustomPlugin()->ObjectPath;
	case EWwiseRefType::PluginShareSet: return GetPluginShareSet()->ObjectPath;
	case EWwiseRefType::AudioDevice: return GetAudioDevice()->ObjectPath;
	case EWwiseRefType::Event: return GetEvent()->ObjectPath;
	case EWwiseRefType::SwitchContainer: return {};
	case EWwiseRefType::DialogueEvent: return GetDialogueEvent()->ObjectPath;
	case EWwiseRefType::DialogueArgument: return GetDialogueArgument()->ObjectPath;
	case EWwiseRefType::Bus: return GetBus()->ObjectPath;
	case EWwiseRefType::AuxBus: return GetAuxBus()->ObjectPath;
	case EWwiseRefType::GameParameter: return GetGameParameter()->ObjectPath;
	case EWwiseRefType::StateGroup: return GetStateGroup()->ObjectPath;
	case EWwiseRefType::State: return GetState()->ObjectPath;
	case EWwiseRefType::SwitchGroup: return GetSwitchGroup()->ObjectPath;
	case EWwiseRefType::Switch: return GetSwitch()->ObjectPath;
	case EWwiseRefType::Trigger: return GetTrigger()->ObjectPath;
	case EWwiseRefType::ExternalSource: return GetExternalSource()->ObjectPath;
	case EWwiseRefType::AcousticTexture: return GetAcousticTexture()->ObjectPath;
	case EWwiseRefType::None:
	default:
		return {};
	}
}
