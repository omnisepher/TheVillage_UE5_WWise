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

#include "Wwise/WwiseProjectDatabase.h"

#include "WwiseUnrealHelper.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseProjectDatabaseDelegates.h"

#include "Async/Async.h"
#include "Misc/ScopedSlowTask.h"
#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/Stats/Global.h"

#define LOCTEXT_NAMESPACE "WwiseProjectDatabase"

FWwiseDataStructureScopeLock::FWwiseDataStructureScopeLock(const FWwiseProjectDatabase& InProjectDatabase) :
	FRWScopeLock(const_cast<FRWLock&>(InProjectDatabase.GetLockedDataStructure()->Lock), SLT_ReadOnly),
	DataStructure(*InProjectDatabase.GetLockedDataStructure()),
	CurrentLanguage(InProjectDatabase.GetCurrentLanguage()),
	CurrentPlatform(InProjectDatabase.GetCurrentPlatform()),
	bDisableDefaultPlatforms(InProjectDatabase.DisableDefaultPlatforms())
{
}

const WwiseAcousticTextureGlobalIdsMap& FWwiseDataStructureScopeLock::GetAcousticTextures() const
{
	static const auto Empty = WwiseAcousticTextureGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->AcousticTextures;
}

FWwiseRefAcousticTexture FWwiseDataStructureScopeLock::GetAcousticTexture(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefAcousticTexture Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseAudioDeviceGlobalIdsMap& FWwiseDataStructureScopeLock::GetAudioDevices() const
{
	static const auto Empty = WwiseAudioDeviceGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->AudioDevices;
}

FWwiseRefAudioDevice FWwiseDataStructureScopeLock::GetAudioDevice(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefAudioDevice Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseAuxBusGlobalIdsMap& FWwiseDataStructureScopeLock::GetAuxBusses() const
{
	static const auto Empty = WwiseAuxBusGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->AuxBusses;
}

FWwiseRefAuxBus FWwiseDataStructureScopeLock::GetAuxBus(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefAuxBus Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseBusGlobalIdsMap& FWwiseDataStructureScopeLock::GetBusses() const
{
	static const auto Empty = WwiseBusGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->Busses;
}

FWwiseRefBus FWwiseDataStructureScopeLock::GetBus(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefBus Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseCustomPluginGlobalIdsMap& FWwiseDataStructureScopeLock::GetCustomPlugins() const
{
	static const auto Empty = WwiseCustomPluginGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->CustomPlugins;
}

FWwiseRefCustomPlugin FWwiseDataStructureScopeLock::GetCustomPlugin(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefCustomPlugin Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseDialogueArgumentGlobalIdsMap& FWwiseDataStructureScopeLock::GetDialogueArguments() const
{
	static const auto Empty = WwiseDialogueArgumentGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->DialogueArguments;
}

FWwiseRefDialogueArgument FWwiseDataStructureScopeLock::GetDialogueArgument(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefDialogueArgument Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseDialogueEventGlobalIdsMap& FWwiseDataStructureScopeLock::GetDialogueEvents() const
{
	static const auto Empty = WwiseDialogueEventGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->DialogueEvents;
}

FWwiseRefDialogueEvent FWwiseDataStructureScopeLock::GetDialogueEvent(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefDialogueEvent Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseEventGlobalIdsMap& FWwiseDataStructureScopeLock::GetEvents() const
{
	static const auto Empty = WwiseEventGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->Events;
}

TSet<FWwiseRefEvent> FWwiseDataStructureScopeLock::GetEvent(const FWwiseEventInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	TSet<FWwiseRefEvent> Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseExternalSourceGlobalIdsMap& FWwiseDataStructureScopeLock::GetExternalSources() const
{
	static const auto Empty = WwiseExternalSourceGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->ExternalSources;
}

FWwiseRefExternalSource FWwiseDataStructureScopeLock::GetExternalSource(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefExternalSource Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseGameParameterGlobalIdsMap& FWwiseDataStructureScopeLock::GetGameParameters() const
{
	static const auto Empty = WwiseGameParameterGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->GameParameters;
}

FWwiseRefGameParameter FWwiseDataStructureScopeLock::GetGameParameter(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefGameParameter Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseMediaGlobalIdsMap& FWwiseDataStructureScopeLock::GetMediaFiles() const
{
	static const auto Empty = WwiseMediaGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->MediaFiles;
}

FWwiseRefMedia FWwiseDataStructureScopeLock::GetMediaFile(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefMedia Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwisePluginLibGlobalIdsMap& FWwiseDataStructureScopeLock::GetPluginLibs() const
{
	static const auto Empty = WwisePluginLibGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->PluginLibs;
}

FWwiseRefPluginLib FWwiseDataStructureScopeLock::GetPluginLib(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefPluginLib Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwisePluginShareSetGlobalIdsMap& FWwiseDataStructureScopeLock::GetPluginShareSets() const
{
	static const auto Empty = WwisePluginShareSetGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->PluginShareSets;
}

FWwiseRefPluginShareSet FWwiseDataStructureScopeLock::GetPluginShareSet(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefPluginShareSet Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseSoundBankGlobalIdsMap& FWwiseDataStructureScopeLock::GetSoundBanks() const
{
	static const auto Empty = WwiseSoundBankGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->SoundBanks;
}

FWwiseRefSoundBank FWwiseDataStructureScopeLock::GetSoundBank(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefSoundBank Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseStateGlobalIdsMap& FWwiseDataStructureScopeLock::GetStates() const
{
	static const auto Empty = WwiseStateGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->States;
}

FWwiseRefState FWwiseDataStructureScopeLock::GetState(const FWwiseGroupValueInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefState Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseStateGroupGlobalIdsMap& FWwiseDataStructureScopeLock::GetStateGroups() const
{
	static const auto Empty = WwiseStateGroupGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->StateGroups;
}

FWwiseRefStateGroup FWwiseDataStructureScopeLock::GetStateGroup(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefStateGroup Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseSwitchGlobalIdsMap& FWwiseDataStructureScopeLock::GetSwitches() const
{
	static const auto Empty = WwiseSwitchGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->Switches;
}

FWwiseRefSwitch FWwiseDataStructureScopeLock::GetSwitch(const FWwiseGroupValueInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefSwitch Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseSwitchGroupGlobalIdsMap& FWwiseDataStructureScopeLock::GetSwitchGroups() const
{
	static const auto Empty = WwiseSwitchGroupGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->SwitchGroups;
}

FWwiseRefSwitchGroup FWwiseDataStructureScopeLock::GetSwitchGroup(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefSwitchGroup Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const WwiseTriggerGlobalIdsMap& FWwiseDataStructureScopeLock::GetTriggers() const
{
	static const auto Empty = WwiseTriggerGlobalIdsMap();

	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return Empty;

	return PlatformData->Triggers;
}

FWwiseRefTrigger FWwiseDataStructureScopeLock::GetTrigger(const FWwiseObjectInfo& InInfo) const
{
	const auto* PlatformData = GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData)) return {};

	FWwiseRefTrigger Result;
	PlatformData->GetRef(Result, GetCurrentLanguage(), InInfo);
	return Result;
}

const FWwisePlatformDataStructure* FWwiseDataStructureScopeLock::GetCurrentPlatformData() const
{
	if (DisableDefaultPlatforms())
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose,
		       TEXT("Trying to access current platform data when none is loaded by design (cooking)"));
		return nullptr;
	}

	const auto& Platform = GetCurrentPlatform();
	const auto* PlatformData = DataStructure.Platforms.Find(Platform);

	if (UNLIKELY(!PlatformData))
	{
		if(Platform.GetPlatformName().ToString() != TEXT("None"))
		{
			UE_LOG(LogWwiseProjectDatabase, Error,
				TEXT(
					"Current platform %s not found."
				),
            	*Platform.GetPlatformName().ToString());
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, Error,
				TEXT(
					"No JSON Metadata file found. Have SoundBanks been generated?"
				));
		}


		if (!UE_LOG_ACTIVE(LogWwiseProjectDatabase, Verbose) && !UE_LOG_ACTIVE(LogWwiseProjectDatabase, VeryVerbose))
		{
			UE_LOG(LogWwiseHints, Warning,
			       TEXT("Enable Verbose or VeryVerbose logs for LogWwiseProjectDatabase for more details on why %s is missing from your current platforms."),
			       *Platform.GetPlatformName().ToString());
		}

		if (DataStructure.RootData.JsonFiles.Num() == 0 &&  Platform.GetPlatformName().ToString() != TEXT("None"))
		{
			FString SoundBankPath = WwiseUnrealHelper::GetSoundBankDirectory() /  Platform.Platform->PathRelativeToGeneratedSoundBanks.ToString();
			UE_LOG(LogWwiseProjectDatabase, Error,
			       TEXT("No JSON Metadata file found for platform %s at %s. Have SoundBanks been generated?"),
			       *SoundBankPath,
			       *Platform.GetPlatformName().ToString());
		}

		return nullptr;
	}
	return PlatformData;
}

const TSet<FWwiseSharedLanguageId>& FWwiseDataStructureScopeLock::GetLanguages() const
{
	return DataStructure.RootData.Languages;
}

const TSet<FWwiseSharedPlatformId>& FWwiseDataStructureScopeLock::GetPlatforms() const
{
	return DataStructure.RootData.Platforms;
}

FWwiseRefPlatform FWwiseDataStructureScopeLock::GetPlatform(const FWwiseSharedPlatformId& InPlatformId) const
{
	if (const auto* Platform = DataStructure.RootData.PlatformGuids.Find(InPlatformId.GetPlatformGuid()))
	{
		return *Platform;
	}
	return {};
}


FWwiseDataStructureWriteScopeLock::FWwiseDataStructureWriteScopeLock(FWwiseProjectDatabase& InProjectDatabase) :
	FRWScopeLock(InProjectDatabase.GetLockedDataStructure()->Lock, SLT_Write),
	DataStructure(*InProjectDatabase.GetLockedDataStructure())
{
}

#if PLATFORM_LINUX
const FGuid FWwiseProjectDatabase::BasePlatformGuid(0xbd0bdf13, 0x3125454f, 0x8bfd3195, 0x37169f81);
#elif PLATFORM_MAC
const FGuid FWwiseProjectDatabase::BasePlatformGuid(0x9c6217d5, 0xdd114795, 0x87c16ce0, 0x2853c540);
#elif PLATFORM_WINDOWS
const FGuid FWwiseProjectDatabase::BasePlatformGuid(0x6e0cb257, 0xc6c84c5c, 0x83662740, 0xdfc441eb);
#else
static_assert(false);
#endif

FWwiseSharedLanguageId FWwiseProjectDatabase::GetCurrentLanguage() const
{
	auto* ResourceLoader = GetResourceLoader();
	if (UNLIKELY(!ResourceLoader))
	{
		return {};
	}

	const auto CurrentLanguage = ResourceLoader->GetCurrentLanguage();
	return FWwiseSharedLanguageId(CurrentLanguage.GetLanguageId(), CurrentLanguage.GetLanguageName(), CurrentLanguage.LanguageRequirement);
}

FWwiseSharedPlatformId FWwiseProjectDatabase::GetCurrentPlatform() const
{
	auto* ResourceLoader = GetResourceLoader();
	if (UNLIKELY(!ResourceLoader))
	{
		return {};
	}

	return ResourceLoader->GetCurrentPlatform();
}

bool FWwiseProjectDatabase::DisableDefaultPlatforms() const
{
	return UNLIKELY(!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase()) && (Get() == this);
}

#undef LOCTEXT_NAMESPACE
