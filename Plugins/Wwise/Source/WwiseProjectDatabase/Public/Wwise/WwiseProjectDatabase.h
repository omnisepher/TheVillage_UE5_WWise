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

#include "Wwise/WwiseDataStructure.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Misc/CommandLine.h"

class FWwiseResourceLoader;
class FWwiseProjectDatabase;
using FSharedWwiseDataStructure = TSharedRef<FWwiseDataStructure, ESPMode::ThreadSafe>;

class WWISEPROJECTDATABASE_API FWwiseDataStructureScopeLock : public FRWScopeLock
{
public:
	FWwiseDataStructureScopeLock(const FWwiseProjectDatabase& InProjectDatabase);

	const FWwiseDataStructure& operator*() const
	{
		return DataStructure;
	}

	const FWwiseDataStructure* operator->() const
	{
		return &DataStructure;
	}

	const WwiseAcousticTextureGlobalIdsMap& GetAcousticTextures() const;
	FWwiseRefAcousticTexture GetAcousticTexture(const FWwiseObjectInfo& InInfo) const;

	const WwiseAudioDeviceGlobalIdsMap& GetAudioDevices() const;
	FWwiseRefAudioDevice GetAudioDevice(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseAuxBusGlobalIdsMap& GetAuxBusses() const;
	FWwiseRefAuxBus GetAuxBus(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseBusGlobalIdsMap& GetBusses() const;
	FWwiseRefBus GetBus(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseCustomPluginGlobalIdsMap& GetCustomPlugins() const;
	FWwiseRefCustomPlugin GetCustomPlugin(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseDialogueArgumentGlobalIdsMap& GetDialogueArguments() const;
	FWwiseRefDialogueArgument GetDialogueArgument(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseDialogueEventGlobalIdsMap& GetDialogueEvents() const;
	FWwiseRefDialogueEvent GetDialogueEvent(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseEventGlobalIdsMap& GetEvents() const;
	TSet<FWwiseRefEvent> GetEvent(const FWwiseEventInfo& InInfo) const;
	
	const WwiseExternalSourceGlobalIdsMap& GetExternalSources() const;
	FWwiseRefExternalSource GetExternalSource(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseGameParameterGlobalIdsMap& GetGameParameters() const;
	FWwiseRefGameParameter GetGameParameter(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseMediaGlobalIdsMap& GetMediaFiles() const;
	FWwiseRefMedia GetMediaFile(const FWwiseObjectInfo& InInfo) const;
	
	const WwisePluginLibGlobalIdsMap& GetPluginLibs() const;
	FWwiseRefPluginLib GetPluginLib(const FWwiseObjectInfo& InInfo) const;
	
	const WwisePluginShareSetGlobalIdsMap& GetPluginShareSets() const;
	FWwiseRefPluginShareSet GetPluginShareSet(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseSoundBankGlobalIdsMap& GetSoundBanks() const;
	FWwiseRefSoundBank GetSoundBank(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseStateGlobalIdsMap& GetStates() const;
	FWwiseRefState GetState(const FWwiseGroupValueInfo& InInfo) const;
	
	const WwiseStateGroupGlobalIdsMap& GetStateGroups() const;
	FWwiseRefStateGroup GetStateGroup(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseSwitchGlobalIdsMap& GetSwitches() const;
	FWwiseRefSwitch GetSwitch(const FWwiseGroupValueInfo& InInfo) const;
	
	const WwiseSwitchGroupGlobalIdsMap& GetSwitchGroups() const;
	FWwiseRefSwitchGroup GetSwitchGroup(const FWwiseObjectInfo& InInfo) const;
	
	const WwiseTriggerGlobalIdsMap& GetTriggers() const;
	FWwiseRefTrigger GetTrigger(const FWwiseObjectInfo& InInfo) const;

	const TSet<FWwiseSharedLanguageId>& GetLanguages() const;
	const TSet<FWwiseSharedPlatformId>& GetPlatforms() const;
	FWwiseRefPlatform GetPlatform(const FWwiseSharedPlatformId& InPlatformId) const;

	const FWwisePlatformDataStructure* GetCurrentPlatformData() const;

	const FWwiseSharedLanguageId& GetCurrentLanguage() const { return CurrentLanguage; }
	const FWwiseSharedPlatformId& GetCurrentPlatform() const { return CurrentPlatform; }
	bool DisableDefaultPlatforms() const { return bDisableDefaultPlatforms; }
	
private:
	const FWwiseDataStructure& DataStructure;

	FWwiseSharedLanguageId CurrentLanguage;
	FWwiseSharedPlatformId CurrentPlatform;
	bool bDisableDefaultPlatforms;

	UE_NONCOPYABLE(FWwiseDataStructureScopeLock);
};

class WWISEPROJECTDATABASE_API FWwiseDataStructureWriteScopeLock : public FRWScopeLock
{
public:
	FWwiseDataStructureWriteScopeLock(FWwiseProjectDatabase& InProjectDatabase);

	FWwiseDataStructure& operator*()
	{
		return DataStructure;
	}

	FWwiseDataStructure* operator->()
	{
		return &DataStructure;
	}

private:
	FWwiseDataStructure& DataStructure;
	UE_NONCOPYABLE(FWwiseDataStructureWriteScopeLock);
};

class WWISEPROJECTDATABASE_API FWwiseProjectDatabase
{
	friend class FWwiseDataStructureScopeLock;
	friend class FWwiseDataStructureWriteScopeLock;

public:
	static const FGuid BasePlatformGuid;

	inline static FWwiseProjectDatabase* Get()
	{
		if (auto* Module = IWwiseProjectDatabaseModule::GetModule())
		{
			if(Module->CanHaveDefaultInstance())
			{
				return Module->GetProjectDatabase();
			}
		}
		return nullptr;
	}
	static FWwiseProjectDatabase* Instantiate()
	{
		if (auto* Module = IWwiseProjectDatabaseModule::GetModule())
		{
			return Module->InstantiateProjectDatabase();
		}
		return nullptr;
	}


	FWwiseProjectDatabase() {}
	virtual ~FWwiseProjectDatabase() {}

	virtual void UpdateDataStructure(
		const FDirectoryPath* InUpdateGeneratedSoundBanksPath = nullptr,
		const FGuid* InBasePlatformGuid = &BasePlatformGuid) {}

	virtual void PrepareProjectDatabaseForPlatform(FWwiseResourceLoader*&& InResourceLoader) {}
	virtual FWwiseResourceLoader* GetResourceLoader() { return nullptr; }
	virtual const FWwiseResourceLoader* GetResourceLoader() const { return nullptr; }

	FWwiseSharedLanguageId GetCurrentLanguage() const;
	FWwiseSharedPlatformId GetCurrentPlatform() const;
	virtual bool IsProjectDatabaseParsed() const {return bIsDatabaseParsed;};


protected:
	virtual FSharedWwiseDataStructure& GetLockedDataStructure() { check(false); UE_ASSUME(false); }
	virtual const FSharedWwiseDataStructure& GetLockedDataStructure() const { check(false); UE_ASSUME(false); }

	template <typename RequiredRef>
	bool GetRef(RequiredRef& OutRef, const FWwiseObjectInfo& InInfo)
	{
		const auto* ResourceLoader = GetResourceLoader();
		check(ResourceLoader);
		const auto& PlatformRef = ResourceLoader->GetCurrentPlatform();

		const auto& DataStructure = *GetLockedDataStructure();

		const auto* Platform = DataStructure.Platforms.Find(PlatformRef);
		if (UNLIKELY(!Platform))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("GetRef: Platform not found"));
			return false;
		}

		return Platform->GetRef(OutRef, InInfo);
	}

	bool DisableDefaultPlatforms() const;
	bool bIsDatabaseParsed = false;
};