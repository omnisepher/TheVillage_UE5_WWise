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

#include "Wwise/WwiseResourceCookerImpl.h"

#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseCookingCache.h"
#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Stats/ResourceCooker.h"

#include "Async/Async.h"
#include "Async/MappedFileHandle.h"
#include "Misc/FileHelper.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "Wwise/CookedData/WwiseSoundBankCookedData.h"
#include "Wwise/Stats/ResourceCooker.h"

FWwiseResourceCookerImpl::FWwiseResourceCookerImpl() :
	ExportDebugNameRule(EWwiseExportDebugNameRule::ObjectPath),
	CookingCache(nullptr),
	ProjectDatabaseOverride(nullptr)
{
}

FWwiseResourceCookerImpl::~FWwiseResourceCookerImpl()
{
}

FWwiseProjectDatabase* FWwiseResourceCookerImpl::GetProjectDatabase()
{
	if (ProjectDatabaseOverride.IsValid())
	{
		return ProjectDatabaseOverride.Get();
	}
	else
	{
		return FWwiseProjectDatabase::Get();
	}
}

const FWwiseProjectDatabase* FWwiseResourceCookerImpl::GetProjectDatabase() const
{
	if (ProjectDatabaseOverride.IsValid())
	{
		return ProjectDatabaseOverride.Get();
	}
	else
	{
		return FWwiseProjectDatabase::Get();
	}
}

void FWwiseResourceCookerImpl::PrepareResourceCookerForPlatform(FWwiseProjectDatabase*&& InProjectDatabaseOverride, EWwiseExportDebugNameRule InExportDebugNameRule)
{
	ProjectDatabaseOverride.Reset(InProjectDatabaseOverride);
	ExportDebugNameRule = InExportDebugNameRule;
	CookingCache.Reset(new FWwiseCookingCache);
	CookingCache->ExternalSourceManager = IWwiseExternalSourceManager::Get();
}



void FWwiseResourceCookerImpl::CookAuxBusToSandbox(const FWwiseAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking AuxBus %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);
	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		CookSoundBankToSandbox(SoundBank, WriteAdditionalFile);
	}
	for (const auto& Media : InCookedData.Media)
	{
		CookMediaToSandbox(Media, WriteAdditionalFile);
	}
	UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("Done cooking AuxBus %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);
}

void FWwiseResourceCookerImpl::CookEventToSandbox(const FWwiseEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking Event %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		CookSoundBankToSandbox(SoundBank, WriteAdditionalFile);
	}
	for (const auto& Media : InCookedData.Media)
	{
		CookMediaToSandbox(Media, WriteAdditionalFile);
	}
	for (const auto& ExternalSource : InCookedData.ExternalSources)
	{
		CookExternalSourceToSandbox(ExternalSource, WriteAdditionalFile);
	}
	for (const auto& SwitchContainerLeaf : InCookedData.SwitchContainerLeaves)
	{
		UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking Event %s %" PRIu32 " Switched Media"), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
		for (const auto& SoundBank : SwitchContainerLeaf.SoundBanks)
		{
			CookSoundBankToSandbox(SoundBank, WriteAdditionalFile);
		}
		for (const auto& Media : SwitchContainerLeaf.Media)
		{
			CookMediaToSandbox(Media, WriteAdditionalFile);
		}
		for (const auto& ExternalSource : SwitchContainerLeaf.ExternalSources)
		{
			CookExternalSourceToSandbox(ExternalSource, WriteAdditionalFile);
		}
	}
	UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("Done cooking Event %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
}

void FWwiseResourceCookerImpl::CookExternalSourceToSandbox(const FWwiseExternalSourceCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	if (LIKELY(CookingCache && CookingCache->ExternalSourceManager))
	{
		CookingCache->ExternalSourceManager->Cook(*this, InCookedData, WriteAdditionalFile, GetCurrentPlatform(), GetCurrentLanguage());
	}
	else
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("No External Source Manager while cooking External Source %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.Cookie);
	}
}

void FWwiseResourceCookerImpl::CookInitBankToSandbox(const FWwiseInitBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking Init SoundBank %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
	CookSoundBankToSandbox(InCookedData, WriteAdditionalFile);

	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		CookSoundBankToSandbox(SoundBank, WriteAdditionalFile);
	}

	for (const auto& Media : InCookedData.Media)
	{
		CookMediaToSandbox(Media, WriteAdditionalFile);
	}
	UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("Done cooking Init SoundBank %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
}

void FWwiseResourceCookerImpl::CookMediaToSandbox(const FWwiseMediaCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking Media %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);

	if (UNLIKELY(InCookedData.MediaPathName.IsNone()))
	{
		UE_LOG(LogWwiseResourceCooker, Fatal, TEXT("Empty pathname for Media %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);
		return;
	}

	auto* ResourceLoader = GetResourceLoader();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	const FString GeneratedSoundBanksPath = ResourceLoader->GetUnrealGeneratedSoundBanksPath(InCookedData.MediaPathName);

	CookFileToSandbox(GeneratedSoundBanksPath, InCookedData.MediaPathName, WriteAdditionalFile);
}

void FWwiseResourceCookerImpl::CookShareSetToSandbox(const FWwiseShareSetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking ShareSet %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);
	for (const auto& SoundBank : InCookedData.SoundBanks)
	{
		CookSoundBankToSandbox(SoundBank, WriteAdditionalFile);
	}
	for (const auto& Media : InCookedData.Media)
	{
		CookMediaToSandbox(Media, WriteAdditionalFile);
	}
	UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("Done cooking ShareSet %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);
}

void FWwiseResourceCookerImpl::CookSoundBankToSandbox(const FWwiseSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile)
{
	UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("Cooking SoundBank %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);

	if (UNLIKELY(InCookedData.SoundBankPathName.IsNone()))
	{
		UE_LOG(LogWwiseResourceCooker, Fatal, TEXT("Empty pathname for SoundBank %s %" PRIu32), *InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		return;
	}

	auto* ResourceLoader = GetResourceLoader();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}
	const FString GeneratedSoundBanksPath = ResourceLoader->GetUnrealGeneratedSoundBanksPath(InCookedData.SoundBankPathName);

	CookFileToSandbox(GeneratedSoundBanksPath, InCookedData.SoundBankPathName, WriteAdditionalFile);
}

void FWwiseResourceCookerImpl::CookFileToSandbox(const FString& InInputPathName, const FName& InOutputPathName, WriteAdditionalFileFunction WriteAdditionalFile, bool bInStageRelativeToContent)
{
	auto* ResourceLoader = GetResourceLoader();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}

	FString StagePath = bInStageRelativeToContent
		? SandboxRootPath / InOutputPathName.ToString()
		: SandboxRootPath / ResourceLoader->GetUnrealStagePath(InOutputPathName);
	auto& StageFiles = CookingCache->StagedFiles;

	if (const auto* AlreadyStaged = StageFiles.Find(StagePath))
	{
		if (*AlreadyStaged == InInputPathName)
		{
			UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("Cook: Skipping already present file %s -> %s"), *InInputPathName, *StagePath);
		}
		else
		{
			UE_LOG(LogWwiseResourceCooker, Error, TEXT("Cook: Trying to stage two different files to the same path: [%s and %s] -> %s"), *InInputPathName, **AlreadyStaged, *StagePath);
		}

		return;
	}
	StageFiles.Add(StagePath, InInputPathName);

	if (auto* MappedHandle = FPlatformFileManager::Get().GetPlatformFile().OpenMapped(*InInputPathName))
	{
		auto* MappedRegion = MappedHandle->MapRegion();
		if (MappedRegion)
		{
			UE_LOG(LogWwiseResourceCooker, Display, TEXT("Adding file %s [%" PRIi64 " bytes]"), *StagePath, MappedRegion->GetMappedSize());
			WriteAdditionalFile(*StagePath, (void*)MappedRegion->GetMappedPtr(), MappedRegion->GetMappedSize());
			delete MappedRegion;
			delete MappedHandle;
			return;
		}
		else
		{
			delete MappedHandle;
		}
	}

	TArray<uint8> Data;
	if (!FFileHelper::LoadFileToArray(Data, *InInputPathName))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("Cook: Could not read file %s"), *InInputPathName);
		return;
	}

	UE_LOG(LogWwiseResourceCooker, Display, TEXT("Adding file %s [%" PRIi64 " bytes]"), *StagePath, (int64)Data.Num());
	WriteAdditionalFile(*StagePath, (void*)Data.GetData(), Data.Num());
}

bool FWwiseResourceCookerImpl::GetAcousticTextureCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetAcousticTextureCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetAcousticTextureCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	FWwiseRefAcousticTexture AcousticTextureRef;

	if (UNLIKELY(!PlatformData->GetRef(AcousticTextureRef, FWwiseSharedLanguageId(), InInfo)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAcousticTextureCookedData (%s %" PRIu32 " %s): No acoustic texture data found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const auto* AcousticTexture = AcousticTextureRef.GetAcousticTexture();

	OutCookedData.AbsorptionLow = AcousticTexture->AbsorptionLow;
	OutCookedData.AbsorptionMidLow = AcousticTexture->AbsorptionMidLow;
	OutCookedData.AbsorptionMidHigh = AcousticTexture->AbsorptionMidHigh;
	OutCookedData.AbsorptionHigh = AcousticTexture->AbsorptionHigh;
	OutCookedData.ShortId = AcousticTexture->Id;
	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutCookedData.DebugName = FName();
	}
	else
	{
		OutCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? AcousticTexture->Name : AcousticTexture->ObjectPath);
	}

	return true;
}

bool FWwiseResourceCookerImpl::GetAuxBusCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const auto* PlatformInfo = PlatformData->PlatformRef.GetPlatformInfo();
	if (UNLIKELY(!PlatformInfo)) return false;

	const TSet<FWwiseSharedLanguageId>& Languages = DataStructure.GetLanguages();

	TMap<FWwiseSharedLanguageId, TSet<FWwiseRefAuxBus>> RefLanguageMap;
	PlatformData->GetRefMap(RefLanguageMap, Languages, InInfo);
	if (UNLIKELY(RefLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): No ref found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	OutCookedData.AuxBusLanguageMap.Empty(RefLanguageMap.Num());

	for (auto& Ref : RefLanguageMap)
	{
		FWwiseAuxBusCookedData CookedData;

		TSet<FWwiseSoundBankCookedData> SoundBankSet;
		TSet<FWwiseMediaCookedData> MediaSet;
		TSet<FWwiseRefAuxBus>& AuxBusses = Ref.Value;
		
		if (UNLIKELY(AuxBusses.Num() == 0))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): Empty ref for language"),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
			return false;
		}

		// Set up basic global Aux Bus information
		{
			TSet<FWwiseRefAuxBus>::TConstIterator FirstAuxBus(AuxBusses);
			CookedData.AuxBusId = FirstAuxBus->AuxBusId();
			if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
			{
				OutCookedData.DebugName = FName();
			}
			else
			{
				CookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? FirstAuxBus->AuxBusName() : FirstAuxBus->AuxBusObjectPath());
				OutCookedData.DebugName = CookedData.DebugName;
			}
			OutCookedData.AuxBusId = CookedData.AuxBusId;
		}

		for (auto& AuxBusRef : AuxBusses)
		{
			const auto* AuxBus = AuxBusRef.GetAuxBus();
			if (UNLIKELY(!AuxBus))
			{
				UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): Could not get AuxBus from Ref"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				continue;
			}

			TSet<const FWwiseRefAuxBus*> SubAuxBusRefs;
			AuxBusRef.GetAllAuxBusRefs(SubAuxBusRefs, PlatformData->AuxBusses);
			for (const auto* SubAuxBusRef : SubAuxBusRefs)
			{
				const auto* SoundBank = SubAuxBusRef->GetSoundBank();
				if (UNLIKELY(!SoundBank))
				{
					UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Ref"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					continue;
				}

				if (!SoundBank->IsInitBank())
				{
					FWwiseSoundBankCookedData SoundBankCookedData;
					if (UNLIKELY(!FillSoundBankBaseInfo(SoundBankCookedData, *PlatformInfo, *SoundBank)))
					{
						UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Data"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						continue;
					}
					SoundBankSet.Add(SoundBankCookedData);
				}

				{
					WwiseCustomPluginIdsMap CustomPluginsRefs = SubAuxBusRef->GetAuxBusCustomPlugins(PlatformData->CustomPlugins);
					for (const auto& Plugin : CustomPluginsRefs)
					{
						const WwiseMediaIdsMap MediaRefs = Plugin.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
							{
								return false;
							}
						}
					}
				}

				{
					WwisePluginShareSetIdsMap ShareSetRefs = SubAuxBusRef->GetAuxBusPluginShareSets(PlatformData->PluginShareSets);
					for (const auto& ShareSet : ShareSetRefs)
					{
						const WwiseMediaIdsMap MediaRefs = ShareSet.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
							{
								return false;
							}
						}
					}
				}

				{
					WwiseAudioDeviceIdsMap AudioDevicesRefs = SubAuxBusRef->GetAuxBusAudioDevices(PlatformData->AudioDevices);
					for (const auto& AudioDevice : AudioDevicesRefs)
					{
						const WwiseMediaIdsMap MediaRefs = AudioDevice.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
							{
								return false;
							}
						}
					}
				}
			}
		}
		CookedData.SoundBanks = SoundBankSet.Array();
		CookedData.Media = MediaSet.Array();

		OutCookedData.AuxBusLanguageMap.Add(FWwiseLanguageCookedData(Ref.Key.GetLanguageId(), Ref.Key.GetLanguageName(), Ref.Key.LanguageRequirement), MoveTemp(CookedData));
	}

	if (UNLIKELY(OutCookedData.AuxBusLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): No AuxBus"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	// Make this a SFX if all CookedData are identical
	{
		auto& Map = OutCookedData.AuxBusLanguageMap;
		TArray<FWwiseLanguageCookedData> Keys;
		Map.GetKeys(Keys);

		auto LhsKey = Keys.Pop(false);
		const auto* Lhs = Map.Find(LhsKey);
		while (Keys.Num() > 0)
		{
			auto RhsKey = Keys.Pop(false);
			const auto* Rhs = Map.Find(RhsKey);

			if (Lhs->AuxBusId != Rhs->AuxBusId
				|| Lhs->DebugName != Rhs->DebugName
				|| Lhs->SoundBanks.Num() != Rhs->SoundBanks.Num()
				|| Lhs->Media.Num() != Rhs->Media.Num())
			{
				UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): AuxBus has languages"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				return true;
			}
			for (const auto& Elem : Lhs->SoundBanks)
			{
				if (!Rhs->SoundBanks.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): AuxBus has languages due to banks"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
			for (const auto& Elem : Lhs->Media)
			{
				if (!Rhs->Media.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): AuxBus has languages due to media"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
		}

		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): AuxBus is a SFX"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		std::remove_reference_t<decltype(Map)> SfxMap;
		SfxMap.Add(FWwiseLanguageCookedData::Sfx, *Lhs);

		Map = SfxMap;
	}

	return true;
}

bool FWwiseResourceCookerImpl::GetEventCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetEventCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetEventCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const TSet<FWwiseSharedLanguageId>& Languages = DataStructure.GetLanguages();

	const auto* PlatformInfo = PlatformData->PlatformRef.GetPlatformInfo();
	if (UNLIKELY(!PlatformInfo))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): No Platform Info"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	TMap<FWwiseSharedLanguageId, TSet<FWwiseRefEvent>> RefLanguageMap;
	PlatformData->GetRefMap(RefLanguageMap, Languages, InInfo);
	if (UNLIKELY(RefLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): No ref found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	OutCookedData.EventLanguageMap.Empty(RefLanguageMap.Num());
	UE_LOG(LogWwiseResourceCooker, Verbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Adding %d languages to map"),
		*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString(), RefLanguageMap.Num());

	for (auto& Ref : RefLanguageMap)
	{
		FWwiseEventCookedData CookedData;

		TSet<FWwiseSoundBankCookedData> SoundBankSet;
		TSet<FWwiseMediaCookedData> MediaSet;

		const FWwiseSharedLanguageId& LanguageId = Ref.Key;
		TSet<FWwiseRefEvent>& Events = Ref.Value;
		WwiseSwitchContainerArray SwitchContainerRefs;

		if (UNLIKELY(Events.Num() == 0))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Empty ref for language"),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
			return false;
		}

		// Set up basic global Event information
		{
			TSet<FWwiseRefEvent>::TConstIterator FirstEvent(Events);
			CookedData.EventId = FirstEvent->EventId();
			if (ExportDebugNameRule != EWwiseExportDebugNameRule::Release)
			{
				CookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? FirstEvent->EventName() : FirstEvent->EventObjectPath());
				OutCookedData.DebugName = CookedData.DebugName;
			}

			OutCookedData.EventId = CookedData.EventId;
			SwitchContainerRefs = FirstEvent->GetSwitchContainers(PlatformData->SwitchContainersByEvent);
		}

		// Add extra events recursively
		{
			TSet<FWwiseRefEvent> DiffEvents = Events;
			while (true)
			{
				bool bHaveMore = false;
				TSet<FWwiseRefEvent> OldEvents(Events);
				for (auto& EventRef : OldEvents)
				{
					const FWwiseMetadataEvent* Event = EventRef.GetEvent();
					if (UNLIKELY(!Event))
					{
						UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not get Event from Ref"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						return false;
					}

					for (const auto& ActionPostEvent : Event->ActionPostEvent)
					{
						bool bHaveMoreInThisEvent = PlatformData->GetRef(Events, LanguageId, FWwiseEventInfo(ActionPostEvent.Id, ActionPostEvent.Name));
						bHaveMore = bHaveMore || bHaveMoreInThisEvent;
					}
				}
				if (bHaveMore)
				{
					DiffEvents = Events.Difference(OldEvents);
					if (DiffEvents.Num() == 0)
					{
						UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): GetRef should return false when no more additional Refs"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						break;
					}
					for (auto& EventRef : DiffEvents)
					{
						SwitchContainerRefs.Append(EventRef.GetSwitchContainers(PlatformData->SwitchContainersByEvent));
					}
				}
				else
				{
					break;
				}
			}
		}

		// Add mandatory SoundBank information
		TSet<FWwiseExternalSourceCookedData> ExternalSourceSet;
		TSet<FWwiseAnyRef> RequiredGroupValueSet;
		for (auto& EventRef : Events)
		{
			const FWwiseMetadataEvent* Event = EventRef.GetEvent();
			if (UNLIKELY(!Event))
			{
				UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not get Event from Ref"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				return false;
			}

			if (LIKELY(Event->IsMandatory()) || LIKELY(Events.Num() == 1))
			{
				// Add main SoundBank
				{
					const auto* SoundBank = EventRef.GetSoundBank();
					if (UNLIKELY(!SoundBank))
					{
						UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Ref"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						return false;
					}
					if (!SoundBank->IsInitBank())
					{
						FWwiseSoundBankCookedData MainSoundBank;
						if (UNLIKELY(!FillSoundBankBaseInfo(MainSoundBank, *PlatformInfo, *SoundBank)))
						{
							UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Data"),
								*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
							return false;
						}
						SoundBankSet.Add(MainSoundBank);
					}
				}

				// Get Aux Bus banks & media
				{
					WwiseAuxBusIdsMap AuxBusRefs = EventRef.GetEventAuxBusses(PlatformData->AuxBusses);
					TSet<const FWwiseRefAuxBus*> SubAuxBusRefs;
					for (const auto& AuxBusRef : AuxBusRefs)
					{
						AuxBusRef.Value.GetAllAuxBusRefs(SubAuxBusRefs, PlatformData->AuxBusses);
					}
					for (const auto* SubAuxBusRef : SubAuxBusRefs)
					{
						const auto* SoundBank = SubAuxBusRef->GetSoundBank();
						if (UNLIKELY(!SoundBank))
						{
							UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Ref"),
								*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
							return false;
						}

						if (!SoundBank->IsInitBank())
						{
							FWwiseSoundBankCookedData SoundBankCookedData;
							if (UNLIKELY(!FillSoundBankBaseInfo(SoundBankCookedData, *PlatformInfo, *SoundBank)))
							{
								UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Data"),
									*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
								return false;
							}
							SoundBankSet.Add(SoundBankCookedData);
						}

						{
							WwiseCustomPluginIdsMap CustomPluginsRefs = SubAuxBusRef->GetAuxBusCustomPlugins(PlatformData->CustomPlugins);
							for (const auto& Plugin : CustomPluginsRefs)
							{
								const WwiseMediaIdsMap MediaRefs = Plugin.Value.GetPluginMedia(PlatformData->MediaFiles);
								for (const auto& MediaRef : MediaRefs)
								{
									if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
									{
										return false;
									}
								}
							}
						}

						{
							WwisePluginShareSetIdsMap ShareSetRefs = SubAuxBusRef->GetAuxBusPluginShareSets(PlatformData->PluginShareSets);
							for (const auto& ShareSet : ShareSetRefs)
							{
								const WwiseMediaIdsMap MediaRefs = ShareSet.Value.GetPluginMedia(PlatformData->MediaFiles);
								for (const auto& MediaRef : MediaRefs)
								{
									if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
									{
										return false;
									}
								}
							}
						}

						{
							WwiseAudioDeviceIdsMap AudioDevicesRefs = SubAuxBusRef->GetAuxBusAudioDevices(PlatformData->AudioDevices);
							for (const auto& AudioDevice : AudioDevicesRefs)
							{
								const WwiseMediaIdsMap MediaRefs = AudioDevice.Value.GetPluginMedia(PlatformData->MediaFiles);
								for (const auto& MediaRef : MediaRefs)
								{
									if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
									{
										return false;
									}
								}
							}
						}
					}
				}

				// Get media
				{
					WwiseMediaIdsMap MediaRefs = EventRef.GetEventMedia(PlatformData->MediaFiles);
					for (const auto& MediaRef : MediaRefs)
					{
						if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, LanguageId, *PlatformData)))
						{
							return false;
						}
					}
				}

				// Get Media from custom plugins
				{
					WwiseCustomPluginIdsMap CustomPluginsRefs = EventRef.GetEventCustomPlugins(PlatformData->CustomPlugins);
					for (const auto& Plugin : CustomPluginsRefs)
					{
						const WwiseMediaIdsMap MediaRefs = Plugin.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, LanguageId, *PlatformData)))
							{
								return false;
							}
						}
					}
				}

				// Get Media from plugin ShareSets
				{
					WwisePluginShareSetIdsMap ShareSetRefs = EventRef.GetEventPluginShareSets(PlatformData->PluginShareSets);
					for (const auto& ShareSet : ShareSetRefs)
					{
						const WwiseMediaIdsMap MediaRefs = ShareSet.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, LanguageId, *PlatformData)))
							{
								return false;
							}
						}
					}
				}

				// Get Media from audio devices
				{
					WwiseAudioDeviceIdsMap AudioDevicesRefs = EventRef.GetEventAudioDevices(PlatformData->AudioDevices);
					for (const auto& AudioDevice : AudioDevicesRefs)
					{
						const WwiseMediaIdsMap MediaRefs = AudioDevice.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, LanguageId, *PlatformData)))
							{
								return false;
							}
						}
					}
				}

				// Get External Sources
				{
					WwiseExternalSourceIdsMap ExternalSourceRefs = EventRef.GetEventExternalSources(PlatformData->ExternalSources);
					for (const auto& ExternalSourceRef : ExternalSourceRefs)
					{
						if (UNLIKELY(!AddRequirementsForExternalSource(ExternalSourceSet, ExternalSourceRef.Value)))
						{
							return false;
						}
					}
				}

				// Get required GroupValues
				{
					for (const auto& Switch : EventRef.GetActionSetSwitch(PlatformData->Switches))
					{
						if (LIKELY(Switch.Value.IsValid()))
						{
							RequiredGroupValueSet.Add(FWwiseAnyRef::Create(Switch.Value));
						}
					}

					for (const auto& State : EventRef.GetActionSetState(PlatformData->States))
					{
						if (LIKELY(State.Value.IsValid()))
						{
							RequiredGroupValueSet.Add(FWwiseAnyRef::Create(State.Value));
						}
					}
				}
			}
		}

		// Get Switched Media, negating required switches.
		{
			TMap<FWwiseRefSwitchContainer, TSet<FWwiseAnyRef>> SwitchValuesMap;

			for (const auto& SwitchContainerRef : SwitchContainerRefs)
			{
				const auto* SwitchContainer = SwitchContainerRef.GetSwitchContainer();
				if (UNLIKELY(!SwitchContainer))
				{
					return false;
				}

				auto SwitchValues = TSet<FWwiseAnyRef>(SwitchContainerRef.GetSwitchValues(PlatformData->Switches, PlatformData->States));

				TSet<FWwiseAnyRef> SwitchesToRemove;
				for (const auto& SwitchValue : SwitchValues)
				{
					// Remove all SwitchValues if we load them all by default
					if (InInfo.SwitchContainerLoading == EWwiseEventSwitchContainerLoading::AlwaysLoad)
					{
						UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Skip value %s (%" PRIu32 ":%" PRIu32 "): Event Switch Container set to AlwaysLoad"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString(), *SwitchValue.GetName().ToString(), SwitchValue.GetGroupId(), SwitchValue.GetId());

						SwitchesToRemove.Add(SwitchValue);
						continue;
					}

					// Remove SwitchValues that are already present in RequiredGroupValueSet
					if (RequiredGroupValueSet.Contains(SwitchValue))
					{
						UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Skip value %s (%" PRIu32 ":%" PRIu32 "): Already in the required set"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString(), *SwitchValue.GetName().ToString(), SwitchValue.GetGroupId(), SwitchValue.GetId());

						SwitchesToRemove.Add(SwitchValue);
						continue;
					}

					// Remove SwitchValues that have an ID of "0" (wildcard in music)
					if (SwitchValue.GetId() == 0)
					{
						UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Skip value %s (%" PRIu32 ":%" PRIu32 "): Wildcard ID"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString(), *SwitchValue.GetName().ToString(), SwitchValue.GetGroupId(), SwitchValue.GetId());

						SwitchesToRemove.Add(SwitchValue);
						continue;
					}

					// Remove Switch groups that are controlled by a Game Parameter (RTPC)
					if (SwitchValue.GetType() == EWwiseRefType::Switch)
					{
						const auto* SwitchRef = SwitchValue.GetSwitchRef();
						check(SwitchRef);

						if (SwitchRef->IsControlledByGameParameter())
						{
							UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Skip value %s (%" PRIu32 ":%" PRIu32 "): Controlled by Game Parameter"),
								*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString(), *SwitchValue.GetName().ToString(), SwitchValue.GetGroupId(), SwitchValue.GetId());

							SwitchesToRemove.Add(SwitchValue);
							continue;
						}
					}
				}
				SwitchValues = SwitchValues.Difference(SwitchesToRemove);

				if (SwitchValues.Num() == 0)
				{
					// Media and SoundBank are compulsory. Add them so they are always loaded.
					const auto* SoundBank = SwitchContainerRef.GetSoundBank();
					if (UNLIKELY(!SoundBank))
					{
						UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Switch Container Ref"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						return false;
					}
					if (!SoundBank->IsInitBank())
					{
						FWwiseSoundBankCookedData SwitchContainerSoundBank;
						if (UNLIKELY(!FillSoundBankBaseInfo(SwitchContainerSoundBank, *PlatformInfo, *SoundBank)))
						{
							UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Switch Container Data"),
								*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
							return false;
						}
						SoundBankSet.Add(SwitchContainerSoundBank);
					}

					{
						TArray<FWwiseRefMedia> MediaToAdd;
						SwitchContainerRef.GetSwitchContainerMedia(PlatformData->MediaFiles).GenerateValueArray(MediaToAdd);
						for (const auto& MediaRef : MediaToAdd)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef, Ref.Key, *PlatformData)))
							{
								return false;
							}
						}
					}

					{
						WwiseCustomPluginIdsMap CustomPluginsRefs = SwitchContainerRef.GetSwitchContainerCustomPlugins(PlatformData->CustomPlugins);
						for (const auto& Plugin : CustomPluginsRefs)
						{
							const WwiseMediaIdsMap MediaRefs = Plugin.Value.GetPluginMedia(PlatformData->MediaFiles);
							for (const auto& MediaRef : MediaRefs)
							{
								if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, LanguageId, *PlatformData)))
								{
									return false;
								}
							}
						}
					}

					{
						WwisePluginShareSetIdsMap ShareSetRefs = SwitchContainerRef.GetSwitchContainerPluginShareSets(PlatformData->PluginShareSets);
						for (const auto& ShareSet : ShareSetRefs)
						{
							const WwiseMediaIdsMap MediaRefs = ShareSet.Value.GetPluginMedia(PlatformData->MediaFiles);
							for (const auto& MediaRef : MediaRefs)
							{
								if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, LanguageId, *PlatformData)))
								{
									return false;
								}
							}
						}
					}

					{
						WwiseAudioDeviceIdsMap AudioDevicesRefs = SwitchContainerRef.GetSwitchContainerAudioDevices(PlatformData->AudioDevices);
						for (const auto& AudioDevice : AudioDevicesRefs)
						{
							const WwiseMediaIdsMap MediaRefs = AudioDevice.Value.GetPluginMedia(PlatformData->MediaFiles);
							for (const auto& MediaRef : MediaRefs)
							{
								if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, LanguageId, *PlatformData)))
								{
									return false;
								}
							}
						}
					}

					TArray<FWwiseRefExternalSource> ExternalSourcesToAdd;
					SwitchContainerRef.GetSwitchContainerExternalSources(PlatformData->ExternalSources).GenerateValueArray(ExternalSourcesToAdd);

					for (const auto& ExternalSourceRef : ExternalSourcesToAdd)
					{
						if (UNLIKELY(!AddRequirementsForExternalSource(ExternalSourceSet, ExternalSourceRef)))
						{
							return false;
						}
					}
				}
				else
				{
					// Media is optional. Will process later
					SwitchValuesMap.Add(SwitchContainerRef, SwitchValues);
				}
			}

			// Process Switch Containers that seemingly contain additional media and conditions
			for (const auto& SwitchContainerRef : SwitchContainerRefs)
			{
				const auto* SwitchValues = SwitchValuesMap.Find(SwitchContainerRef);
				if (!SwitchValues)
				{
					continue;
				}

				// Prepare media and main SoundBank to add
				TSet<FWwiseSoundBankCookedData> SoundBankSetToAdd;
				TSet<FWwiseMediaCookedData> MediaSetToAdd;
				TSet<FWwiseExternalSourceCookedData> ExternalSourceSetToAdd;

				const auto* SoundBank = SwitchContainerRef.GetSoundBank();
				if (UNLIKELY(!SoundBank))
				{
					UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Switch Container Ref"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return false;
				}
				if (!SoundBank->IsInitBank())
				{
					FWwiseSoundBankCookedData SwitchContainerSoundBank;
					if (UNLIKELY(!FillSoundBankBaseInfo(SwitchContainerSoundBank, *PlatformInfo, *SoundBank)))
					{
						UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Switch Container Data"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						return false;
					}
					SoundBankSetToAdd.Add(SwitchContainerSoundBank);
				}

				{
					TArray<FWwiseRefMedia> MediaToAdd;
					SwitchContainerRef.GetSwitchContainerMedia(PlatformData->MediaFiles).GenerateValueArray(MediaToAdd);
					FWwiseMediaCookedData MediaCookedData;
					for (const auto& MediaRef : MediaToAdd)
					{
						if (UNLIKELY(!AddRequirementsForMedia(SoundBankSetToAdd, MediaSetToAdd, MediaRef, Ref.Key, *PlatformData)))
						{
							return false;
						}
					}
				}

				{
					WwiseCustomPluginIdsMap CustomPluginsRefs = SwitchContainerRef.GetSwitchContainerCustomPlugins(PlatformData->CustomPlugins);
					for (const auto& Plugin : CustomPluginsRefs)
					{
						const WwiseMediaIdsMap MediaRefs = Plugin.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSetToAdd, MediaSetToAdd, MediaRef.Value, LanguageId, *PlatformData)))
							{
								return false;
							}
						}
					}
				}

				{
					WwisePluginShareSetIdsMap ShareSetRefs = SwitchContainerRef.GetSwitchContainerPluginShareSets(PlatformData->PluginShareSets);
					for (const auto& ShareSet : ShareSetRefs)
					{
						const WwiseMediaIdsMap MediaRefs = ShareSet.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSetToAdd, MediaSetToAdd, MediaRef.Value, LanguageId, *PlatformData)))
							{
								return false;
							}
						}
					}
				}

				{
					WwiseAudioDeviceIdsMap AudioDevicesRefs = SwitchContainerRef.GetSwitchContainerAudioDevices(PlatformData->AudioDevices);
					for (const auto& AudioDevice : AudioDevicesRefs)
					{
						const WwiseMediaIdsMap MediaRefs = AudioDevice.Value.GetPluginMedia(PlatformData->MediaFiles);
						for (const auto& MediaRef : MediaRefs)
						{
							if (UNLIKELY(!AddRequirementsForMedia(SoundBankSetToAdd, MediaSetToAdd, MediaRef.Value, LanguageId, *PlatformData)))
							{
								return false;
							}
						}
					}
				}


				TArray<FWwiseRefExternalSource> ExternalSourcesToAdd;
				SwitchContainerRef.GetSwitchContainerExternalSources(PlatformData->ExternalSources).GenerateValueArray(ExternalSourcesToAdd);

				for (const auto& ExternalSourceRef : ExternalSourcesToAdd)
				{
					if (UNLIKELY(!AddRequirementsForExternalSource(ExternalSourceSetToAdd, ExternalSourceRef)))
					{
						return false;
					}
				}

				SoundBankSetToAdd = SoundBankSetToAdd.Difference(SoundBankSet);
				MediaSetToAdd = MediaSetToAdd.Difference(MediaSet);
				ExternalSourceSetToAdd = ExternalSourceSetToAdd.Difference(ExternalSourceSet);

				// Have we already included all the external banks and media
				if (SoundBankSetToAdd.Num() == 0 && MediaSetToAdd.Num() == 0 && ExternalSourceSetToAdd.Num() == 0)
				{
					continue;
				}

				// Fill up SwitchContainerCookedData and add it to SwitchContainerLeaves
				FWwiseSwitchContainerLeafCookedData SwitchContainerCookedData;
				for (const auto& SwitchValue : *SwitchValues)
				{
					FWwiseGroupValueCookedData SwitchCookedData;
					switch (SwitchValue.GetType())
					{
					case EWwiseRefType::Switch: SwitchCookedData.Type = EWwiseGroupType::Switch; break;
					case EWwiseRefType::State: SwitchCookedData.Type = EWwiseGroupType::State; break;
					default: SwitchCookedData.Type = EWwiseGroupType::Unknown;
					}
					SwitchCookedData.GroupId = SwitchValue.GetGroupId();
					SwitchCookedData.Id = SwitchValue.GetId();
					if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
					{
						SwitchCookedData.DebugName = FName();
					}
					else
					{
						SwitchCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? SwitchValue.GetName() : SwitchValue.GetObjectPath());
					}
					SwitchContainerCookedData.GroupValueSet.Add(MoveTemp(SwitchCookedData));
				}
				if (auto* ExistingSwitchedMedia = CookedData.SwitchContainerLeaves.FindByPredicate([&SwitchContainerCookedData](const FWwiseSwitchContainerLeafCookedData& RhsValue)
				{
					return RhsValue.GroupValueSet.Difference(SwitchContainerCookedData.GroupValueSet).Num() == 0
						&& SwitchContainerCookedData.GroupValueSet.Difference(RhsValue.GroupValueSet).Num() == 0;
				}))
				{
					SoundBankSetToAdd.Append(ExistingSwitchedMedia->SoundBanks);
					MediaSetToAdd.Append(ExistingSwitchedMedia->Media);
					ExternalSourceSetToAdd.Append(ExistingSwitchedMedia->ExternalSources);

					ExistingSwitchedMedia->SoundBanks = SoundBankSetToAdd.Array();
					ExistingSwitchedMedia->Media = MediaSetToAdd.Array();
					ExistingSwitchedMedia->ExternalSources = ExternalSourceSetToAdd.Array();
				}
				else
				{
					SwitchContainerCookedData.SoundBanks = SoundBankSetToAdd.Array();
					SwitchContainerCookedData.Media = MediaSetToAdd.Array();
					SwitchContainerCookedData.ExternalSources = ExternalSourceSetToAdd.Array();
					CookedData.SwitchContainerLeaves.Add(MoveTemp(SwitchContainerCookedData));
				}
			}
		}

		// Finalize banks and media
		CookedData.SoundBanks.Append(SoundBankSet.Array());
		if (CookedData.SoundBanks.Num() == 0)
		{
			UE_LOG(LogWwiseResourceCooker, Log, TEXT("GetEventCookedData (%s %" PRIu32 " %s): No SoundBank set for Event. Unless Switch values are properly set, no SoundBank will be loaded."),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		}
		CookedData.Media.Append(MediaSet.Array());
		CookedData.ExternalSources.Append(ExternalSourceSet.Array());

		for (const auto& SwitchRef : RequiredGroupValueSet)
		{
			FWwiseGroupValueCookedData SwitchCookedData;
			switch (SwitchRef.GetType())
			{
			case EWwiseRefType::Switch: SwitchCookedData.Type = EWwiseGroupType::Switch; break;
			case EWwiseRefType::State: SwitchCookedData.Type = EWwiseGroupType::State; break;
			default: SwitchCookedData.Type = EWwiseGroupType::Unknown;
			}
			SwitchCookedData.GroupId = SwitchRef.GetGroupId();
			SwitchCookedData.Id = SwitchRef.GetId();
			if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
			{
				SwitchCookedData.DebugName = FName();
			}
			else
			{
				SwitchCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? SwitchRef.GetName() : SwitchRef.GetObjectPath());
			}

			CookedData.RequiredGroupValueSet.Add(MoveTemp(SwitchCookedData));
		}

		CookedData.DestroyOptions = InInfo.DestroyOptions;

		OutCookedData.EventLanguageMap.Add(FWwiseLanguageCookedData(LanguageId.GetLanguageId(), LanguageId.GetLanguageName(), LanguageId.LanguageRequirement), MoveTemp(CookedData));
	}

	if (UNLIKELY(OutCookedData.EventLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetEventCookedData (%s %" PRIu32 " %s): No Event"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	// Make this a SFX if all CookedData are identical
	{
		auto& Map = OutCookedData.EventLanguageMap;
		TArray<FWwiseLanguageCookedData> Keys;
		Map.GetKeys(Keys);

		auto LhsKey = Keys.Pop(false);
		const auto* Lhs = Map.Find(LhsKey);
		while (Keys.Num() > 0)
		{
			auto RhsKey = Keys.Pop(false);
			const auto* Rhs = Map.Find(RhsKey);

			if (Lhs->EventId != Rhs->EventId
				|| Lhs->DebugName != Rhs->DebugName
				|| Lhs->SoundBanks.Num() != Rhs->SoundBanks.Num()
				|| Lhs->ExternalSources.Num() != Rhs->ExternalSources.Num()
				|| Lhs->Media.Num() != Rhs->Media.Num()
				|| Lhs->RequiredGroupValueSet.Num() != Rhs->RequiredGroupValueSet.Num()
				|| Lhs->SwitchContainerLeaves.Num() != Rhs->SwitchContainerLeaves.Num())
			{
				UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				return true;
			}
			for (const auto& Elem : Lhs->SoundBanks)
			{
				if (!Rhs->SoundBanks.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to banks"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
			for (const auto& Elem : Lhs->ExternalSources)
			{
				if (!Rhs->ExternalSources.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to external sources"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
			for (const auto& Elem : Lhs->Media)
			{
				if (!Rhs->Media.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to media"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
			for (const auto& Elem : Lhs->RequiredGroupValueSet)
			{
				if (!Rhs->RequiredGroupValueSet.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to required group values"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
			for (const auto& LhsLeaf : Lhs->SwitchContainerLeaves)
			{
				const auto RhsLeafIndex = Rhs->SwitchContainerLeaves.Find(LhsLeaf);
				if (RhsLeafIndex == INDEX_NONE)
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to switch container"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
				const auto& RhsLeaf = Rhs->SwitchContainerLeaves[RhsLeafIndex];

				for (const auto& Elem : LhsLeaf.SoundBanks)
				{
					if (!RhsLeaf.SoundBanks.Contains(Elem))
					{
						UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to banks in switch container"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						return true;
					}
				}
				for (const auto& Elem : LhsLeaf.ExternalSources)
				{
					if (!RhsLeaf.ExternalSources.Contains(Elem))
					{
						UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to external sources in switch container"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						return true;
					}
				}
				for (const auto& Elem : LhsLeaf.Media)
				{
					if (!RhsLeaf.Media.Contains(Elem))
					{
						UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event has languages due to media in switch container"),
							*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
						return true;
					}
				}
			}
		}

		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetEventCookedData (%s %" PRIu32 " %s): Event is a SFX"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		std::remove_reference_t<decltype(Map)> SfxMap;
		SfxMap.Add(FWwiseLanguageCookedData::Sfx, *Lhs);

		Map = SfxMap;
	}

	return true;
}

bool FWwiseResourceCookerImpl::GetExternalSourceCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetExternalSourceCookedData (%" PRIu32 "): ProjectDatabase not initialized"),
			InCookie);
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetExternalSourceCookedData (%" PRIu32 "): No data for platform"),
			InCookie);
		return false;
	}

	const auto LocalizableId = FWwiseDatabaseLocalizableIdKey(InCookie, FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE);
	const auto* ExternalSourceRef = PlatformData->ExternalSources.Find(LocalizableId);
	if (UNLIKELY(!ExternalSourceRef))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetExternalSourceCookedData (%" PRIu32 "): Could not find External Source"),
			InCookie);
		return false;
	}
	const auto* ExternalSource = ExternalSourceRef->GetExternalSource();
	if (UNLIKELY(!ExternalSource))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetExternalSourceCookedData (%" PRIu32 "): Could not get External Source"),
			InCookie);
		return false;
	}

	if (UNLIKELY(!FillExternalSourceBaseInfo(OutCookedData, *ExternalSource)))
	{
		return false;
	}
	return true;
}

bool FWwiseResourceCookerImpl::GetGameParameterCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetGameParameterCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetGameParameterCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	FWwiseRefGameParameter GameParameterRef;

	if (UNLIKELY(!PlatformData->GetRef(GameParameterRef, FWwiseSharedLanguageId(), InInfo)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetGameParameterCookedData (%s %" PRIu32 " %s): No game parameter found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const auto* GameParameter = GameParameterRef.GetGameParameter();

	OutCookedData.ShortId = GameParameter->Id;
	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutCookedData.DebugName = FName();
	}
	else
	{
		OutCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? GameParameter->Name : GameParameter->ObjectPath);
	}

	return true;
}

bool FWwiseResourceCookerImpl::GetInitBankCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetInitBankCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetInitBankCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const auto* PlatformInfo = PlatformData->PlatformRef.GetPlatformInfo();
	if (UNLIKELY(!PlatformInfo))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetInitBankCookedData (%s %" PRIu32 " %s): No Platform Info"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	FWwiseRefSoundBank SoundBankRef;
	if (UNLIKELY(!PlatformData->GetRef(SoundBankRef, FWwiseSharedLanguageId(), InInfo)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetInitBankCookedData (%s %" PRIu32 " %s): No ref found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	if (UNLIKELY(!SoundBankRef.IsInitBank()))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetInitBankCookedData (%s %" PRIu32 " %s): Not an init SoundBank"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	{
		const auto* SoundBank = SoundBankRef.GetSoundBank();
		if (UNLIKELY(!SoundBank))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetInitBankCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Ref"),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
			return false;
		}
		if (UNLIKELY(!FillSoundBankBaseInfo(OutCookedData, *PlatformInfo, *SoundBank)))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetInitBankCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Data"),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
			return false;
		}

		// Add all Init SoundBank media
		TSet<FWwiseSoundBankCookedData> SoundBankSet;
		TSet<FWwiseMediaCookedData> MediaSet;
		{
			for (const auto& MediaRef : SoundBankRef.GetSoundBankMedia(PlatformData->MediaFiles))
			{
				if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
				{
					return false;
				}
			}
		}

		{
			WwiseCustomPluginIdsMap CustomPluginsRefs = SoundBankRef.GetSoundBankCustomPlugins(PlatformData->CustomPlugins);
			for (const auto& Plugin : CustomPluginsRefs)
			{
				const WwiseMediaIdsMap MediaRefs = Plugin.Value.GetPluginMedia(PlatformData->MediaFiles);
				for (const auto& MediaRef : MediaRefs)
				{
					if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
					{
						return false;
					}
				}
			}
		}

		{
			WwisePluginShareSetIdsMap ShareSetRefs = SoundBankRef.GetSoundBankPluginShareSets(PlatformData->PluginShareSets);
			for (const auto& ShareSet : ShareSetRefs)
			{
				const WwiseMediaIdsMap MediaRefs = ShareSet.Value.GetPluginMedia(PlatformData->MediaFiles);
				for (const auto& MediaRef : MediaRefs)
				{
					if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
					{
						return false;
					}
				}
			}
		}

		{
			WwiseAudioDeviceIdsMap AudioDevicesRefs = SoundBankRef.GetSoundBankAudioDevices(PlatformData->AudioDevices);
			for (const auto& AudioDevice : AudioDevicesRefs)
			{
				const WwiseMediaIdsMap MediaRefs = AudioDevice.Value.GetPluginMedia(PlatformData->MediaFiles);
				for (const auto& MediaRef : MediaRefs)
				{
					if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
					{
						return false;
					}
				}
			}
		}

		OutCookedData.SoundBanks = SoundBankSet.Array();
		OutCookedData.Media = MediaSet.Array();
		const TSet<FWwiseSharedLanguageId>& Languages = DataStructure.GetLanguages();
		OutCookedData.Language.Empty(Languages.Num());
		for (const FWwiseSharedLanguageId& Language : Languages)
		{
			OutCookedData.Language.Add({ Language.GetLanguageId(), Language.GetLanguageName(), Language.LanguageRequirement });
		}

		if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
		{
			OutCookedData.DebugName = FName();
		}
		else
		{
			OutCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? SoundBank->ShortName : SoundBank->ObjectPath);
		}
	}

	return true;
}

bool FWwiseResourceCookerImpl::GetMediaCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetMediaCookedData (%" PRIu32 "): ProjectDatabase not initialized"),
			InInfo.WwiseShortId);
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetMediaCookedData (%" PRIu32 "): No data for platform"),
			InInfo.WwiseShortId);
		return false;
	}

	auto MediaId = FWwiseDatabaseMediaIdKey(InInfo.WwiseShortId, InInfo.HardCodedSoundBankShortId);
	const auto* MediaRef = PlatformData->MediaFiles.Find(MediaId);
	if (UNLIKELY(!MediaRef))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetMediaCookedData (%" PRIu32 "): Could not find Media in SoundBank %" PRIu32),
			InInfo.WwiseShortId, InInfo.HardCodedSoundBankShortId);
		return false;
	}

	const FWwiseSharedLanguageId* LanguageRefPtr = nullptr;
	if (MediaRef->LanguageId)
	{
		const auto& Languages = DataStructure.GetLanguages();
		LanguageRefPtr = Languages.Find(FWwiseSharedLanguageId(MediaRef->LanguageId, TEXT(""), EWwiseLanguageRequirement::IsOptional));
		if (UNLIKELY(!LanguageRefPtr))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetMediaCookedData (%" PRIu32 "): Could not find language %" PRIu32),
				InInfo.WwiseShortId, MediaRef->LanguageId);
			return false;
		}
	}
	const auto& LanguageRef = LanguageRefPtr ? *LanguageRefPtr : FWwiseSharedLanguageId();

	TSet<FWwiseSoundBankCookedData> SoundBankSet;
	TSet<FWwiseMediaCookedData> MediaSet;
	if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, *MediaRef, LanguageRef, *PlatformData)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetMediaCookedData (%" PRIu32 "): Could not get requirements for media."),
			InInfo.WwiseShortId);
		return false;
	}

	if (UNLIKELY(SoundBankSet.Num() > 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetMediaCookedData (%" PRIu32 "): Asking for a media in a particular SoundBank (%" PRIu32 ") must have it fully defined in this SoundBank."),
			InInfo.WwiseShortId, InInfo.HardCodedSoundBankShortId);
		return false;
	}

	if (MediaSet.Num() == 0)
	{
		// Not directly an error: Media is in this SoundBank, without streaming. Can be a logical error, but it's not our error.
		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetMediaCookedData (%" PRIu32 "): Media is fully in SoundBank. Returning no media."),
			InInfo.WwiseShortId);
		return false;
	}

	auto Media = MediaSet.Array()[0];

	OutCookedData = MoveTemp(Media);
	return true;
}

bool FWwiseResourceCookerImpl::GetShareSetCookedData(FWwiseLocalizedShareSetCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const TSet<FWwiseSharedLanguageId>& Languages = DataStructure.GetLanguages();
				
	const auto* PlatformInfo = PlatformData->PlatformRef.GetPlatformInfo();
	if (UNLIKELY(!PlatformInfo))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): No Platform Info"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	TMap<FWwiseSharedLanguageId, TSet<FWwiseRefPluginShareSet>> RefLanguageMap;
	PlatformData->GetRefMap(RefLanguageMap, Languages, InInfo);
	if (UNLIKELY(RefLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): No ref found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	OutCookedData.ShareSetLanguageMap.Empty(RefLanguageMap.Num());

	for (auto& Ref : RefLanguageMap)
	{
		FWwiseShareSetCookedData CookedData;
		TSet<FWwiseSoundBankCookedData> SoundBankSet;
		TSet<FWwiseMediaCookedData> MediaSet;
		TSet<FWwiseRefPluginShareSet>& ShareSets = Ref.Value;

		if (UNLIKELY(ShareSets.Num() == 0))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetAuxBusCookedData (%s %" PRIu32 " %s): Empty ref for language"),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
			return false;
		}

		// Set up basic global Aux bus information
		{
			TSet<FWwiseRefPluginShareSet>::TConstIterator FirstShareSet(ShareSets);
			CookedData.ShareSetId = FirstShareSet->PluginShareSetId();
			if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
			{
				OutCookedData.DebugName = FName();
			}
			else
			{
				CookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? FirstShareSet->PluginShareSetName() : FirstShareSet->PluginShareSetObjectPath());
				OutCookedData.DebugName = CookedData.DebugName;
			}
			OutCookedData.ShareSetId = CookedData.ShareSetId;
		}
		for (auto& ShareSetRef : ShareSets)
		{

			const auto* PluginShareSet = ShareSetRef.GetPlugin();
			if (UNLIKELY(!PluginShareSet))
			{
				UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): Could not get ShareSet from Ref"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				return false;
			}
			CookedData.ShareSetId = PluginShareSet->Id;

			const auto* SoundBank = ShareSetRef.GetSoundBank();
			if (UNLIKELY(!SoundBank))
			{
				UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Ref"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				return false;
			}
			if (!SoundBank->IsInitBank())
			{
				FWwiseSoundBankCookedData MainSoundBank;
				if (UNLIKELY(!FillSoundBankBaseInfo(MainSoundBank, *PlatformInfo, *SoundBank)))
				{
					UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Data"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return false;
				}
				SoundBankSet.Add(MainSoundBank);
			}

			{
				WwiseCustomPluginIdsMap CustomPluginsRefs = ShareSetRef.GetPluginCustomPlugins(PlatformData->CustomPlugins);
				for (const auto& Plugin : CustomPluginsRefs)
				{
					const WwiseMediaIdsMap MediaRefs = Plugin.Value.GetPluginMedia(PlatformData->MediaFiles);
					for (const auto& MediaRef : MediaRefs)
					{
						if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
						{
							return false;
						}
					}
				}
			}

			{
				WwisePluginShareSetIdsMap ShareSetRefs = ShareSetRef.GetPluginPluginShareSets(PlatformData->PluginShareSets);
				for (const auto& ShareSet : ShareSetRefs)
				{
					const WwiseMediaIdsMap MediaRefs = ShareSet.Value.GetPluginMedia(PlatformData->MediaFiles);
					for (const auto& MediaRef : MediaRefs)
					{
						if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
						{
							return false;
						}
					}
				}
			}

			{
				WwiseAudioDeviceIdsMap AudioDevicesRefs = ShareSetRef.GetPluginAudioDevices(PlatformData->AudioDevices);
				for (const auto& AudioDevice : AudioDevicesRefs)
				{
					const WwiseMediaIdsMap MediaRefs = AudioDevice.Value.GetPluginMedia(PlatformData->MediaFiles);
					for (const auto& MediaRef : MediaRefs)
					{
						if (UNLIKELY(!AddRequirementsForMedia(SoundBankSet, MediaSet, MediaRef.Value, FWwiseSharedLanguageId(), *PlatformData)))
						{
							return false;
						}
					}
				}
			}
		}

		CookedData.SoundBanks = SoundBankSet.Array();
		CookedData.Media = MediaSet.Array();
		OutCookedData.ShareSetLanguageMap.Add(FWwiseLanguageCookedData(Ref.Key.GetLanguageId(), Ref.Key.GetLanguageName(), Ref.Key.LanguageRequirement), MoveTemp(CookedData));
	}

	if (UNLIKELY(OutCookedData.ShareSetLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): No ShareSet"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	// Make this a SFX if all CookedData are identical
	{
		auto& Map = OutCookedData.ShareSetLanguageMap;
		TArray<FWwiseLanguageCookedData> Keys;
		Map.GetKeys(Keys);

		auto LhsKey = Keys.Pop(false);
		const auto* Lhs = Map.Find(LhsKey);
		while (Keys.Num() > 0)
		{
			auto RhsKey = Keys.Pop(false);
			const auto* Rhs = Map.Find(RhsKey);

			if (Lhs->ShareSetId != Rhs->ShareSetId
				|| Lhs->DebugName != Rhs->DebugName
				|| Lhs->SoundBanks.Num() != Rhs->SoundBanks.Num()
				|| Lhs->Media.Num() != Rhs->Media.Num())
			{
				UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): ShareSet has languages"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				return true;
			}
			for (const auto& Elem : Lhs->SoundBanks)
			{
				if (!Rhs->SoundBanks.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): ShareSet has languages due to banks"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
			for (const auto& Elem : Lhs->Media)
			{
				if (!Rhs->Media.Contains(Elem))
				{
					UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): ShareSet has languages due to media"),
						*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
					return true;
				}
			}
		}

		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetShareSetCookedData (%s %" PRIu32 " %s): ShareSet is a SFX"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		std::remove_reference_t<decltype(Map)> SfxMap;
		SfxMap.Add(FWwiseLanguageCookedData::Sfx, *Lhs);

		Map = SfxMap;
	}

	return true;
}

bool FWwiseResourceCookerImpl::GetSoundBankCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const TSet<FWwiseSharedLanguageId>& Languages = DataStructure.GetLanguages();

	const auto* PlatformInfo = PlatformData->PlatformRef.GetPlatformInfo();
	if (UNLIKELY(!PlatformInfo))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): No Platform Info"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}
	
	TMap<FWwiseSharedLanguageId, FWwiseRefSoundBank> RefLanguageMap;
	PlatformData->GetRefMap(RefLanguageMap, Languages, InInfo);
	if (UNLIKELY(RefLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): No ref found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	OutCookedData.SoundBankLanguageMap.Empty(RefLanguageMap.Num());

	for (const auto& Ref : RefLanguageMap)
	{
		FWwiseSoundBankCookedData CookedData;
		const auto* SoundBank = Ref.Value.GetSoundBank();
		if (UNLIKELY(!SoundBank))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): Could not get SoundBank from Ref"),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
			return false;
		}
		if (UNLIKELY(!FillSoundBankBaseInfo(CookedData, *PlatformInfo, *SoundBank)))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): Could not fill SoundBank from Data"),
				*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
			return false;
		}

		if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
		{
			OutCookedData.DebugName = FName();
		}
		else
		{
			CookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? SoundBank->ShortName : SoundBank->ObjectPath);
			OutCookedData.DebugName = CookedData.DebugName;
		}

		OutCookedData.SoundBankId = CookedData.SoundBankId;
		OutCookedData.SoundBankLanguageMap.Add(FWwiseLanguageCookedData(Ref.Key.GetLanguageId(), Ref.Key.GetLanguageName(), Ref.Key.LanguageRequirement), MoveTemp(CookedData));
	}

	if (UNLIKELY(OutCookedData.SoundBankLanguageMap.Num() == 0))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): No SoundBank"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	// Make this a SFX if all CookedData are identical
	{
		auto& Map = OutCookedData.SoundBankLanguageMap;
		TArray<FWwiseLanguageCookedData> Keys;
		Map.GetKeys(Keys);

		auto LhsKey = Keys.Pop(false);
		const auto* Lhs = Map.Find(LhsKey);
		while (Keys.Num() > 0)
		{
			auto RhsKey = Keys.Pop(false);
			const auto* Rhs = Map.Find(RhsKey);

			if (GetTypeHash(*Lhs) != GetTypeHash(*Rhs))
			{
				UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): SoundBank has languages"),
					*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
				return true;
			}
		}

		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("GetSoundBankCookedData (%s %" PRIu32 " %s): SoundBank is a SFX"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		std::remove_reference_t<decltype(Map)> SfxMap;
		SfxMap.Add(FWwiseLanguageCookedData::Sfx, *Lhs);

		Map = SfxMap;
	}
	return true;
}

bool FWwiseResourceCookerImpl::GetStateCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetStateCookedData (%s %" PRIu32 " %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetStateCookedData (%s %" PRIu32 " %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	FWwiseRefState StateRef;
	if (UNLIKELY(!PlatformData->GetRef(StateRef, FWwiseSharedLanguageId(), InInfo)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetStateCookedData (%s %" PRIu32 " %" PRIu32 " %s): No state found"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}
	const auto* State = StateRef.GetState();
	const auto* StateGroup = StateRef.GetStateGroup();
	if (UNLIKELY(!State || !StateGroup))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetStateCookedData (%s %" PRIu32 " %" PRIu32 " %s): No state in ref"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	OutCookedData.Type = EWwiseGroupType::State;
	OutCookedData.GroupId = StateGroup->Id;
	OutCookedData.Id = State->Id;
	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutCookedData.DebugName = FName();
	}
	else
	{
		OutCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? State->Name : State->ObjectPath);
	}
	return true;
}

bool FWwiseResourceCookerImpl::GetSwitchCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetSwitchCookedData (%s %" PRIu32 " %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetSwitchCookedData (%s %" PRIu32 " %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	FWwiseRefSwitch SwitchRef;
	if (UNLIKELY(!PlatformData->GetRef(SwitchRef, FWwiseSharedLanguageId(), InInfo)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetSwitchCookedData (%s %" PRIu32 " %" PRIu32 " %s): No switch found"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}
	const auto* Switch = SwitchRef.GetSwitch();
	const auto* SwitchGroup = SwitchRef.GetSwitchGroup();
	if (UNLIKELY(!Switch || !SwitchGroup))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetSwitchCookedData (%s %" PRIu32 " %" PRIu32 " %s): No switch in ref"),
			*InInfo.WwiseGuid.ToString(), InInfo.GroupShortId, InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	OutCookedData.Type = EWwiseGroupType::Switch;
	OutCookedData.GroupId = SwitchGroup->Id;
	OutCookedData.Id = Switch->Id;
	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutCookedData.DebugName = FName();
	}
	else
	{
		OutCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? Switch->Name : Switch->ObjectPath);
	}
	return true;
}

bool FWwiseResourceCookerImpl::GetTriggerCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const
{
	const auto* ProjectDatabase = GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetTriggerCookedData (%s %" PRIu32 " %s): ProjectDatabase not initialized"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const auto* PlatformData = DataStructure.GetCurrentPlatformData();
	if (UNLIKELY(!PlatformData))
	{
		UE_LOG(LogWwiseResourceCooker, Error, TEXT("GetTriggerCookedData (%s %" PRIu32 " %s): No data for platform"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const TSet<FWwiseSharedLanguageId>& Languages = DataStructure.GetLanguages();

	FWwiseRefTrigger TriggerRef;

	if (UNLIKELY(!PlatformData->GetRef(TriggerRef, FWwiseSharedLanguageId(), InInfo)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("GetTriggerCookedData (%s %" PRIu32 " %s): No trigger data found"),
			*InInfo.WwiseGuid.ToString(), InInfo.WwiseShortId, *InInfo.WwiseName.ToString());
		return false;
	}

	const auto* Trigger = TriggerRef.GetTrigger();

	OutCookedData.TriggerId = Trigger->Id;
	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutCookedData.DebugName = FName();
	}
	else
	{
		OutCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? Trigger->Name : Trigger->ObjectPath);
	}

	return true;
}

bool FWwiseResourceCookerImpl::FillSoundBankBaseInfo(FWwiseSoundBankCookedData& OutSoundBankCookedData, const FWwiseMetadataPlatformInfo& InPlatformInfo, const FWwiseMetadataSoundBank& InSoundBank) const
{
	OutSoundBankCookedData.SoundBankId = InSoundBank.Id;
	OutSoundBankCookedData.SoundBankPathName = InSoundBank.Path;
	OutSoundBankCookedData.MemoryAlignment = InSoundBank.Align == 0 ? InPlatformInfo.DefaultAlign : InSoundBank.Align;
	OutSoundBankCookedData.bDeviceMemory = InSoundBank.bDeviceMemory;
	OutSoundBankCookedData.bContainsMedia = InSoundBank.ContainsMedia();
	switch (InSoundBank.Type)
	{
	case EMetadataSoundBankType::Bus:
		OutSoundBankCookedData.SoundBankType = EWwiseSoundBankType::Bus;
		break;
	case EMetadataSoundBankType::Event:
		OutSoundBankCookedData.SoundBankType = EWwiseSoundBankType::Event;
		break;
	case EMetadataSoundBankType::User:
	default:
		OutSoundBankCookedData.SoundBankType = EWwiseSoundBankType::User;
	}
	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutSoundBankCookedData.DebugName = FName();
	}
	else
	{
		OutSoundBankCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? InSoundBank.ShortName : InSoundBank.ObjectPath);
	}
	return true;
}

bool FWwiseResourceCookerImpl::FillMediaBaseInfo(FWwiseMediaCookedData& OutMediaCookedData, const FWwiseMetadataPlatformInfo& InPlatformInfo, const FWwiseMetadataSoundBank& InSoundBank, const FWwiseMetadataMediaReference& InMediaReference) const
{
	for (const auto& Media : InSoundBank.Media)
	{
		if (Media.Id == InMediaReference.Id)
		{
			return FillMediaBaseInfo(OutMediaCookedData, InPlatformInfo, InSoundBank, Media);
		}
	}
	UE_LOG(LogWwiseResourceCooker, Warning, TEXT("FillMediaBaseInfo: Could not get Media Reference %" PRIu32 " in SoundBank %s %" PRIu32),
		InMediaReference.Id, *InSoundBank.ShortName.ToString(), InSoundBank.Id);
	return false;
}

bool FWwiseResourceCookerImpl::FillMediaBaseInfo(FWwiseMediaCookedData& OutMediaCookedData, const FWwiseMetadataPlatformInfo& InPlatformInfo, const FWwiseMetadataSoundBank& InSoundBank, const FWwiseMetadataMedia& InMedia) const
{
	OutMediaCookedData.MediaId = InMedia.Id;
	if (InMedia.Path.IsNone())
	{
		if (UNLIKELY(InMedia.CachePath.IsNone()))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("FillMediaBaseInfo: Empty path for Media %" PRIu32 " in SoundBank %s %" PRIu32),
				InMedia.Id, *InSoundBank.ShortName.ToString(), InSoundBank.Id);
			return false;
		}
		OutMediaCookedData.MediaPathName = InMedia.CachePath;
	}
	else
	{
		OutMediaCookedData.MediaPathName = InMedia.Path;
	}

	if (InMedia.Location == EWwiseMetadataMediaLocation::Memory)
	{
		// In-Memory (User-defined SoundBank) already have the prefetch in the related SoundBank.
		OutMediaCookedData.PrefetchSize = 0;
	}
	else
	{
		OutMediaCookedData.PrefetchSize = InMedia.PrefetchSize;
	}
	
	OutMediaCookedData.MemoryAlignment = InMedia.Align == 0 ? InPlatformInfo.DefaultAlign : InMedia.Align;
	OutMediaCookedData.bDeviceMemory = InMedia.bDeviceMemory;
	OutMediaCookedData.bStreaming = InMedia.bStreaming;

	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutMediaCookedData.DebugName = FName();
	}
	else
	{
		OutMediaCookedData.DebugName = FName(InMedia.ShortName);
	}
	return true;
}

bool FWwiseResourceCookerImpl::FillExternalSourceBaseInfo(FWwiseExternalSourceCookedData& OutExternalSourceCookedData, const FWwiseMetadataExternalSource& InExternalSource) const
{
	OutExternalSourceCookedData.Cookie = InExternalSource.Cookie;
	if (ExportDebugNameRule == EWwiseExportDebugNameRule::Release)
	{
		OutExternalSourceCookedData.DebugName = FName();
	}
	else
	{
		OutExternalSourceCookedData.DebugName = FName((ExportDebugNameRule == EWwiseExportDebugNameRule::Name) ? InExternalSource.Name : InExternalSource.ObjectPath);
	}
	return true;
}

bool FWwiseResourceCookerImpl::AddRequirementsForMedia(TSet<FWwiseSoundBankCookedData>& OutSoundBankSet, TSet<FWwiseMediaCookedData>& OutMediaSet,
	const FWwiseRefMedia& InMediaRef, const FWwiseSharedLanguageId& InLanguage,
	const FWwisePlatformDataStructure& InPlatformData) const
{
	const auto* Media = InMediaRef.GetMedia();
	if (UNLIKELY(!Media))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForMedia: Could not get Media from Media Ref"));
		return false;
	}

	const auto* PlatformInfo = InPlatformData.PlatformRef.GetPlatformInfo();
	if (UNLIKELY(!PlatformInfo)) return false;

	if (Media->Location == EWwiseMetadataMediaLocation::Memory && !Media->bStreaming)
	{
		// In-Memory media is already loaded with current SoundBank
		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Media is in memory and not streaming. Skipping."),
			*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());
	}
	else if (Media->Location == EWwiseMetadataMediaLocation::OtherBank)
	{
		// Media resides in another SoundBank. Find that other SoundBank and add it as a requirement.
		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Media is in another SoundBank. Locate SoundBank and add requirement."),
			*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());

		FWwiseObjectInfo MediaInfo;
		MediaInfo.WwiseShortId = Media->Id;
		MediaInfo.WwiseName = Media->ShortName;

		FWwiseRefMedia OtherSoundBankMediaRef;
		if (UNLIKELY(!InPlatformData.GetRef(OtherSoundBankMediaRef, InLanguage, MediaInfo)))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Could not get Ref for other SoundBank media %s %" PRIu32 " %s"),
				*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId(),
				*MediaInfo.WwiseGuid.ToString(), MediaInfo.WwiseShortId, *MediaInfo.WwiseName.ToString());
			return false;
		}

		const auto* SoundBank = OtherSoundBankMediaRef.GetSoundBank();
		if (UNLIKELY(!SoundBank))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Could not get SoundBank from Media in another SoundBank Ref"),
				*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());
			return false;
		}

		if (SoundBank->IsInitBank())
		{
			// We assume Init SoundBanks are fully loaded
			UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Media is in Init SoundBank. Skipping."),
				*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());
		}

		FWwiseSoundBankCookedData MediaSoundBank;
		if (UNLIKELY(!FillSoundBankBaseInfo(MediaSoundBank, *PlatformInfo, *SoundBank)))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Could not fill SoundBank from Media in another SoundBank Data"),
				*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());
			return false;
		}
		OutSoundBankSet.Add(MoveTemp(MediaSoundBank));
	}
	else
	{
		// Media has a required loose file.
		UE_LOG(LogWwiseResourceCooker, VeryVerbose, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Adding loose media requirement."),
			*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());

		const auto* SoundBank = InMediaRef.GetSoundBank();
		if (UNLIKELY(!SoundBank))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Could not get SoundBank from Media"),
				*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());
			return false;
		}

		FWwiseMediaCookedData MediaCookedData;
		if (UNLIKELY(!FillMediaBaseInfo(MediaCookedData, *PlatformInfo, *SoundBank, *Media)))
		{
			UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForMedia (%s %" PRIu32 " in %s %" PRIu32 "): Could not fill Media from Media Ref"),
				*Media->ShortName.ToString(), Media->Id, *InLanguage.GetLanguageName().ToString(), InLanguage.GetLanguageId());
			return false;
		}

		OutMediaSet.Add(MoveTemp(MediaCookedData));
	}

	return true;
}

bool FWwiseResourceCookerImpl::AddRequirementsForExternalSource(TSet<FWwiseExternalSourceCookedData>& OutExternalSourceSet, const FWwiseRefExternalSource& InExternalSourceRef) const
{
	const auto* ExternalSource = InExternalSourceRef.GetExternalSource();
	if (UNLIKELY(!ExternalSource))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForExternalSource: Could not get External Source from External Source Ref"));
		return false;
	}
	FWwiseExternalSourceCookedData ExternalSourceCookedData;
	if (UNLIKELY(!FillExternalSourceBaseInfo(ExternalSourceCookedData, *ExternalSource)))
	{
		UE_LOG(LogWwiseResourceCooker, Warning, TEXT("AddRequirementsForExternalSource (%s %" PRIu32 "): Could not fill External Source from External Source Ref"),
			*ExternalSource->Name.ToString(), ExternalSource->Cookie);
		return false;
	}
	OutExternalSourceSet.Add(MoveTemp(ExternalSourceCookedData));
	return true;
}
