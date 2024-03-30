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

#include "Wwise/WwiseDataStructure.h"
#include "Wwise/WwiseDirectoryVisitor.h"

#include "Wwise/Metadata/WwiseMetadataAcousticTexture.h"
#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataDialogue.h"
#include "Wwise/Metadata/WwiseMetadataEvent.h"
#include "Wwise/Metadata/WwiseMetadataExternalSource.h"
#include "Wwise/Metadata/WwiseMetadataGameParameter.h"
#include "Wwise/Metadata/WwiseMetadataLanguage.h"
#include "Wwise/Metadata/WwiseMetadataMedia.h"
#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataPluginInfo.h"
#include "Wwise/Metadata/WwiseMetadataPluginLib.h"
#include "Wwise/Metadata/WwiseMetadataProjectInfo.h"
#include "Wwise/Metadata/WwiseMetadataRootFile.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Metadata/WwiseMetadataSoundBanksInfo.h"
#include "Wwise/Metadata/WwiseMetadataState.h"
#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/Metadata/WwiseMetadataSwitch.h"
#include "Wwise/Metadata/WwiseMetadataSwitchContainer.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"
#include "Wwise/Metadata/WwiseMetadataTrigger.h"

#include "WwiseDefines.h"
#include "WwiseUnrealDefines.h"

#include "Async/Async.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "Misc/LocalTimestampDirectoryVisitor.h"
#include "Misc/Paths.h"

FWwiseDataStructure::FWwiseDataStructure(const FDirectoryPath& InDirectoryPath, const FName* InPlatform, const FGuid* InBasePlatformGuid)
{
    if (InDirectoryPath.Path.IsEmpty())
    {
        return;
    }

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FWwiseDirectoryVisitor Visitor(PlatformFile, InPlatform, InBasePlatformGuid);
    PlatformFile.IterateDirectory(*InDirectoryPath.Path, Visitor);
    auto Directory = Visitor.Get();

    if (!Directory.IsValid())
    {
        UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Invalid Generated Directory %s"), *InDirectoryPath.Path);
        return;
    }

    FString RequestedPlatformPath;
    if (InPlatform)
    {
        const auto& ProjectInfoPlatforms = Directory.ProjectInfo->ProjectInfo->Platforms;
        for (const auto& Platform : ProjectInfoPlatforms)
        {
            if (Platform.Name == *InPlatform)
            {
                FString PlatformPath = Platform.Path.ToString();

                //If the platfrom path contains a drive, it means that the directory path and platform path are on different drives
                if (FPaths::IsRelative(PlatformPath))
                {
                    RequestedPlatformPath = InDirectoryPath.Path / PlatformPath;
                    FPaths::CollapseRelativeDirectories(RequestedPlatformPath);
                }
                else
                {
                    RequestedPlatformPath = PlatformPath;
                }
            }
        }
    }

    if (InPlatform)
    {
	    if (Directory.Platforms.Num() == 0)
	    {
	        UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not find platform %s in Generated Directory %s"), *InPlatform->ToString(), *RequestedPlatformPath);
	        return;
	    }

        UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Parsing Wwise data structure for platform %s at: %s..."), *InPlatform->ToString(), *RequestedPlatformPath);
    }
    else
    {
        UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Parsing Wwise data structure at: %s..."), *InDirectoryPath.Path);
    }


    LoadDataStructure(MoveTemp(Directory));
}

FWwiseDataStructure::~FWwiseDataStructure()
{
    FWriteScopeLock ScopeLock(Lock);
}

void FWwiseDataStructure::LoadDataStructure(FWwiseGeneratedFiles&& Directory)
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_2(TEXT("FWwiseDataStructure::LoadDataStructure"));
	UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Starting load task"));

    // Create the file lists to be used in loading root files
    TArray<FString> FileListForRoot;
    {
        const FString ProjectInfoPath = Directory.GeneratedRootFiles.ProjectInfoFile.Get<0>();
        if (ProjectInfoPath.IsEmpty())
        {
            UE_LOG(LogWwiseProjectDatabase, Error, TEXT("- Could not find project info"));
        }
        else
        {
            UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("- Adding project info: %s"), *ProjectInfoPath);
            FileListForRoot.Add(ProjectInfoPath);
        }
    }

    TSharedFuture<FWwiseRootDataStructure*> RootFuture = Async(EAsyncExecution::TaskGraph, [this, &FileListForRoot, &Directory] {
        UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Loading Generated file contents for root"));
        auto JsonFiles = FWwiseMetadataRootFile::LoadFiles(FileListForRoot);

        UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Parsing Generated file contents for root"));
        auto RootDataStructure = new FWwiseRootDataStructure(MoveTemp(JsonFiles));
        RootDataStructure->GeneratedRootFiles = MoveTemp(Directory.GeneratedRootFiles);
        return RootDataStructure;
        }).Share();

        // Create the file lists to be used in loading files per platform
        TMap<FWwiseSharedPlatformId, TFuture<FWwisePlatformDataStructure*>> Futures;
        for (const auto& Platform : Directory.Platforms)
        {
            UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Loading platform files: %s"), *Platform.Key.GetPlatformName().ToString());
            TArray<FString> FileList;
            const FWwiseSharedPlatformId& PlatformRef = Platform.Key;
            const FWwiseGeneratedFiles::FPlatformFiles& Files = Platform.Value;

            // Add Platform and Plug-in files
            const FString PlatformInfoPath = Files.PlatformInfoFile.Get<0>();
            {
                if (UNLIKELY(PlatformInfoPath.IsEmpty()))
                {
                    UE_LOG(LogWwiseProjectDatabase, Error, TEXT("No PlatformInfo file for platform %s"), *PlatformRef.GetPlatformName().ToString());
                    continue;
                }
                UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("- Adding platform info: %s"), *PlatformInfoPath);
                FileList.Add(PlatformInfoPath);
            }
            {
                const FString PluginInfoPath = Files.PluginInfoFile.Get<0>();
                if (UNLIKELY(PluginInfoPath.IsEmpty()))
                {
                    UE_LOG(LogWwiseProjectDatabase, Error, TEXT("No PluginInfo file for platform %s"), *PlatformRef.GetPlatformName().ToString());
                    continue;
                }
                UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("- Adding plugin info: %s"), *PluginInfoPath);
                FileList.Add(PluginInfoPath);
            }

            // Parse PlatformInfo file to detect settings
            // (will be parsed twice. Now once, and officially later - since the file is small, it's not a big worry)
            UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Pre-parsing platform info file for settings"));
            auto PlatformInfoFile = FWwiseMetadataRootFile::LoadFile(PlatformInfoPath);
            if (!PlatformInfoFile || !PlatformInfoFile->PlatformInfo)
            {
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not read PlatformInfo for platform %s."), *PlatformRef.GetPlatformName().ToString());
                continue;
            }
            const auto& Settings = PlatformInfoFile->PlatformInfo->Settings;
            bool bIsValid = true;
            if (!Settings.bCopyLooseStreamedMediaFiles)
            {
                bIsValid = false;
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Requires \"Copy Loose/Streamed Media\"."), *PlatformRef.GetPlatformName().ToString());
            }
            if (!Settings.bGenerateMetadataJSON)
            {
                bIsValid = false;
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Requires \"Generate JSON Metadata\"."), *PlatformRef.GetPlatformName().ToString());
            }
            if (Settings.bGenerateAllBanksMetadata && Settings.bGeneratePerBankMetadata)
            {
                UE_LOG(LogWwiseProjectDatabase, Log, TEXT("Platform %s: Having both \"Generate All Banks Metadata file\" and \"Generate Per Bank Metadata file\" will use the latter."), *PlatformRef.GetPlatformName().ToString());
            }
            else if (Settings.bGenerateAllBanksMetadata)
            {
                UE_LOG(LogWwiseProjectDatabase, Log, TEXT("Platform %s: Using \"Generate All Banks Metadata file\" is less efficient than Per Bank."), *PlatformRef.GetPlatformName().ToString());
            }
            else if (!Settings.bGeneratePerBankMetadata)
            {
                bIsValid = false;
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: No metadata generated. Requires one of the \"Generate Metadata file\" option set."), *PlatformRef.GetPlatformName().ToString());
            }
            if (!Settings.bPrintObjectGuid)
            {
                bIsValid = false;
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Requires \"Object GUID\" Metadata."), *PlatformRef.GetPlatformName().ToString());
            }
            if (!Settings.bPrintObjectPath)
            {
                bIsValid = false;
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Requires \"Object Path\" Metadata."), *PlatformRef.GetPlatformName().ToString());
            }
            if (!Settings.bMaxAttenuationInfo)
            {
                bIsValid = false;
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Requires \"Max Attenuation\" Metadata."), *PlatformRef.GetPlatformName().ToString());
            }
            if (!Settings.bEstimatedDurationInfo)
            {
                bIsValid = false;
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Requires \"Estimated Duration\" Metadata."), *PlatformRef.GetPlatformName().ToString());
            }
            if (!bIsValid)
            {
                UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Skipping platform"));
                continue;
            }

            // Monolithic SoundBanksInfo or split files
            if (Settings.bGeneratePerBankMetadata)
            {
                if (UNLIKELY(Files.MetadataFiles.Num() == 0))
                {
                    UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Generated Per Bank metadata, but no metadata file found."), *PlatformRef.GetPlatformName().ToString());
                    continue;
                }

                FileList.Reserve(FileList.Num() + Files.MetadataFiles.Num());
                for (const auto& MetadataFile : Files.MetadataFiles)
                {
                    UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("- Adding metadata file: %s"), *MetadataFile.Key);
                    FileList.Add(MetadataFile.Key);
                }
            }
            else if (!Files.SoundbanksInfoFile.Get<0>().IsEmpty())
            {
                UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("- Adding monolithic SoundBanks info file: %s"), *Files.SoundbanksInfoFile.Get<0>());
                FileList.Add(Files.SoundbanksInfoFile.Get<0>());
            }
            else
            {
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Platform %s: Generated All Banks metadata, but SoundBanksInfo.json file not found."), *PlatformRef.GetPlatformName().ToString());
                continue;
            }

            Futures.Add(Platform.Key, Async(EAsyncExecution::TaskGraph, [this, PlatformRef, RootFuture, FileList, &Directory] {
                UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Loading Generated file contents for platform %s"), *PlatformRef.GetPlatformName().ToString());
                auto JsonFiles = FWwiseMetadataRootFile::LoadFiles(FileList);

                auto PlatformData = new FWwisePlatformDataStructure(PlatformRef, *RootFuture.Get(), MoveTemp(JsonFiles));
                PlatformData->GeneratedPlatformFiles = MoveTemp(Directory.Platforms[PlatformRef]);
                return PlatformData;
                }));
        }

        UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Finalizing parsing %d files"), Futures.Num());
        for (const auto& Future : Futures)
        {
            auto* Result = Future.Value.Get();
            if (UNLIKELY(!Result))
            {
                UE_LOG(LogWwiseProjectDatabase, Error, TEXT("File parsing failed"));
            }
            else
            {
                Platforms.Add(Future.Key, MoveTemp(*Result));
                delete Result;
            }
        }
        Futures.Empty();

        UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Finalizing parsing root"));
        if (auto* Result = RootFuture.Get())
        {
            // Will move result. Must have all other Futures done
            RootData = MoveTemp(*Result);
            delete Result;
        }
}


FWwiseRootDataStructure::FWwiseRootDataStructure(WwiseMetadataFileMap&& InJsonFiles) :
    JsonFiles(MoveTemp(InJsonFiles))
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_2(TEXT("FWwiseRootDataStructure::FWwiseRootDataStructure"));
	for (const auto& JsonFileKV : JsonFiles)
    {
        const auto JsonFilePath = FName(JsonFileKV.Key);
        if (JsonFileKV.Value)
        {
            const WwiseMetadataSharedRootFileConstPtr SharedRootFile = JsonFileKV.Value;
            const FWwiseMetadataRootFile& RootFile = *SharedRootFile;

            if (RootFile.ProjectInfo)
            {
                const FWwiseMetadataProjectInfo& ProjectInfo = *RootFile.ProjectInfo;

                // PlatformReferenceNames + PlatformReferenceGuids;
                for (WwiseRefIndexType PlatformIndex = 0; PlatformIndex < ProjectInfo.Platforms.Num(); ++PlatformIndex)
                {
                    const FWwiseMetadataPlatformReference& PlatformReference = ProjectInfo.Platforms[PlatformIndex];
                    PlatformNames.Add(PlatformReference.Name, FWwiseRefPlatform(SharedRootFile, JsonFilePath, PlatformIndex));
                    PlatformGuids.Add(PlatformReference.GUID, FWwiseRefPlatform(SharedRootFile, JsonFilePath, PlatformIndex));
                    Platforms.Emplace(FWwiseSharedPlatformId(PlatformReference.GUID, PlatformReference.Name, PlatformReference.Path), nullptr);
                }

                // LanguageNames, LanguageIds, LanguageRefs
                for (WwiseRefIndexType LanguageIndex = 0; LanguageIndex < ProjectInfo.Languages.Num(); ++LanguageIndex)
                {
                    const FWwiseMetadataLanguage& Language = ProjectInfo.Languages[LanguageIndex];
                    LanguageNames.Add(Language.Name, FWwiseRefLanguage(SharedRootFile, JsonFilePath, LanguageIndex));
                    LanguageIds.Add(Language.Id, FWwiseRefLanguage(SharedRootFile, JsonFilePath, LanguageIndex));
                    Languages.Emplace(FWwiseSharedLanguageId(Language.Id, Language.Name, Language.bDefault ? EWwiseLanguageRequirement::IsDefault : EWwiseLanguageRequirement::IsOptional), nullptr);
                }
            }
        }
    }
}

FWwisePlatformDataStructure::FWwisePlatformDataStructure() :
    AcousticTextures(FWwiseRefAcousticTexture::FGlobalIdsMap::GlobalIdsMap),
    AudioDevices(FWwiseRefAudioDevice::FGlobalIdsMap::GlobalIdsMap),
    AuxBusses(FWwiseRefAuxBus::FGlobalIdsMap::GlobalIdsMap),
    Busses(FWwiseRefBus::FGlobalIdsMap::GlobalIdsMap),
    CustomPlugins(FWwiseRefCustomPlugin::FGlobalIdsMap::GlobalIdsMap),
    DialogueArguments(FWwiseRefDialogueArgument::FGlobalIdsMap::GlobalIdsMap),
    DialogueEvents(FWwiseRefDialogueEvent::FGlobalIdsMap::GlobalIdsMap),
    Events(FWwiseRefEvent::FGlobalIdsMap::GlobalIdsMap),
    ExternalSources(FWwiseRefExternalSource::FGlobalIdsMap::GlobalIdsMap),
    GameParameters(FWwiseRefGameParameter::FGlobalIdsMap::GlobalIdsMap),
    MediaFiles(FWwiseRefMedia::FGlobalIdsMap::GlobalIdsMap),
    PluginLibs(FWwiseRefPluginLib::FGlobalIdsMap::GlobalIdsMap),
    PluginShareSets(FWwiseRefPluginShareSet::FGlobalIdsMap::GlobalIdsMap),
    SoundBanks(FWwiseRefSoundBank::FGlobalIdsMap::GlobalIdsMap),
    States(FWwiseRefState::FGlobalIdsMap::GlobalIdsMap),
    StateGroups(FWwiseRefStateGroup::FGlobalIdsMap::GlobalIdsMap),
    Switches(FWwiseRefSwitch::FGlobalIdsMap::GlobalIdsMap),
    SwitchGroups(FWwiseRefSwitchGroup::FGlobalIdsMap::GlobalIdsMap),
    Triggers(FWwiseRefTrigger::FGlobalIdsMap::GlobalIdsMap)
{}

FWwisePlatformDataStructure::FWwisePlatformDataStructure(const FWwiseSharedPlatformId& InPlatform, FWwiseRootDataStructure& InRootData, WwiseMetadataFileMap&& InJsonFiles) :
    Platform(InPlatform),
    JsonFiles(MoveTemp(InJsonFiles)),
    AcousticTextures(FWwiseRefAcousticTexture::FGlobalIdsMap::GlobalIdsMap),
    AudioDevices(FWwiseRefAudioDevice::FGlobalIdsMap::GlobalIdsMap),
    AuxBusses(FWwiseRefAuxBus::FGlobalIdsMap::GlobalIdsMap),
    Busses(FWwiseRefBus::FGlobalIdsMap::GlobalIdsMap),
    CustomPlugins(FWwiseRefCustomPlugin::FGlobalIdsMap::GlobalIdsMap),
    DialogueArguments(FWwiseRefDialogueArgument::FGlobalIdsMap::GlobalIdsMap),
    DialogueEvents(FWwiseRefDialogueEvent::FGlobalIdsMap::GlobalIdsMap),
    Events(FWwiseRefEvent::FGlobalIdsMap::GlobalIdsMap),
    ExternalSources(FWwiseRefExternalSource::FGlobalIdsMap::GlobalIdsMap),
    GameParameters(FWwiseRefGameParameter::FGlobalIdsMap::GlobalIdsMap),
    MediaFiles(FWwiseRefMedia::FGlobalIdsMap::GlobalIdsMap),
    PluginLibs(FWwiseRefPluginLib::FGlobalIdsMap::GlobalIdsMap),
    PluginShareSets(FWwiseRefPluginShareSet::FGlobalIdsMap::GlobalIdsMap),
    SoundBanks(FWwiseRefSoundBank::FGlobalIdsMap::GlobalIdsMap),
    States(FWwiseRefState::FGlobalIdsMap::GlobalIdsMap),
    StateGroups(FWwiseRefStateGroup::FGlobalIdsMap::GlobalIdsMap),
    Switches(FWwiseRefSwitch::FGlobalIdsMap::GlobalIdsMap),
    SwitchGroups(FWwiseRefSwitchGroup::FGlobalIdsMap::GlobalIdsMap),
    Triggers(FWwiseRefTrigger::FGlobalIdsMap::GlobalIdsMap)
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_2(TEXT("FWwisePlatformDataStructure::FWwisePlatformDataStructure"));
	for (const auto& JsonFileKV : JsonFiles)
    {
        const auto JsonFilePath = FName(JsonFileKV.Key);
        if (JsonFileKV.Value)
        {
            const WwiseMetadataSharedRootFileConstPtr SharedRootFile = JsonFileKV.Value;
            const FWwiseMetadataRootFile& RootFile = *SharedRootFile;

            if (RootFile.PlatformInfo)
            {
                // Platform have different information depending on its location.
                // Project's Platform contains the Path, generator version, and Guid.
                // PlatformInfo contains all the other information, including generation data.
                // So we must merge one into the other.
                const FWwiseMetadataPlatformInfo& PlatformInfo = *RootFile.PlatformInfo;
                FWwiseRefPlatform NewPlatformRef(SharedRootFile, JsonFilePath);

                const auto& PlatformName = PlatformInfo.Platform.Name;

                // Update PlatformNames
                FWwiseRefPlatform* RootPlatformByName = InRootData.PlatformNames.Find(PlatformName);
                if (UNLIKELY(!RootPlatformByName))
                {
                    UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not find platform %s in ProjectInfo"), *PlatformName.ToString());
                    continue;
                }
                RootPlatformByName->Merge(MoveTemp(NewPlatformRef));

                // Update PlatformGUID
                const auto* PlatformReference = RootPlatformByName->GetPlatformReference();
                check(PlatformReference);
                const auto& Guid = PlatformReference->GUID;
                FWwiseRefPlatform* RootPlatformByGuid = InRootData.PlatformGuids.Find(Guid);
                if (UNLIKELY(!RootPlatformByGuid))
                {
                    UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not find platform %s guid %s in ProjectInfo"), *PlatformName.ToString(), *Guid.ToString());
                    continue;
                }
                *RootPlatformByGuid = *RootPlatformByName;
                PlatformRef = *RootPlatformByName;
            }

            if (RootFile.PluginInfo)
            {
                const FWwiseMetadataPluginInfo& PluginInfo = *RootFile.PluginInfo;

                // PluginLibNames + PluginLibIDs
                for (WwiseRefIndexType PluginLibIndex = 0; PluginLibIndex < PluginInfo.PluginLibs.Num(); ++PluginLibIndex)
                {
                    const FWwiseMetadataPluginLib& PluginLib = PluginInfo.PluginLibs[PluginLibIndex];
                    const auto& PluginRef = FWwiseRefPluginLib(SharedRootFile, JsonFilePath, PluginLibIndex);
                    AddRefToMap(PluginLibs, PluginRef, PluginLib.LibId, &PluginLib.LibName, nullptr, nullptr);
                    PluginLibNames.Add(PluginLib.LibName, PluginRef);
                }
            }

            if (RootFile.ProjectInfo)
            {
                const FWwiseMetadataProjectInfo& ProjectInfo = *RootFile.ProjectInfo;
                // Should be loaded in FWwiseRootDataStructure
            }

            if (RootFile.SoundBanksInfo)
            {
                const FWwiseMetadataSoundBanksInfo& SoundBanksInfo = *RootFile.SoundBanksInfo;

                // SoundBanks
                for (WwiseRefIndexType SoundBankIndex = 0; SoundBankIndex < SoundBanksInfo.SoundBanks.Num(); ++SoundBankIndex)
                {
                    const FWwiseMetadataSoundBank& SoundBank = SoundBanksInfo.SoundBanks[SoundBankIndex];
                    const uint32 LanguageId = InRootData.GetLanguageId(SoundBank.Language);
                    AddRefToMap(SoundBanks, FWwiseRefSoundBank(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId), SoundBank.Id, &SoundBank.ShortName, &SoundBank.ObjectPath, &SoundBank.GUID);

                    // Media
                    for (WwiseRefIndexType MediaIndex = 0; MediaIndex < SoundBank.Media.Num(); ++MediaIndex)
                    {
                        const FWwiseMetadataMedia& File = SoundBank.Media[MediaIndex];
                        MediaFiles.Add(FWwiseDatabaseMediaIdKey(File.Id, SoundBank.Id),
                            FWwiseRefMedia(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, MediaIndex));
                    }

                    // DialogueEvents
                    for (WwiseRefIndexType DialogueEventIndex = 0; DialogueEventIndex < SoundBank.DialogueEvents.Num(); ++DialogueEventIndex)
                    {
                        const FWwiseMetadataDialogueEvent& DialogueEvent = SoundBank.DialogueEvents[DialogueEventIndex];
                        AddBasicRefToMap(DialogueEvents, FWwiseRefDialogueEvent(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, DialogueEventIndex), DialogueEvent);

                        // DialogueArguments
                        for (WwiseRefIndexType DialogueArgumentIndex = 0; DialogueArgumentIndex < DialogueEvent.Arguments.Num(); ++DialogueArgumentIndex)
                        {
                            const FWwiseMetadataDialogueArgument& DialogueArgument = DialogueEvent.Arguments[DialogueArgumentIndex];
                            AddBasicRefToMap(DialogueArguments, FWwiseRefDialogueArgument(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, DialogueEventIndex, DialogueArgumentIndex), DialogueArgument);
                        }
                    }

                    // We have multiple copies of the Busses. We currently want the Init Bank version.
                    if (SoundBank.IsInitBank())
                    {
	                    // Busses
	                    for (WwiseRefIndexType BusIndex = 0; BusIndex < SoundBank.Busses.Num(); ++BusIndex)
	                    {
	                        const FWwiseMetadataBus& Bus = SoundBank.Busses[BusIndex];
	                        AddBasicRefToMap(Busses, FWwiseRefBus(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, BusIndex), Bus);
	                    }

	                    // AuxBusses
	                    for (WwiseRefIndexType AuxBusIndex = 0; AuxBusIndex < SoundBank.AuxBusses.Num(); ++AuxBusIndex)
	                    {
	                        const FWwiseMetadataBus& AuxBus = SoundBank.AuxBusses[AuxBusIndex];
	                        AddBasicRefToMap(AuxBusses, FWwiseRefAuxBus(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, AuxBusIndex), AuxBus);
	                    }
                    }

                    // Plugins
                    if (SoundBank.Plugins)
                    {
                        const auto& Plugins = *SoundBank.Plugins;

                        // CustomPlugins
                        for (WwiseRefIndexType CustomPluginIndex = 0; CustomPluginIndex < Plugins.Custom.Num(); ++CustomPluginIndex)
                        {
                            const FWwiseMetadataPlugin& CustomPlugin = Plugins.Custom[CustomPluginIndex];
                            AddBasicRefToMap(CustomPlugins, FWwiseRefCustomPlugin(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, CustomPluginIndex), CustomPlugin);
                        }

                        // PluginShareSets
                        for (WwiseRefIndexType PluginShareSetIndex = 0; PluginShareSetIndex < Plugins.ShareSets.Num(); ++PluginShareSetIndex)
                        {
                            const FWwiseMetadataPlugin& PluginShareSet = Plugins.ShareSets[PluginShareSetIndex];
                            AddBasicRefToMap(PluginShareSets, FWwiseRefPluginShareSet(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, PluginShareSetIndex), PluginShareSet);
                        }

                        // AudioDevices
                        for (WwiseRefIndexType AudioDeviceIndex = 0; AudioDeviceIndex < Plugins.AudioDevices.Num(); ++AudioDeviceIndex)
                        {
                            const FWwiseMetadataPlugin& AudioDevice = Plugins.AudioDevices[AudioDeviceIndex];
                            AddBasicRefToMap(AudioDevices, FWwiseRefAudioDevice(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, AudioDeviceIndex), AudioDevice);
                        }
                    }

                    // Events
                    for (WwiseRefIndexType EventIndex = 0; EventIndex < SoundBank.Events.Num(); ++EventIndex)
                    {
                        const FWwiseMetadataEvent& Event = SoundBank.Events[EventIndex];
                        AddEventRefToMap(Events, FWwiseRefEvent(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, EventIndex), Event);

                        // Switch Containers
                        TArray<WwiseRefIndexType> ContainerIndex;
                        ContainerIndex.Add(0);
                        while (true)
                        {
                            if (ContainerIndex.Num() == 0)
                            {
                                // Fully done
                                break;
                            }

                            // Retrieve Container
                            const FWwiseMetadataSwitchContainer* Container = nullptr;
                            const auto* ContainerArray = &Event.SwitchContainers;
                            for (WwiseRefIndexType ContainerLevel = 0; ContainerLevel < ContainerIndex.Num() && ContainerArray; ++ContainerLevel)
                            {
                                WwiseRefIndexType CurrentIndex = ContainerIndex[ContainerLevel];
                                if (!ContainerArray->IsValidIndex(CurrentIndex))
                                {
                                    // Done last level
                                    ContainerArray = nullptr;
                                    Container = nullptr;
                                    break;
                                }
                                Container = &(*ContainerArray)[CurrentIndex];
                                ContainerArray = &Container->Children;
                            }

                            if (Container == nullptr)
                            {
                                // Done this level
                                ContainerIndex.Pop();

                                if (ContainerIndex.Num() > 0)
                                {
                                    ++ContainerIndex[ContainerIndex.Num() - 1];
                                }
                                continue;
                            }

                            if (Container->MediaRefs.Num() > 0 || Container->ExternalSourceRefs.Num() >0 || Container->PluginRefs != nullptr)
                            {
                                const auto& Ref = FWwiseRefSwitchContainer(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, EventIndex, ContainerIndex);
                                SwitchContainersByEvent.Add(FWwiseDatabaseLocalizableIdKey(Event.Id, LanguageId), Ref);
                            }

                            if (ContainerArray->Num() > 0)
                            {
                                // There are children. Add one sublevel
                                ContainerIndex.Add(0);
                            }
                            else
                            {
                                // No children. Next.
                                ++ContainerIndex[ContainerIndex.Num() - 1];
                            }
                        }
                    }

                    // ExternalSources
                    for (WwiseRefIndexType ExternalSourceIndex = 0; ExternalSourceIndex < SoundBank.ExternalSources.Num(); ++ExternalSourceIndex)
                    {
                        const FWwiseMetadataExternalSource& ExternalSource = SoundBank.ExternalSources[ExternalSourceIndex];
                        AddRefToMap(ExternalSources, FWwiseRefExternalSource(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, ExternalSourceIndex), ExternalSource.Cookie, &ExternalSource.Name, &ExternalSource.ObjectPath, &ExternalSource.GUID);
                    }

                    // AcousticTextures
                    for (WwiseRefIndexType AcousticTextureIndex = 0; AcousticTextureIndex < SoundBank.AcousticTextures.Num(); ++AcousticTextureIndex)
                    {
                        const FWwiseMetadataAcousticTexture& AcousticTexture = SoundBank.AcousticTextures[AcousticTextureIndex];
                        AddBasicRefToMap(AcousticTextures, FWwiseRefAcousticTexture(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, AcousticTextureIndex), AcousticTexture);
                    }

                    // GameParameters
                    for (WwiseRefIndexType GameParameterIndex = 0; GameParameterIndex < SoundBank.GameParameters.Num(); ++GameParameterIndex)
                    {
                        const FWwiseMetadataGameParameter& GameParameter = SoundBank.GameParameters[GameParameterIndex];
                        AddBasicRefToMap(GameParameters, FWwiseRefGameParameter(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, GameParameterIndex), GameParameter);
                    }

                    // StateGroups
                    for (WwiseRefIndexType StateGroupIndex = 0; StateGroupIndex < SoundBank.StateGroups.Num(); ++StateGroupIndex)
                    {
                        const FWwiseMetadataStateGroup& StateGroup = SoundBank.StateGroups[StateGroupIndex];
                        AddBasicRefToMap(StateGroups, FWwiseRefStateGroup(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, StateGroupIndex), StateGroup);
                        
                        // States
                        for (WwiseRefIndexType StateIndex = 0; StateIndex < StateGroup.States.Num(); ++StateIndex)
                        {
                            const FWwiseMetadataState& State = StateGroup.States[StateIndex];
                            const FWwiseRefState StateRef(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, StateGroupIndex, StateIndex);
                            const FWwiseAnyRef AnyRef = FWwiseAnyRef::Create(StateRef);
                            States.Add(FWwiseDatabaseLocalizableGroupValueKey(StateGroup.Id, State.Id, LanguageId), StateRef);
                            if (State.GUID != FGuid()) Guids.Add(FWwiseDatabaseLocalizableGuidKey(State.GUID, LanguageId), AnyRef);
                            if (!State.Name.IsNone()) Names.Add(FWwiseDatabaseLocalizableNameKey(State.Name, LanguageId), AnyRef);
                            if (!State.ObjectPath.IsNone()) Names.Add(FWwiseDatabaseLocalizableNameKey(State.ObjectPath, LanguageId), AnyRef);
                        }
                    }

                    // SwitchGroups
                    for (WwiseRefIndexType SwitchGroupIndex = 0; SwitchGroupIndex < SoundBank.SwitchGroups.Num(); ++SwitchGroupIndex)
                    {
                        const FWwiseMetadataSwitchGroup& SwitchGroup = SoundBank.SwitchGroups[SwitchGroupIndex];
                        AddBasicRefToMap(SwitchGroups, FWwiseRefSwitchGroup(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, SwitchGroupIndex), SwitchGroup);

                        // Switches
                        for (WwiseRefIndexType SwitchIndex = 0; SwitchIndex < SwitchGroup.Switches.Num(); ++SwitchIndex)
                        {
                            const FWwiseMetadataSwitch& Switch = SwitchGroup.Switches[SwitchIndex];
                            const FWwiseRefSwitch SwitchRef(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, SwitchGroupIndex, SwitchIndex);
                            const FWwiseAnyRef AnyRef = FWwiseAnyRef::Create(SwitchRef);
                            Switches.Add(FWwiseDatabaseLocalizableGroupValueKey(SwitchGroup.Id, Switch.Id, LanguageId), SwitchRef);
                            if (Switch.GUID != FGuid()) Guids.Add(FWwiseDatabaseLocalizableGuidKey(Switch.GUID, LanguageId), AnyRef);
                            if (!Switch.Name.IsNone()) Names.Add(FWwiseDatabaseLocalizableNameKey(Switch.Name, LanguageId), AnyRef);
                            if (!Switch.ObjectPath.IsNone()) Names.Add(FWwiseDatabaseLocalizableNameKey(Switch.ObjectPath, LanguageId), AnyRef);
                        }
                    }

                    // Triggers
                    for (WwiseRefIndexType TriggerIndex = 0; TriggerIndex < SoundBank.Triggers.Num(); ++TriggerIndex)
                    {
                        const FWwiseMetadataTrigger& Trigger = SoundBank.Triggers[TriggerIndex];
                        AddBasicRefToMap(Triggers, FWwiseRefTrigger(SharedRootFile, JsonFilePath, SoundBankIndex, LanguageId, TriggerIndex), Trigger);
                    }
                }
            }
        }
    }
}

FWwisePlatformDataStructure::FWwisePlatformDataStructure(const FWwisePlatformDataStructure& Rhs) :
    FWwiseRefAcousticTexture::FGlobalIdsMap(Rhs),
    FWwiseRefAudioDevice::FGlobalIdsMap(Rhs),
    FWwiseRefAuxBus::FGlobalIdsMap(Rhs),
    FWwiseRefBus::FGlobalIdsMap(Rhs),
    FWwiseRefCustomPlugin::FGlobalIdsMap(Rhs),
    FWwiseRefDialogueArgument::FGlobalIdsMap(Rhs),
    FWwiseRefDialogueEvent::FGlobalIdsMap(Rhs),
    FWwiseRefEvent::FGlobalIdsMap(Rhs),
    FWwiseRefExternalSource::FGlobalIdsMap(Rhs),
    FWwiseRefGameParameter::FGlobalIdsMap(Rhs),
    FWwiseRefMedia::FGlobalIdsMap(Rhs),
    FWwiseRefPluginLib::FGlobalIdsMap(Rhs),
    FWwiseRefPluginShareSet::FGlobalIdsMap(Rhs),
    FWwiseRefSoundBank::FGlobalIdsMap(Rhs),
    FWwiseRefState::FGlobalIdsMap(Rhs),
    FWwiseRefStateGroup::FGlobalIdsMap(Rhs),
    FWwiseRefSwitch::FGlobalIdsMap(Rhs),
    FWwiseRefSwitchGroup::FGlobalIdsMap(Rhs),
    FWwiseRefTrigger::FGlobalIdsMap(Rhs),
    Platform(Rhs.Platform),
    PlatformRef(Rhs.PlatformRef),
    GeneratedPlatformFiles(Rhs.GeneratedPlatformFiles),
    JsonFiles(Rhs.JsonFiles),
    AcousticTextures(FWwiseRefAcousticTexture::FGlobalIdsMap::GlobalIdsMap),
    AudioDevices(FWwiseRefAudioDevice::FGlobalIdsMap::GlobalIdsMap),
    AuxBusses(FWwiseRefAuxBus::FGlobalIdsMap::GlobalIdsMap),
    Busses(FWwiseRefBus::FGlobalIdsMap::GlobalIdsMap),
    CustomPlugins(FWwiseRefCustomPlugin::FGlobalIdsMap::GlobalIdsMap),
    DialogueArguments(FWwiseRefDialogueArgument::FGlobalIdsMap::GlobalIdsMap),
    DialogueEvents(FWwiseRefDialogueEvent::FGlobalIdsMap::GlobalIdsMap),
    Events(FWwiseRefEvent::FGlobalIdsMap::GlobalIdsMap),
    ExternalSources(FWwiseRefExternalSource::FGlobalIdsMap::GlobalIdsMap),
    GameParameters(FWwiseRefGameParameter::FGlobalIdsMap::GlobalIdsMap),
    MediaFiles(FWwiseRefMedia::FGlobalIdsMap::GlobalIdsMap),
    PluginLibs(FWwiseRefPluginLib::FGlobalIdsMap::GlobalIdsMap),
    PluginShareSets(FWwiseRefPluginShareSet::FGlobalIdsMap::GlobalIdsMap),
    SoundBanks(FWwiseRefSoundBank::FGlobalIdsMap::GlobalIdsMap),
    States(FWwiseRefState::FGlobalIdsMap::GlobalIdsMap),
    StateGroups(FWwiseRefStateGroup::FGlobalIdsMap::GlobalIdsMap),
    Switches(FWwiseRefSwitch::FGlobalIdsMap::GlobalIdsMap),
    SwitchGroups(FWwiseRefSwitchGroup::FGlobalIdsMap::GlobalIdsMap),
    Triggers(FWwiseRefTrigger::FGlobalIdsMap::GlobalIdsMap),
    PluginLibNames(Rhs.PluginLibNames),
    SwitchContainersByEvent(Rhs.SwitchContainersByEvent),
    Guids(Rhs.Guids),
    Names(Rhs.Names)
{}

FWwisePlatformDataStructure::FWwisePlatformDataStructure(FWwisePlatformDataStructure&& Rhs) :
    FWwiseRefAcousticTexture::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefAudioDevice::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefAuxBus::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefBus::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefCustomPlugin::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefDialogueArgument::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefDialogueEvent::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefEvent::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefExternalSource::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefGameParameter::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefMedia::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefPluginLib::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefPluginShareSet::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefSoundBank::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefState::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefStateGroup::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefSwitch::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefSwitchGroup::FGlobalIdsMap(MoveTemp(Rhs)),
    FWwiseRefTrigger::FGlobalIdsMap(MoveTemp(Rhs)),
    Platform(MoveTemp(Rhs.Platform)),
    PlatformRef(MoveTemp(Rhs.PlatformRef)),
    GeneratedPlatformFiles(MoveTemp(Rhs.GeneratedPlatformFiles)),
    JsonFiles(MoveTemp(Rhs.JsonFiles)),
    AcousticTextures(FWwiseRefAcousticTexture::FGlobalIdsMap::GlobalIdsMap),
    AudioDevices(FWwiseRefAudioDevice::FGlobalIdsMap::GlobalIdsMap),
    AuxBusses(FWwiseRefAuxBus::FGlobalIdsMap::GlobalIdsMap),
    Busses(FWwiseRefBus::FGlobalIdsMap::GlobalIdsMap),
    CustomPlugins(FWwiseRefCustomPlugin::FGlobalIdsMap::GlobalIdsMap),
    DialogueArguments(FWwiseRefDialogueArgument::FGlobalIdsMap::GlobalIdsMap),
    DialogueEvents(FWwiseRefDialogueEvent::FGlobalIdsMap::GlobalIdsMap),
    Events(FWwiseRefEvent::FGlobalIdsMap::GlobalIdsMap),
    ExternalSources(FWwiseRefExternalSource::FGlobalIdsMap::GlobalIdsMap),
    GameParameters(FWwiseRefGameParameter::FGlobalIdsMap::GlobalIdsMap),
    MediaFiles(FWwiseRefMedia::FGlobalIdsMap::GlobalIdsMap),
    PluginLibs(FWwiseRefPluginLib::FGlobalIdsMap::GlobalIdsMap),
    PluginShareSets(FWwiseRefPluginShareSet::FGlobalIdsMap::GlobalIdsMap),
    SoundBanks(FWwiseRefSoundBank::FGlobalIdsMap::GlobalIdsMap),
    States(FWwiseRefState::FGlobalIdsMap::GlobalIdsMap),
    StateGroups(FWwiseRefStateGroup::FGlobalIdsMap::GlobalIdsMap),
    Switches(FWwiseRefSwitch::FGlobalIdsMap::GlobalIdsMap),
    SwitchGroups(FWwiseRefSwitchGroup::FGlobalIdsMap::GlobalIdsMap),
    Triggers(FWwiseRefTrigger::FGlobalIdsMap::GlobalIdsMap),
    PluginLibNames(MoveTemp(Rhs.PluginLibNames)),
    SwitchContainersByEvent(MoveTemp(Rhs.SwitchContainersByEvent)),
    Guids(MoveTemp(Rhs.Guids)),
    Names(MoveTemp(Rhs.Names))
{}

FWwiseRootDataStructure& FWwiseRootDataStructure::operator+=(FWwiseRootDataStructure&& Rhs)
{
    // GeneratedRootFiles += MoveTemp(Rhs.GeneratedRootFiles);
    JsonFiles.Append(MoveTemp(Rhs.JsonFiles));
    LanguageNames.Append(MoveTemp(Rhs.LanguageNames));
    LanguageIds.Append(MoveTemp(Rhs.LanguageIds));
    PlatformNames.Append(MoveTemp(Rhs.PlatformNames));
    PlatformGuids.Append(MoveTemp(Rhs.PlatformGuids));
    return *this;
}

FWwisePlatformDataStructure& FWwisePlatformDataStructure::operator+=(FWwisePlatformDataStructure&& Rhs)
{
    // GeneratedPlatformFiles += MoveTemp(Rhs.GeneratedPlatformFiles);
    JsonFiles.Append(MoveTemp(Rhs.JsonFiles));
    MediaFiles.Append(MoveTemp(Rhs.MediaFiles));
    PluginLibNames.Append(MoveTemp(Rhs.PluginLibNames));
    PluginLibs.Append(MoveTemp(Rhs.PluginLibs));
    SoundBanks.Append(MoveTemp(Rhs.SoundBanks));
    DialogueEvents.Append(MoveTemp(Rhs.DialogueEvents));
    DialogueArguments.Append(MoveTemp(Rhs.DialogueArguments));
    Busses.Append(MoveTemp(Rhs.Busses));
    AuxBusses.Append(MoveTemp(Rhs.AuxBusses));
    Events.Append(MoveTemp(Rhs.Events));
    ExternalSources.Append(MoveTemp(Rhs.ExternalSources));
    AcousticTextures.Append(MoveTemp(Rhs.AcousticTextures));
    GameParameters.Append(MoveTemp(Rhs.GameParameters));
    StateGroups.Append(MoveTemp(Rhs.StateGroups));
    SwitchGroups.Append(MoveTemp(Rhs.SwitchGroups));
    Triggers.Append(MoveTemp(Rhs.Triggers));
    States.Append(MoveTemp(Rhs.States));
    Switches.Append(MoveTemp(Rhs.Switches));
    SwitchContainersByEvent.Append(MoveTemp(Rhs.SwitchContainersByEvent));
    return *this;
}

bool FWwisePlatformDataStructure::GetFromId(FWwiseRefMedia& OutRef, uint32 InShortId, uint32 InLanguageId, uint32 InSoundBankId) const
{
    const FWwiseRefMedia* Result = nullptr;
    if (LIKELY(InSoundBankId != 0))
    {
        FWwiseDatabaseMediaIdKey MediaId(InShortId, InSoundBankId);
        Result = MediaFiles.Find(MediaId);
    }
    else
    {
        for (const auto& MediaFile : MediaFiles)
        {
            if (MediaFile.Key.MediaId == InShortId
                && MediaFile.Value.GetMedia()->Location != EWwiseMetadataMediaLocation::OtherBank)
            {
                Result = &MediaFile.Value;
                break;
            }
        }
    }

    if (UNLIKELY(!Result))
    {
        UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("Could not find Media %" PRIu32 " (Lang=%" PRIu32 "; SB=%" PRIu32 ")"), InShortId, InLanguageId, InSoundBankId);
        return false;
    }
    
    OutRef = *Result;
    return true;
}


FWwiseDataStructure& FWwiseDataStructure::operator+=(FWwiseDataStructure&& Rhs)
{
    FWriteScopeLock ScopeLockLhs(Lock);
    FWriteScopeLock ScopeLockRhs(Rhs.Lock);

    Platforms.Append(MoveTemp(Rhs.Platforms));

    return *this;
}
