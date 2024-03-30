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

#include "Wwise/WwiseResourceLoaderImpl.h"

#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedSoundBankCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/CookedData/WwiseLocalizedShareSetCookedData.h"

#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AsyncStats.h"

#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseGlobalCallbacks.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseSoundBankManager.h"

#include "Async/Async.h"

#include <inttypes.h>

#include "Wwise/WwiseTask.h"

FWwiseSwitchContainerLeafGroupValueUsageCount::FLoadedData::FLoadedData()
{
}

bool FWwiseSwitchContainerLeafGroupValueUsageCount::FLoadedData::IsLoaded() const
{
	return LoadedSoundBanks.Num() > 0 || LoadedExternalSources.Num() > 0 || LoadedMedia.Num() > 0;
}

FWwiseSwitchContainerLeafGroupValueUsageCount::FWwiseSwitchContainerLeafGroupValueUsageCount(
	const FWwiseSwitchContainerLeafCookedData& InKey):
	Key(InKey)
{}

bool FWwiseSwitchContainerLeafGroupValueUsageCount::HaveAllKeys() const
{
	if (UNLIKELY(Key.GroupValueSet.Num() < LoadedGroupValues.Num()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("Have more keys loaded (%d) than existing in key (%d) @ %p for key %s"),
			LoadedGroupValues.Num(), Key.GroupValueSet.Num(), &LoadedData, *Key.GetDebugString());
		return true;
	}

	return Key.GroupValueSet.Num() == LoadedGroupValues.Num();
}

WWISE_RESOURCELOADERIMPL_TEST_CONST bool FWwiseResourceLoaderImpl::Test::bMockSleepOnMediaLoad{ false };

FWwiseResourceLoaderImpl::FWwiseResourceLoaderImpl() :
	ExecutionQueue(WWISE_EQ_NAME("FWwiseResourceLoaderImpl::ExecutionQueue"))
{
}

FWwiseResourceLoaderImpl::FWwiseResourceLoaderImpl(
	IWwiseExternalSourceManager& ExternalSourceManager,
	IWwiseMediaManager& MediaManager,
	IWwiseSoundBankManager& SoundBankManager) :
	ExecutionQueue(WWISE_EQ_NAME("FWwiseResourceLoaderImpl::ExecutionQueue")),
	ExternalSourceManager(&ExternalSourceManager),
	MediaManager(&MediaManager),
	SoundBankManager(&SoundBankManager)
{
#if WITH_EDITORONLY_DATA
	GeneratedSoundBanksPath.Path = TEXT("/");
#endif
}

FName FWwiseResourceLoaderImpl::GetUnrealExternalSourcePath() const
{
#if WITH_EDITORONLY_DATA
	if(FPaths::IsRelative( CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString()))
	{
		return FName(GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString() / CurrentPlatform.Platform->ExternalSourceRootPath.ToString());
	}
	else
	{
		return FName(CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString() / CurrentPlatform.Platform->ExternalSourceRootPath.ToString());
	}
#else
	if (UNLIKELY(!ExternalSourceManager))
	{
		ExternalSourceManager = IWwiseExternalSourceManager::Get();
		if (UNLIKELY(!ExternalSourceManager))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve External Source Manager"));
			return {};
		}
	}
	return FName(FPaths::ProjectContentDir() / ExternalSourceManager->GetStagingDirectory());
#endif
}

FString FWwiseResourceLoaderImpl::GetUnrealPath() const
{
#if WITH_EDITOR
	if(FPaths::IsRelative( CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString()))
	{
		return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString();
	}
	else
	{
		return CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString();
	}
#elif WITH_EDITORONLY_DATA
	UE_LOG(LogWwiseResourceLoader, Error, TEXT("GetUnrealPath should not be used in WITH_EDITORONLY_DATA (Getting path for %s)"), *InPath);
	if(FPaths::IsRelative( CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString()))
	{
		return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString();
	}
	else
	{
		return CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString();
	}
#else
	return StagePath;
#endif
}

FString FWwiseResourceLoaderImpl::GetUnrealPath(const FString& InPath) const
{
#if WITH_EDITOR
	return GetUnrealGeneratedSoundBanksPath(InPath);
#elif WITH_EDITORONLY_DATA
	UE_LOG(LogWwiseResourceLoader, Error, TEXT("GetUnrealPath should not be used in WITH_EDITORONLY_DATA (Getting path for %s)"), *InPath);
	return GetUnrealGeneratedSoundBanksPath(InPath);
#else
	return GetUnrealStagePath(InPath);
#endif
}

FString FWwiseResourceLoaderImpl::GetUnrealStagePath(const FString& InPath) const
{
	if (UNLIKELY(StagePath.IsEmpty()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("StagePath not set up (GetUnrealStagePath for %s)"), *InPath);
	}
	return StagePath / InPath;
}

#if WITH_EDITORONLY_DATA
FString FWwiseResourceLoaderImpl::GetUnrealGeneratedSoundBanksPath(const FString& InPath) const
{
	if (UNLIKELY(GeneratedSoundBanksPath.Path.IsEmpty()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("GeneratedSoundBanksPath not set up (GetUnrealGeneratedSoundBanksPath for %s)"), *InPath);
	}

	return GeneratedSoundBanksPath.Path / CurrentPlatform.Platform->PathRelativeToGeneratedSoundBanks.ToString() / InPath;
}
#endif


EWwiseResourceLoaderState FWwiseResourceLoaderImpl::GetResourceLoaderState()
{
	return WwiseResourceLoaderState;
}

void FWwiseResourceLoaderImpl::SetResourceLoaderState(EWwiseResourceLoaderState State)
{
	WwiseResourceLoaderState = State;
}

bool FWwiseResourceLoaderImpl::IsEnabled()
{
	return WwiseResourceLoaderState == EWwiseResourceLoaderState::Enabled;
}

void FWwiseResourceLoaderImpl::Disable()
{
	SetResourceLoaderState(EWwiseResourceLoaderState::AlwaysDisabled);
}

void FWwiseResourceLoaderImpl::Enable()
{
	SetResourceLoaderState(EWwiseResourceLoaderState::Enabled);
}

void FWwiseResourceLoaderImpl::SetLanguageAsync(FWwiseSetLanguagePromise&& Promise, const FWwiseLanguageCookedData& InLanguage, EWwiseReloadLanguage InReloadLanguage)
{
	SCOPED_WWISERESOURCELOADER_EVENT(TEXT("FWwiseResourceLoaderImpl::SetLanguageAsync"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseResourceLoaderTiming);

	auto OldLanguage = CurrentLanguage;
	auto NewLanguage = InLanguage;

	if (OldLanguage == NewLanguage)
	{
		return Promise.EmplaceValue();
	}

	UE_CLOG(!OldLanguage.GetLanguageName().IsValid(), LogWwiseResourceLoader, Log, TEXT("[SetLanguage] To %s"), *NewLanguage.GetLanguageName().ToString());
	UE_CLOG(OldLanguage.GetLanguageName().IsValid(), LogWwiseResourceLoader, Log, TEXT("[SetLanguage] from %s to %s"), *OldLanguage.GetLanguageName().ToString(), *NewLanguage.GetLanguageName().ToString());

	FCompletionFuture Future = MakeFulfilledWwisePromise<void>().GetFuture();

	if (InReloadLanguage == EWwiseReloadLanguage::Safe)
	{
		UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Stopping all sounds"));
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: SoundEngine not available to stop all sounds"));
		}
		else
		{
			SoundEngine->StopAll();

			// Wait two audio processing passes to make sure our StopAll was processed.
			FCompletionPromise EndPromise;
			auto EndFuture = EndPromise.GetFuture();
			Future.Next([Promise = MoveTemp(EndPromise)](int) mutable
			{
				if(auto* WwiseGlobalCallbacks = FWwiseGlobalCallbacks::Get())
				{
					FWwiseGlobalCallbacks::FCompletionPromise WaitPromise;
					auto WaitFuture = WaitPromise.GetFuture();
					WwiseGlobalCallbacks->EndCompletion(MoveTemp(WaitPromise), 2);
					WaitFuture.Next([Promise = MoveTemp(Promise)](int) mutable
					{
						Promise.EmplaceValue();
					});
				}
				else
				{
					Promise.EmplaceValue();
				}
			});
			Future = MoveTemp(EndFuture);
		}
	}

	CurrentLanguage = NewLanguage;

	if (InReloadLanguage == EWwiseReloadLanguage::Manual)
	{
		Future.Next([Promise = MoveTemp(Promise)](int) mutable
		{
			UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("SetLanguage: Done (Manual)"));
			Promise.EmplaceValue();
		});
		return;
	}

	Future.Next([this, OldLanguage = MoveTemp(OldLanguage), NewLanguage = MoveTemp(NewLanguage), Promise = MoveTemp(Promise)](int) mutable
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::SetLanguageAsync Unloading"), [this, OldLanguage = MoveTemp(OldLanguage), NewLanguage = MoveTemp(NewLanguage), Promise = MoveTemp(Promise)]() mutable
		{
			// Note: these are written as "Log" since it's more dangerous to do loading and unloading operations while the
			//		 asynchronous SetLanguage is executed. This allows for better debugging.

			UE_LOG(LogWwiseResourceLoader, Log, TEXT("SetLanguage: Switching languages. Unloading old language %s."),
				*OldLanguage.GetLanguageName().ToString());

			TArray<FWwiseLoadedSoundBankInfo*> AffectedSoundBanks;
			TArray<FWwiseLoadedAuxBusInfo*> AffectedAuxBusses;
			TArray<FWwiseLoadedShareSetInfo*> AffectedShareSets;
			TArray<FWwiseLoadedEventInfo*> AffectedEvents;

			// Unload all objects with a language equal to the old language
			FCompletionFutureArray UnloadFutureArray;

			for (auto& LoadedSoundBank : LoadedSoundBankList)
			{
				if (LoadedSoundBank.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping SoundBank %s with language %s"),
						*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), *LoadedSoundBank.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
				if (LIKELY(SoundBank))
				{
					AffectedSoundBanks.Add(&LoadedSoundBank);

					FCompletionPromise UnloadPromise;
					UnloadFutureArray.Add(UnloadPromise.GetFuture());
					UnloadSoundBankResources(MoveTemp(UnloadPromise), LoadedSoundBank.LoadedData, *SoundBank);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find SoundBank %s with language %s"),
						*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), *LoadedSoundBank.LanguageRef.GetLanguageName().ToString());
				}
			}

			for (auto& LoadedAuxBus : LoadedAuxBusList)
			{
				if (LoadedAuxBus.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping AuxBus %s with language %s"),
						*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), *LoadedAuxBus.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
				if (LIKELY(AuxBus))
				{
					AffectedAuxBusses.Add(&LoadedAuxBus);

					FCompletionPromise UnloadPromise;
					UnloadFutureArray.Add(UnloadPromise.GetFuture());
					UnloadAuxBusResources(MoveTemp(UnloadPromise), LoadedAuxBus.LoadedData, *AuxBus);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find AuxBus %s with language %s"),
						*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), *LoadedAuxBus.LanguageRef.GetLanguageName().ToString());
				}
			}

			for (auto& LoadedShareSet : LoadedShareSetList)
			{
				if (LoadedShareSet.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping ShareSet %s with language %s"),
						*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), *LoadedShareSet.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* ShareSet = LoadedShareSet.LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet.LanguageRef);
				if (LIKELY(ShareSet))
				{
					AffectedShareSets.Add(&LoadedShareSet);

					FCompletionPromise UnloadPromise;
					UnloadFutureArray.Add(UnloadPromise.GetFuture());
					UnloadShareSetResources(MoveTemp(UnloadPromise), LoadedShareSet.LoadedData, *ShareSet);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find ShareSet %s with language %s"),
						*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), *LoadedShareSet.LanguageRef.GetLanguageName().ToString());
				}
			}
			for (auto& LoadedEvent : LoadedEventList)
			{
				if (LoadedEvent.LanguageRef != OldLanguage)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("SetLanguage: Skipping Event %s with language %s"),
						*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), *LoadedEvent.LanguageRef.GetLanguageName().ToString());
					continue;
				}

				auto* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
				if (LIKELY(Event))
				{
					AffectedEvents.Add(&LoadedEvent);

					FCompletionPromise UnloadPromise;
					UnloadFutureArray.Add(UnloadPromise.GetFuture());
					UnloadEventResources(MoveTemp(UnloadPromise), LoadedEvent.LoadedData, *Event);
				}
				else
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Event %s with language %s"),
						*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), *LoadedEvent.LanguageRef.GetLanguageName().ToString());
				}
			}

			WaitForFutures(MoveTemp(UnloadFutureArray), [this,
				OldLanguage = MoveTemp(OldLanguage),
				NewLanguage = MoveTemp(NewLanguage),
				Promise = MoveTemp(Promise),
				AffectedAuxBusses = MoveTemp(AffectedAuxBusses),
				AffectedEvents = MoveTemp(AffectedEvents),
				AffectedShareSets = MoveTemp(AffectedShareSets),
				AffectedSoundBanks = MoveTemp(AffectedSoundBanks)]() mutable
			{
				SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::SetLanguageAsync Loading"));
				UE_LOG(LogWwiseResourceLoader, Log, TEXT("SetLanguage: Loading new language %s."),
					*NewLanguage.GetLanguageName().ToString());

				FCompletionFutureArray LoadFutureArray;

				// Note: The results are ignored. Reloading Wwise objects can individually fail for any given reasons, and it's Out Of Scope
				//       for the end product to know SetLanguage wasn't totally successful, since there's no real recourse at that point.

				for (auto* LoadedSoundBank : AffectedSoundBanks)
				{
					LoadedSoundBank->LanguageRef = NewLanguage;
					auto* SoundBank = LoadedSoundBank->LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank->LanguageRef);
					if (LIKELY(SoundBank))
					{
						FCompletionPromise LoadPromise;
						LoadFutureArray.Add(LoadPromise.GetFuture());
						FWwiseResourceLoadPromise ResourceLoadPromise;
						ResourceLoadPromise.GetFuture().Next([LoadPromise = MoveTemp(LoadPromise)](int) mutable
						{
							LoadPromise.EmplaceValue();
						});
						LoadSoundBankResources(MoveTemp(ResourceLoadPromise), LoadedSoundBank->LoadedData, *SoundBank);
					}
					else
					{
						UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find SoundBank %s with language %s"),
							*LoadedSoundBank->LocalizedSoundBankCookedData.DebugName.ToString(), *LoadedSoundBank->LanguageRef.GetLanguageName().ToString());
					}
				}

				for (auto* LoadedAuxBus : AffectedAuxBusses)
				{
					LoadedAuxBus->LanguageRef = NewLanguage;
					auto* AuxBus = LoadedAuxBus->LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus->LanguageRef);
					if (LIKELY(AuxBus))
					{
						FCompletionPromise LoadPromise;
						LoadFutureArray.Add(LoadPromise.GetFuture());
						FWwiseResourceLoadPromise ResourceLoadPromise;
						ResourceLoadPromise.GetFuture().Next([LoadPromise = MoveTemp(LoadPromise)](int) mutable
						{
							LoadPromise.EmplaceValue();
						});
						LoadAuxBusResources(MoveTemp(ResourceLoadPromise), LoadedAuxBus->LoadedData, *AuxBus);
					}
					else
					{
						UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find AuxBus %s with language %s"),
							*LoadedAuxBus->LocalizedAuxBusCookedData.DebugName.ToString(), *LoadedAuxBus->LanguageRef.GetLanguageName().ToString());
					}
				}

				for (auto* LoadedShareSet : AffectedShareSets)
				{
					LoadedShareSet->LanguageRef = NewLanguage;
					auto* ShareSet = LoadedShareSet->LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet->LanguageRef);
					if (LIKELY(ShareSet))
					{
						FCompletionPromise LoadPromise;
						LoadFutureArray.Add(LoadPromise.GetFuture());
						FWwiseResourceLoadPromise ResourceLoadPromise;
						ResourceLoadPromise.GetFuture().Next([LoadPromise = MoveTemp(LoadPromise)](int) mutable
						{
							LoadPromise.EmplaceValue();
						});
						LoadShareSetResources(MoveTemp(ResourceLoadPromise), LoadedShareSet->LoadedData, *ShareSet);
					}
					else
					{
						UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find ShareSet %s with language %s"),
							*LoadedShareSet->LocalizedShareSetCookedData.DebugName.ToString(), *LoadedShareSet->LanguageRef.GetLanguageName().ToString());
					}
				}

				for (auto* LoadedEvent : AffectedEvents)
				{
					LoadedEvent->LanguageRef = NewLanguage;
					auto* Event = LoadedEvent->LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent->LanguageRef);
					if (LIKELY(Event))
					{
						FCompletionPromise LoadPromise;
						LoadFutureArray.Add(LoadPromise.GetFuture());
						FWwiseResourceLoadPromise ResourceLoadPromise;
						ResourceLoadPromise.GetFuture().Next([LoadPromise = MoveTemp(LoadPromise)](int) mutable
						{
							LoadPromise.EmplaceValue();
						});
						LoadEventResources(MoveTemp(ResourceLoadPromise), LoadedEvent->LoadedData, *Event);
					}
					else
					{
						UE_LOG(LogWwiseResourceLoader, Error, TEXT("SetLanguage: Could not find Event %s with language %s"),
							*LoadedEvent->LocalizedEventCookedData.DebugName.ToString(), *LoadedEvent->LanguageRef.GetLanguageName().ToString());
					}
				}

				WaitForFutures(MoveTemp(LoadFutureArray), [
					OldLanguage = MoveTemp(OldLanguage),
					NewLanguage = MoveTemp(NewLanguage),
					Promise = MoveTemp(Promise)]() mutable
				{
					UE_LOG(LogWwiseResourceLoader, Log, TEXT("SetLanguage: Done switching assets from language %s to language %s."),
						*OldLanguage.GetLanguageName().ToString(), *NewLanguage.GetLanguageName().ToString());
					Promise.EmplaceValue();
				});
			});
		});
	});
}

void FWwiseResourceLoaderImpl::SetPlatform(const FWwiseSharedPlatformId& InPlatform)
{
	UE_LOG(LogWwiseResourceLoader, Log, TEXT("SetPlatform: Updating platform from %s (%s) to %s (%s)."),
		*CurrentPlatform.GetPlatformName().ToString(), *CurrentPlatform.GetPlatformGuid().ToString(),
		*InPlatform.GetPlatformName().ToString(), *InPlatform.GetPlatformGuid().ToString());

	CurrentPlatform = InPlatform;
}


FWwiseLoadedAuxBusPtr FWwiseResourceLoaderImpl::CreateAuxBusNode(
	const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InAuxBusCookedData.AuxBusLanguageMap, InLanguageOverride, InAuxBusCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateAuxBusNode: Could not find language for Aux Bus %s"), *InAuxBusCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedAuxBusListNode(FWwiseLoadedAuxBusInfo(InAuxBusCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadAuxBusAsync(FWwiseLoadedAuxBusPromise&& Promise, FWwiseLoadedAuxBusPtr&& InAuxBusListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadAuxBusAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedAuxBus = InAuxBusListNode->GetValue();
	LogLoad(LoadedAuxBus);
	const FWwiseAuxBusCookedData* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
	if (UNLIKELY(!AuxBus))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusAsync: Could not find AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);
		delete InAuxBusListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadAuxBusResources(MoveTemp(ResourceLoadPromise), LoadedAuxBus.LoadedData, *AuxBus);

	Future.Next([this, &LoadedAuxBus, AuxBus, InAuxBusListNode = MoveTemp(InAuxBusListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusAsync: Could not load AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);
			delete InAuxBusListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachAuxBusNode(InAuxBusListNode);

		Timing.Stop();
		Promise.EmplaceValue(InAuxBusListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadAuxBusAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedAuxBusPtr&& InAuxBusListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadAuxBusAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedAuxBus = InAuxBusListNode->GetValue();

	LogUnload(LoadedAuxBus);

	const FWwiseAuxBusCookedData* AuxBus = LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusLanguageMap.Find(LoadedAuxBus.LanguageRef);
	if (UNLIKELY(!AuxBus))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadAuxBusAsync: Could not find AuxBus %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedAuxBus.LocalizedAuxBusCookedData.DebugName.ToString(), LoadedAuxBus.LocalizedAuxBusCookedData.AuxBusId, *LoadedAuxBus.LanguageRef.LanguageName.ToString(), LoadedAuxBus.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	DetachAuxBusNode(InAuxBusListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadAuxBusResources(MoveTemp(ResourceUnloadPromise), LoadedAuxBus.LoadedData, *AuxBus);

	Future.Next([this, &LoadedAuxBus, AuxBus, InAuxBusListNode = MoveTemp(InAuxBusListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InAuxBusListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}


FWwiseLoadedEventPtr FWwiseResourceLoaderImpl::CreateEventNode(
	const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InEventCookedData.EventLanguageMap, InLanguageOverride, InEventCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateEventNode: Could not find language for Event %s"), *InEventCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedEventListNode(FWwiseLoadedEventInfo(InEventCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadEventAsync(FWwiseLoadedEventPromise&& Promise, FWwiseLoadedEventPtr&& InEventListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadEventAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedEvent = InEventListNode->GetValue();

	LogLoad(LoadedEvent);

	const FWwiseEventCookedData* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
	if (UNLIKELY(!Event))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventAsync: Could not find Event %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);
		delete InEventListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadEventResources(MoveTemp(ResourceLoadPromise), LoadedEvent.LoadedData, *Event);

	Future.Next([this, &LoadedEvent, Event, InEventListNode = MoveTemp(InEventListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventAsync: Could not load Event %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);
			delete InEventListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachEventNode(InEventListNode);
		Timing.Stop();
		Promise.EmplaceValue(InEventListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadEventAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEventPtr&& InEventListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadEventAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedEvent = InEventListNode->GetValue();

	LogUnload(LoadedEvent);

	const FWwiseEventCookedData* Event = LoadedEvent.LocalizedEventCookedData.EventLanguageMap.Find(LoadedEvent.LanguageRef);
	if (UNLIKELY(!Event))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadEventAsync: Could not find Event %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedEvent.LocalizedEventCookedData.DebugName.ToString(), LoadedEvent.LocalizedEventCookedData.EventId, *LoadedEvent.LanguageRef.LanguageName.ToString(), LoadedEvent.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	DetachEventNode(InEventListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadEventResources(MoveTemp(ResourceUnloadPromise), LoadedEvent.LoadedData, *Event);

	Future.Next([this, &LoadedEvent, Event, InEventListNode = MoveTemp(InEventListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InEventListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}


FWwiseLoadedExternalSourcePtr FWwiseResourceLoaderImpl::CreateExternalSourceNode(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData)
{
	return new FWwiseLoadedExternalSourceListNode(FWwiseLoadedExternalSourceInfo(InExternalSourceCookedData));
}

void FWwiseResourceLoaderImpl::LoadExternalSourceAsync(FWwiseLoadedExternalSourcePromise&& Promise, FWwiseLoadedExternalSourcePtr&& InExternalSourceListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadExternalSourceAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedExternalSource = InExternalSourceListNode->GetValue();

	LogLoad(LoadedExternalSource);

	const FWwiseExternalSourceCookedData* ExternalSource = &LoadedExternalSource.ExternalSourceCookedData;

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadExternalSourceResources(MoveTemp(ResourceLoadPromise), LoadedExternalSource.LoadedData, *ExternalSource);

	Future.Next([this, &LoadedExternalSource, ExternalSource, InExternalSourceListNode = MoveTemp(InExternalSourceListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadExternalSourceAsync: Could not load ExternalSource %s (%" PRIu32 ")"),
			*LoadedExternalSource.ExternalSourceCookedData.DebugName.ToString(), LoadedExternalSource.ExternalSourceCookedData.Cookie);
			delete InExternalSourceListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachExternalSourceNode(InExternalSourceListNode);
		Timing.Stop();
		Promise.EmplaceValue(InExternalSourceListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadExternalSourceAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedExternalSourcePtr&& InExternalSourceListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadExternalSourceAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedExternalSource = InExternalSourceListNode->GetValue();

	LogUnload(LoadedExternalSource);

	const FWwiseExternalSourceCookedData* ExternalSource = &LoadedExternalSource.ExternalSourceCookedData;

	DetachExternalSourceNode(InExternalSourceListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadExternalSourceResources(MoveTemp(ResourceUnloadPromise), LoadedExternalSource.LoadedData, *ExternalSource);

	Future.Next([this, &LoadedExternalSource, ExternalSource, InExternalSourceListNode = MoveTemp(InExternalSourceListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InExternalSourceListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}


FWwiseLoadedGroupValuePtr FWwiseResourceLoaderImpl::CreateGroupValueNode(
	const FWwiseGroupValueCookedData& InGroupValueCookedData)
{
	return new FWwiseLoadedGroupValueListNode(FWwiseLoadedGroupValueInfo(InGroupValueCookedData));
}

void FWwiseResourceLoaderImpl::LoadGroupValueAsync(FWwiseLoadedGroupValuePromise&& Promise, FWwiseLoadedGroupValuePtr&& InGroupValueListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadGroupValueAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedGroupValue = InGroupValueListNode->GetValue();

	LogLoad(LoadedGroupValue);

	const FWwiseGroupValueCookedData* GroupValue = &LoadedGroupValue.GroupValueCookedData;

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadGroupValueResources(MoveTemp(ResourceLoadPromise), LoadedGroupValue.LoadedData, *GroupValue);

	Future.Next([this, &LoadedGroupValue, GroupValue, InGroupValueListNode = MoveTemp(InGroupValueListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadGroupValueAsync: Could not load GroupValue %s (%s %" PRIu32 ":%" PRIu32 ")"),
				*LoadedGroupValue.GroupValueCookedData.DebugName.ToString(), *LoadedGroupValue.GroupValueCookedData.GetTypeName(), LoadedGroupValue.GroupValueCookedData.GroupId, LoadedGroupValue.GroupValueCookedData.Id);
			delete InGroupValueListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachGroupValueNode(InGroupValueListNode);
		Timing.Stop();
		Promise.EmplaceValue(InGroupValueListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadGroupValueAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedGroupValuePtr&& InGroupValueListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedGroupValue = InGroupValueListNode->GetValue();

	LogUnload(LoadedGroupValue);

	const FWwiseGroupValueCookedData* GroupValue = &LoadedGroupValue.GroupValueCookedData;

	DetachGroupValueNode(InGroupValueListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadGroupValueResources(MoveTemp(ResourceUnloadPromise), LoadedGroupValue.LoadedData, *GroupValue);

	Future.Next([this, &LoadedGroupValue, GroupValue, InGroupValueListNode = MoveTemp(InGroupValueListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InGroupValueListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}


FWwiseLoadedInitBankPtr FWwiseResourceLoaderImpl::CreateInitBankNode(
	const FWwiseInitBankCookedData& InInitBankCookedData)
{
	return new FWwiseLoadedInitBankListNode(FWwiseLoadedInitBankInfo(InInitBankCookedData));
}

void FWwiseResourceLoaderImpl::LoadInitBankAsync(FWwiseLoadedInitBankPromise&& Promise, FWwiseLoadedInitBankPtr&& InInitBankListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadInitBankAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedInitBank = InInitBankListNode->GetValue();

	LogLoad(LoadedInitBank);

	const FWwiseInitBankCookedData* InitBank = &LoadedInitBank.InitBankCookedData;

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadInitBankResources(MoveTemp(ResourceLoadPromise), LoadedInitBank.LoadedData, *InitBank);

	Future.Next([this, &LoadedInitBank, InitBank, InInitBankListNode = MoveTemp(InInitBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankAsync: Could not load InitBank %s (%" PRIu32 ")"),
			*LoadedInitBank.InitBankCookedData.DebugName.ToString(), LoadedInitBank.InitBankCookedData.SoundBankId);
			delete InInitBankListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachInitBankNode(InInitBankListNode);
		Timing.Stop();
		Promise.EmplaceValue(InInitBankListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadInitBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedInitBankPtr&& InInitBankListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadInitBankAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedInitBank = InInitBankListNode->GetValue();

	LogUnload(LoadedInitBank);

	const FWwiseInitBankCookedData* InitBank = &LoadedInitBank.InitBankCookedData;

	DetachInitBankNode(InInitBankListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadInitBankResources(MoveTemp(ResourceUnloadPromise), LoadedInitBank.LoadedData, *InitBank);

	Future.Next([this, &LoadedInitBank, InitBank, InInitBankListNode = MoveTemp(InInitBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InInitBankListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}


FWwiseLoadedMediaPtr FWwiseResourceLoaderImpl::CreateMediaNode(const FWwiseMediaCookedData& InMediaCookedData)
{
	return new FWwiseLoadedMediaListNode(FWwiseLoadedMediaInfo(InMediaCookedData));
}

void FWwiseResourceLoaderImpl::LoadMediaAsync(FWwiseLoadedMediaPromise&& Promise, FWwiseLoadedMediaPtr&& InMediaListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadMediaAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedMedia = InMediaListNode->GetValue();

	LogLoad(LoadedMedia);

	const FWwiseMediaCookedData* Media = &LoadedMedia.MediaCookedData;

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadMediaResources(MoveTemp(ResourceLoadPromise), LoadedMedia.LoadedData, *Media);

	Future.Next([this, &LoadedMedia, Media, InMediaListNode = MoveTemp(InMediaListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMediaAsync: Could not load Media %s (%" PRIu32 ")"),
			*LoadedMedia.MediaCookedData.DebugName.ToString(), LoadedMedia.MediaCookedData.MediaId);
			delete InMediaListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachMediaNode(InMediaListNode);
		Timing.Stop();
		Promise.EmplaceValue(InMediaListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadMediaAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedMediaPtr&& InMediaListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadMediaAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedMedia = InMediaListNode->GetValue();

	LogUnload(LoadedMedia);

	const FWwiseMediaCookedData* Media = &LoadedMedia.MediaCookedData;

	DetachMediaNode(InMediaListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadMediaResources(MoveTemp(ResourceUnloadPromise), LoadedMedia.LoadedData, *Media);

	Future.Next([this, &LoadedMedia, Media, InMediaListNode = MoveTemp(InMediaListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InMediaListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}


FWwiseLoadedShareSetPtr FWwiseResourceLoaderImpl::CreateShareSetNode(
	const FWwiseLocalizedShareSetCookedData& InShareSetCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InShareSetCookedData.ShareSetLanguageMap, InLanguageOverride, InShareSetCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateShareSetNode: Could not find language for ShareSet %s"), *InShareSetCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedShareSetListNode(FWwiseLoadedShareSetInfo(InShareSetCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadShareSetAsync(FWwiseLoadedShareSetPromise&& Promise, FWwiseLoadedShareSetPtr&& InShareSetListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadShareSetAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedShareSet = InShareSetListNode->GetValue();

	LogLoad(LoadedShareSet);

	const FWwiseShareSetCookedData* ShareSet = LoadedShareSet.LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet.LanguageRef);
	if (UNLIKELY(!ShareSet))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareSetAsync: Could not find ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);
		delete InShareSetListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadShareSetResources(MoveTemp(ResourceLoadPromise), LoadedShareSet.LoadedData, *ShareSet);

	Future.Next([this, &LoadedShareSet, ShareSet, InShareSetListNode = MoveTemp(InShareSetListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareSetAsync: Could not load ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);
			delete InShareSetListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachShareSetNode(InShareSetListNode);
		Timing.Stop();
		Promise.EmplaceValue(InShareSetListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadShareSetAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedShareSetPtr&& InShareSetListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadShareSetAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedShareSet = InShareSetListNode->GetValue();

	LogUnload(LoadedShareSet);

	const FWwiseShareSetCookedData* ShareSet = LoadedShareSet.LocalizedShareSetCookedData.ShareSetLanguageMap.Find(LoadedShareSet.LanguageRef);
	if (UNLIKELY(!ShareSet))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadShareSetAsync: Could not find ShareSet %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedShareSet.LocalizedShareSetCookedData.DebugName.ToString(), LoadedShareSet.LocalizedShareSetCookedData.ShareSetId, *LoadedShareSet.LanguageRef.LanguageName.ToString(), LoadedShareSet.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	DetachShareSetNode(InShareSetListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadShareSetResources(MoveTemp(ResourceUnloadPromise), LoadedShareSet.LoadedData, *ShareSet);

	Future.Next([this, &LoadedShareSet, ShareSet, InShareSetListNode = MoveTemp(InShareSetListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InShareSetListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}

FWwiseLoadedSoundBankPtr FWwiseResourceLoaderImpl::CreateSoundBankNode(
	const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride)
{
	const auto* LanguageKey = GetLanguageMapKey(InSoundBankCookedData.SoundBankLanguageMap, InLanguageOverride, InSoundBankCookedData.DebugName);
	if (UNLIKELY(!LanguageKey))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("CreateSoundBankNode: Could not find language for SoundBank %s"), *InSoundBankCookedData.DebugName.ToString());
		return nullptr;
	}

	return new FWwiseLoadedSoundBankListNode(FWwiseLoadedSoundBankInfo(InSoundBankCookedData, *LanguageKey));
}

void FWwiseResourceLoaderImpl::LoadSoundBankAsync(FWwiseLoadedSoundBankPromise&& Promise, FWwiseLoadedSoundBankPtr&& InSoundBankListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadSoundBankAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedSoundBank = InSoundBankListNode->GetValue();

	LogLoad(LoadedSoundBank);

	const FWwiseSoundBankCookedData* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
	if (UNLIKELY(!SoundBank))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankAsync: Could not find SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);
		delete InSoundBankListNode;
		Timing.Stop();
		Promise.EmplaceValue(nullptr);
		return;
	}

	FWwiseResourceLoadPromise ResourceLoadPromise;
	auto Future = ResourceLoadPromise.GetFuture();
	LoadSoundBankResources(MoveTemp(ResourceLoadPromise), LoadedSoundBank.LoadedData, *SoundBank);

	Future.Next([this, &LoadedSoundBank, SoundBank, InSoundBankListNode = MoveTemp(InSoundBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankAsync: Could not load SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 ")"),
			*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);
			delete InSoundBankListNode;
			Timing.Stop();
			Promise.EmplaceValue(nullptr);
			return;
		}

		AttachSoundBankNode(InSoundBankListNode);
		Timing.Stop();
		Promise.EmplaceValue(InSoundBankListNode);
	});
}

void FWwiseResourceLoaderImpl::UnloadSoundBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedSoundBankPtr&& InSoundBankListNode)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadSoundBankAsync"));
	FWwiseAsyncCycleCounter Timing(GET_STATID(STAT_WwiseResourceLoaderTiming));

	auto& LoadedSoundBank = InSoundBankListNode->GetValue();

	LogUnload(LoadedSoundBank);

	const FWwiseSoundBankCookedData* SoundBank = LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankLanguageMap.Find(LoadedSoundBank.LanguageRef);
	if (UNLIKELY(!SoundBank))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("UnloadSoundBankAsync: Could not find SoundBank %s (%" PRIu32 ") in language %s (%" PRIu32 "). Leaking!"),
			*LoadedSoundBank.LocalizedSoundBankCookedData.DebugName.ToString(), LoadedSoundBank.LocalizedSoundBankCookedData.SoundBankId, *LoadedSoundBank.LanguageRef.LanguageName.ToString(), LoadedSoundBank.LanguageRef.LanguageId);
		Timing.Stop();
		Promise.EmplaceValue();
		return;
	}

	DetachSoundBankNode(InSoundBankListNode);

	FWwiseResourceUnloadPromise ResourceUnloadPromise;
	auto Future = ResourceUnloadPromise.GetFuture();
	UnloadSoundBankResources(MoveTemp(ResourceUnloadPromise), LoadedSoundBank.LoadedData, *SoundBank);

	Future.Next([this, &LoadedSoundBank, SoundBank, InSoundBankListNode = MoveTemp(InSoundBankListNode), Promise = MoveTemp(Promise), Timing = MoveTemp(Timing)](int) mutable
	{
		delete InSoundBankListNode;

		Timing.Stop();
		Promise.EmplaceValue();
	});
}

void FWwiseResourceLoaderImpl::LoadAuxBusResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadAuxBusResources"));
	
	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadAuxBusResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadAuxBusResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadAuxBusResources: AuxBus %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);
		return Promise.EmplaceValue(false);
	}

	++LoadedData.IsProcessing;
	FCompletionFutureArray FutureArray;

	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("AuxBus"), InCookedData.DebugName.ToString(), InCookedData.AuxBusId);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("AuxBus"), InCookedData.DebugName.ToString(), InCookedData.AuxBusId);
	WaitForFutures(MoveTemp(FutureArray), [this, Promise = MoveTemp(Promise), &LoadedData, &LoadedSoundBanks, &InCookedData]() mutable
	{
		--LoadedData.IsProcessing;
		if (UNLIKELY(LoadedSoundBanks.Num() != InCookedData.SoundBanks.Num()))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::LoadAuxBusResources: Could not load %d prerequisites for AuxBus %s (%" PRIu32 "). Unloading and failing."),
				InCookedData.SoundBanks.Num() - LoadedSoundBanks.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.AuxBusId);
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			
			ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadAuxBusResources Error"), [this, UnloadPromise = MoveTemp(UnloadPromise), &LoadedData, &InCookedData]() mutable
			{
				UnloadAuxBusResources(MoveTemp(UnloadPromise), LoadedData, InCookedData);
			});
			
			UnloadFuture.Next([Promise = MoveTemp(Promise)](int) mutable
			{
				SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadAuxBusResources UnloadFuture.Done"));
				return Promise.EmplaceValue(false);
			});
		}
		else
		{
			return Promise.EmplaceValue(true);
		}
	});
}

void FWwiseResourceLoaderImpl::LoadEventResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadEventResources"));
	
	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadEventResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadEventResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Event %s (%" PRIu32 ") is already loaded."),
			*InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
		return Promise.EmplaceValue(false);
	}

	++LoadedData.IsProcessing;

	FCompletionFutureArray FutureArray;

	if (InCookedData.RequiredGroupValueSet.Num() > 0 || InCookedData.SwitchContainerLeaves.Num() > 0)
	{
		FCompletionPromise CompletionPromise;
		FutureArray.Add(CompletionPromise.GetFuture());
		
		FWwiseResourceLoadPromise SwitchContainerPromise;
		auto SwitchContainerFuture = SwitchContainerPromise.GetFuture();
		LoadEventSwitchContainerResources(MoveTemp(SwitchContainerPromise), LoadedData, InCookedData);
		SwitchContainerFuture.Next([CompletionPromise = MoveTemp(CompletionPromise)](bool bResult) mutable
		{
			CompletionPromise.EmplaceValue();
		});
	}
	
	AddLoadExternalSourceFutures(FutureArray, LoadedExternalSources, InCookedData.ExternalSources, TEXT("Event"), InCookedData.DebugName.ToString(), InCookedData.EventId);
	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("Event"), InCookedData.DebugName.ToString(), InCookedData.EventId);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("Event"), InCookedData.DebugName.ToString(), InCookedData.EventId);

	WaitForFutures(MoveTemp(FutureArray), [this, Promise = MoveTemp(Promise), &LoadedData, &LoadedSoundBanks, &InCookedData]() mutable
	{
		SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadEventResources WaitForFutures"));
		--LoadedData.IsProcessing;
		if (UNLIKELY(LoadedSoundBanks.Num() != InCookedData.SoundBanks.Num()))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadEventResources: Could not load %d prerequisites for Event %s (%" PRIu32 "). Unloading and failing."),
				InCookedData.SoundBanks.Num() - LoadedSoundBanks.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			
			ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadEventResources Error UnloadEventResource"), [this, UnloadPromise = MoveTemp(UnloadPromise), &LoadedData, &InCookedData]() mutable
			{
				UnloadEventResources(MoveTemp(UnloadPromise), LoadedData, InCookedData);
			});
			
			UnloadFuture.Next([Promise = MoveTemp(Promise)](int) mutable
			{
				SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadEventResources UnloadFuture.Done"));
				return Promise.EmplaceValue(false);
			});
		}
		else
		{
			SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadEventResources Done"));
			return Promise.EmplaceValue(true);
		}
	});
}

void FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources"));

	// Load required GroupValues
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading %d GroupValues for Event %s (%" PRIu32 ")"),
		(int)InCookedData.RequiredGroupValueSet.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	FWwiseLoadedGroupValueList& LoadedRequiredGroupValues = LoadedData.LoadedRequiredGroupValues;
	bool& bLoadedSwitchContainerLeaves = LoadedData.bLoadedSwitchContainerLeaves;
	FCompletionFutureArray FutureArray;

	for (const auto& GroupValue : InCookedData.RequiredGroupValueSet)
	{
		FCompletionPromise GroupValuePromise;
		FutureArray.Add(GroupValuePromise.GetFuture());

		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources GroupValue"), [this, &LoadedRequiredGroupValues, &InCookedData, &GroupValue, GroupValuePromise = MoveTemp(GroupValuePromise)]() mutable
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading GroupValue %s for Event %s (%" PRIu32 ")"),
				*GroupValue.GetDebugString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

			auto* LoadedNode = new FWwiseLoadedGroupValueListNode(FWwiseLoadedGroupValueInfo(GroupValue));
			auto& GroupValueLoadedData = LoadedNode->GetValue().LoadedData;

			FWwiseResourceLoadPromise GroupValueResourcePromise;
			auto GroupValueResourceFuture = GroupValueResourcePromise.GetFuture();
			LoadGroupValueResources(MoveTemp(GroupValueResourcePromise), GroupValueLoadedData, GroupValue);
			GroupValueResourceFuture.Next([this, &LoadedRequiredGroupValues, &InCookedData, &GroupValue, GroupValuePromise = MoveTemp(GroupValuePromise), LoadedNode](bool bResult) mutable
			{
				SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources GroupValue.SwitchContainer ResourceFuture.Next"));
				const auto& GroupValueLoadedData = LoadedNode->GetValue().LoadedData;
				if (UNLIKELY(!bResult || !GroupValueLoadedData.IsLoaded()))
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("Could not load required GroupValue %s for Event %s (%" PRIu32 ")"),
						*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
					delete LoadedNode;
					GroupValuePromise.EmplaceValue();
				}
				else
				{
					ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources GroupValue Emplace"), [this, &LoadedRequiredGroupValues, LoadedNode, GroupValuePromise = MoveTemp(GroupValuePromise)]() mutable
					{
						LoadedRequiredGroupValues.AddTail(LoadedNode);
						GroupValuePromise.EmplaceValue();
					});
				}
			});
		});
	}

	// Load Switch Container Leaves
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading %d Leaves for Event %s (%" PRIu32 ")"),
		(int)InCookedData.SwitchContainerLeaves.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	for (const auto& SwitchContainerLeaf : InCookedData.SwitchContainerLeaves)
	{
		check(SwitchContainerLeaf.GroupValueSet.Num() > 0);
		auto UsageCount = MakeShared<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe>(SwitchContainerLeaf);
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources UsageCount[%p]: Created %s for Event %s (%" PRIu32 ")"), 
			&UsageCount.Get(), *UsageCount->Key.GetDebugString(),
			*InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
		for (const auto& GroupValue : SwitchContainerLeaf.GroupValueSet)
		{
			FCompletionPromise SwitchContainerLeafPromise;
			FutureArray.Add(SwitchContainerLeafPromise.GetFuture());

			ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources SwitchContainerLeaf"), [this, &bLoadedSwitchContainerLeaves, &InCookedData, &GroupValue, UsageCount, SwitchContainerLeafPromise = MoveTemp(SwitchContainerLeafPromise)]() mutable
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Adding optional %s for %s in Event %s (%" PRIu32 ")"),
					*GroupValue.GetDebugString(), *UsageCount->Key.GetDebugString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

				auto FoundInfoId = LoadedGroupValueInfo.FindId(FWwiseSwitchContainerLoadedGroupValueInfo(GroupValue));
				auto InfoId = FoundInfoId.IsValidId() ? FoundInfoId : LoadedGroupValueInfo.Add(FWwiseSwitchContainerLoadedGroupValueInfo(GroupValue), nullptr);
				FWwiseSwitchContainerLoadedGroupValueInfo& Info = LoadedGroupValueInfo[InfoId];
				bool bIsAlreadyCreated = false;
				auto UsageCountId = Info.Leaves.Add(UsageCount, &bIsAlreadyCreated);
				if (UNLIKELY(bIsAlreadyCreated))
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("Creating already created Switch Container Leaf Usage Count @ %p for %s"),
						&UsageCount->LoadedData, *UsageCount->Key.GetDebugString());
					return SwitchContainerLeafPromise.EmplaceValue();
				}

				bLoadedSwitchContainerLeaves = true;
				UE_CLOG(!Info.ResourcesAreLoaded(), LogWwiseResourceLoader, VeryVerbose, TEXT("Don't have referencing GroupValues yet: %d for key %s"), Info.GroupValueCount, *UsageCount->Key.GetDebugString());
				UE_CLOG(Info.ResourcesAreLoaded(), LogWwiseResourceLoader, VeryVerbose, TEXT("Have referencing GroupValues: %d for key %s"), Info.GroupValueCount, *UsageCount->Key.GetDebugString());
				if (Info.ResourcesAreLoaded())
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Number of GroupValues required for this leaf: %d/%d @ %p for key %s (+1 in Event)"),
						(int)UsageCount->LoadedGroupValues.Num() + 1, UsageCount->Key.GroupValueSet.Num(), &UsageCount->LoadedData, *UsageCount->Key.GetDebugString());
					bIsAlreadyCreated = false;
					UsageCount->LoadedGroupValues.Add(GroupValue, &bIsAlreadyCreated);
					if (UNLIKELY(bIsAlreadyCreated))
					{
						UE_LOG(LogWwiseResourceLoader, Error, TEXT("Loading already created Switch Container Leaf LoadedGoupValueCount %d @ %p for %s"),
							(int)UsageCount->LoadedGroupValues.Num(), &UsageCount->LoadedData, *UsageCount->Key.GetDebugString());
						SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources Leaf.SwitchContainer AlreadyLoaded Done"));
						return SwitchContainerLeafPromise.EmplaceValue();
					}

					LoadSwitchContainerLeafResources(MoveTemp(SwitchContainerLeafPromise), UsageCount);
				}
				else
				{
					SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources Leaf.SwitchContainer !ShouldBeLoaded.Done"));
					SwitchContainerLeafPromise.EmplaceValue();
				}
			});
		}
	}

	WaitForFutures(MoveTemp(FutureArray), [Promise = MoveTemp(Promise)]() mutable
	{
		SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadEventSwitchContainerResources Wait.Done"));
		Promise.EmplaceValue(true);
	});
}

void FWwiseResourceLoaderImpl::LoadExternalSourceResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadExternalSourceResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadExternalSourceResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadExternalSourceResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadExternalSourceResources: ExternalSource %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.Cookie);
		return Promise.EmplaceValue(false);
	}

	++LoadedData.IsProcessing;
	LoadExternalSourceFile(InCookedData, [Promise = MoveTemp(Promise), &LoadedData, &InCookedData](bool bResult) mutable
	{
		LoadedData.bLoaded = bResult;
		--LoadedData.IsProcessing;
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadExternalSourceResources: Could not load ExternalSource %s (%" PRIu32 ")"),
				*InCookedData.DebugName.ToString(), (uint32)InCookedData.Cookie);
		}
		Promise.EmplaceValue(bResult);
	});
}

void FWwiseResourceLoaderImpl::LoadGroupValueResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadGroupValueResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadGroupValueResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadGroupValueResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);
	++LoadedData.IsProcessing;
	
	ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("WwiseResourceLoaderImpl::LoadGroupValueResources GroupValue.SwitchContainer"), [this, &LoadedData, &InCookedData, Promise = MoveTemp(Promise)]() mutable
	{
		auto FoundInfoId = LoadedGroupValueInfo.FindId(FWwiseSwitchContainerLoadedGroupValueInfo(InCookedData));
		auto InfoId = FoundInfoId.IsValidId() ? FoundInfoId : LoadedGroupValueInfo.Add(FWwiseSwitchContainerLoadedGroupValueInfo(InCookedData), nullptr);
		FWwiseSwitchContainerLoadedGroupValueInfo& Info = LoadedGroupValueInfo[InfoId];
		const bool bWasLoaded = Info.ResourcesAreLoaded();
		++Info.GroupValueCount;

		FCompletionFutureArray FutureArray;

		if (!bWasLoaded)
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("First GroupValue %s (%s %" PRIu32 ":%" PRIu32 ") load. Loading %d leaves."),
				*InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info.Leaves.Num());

			for (const auto& UsageCount : Info.Leaves)
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Number of GroupValues required for a leaf: %d/%d @ %p for key %s (+1 in GroupValue)"),
					(int)UsageCount->LoadedGroupValues.Num() + 1, UsageCount->Key.GroupValueSet.Num(), &UsageCount->LoadedData, *UsageCount->Key.GetDebugString());
				bool bIsAlreadyCreated = false;
				UsageCount->LoadedGroupValues.Add(InCookedData, &bIsAlreadyCreated);
				if (UNLIKELY(bIsAlreadyCreated))
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("Loading already created LoadedGroupValue @ %p for key %s"),
						&UsageCount->LoadedData, *UsageCount->Key.GetDebugString());
					continue;
				}

				FCompletionPromise CompletionPromise;
				FutureArray.Add(CompletionPromise.GetFuture());

				LoadSwitchContainerLeafResources(MoveTemp(CompletionPromise), UsageCount);
			}
		}
		else
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("GroupValue %s (%s %" PRIu32 ":%" PRIu32 ") already loaded (Count: %d times)."),
				*InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info.GroupValueCount);
		}
		WaitForFutures(MoveTemp(FutureArray), [&LoadedData, Promise = MoveTemp(Promise)]() mutable
		{
			SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadGroupValueResources GroupValue.SwitchContainer WaitForFutures.Done"));
			LoadedData.bLoaded = true;
			--LoadedData.IsProcessing;
			// We always return success, as GroupValues are not complete deal-breaks and we cannot do anything if they fail.
			return Promise.EmplaceValue(true);
		});
	});
}

void FWwiseResourceLoaderImpl::LoadInitBankResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadInitBankResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadInitBankResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadInitBankResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);

	// Init Bank must be loaded before the SoundBanks (in case they contain something else than Media), but Media
	// can be loaded at the same time than the Init Bank itself.

	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankResources: InitBank %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		return Promise.EmplaceValue(false);
	}

	++LoadedData.IsProcessing;

	FCompletionPromise InitBankPromise;
	auto InitBankFuture = InitBankPromise.GetFuture();
	LoadSoundBankFile(InCookedData, [this, InitBankPromise = MoveTemp(InitBankPromise), &LoadedData, &InCookedData](bool bInResult) mutable
	{
		LoadedData.bLoaded = bInResult;
		if (UNLIKELY(!LoadedData.bLoaded))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadInitBankResources: Could not load InitBank %s (%" PRIu32 ")"),
				*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		}

		// Once the Init Bank is loaded, we can load the supplemental SoundBanks
		auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;

		FCompletionFutureArray SoundBanksFutureArray;
		AddLoadSoundBankFutures(SoundBanksFutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("InitBank"), InCookedData.DebugName.ToString(), InCookedData.SoundBankId);
		WaitForFutures(MoveTemp(SoundBanksFutureArray), [InitBankPromise = MoveTemp(InitBankPromise), bInResult]() mutable
		{
			// Done loading both the Init Bank and the SoundBanks
			InitBankPromise.EmplaceValue();
		});
	});
	
	FCompletionFutureArray MediaFutureArray;
	MediaFutureArray.Add(MoveTemp(InitBankFuture));		// Include the Init Bank & SoundBanks to the MediaFutureArray
	
	AddLoadMediaFutures(MediaFutureArray, LoadedMedia, InCookedData.Media, TEXT("InitBank"), InCookedData.DebugName.ToString(), InCookedData.SoundBankId);
	WaitForFutures(MoveTemp(MediaFutureArray), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
	{
		--LoadedData.IsProcessing;

		return Promise.EmplaceValue(LoadedData.bLoaded);
	});
}

void FWwiseResourceLoaderImpl::LoadMediaResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadMediaResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadMediaResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadMediaResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMediaResources: Media %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);
		return Promise.EmplaceValue(false);
	}

	++LoadedData.IsProcessing;

	LoadMediaFile(InCookedData, [Promise = MoveTemp(Promise), &LoadedData, &InCookedData](bool bResult) mutable
	{
		LoadedData.bLoaded = bResult;
		--LoadedData.IsProcessing;

		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadMediaResources: Could not load Media %s (%" PRIu32 ")"),
				*InCookedData.DebugName.ToString(), (uint32)InCookedData.MediaId);
		}
		Promise.EmplaceValue(bResult);
	});
}

void FWwiseResourceLoaderImpl::LoadShareSetResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadShareSetResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadShareSetResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadShareSetResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadShareSetResources: ShareSet %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);
		return Promise.EmplaceValue(false);
	}

	++LoadedData.IsProcessing;
	FCompletionFutureArray FutureArray;

	AddLoadMediaFutures(FutureArray, LoadedMedia, InCookedData.Media, TEXT("ShareSet"), InCookedData.DebugName.ToString(), InCookedData.ShareSetId);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, InCookedData.SoundBanks, TEXT("ShareSet"), InCookedData.DebugName.ToString(), InCookedData.ShareSetId);
	WaitForFutures(MoveTemp(FutureArray), [this, Promise = MoveTemp(Promise), &LoadedData, &LoadedSoundBanks, &InCookedData]() mutable
	{
		--LoadedData.IsProcessing;
		if (UNLIKELY(LoadedSoundBanks.Num() != InCookedData.SoundBanks.Num()))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::LoadShareSetResources: Could not load %d prerequisites for ShareSet %s (%" PRIu32 "). Unloading and failing."),
				InCookedData.SoundBanks.Num() - LoadedSoundBanks.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.ShareSetId);
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			
			ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadShareSetResources Error"), [this, UnloadPromise = MoveTemp(UnloadPromise), &LoadedData, &InCookedData]() mutable
			{
				UnloadShareSetResources(MoveTemp(UnloadPromise), LoadedData, InCookedData);
			});
			
			UnloadFuture.Next([Promise = MoveTemp(Promise)](int) mutable
			{
				SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadShareSetResources UnloadFuture.Done"));
				return Promise.EmplaceValue(false);
			});
		}
		else
		{
			return Promise.EmplaceValue(true);
		}
	});
}

void FWwiseResourceLoaderImpl::LoadSoundBankResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadSoundBankResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadSoundBankResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			LoadSoundBankResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogLoadResources(InCookedData);

	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankResources: SoundBank %s (%" PRIu32 ") is already loaded."),
		*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		return Promise.EmplaceValue(false);
	}

	++LoadedData.IsProcessing;
	LoadSoundBankFile(InCookedData, [Promise = MoveTemp(Promise), &LoadedData, &InCookedData](bool bResult) mutable
	{
		LoadedData.bLoaded = bResult;
		--LoadedData.IsProcessing;

		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("LoadSoundBankResources: Could not load SoundBank %s (%" PRIu32 ")"),
				*InCookedData.DebugName.ToString(), (uint32)InCookedData.SoundBankId);
		}

		Promise.EmplaceValue(bResult);
	});
}

void FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources(FCompletionPromise&& Promise, TSharedRef<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe> UsageCount)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources"));
	check(ExecutionQueue.IsRunningInThisThread());

	auto& LoadedData = UsageCount->LoadedData;
	const auto& CookedData = UsageCount->Key;
	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources IsProcessing"), [this, Promise = MoveTemp(Promise), UsageCount]() mutable
		{
			LoadSwitchContainerLeafResources(MoveTemp(Promise), UsageCount);
		});
		return;
	}
	
	if (UNLIKELY(LoadedData.IsLoaded()))
	{
		SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources AlreadyLoaded.Done"));
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources[%p]: Loading Switch Container Leaf %s that's already loaded. Skipping."), &UsageCount.Get(), *UsageCount->Key.GetDebugString())
		return Promise.EmplaceValue();
	}

	if (!UsageCount->HaveAllKeys())
	{
		SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources !HaveAllKeys.Done"));
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources[%p]: Loading Switch Container Leaf %s that don't have all the keys anymore. Skipping."), &UsageCount.Get(), *UsageCount->Key.GetDebugString())
		return Promise.EmplaceValue();
	}

	LogLoadResources(CookedData, &LoadedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	++LoadedData.IsProcessing;
	FCompletionFutureArray FutureArray;

	AddLoadExternalSourceFutures(FutureArray, LoadedExternalSources, CookedData.ExternalSources, TEXT("Switch Container Leaf"), CookedData.GetDebugString(), 0);
	AddLoadMediaFutures(FutureArray, LoadedMedia, CookedData.Media, TEXT("Switch Container Leaf"), CookedData.GetDebugString(), 0);
	AddLoadSoundBankFutures(FutureArray, LoadedSoundBanks, CookedData.SoundBanks, TEXT("Switch Container Leaf"), CookedData.GetDebugString(), 0);
	WaitForFutures(MoveTemp(FutureArray), [this, UsageCount, &LoadedData, Promise = MoveTemp(Promise)]() mutable
	{
		SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::LoadSwitchContainerLeafResources WaitForFutures.Done"));
		INC_DWORD_STAT(STAT_WwiseResourceLoaderSwitchContainerCombinations);

		--LoadedData.IsProcessing;
		Promise.EmplaceValue();
	});
}


void FWwiseResourceLoaderImpl::UnloadAuxBusResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadAuxBusResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadAuxBusResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadAuxBusResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	++LoadedData.IsProcessing;
	FCompletionFutureArray FutureArray;
	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks, TEXT("AuxBus"), InCookedData.DebugName.ToString(), InCookedData.AuxBusId);
	AddUnloadMediaFutures(FutureArray, LoadedMedia, TEXT("AuxBus"), InCookedData.DebugName.ToString(), InCookedData.AuxBusId);
	WaitForFutures(MoveTemp(FutureArray), [Promise = MoveTemp(Promise), &LoadedData]() mutable
	{
		--LoadedData.IsProcessing;
		Promise.EmplaceValue();
	});
}

void FWwiseResourceLoaderImpl::UnloadEventResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadEventResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadEventResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadEventResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;

	++LoadedData.IsProcessing;
	FCompletionFutureArray FutureArray;
	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks, TEXT("Event"), InCookedData.DebugName.ToString(), InCookedData.EventId);

	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;
	if (LoadedData.bLoadedSwitchContainerLeaves || LoadedData.LoadedRequiredGroupValues.Num() > 0)
	{
		FCompletionPromise SwitchContainerLeavesPromise;
		FutureArray.Add(SwitchContainerLeavesPromise.GetFuture());

		UnloadEventSwitchContainerResources(MoveTemp(SwitchContainerLeavesPromise), LoadedData, InCookedData);
	}
	AddUnloadExternalSourceFutures(FutureArray, LoadedExternalSources, TEXT("Event"), InCookedData.DebugName.ToString(), InCookedData.EventId);
	AddUnloadMediaFutures(FutureArray, LoadedMedia, TEXT("Event"), InCookedData.DebugName.ToString(), InCookedData.EventId);
	WaitForFutures(MoveTemp(FutureArray), [Promise = MoveTemp(Promise), &LoadedData]() mutable
	{
		SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadEventResources SoundBank.Done Async WaitForFutures.Done"));
		--LoadedData.IsProcessing;
		Promise.EmplaceValue();
	});
}

void FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources"));

	// Unload required GroupValues
	FWwiseLoadedGroupValueList& LoadedRequiredGroupValues = LoadedData.LoadedRequiredGroupValues;
	bool& bLoadedSwitchContainerLeaves = LoadedData.bLoadedSwitchContainerLeaves;

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading %d GroupValues for Event %s (%" PRIu32 ")"),
		(int)LoadedRequiredGroupValues.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	FCompletionFutureArray FutureArray;

	for (auto& GroupValue : LoadedRequiredGroupValues)
	{
		FCompletionPromise GroupValuePromise;
		FutureArray.Add(GroupValuePromise.GetFuture());

		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources GroupValue"), [this, &InCookedData, &GroupValue, GroupValuePromise = MoveTemp(GroupValuePromise)]() mutable
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading GroupValue %s for Event %s (%" PRIu32 ")"),
				*GroupValue.GroupValueCookedData.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

			UnloadGroupValueResources(MoveTemp(GroupValuePromise), GroupValue.LoadedData, GroupValue.GroupValueCookedData);
		});
	}

	// Unload Switch Container Leaves
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading %d Leaves for Event %s (%" PRIu32 ")"),
		(int)InCookedData.SwitchContainerLeaves.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

	if (bLoadedSwitchContainerLeaves) for (const auto& SwitchContainerLeaf : InCookedData.SwitchContainerLeaves)
	{
		FCompletionPromise SwitchContainerLeavesPromise;
		FutureArray.Add(SwitchContainerLeavesPromise.GetFuture());

		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Leaf.SwitchContainer"), [this, &SwitchContainerLeaf, &InCookedData, SwitchContainerLeavesPromise = MoveTemp(SwitchContainerLeavesPromise)]() mutable
		{
			TSharedPtr<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe> UsageCountPtr;
			for (const auto& GroupValue : SwitchContainerLeaf.GroupValueSet)
			{
				const auto InfoKey = FWwiseSwitchContainerLoadedGroupValueInfo(GroupValue);
				FWwiseSwitchContainerLoadedGroupValueInfo* Info = LoadedGroupValueInfo.Find(InfoKey);
				if (UNLIKELY(!Info))
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p]: Could not find requested GroupValue %s for Leaf in Event %s (%" PRIu32 ")"),
						Info,
						*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
					continue;
				}

				for (auto& Leaf : Info->Leaves)
				{
					if (&Leaf->Key == &SwitchContainerLeaf)
					{
						UE_CLOG(UsageCountPtr.IsValid() && UsageCountPtr != Leaf, LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p]: Have two different leaves (%p and %p) for the same GroupValue %s in Event %s (%" PRIu32 ")"),
							Info, UsageCountPtr.Get(), &Leaf.Get(),
							*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
						UsageCountPtr = Leaf;
						Info->Leaves.Remove(Leaf);
						break;
					}
				}

				if (UNLIKELY(!UsageCountPtr))
				{
					UE_LOG(LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p]: Could not find requested Leaf in GroupValue %s in Event %s (%" PRIu32 ")"),
						Info,
						*GroupValue.DebugName.ToString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);
					continue;
				}

				const auto bResourcesAreLoaded = Info->ResourcesAreLoaded();
				auto UsageCount = UsageCountPtr.ToSharedRef();
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p] UsageCount[%p]: Removing requested GroupValue %s for %s in Event %s (%" PRIu32 ")"),
					Info, &UsageCount.Get(),
					*GroupValue.DebugName.ToString(), *UsageCount->Key.GetDebugString(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

				UE_CLOG(!bResourcesAreLoaded, LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p] UsageCount[%p]: Don't have referencing GroupValues: %d for key %s"), Info, &UsageCount.Get(), Info->GroupValueCount, *UsageCount->Key.GetDebugString());
				UE_CLOG(bResourcesAreLoaded, LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p] UsageCount[%p]: Have referencing GroupValues: %d for key %s"), Info, &UsageCount.Get(), Info->GroupValueCount, *UsageCount->Key.GetDebugString());

				const auto bUnloaded = UsageCount->LoadedGroupValues.Remove(GroupValue) == 1;
				UE_CLOG(bUnloaded != bResourcesAreLoaded, LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p] UsageCount[%p]: bUnloaded(%s) != bShouldUnload(%s) @ %p for key %s"),
					Info, &UsageCount.Get(),
					bUnloaded ? TEXT("true") : TEXT("false"), bResourcesAreLoaded ? TEXT("true") : TEXT("false"),
					&UsageCount->LoadedData, *UsageCount->Key.GetDebugString());

				if (!bResourcesAreLoaded && Info->Leaves.Num() == 0)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Info[%p]: No more users. Removing GroupValueInfo for key %s"), Info, *Info->Key.GroupValueCookedData->GetDebugString());
					LoadedGroupValueInfo.Remove(InfoKey);
				}
			}

			if (LIKELY(UsageCountPtr))
			{
				auto UsageCount = UsageCountPtr.ToSharedRef();
				UE_CLOG(UNLIKELY(UsageCount->LoadedGroupValues.Num() > 0), LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources UsageCount[%p]: There are still %d loaded elements for %s in Event %s (%" PRIu32 ")"),
					&UsageCount.Get(), *UsageCount->Key.GetDebugString(),
					(int)UsageCount->LoadedGroupValues.Num(), *InCookedData.DebugName.ToString(), (uint32)InCookedData.EventId);

				
				FCompletionPromise UnloadLeafResourcesPromise;
				auto UnloadLeafResourcesFuture = UnloadLeafResourcesPromise.GetFuture();
				UnloadSwitchContainerLeafResources(MoveTemp(UnloadLeafResourcesPromise), UsageCount);

				UnloadLeafResourcesFuture.Next([this, SwitchContainerLeavesPromise = MoveTemp(SwitchContainerLeavesPromise), UsageCount](int) mutable
				{
					DeleteSwitchContainerLeafGroupValueUsageCount(MoveTemp(SwitchContainerLeavesPromise), UsageCount);
				});
			}
			else
			{
				SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Leaf.SwitchContainer !UsageCountPtr"));
				SwitchContainerLeavesPromise.EmplaceValue();
			}
		});
	}

	WaitForFutures(MoveTemp(FutureArray), [Promise = MoveTemp(Promise), &LoadedRequiredGroupValues, &bLoadedSwitchContainerLeaves]() mutable
	{
		SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::UnloadEventSwitchContainerResources Leaf.SwitchContainer WaitForFutures.Done"));
		LoadedRequiredGroupValues.Empty();
		bLoadedSwitchContainerLeaves = false;
		Promise.EmplaceValue();
	});
}

void FWwiseResourceLoaderImpl::UnloadExternalSourceResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadExternalSourceResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadExternalSourceResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadExternalSourceResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);

	if (LoadedData.IsLoaded())
	{
		++LoadedData.IsProcessing;
		UnloadExternalSourceFile(InCookedData, [&LoadedData, Promise = MoveTemp(Promise)]() mutable
		{
			--LoadedData.IsProcessing;
			LoadedData.bLoaded = false;
			Promise.EmplaceValue();
		});
	}
	else
	{
		Promise.EmplaceValue();
	}
}

void FWwiseResourceLoaderImpl::UnloadGroupValueResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadGroupValueResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadGroupValueResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);
	++LoadedData.IsProcessing;
	
	ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadGroupValueResources Async"), [this, &LoadedData, &InCookedData, Promise = MoveTemp(Promise)]() mutable
	{
		const auto InfoKey = FWwiseSwitchContainerLoadedGroupValueInfo(InCookedData);
		FWwiseSwitchContainerLoadedGroupValueInfo* Info = LoadedGroupValueInfo.Find(InfoKey);
		if (UNLIKELY(!Info))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources: Could not find requested GroupValue %s (%s %" PRIu32 ":%" PRIu32 ")"),
				*InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id);
			return Promise.EmplaceValue();
		}
		check(Info->ResourcesAreLoaded());
		--Info->GroupValueCount;
		const bool bResourcesShouldBeUnloaded = !Info->ResourcesAreLoaded(); 

		FCompletionFutureArray FutureArray;

		if (bResourcesShouldBeUnloaded)
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources Info[%p]: Last GroupValue %s (%s %" PRIu32 ":%" PRIu32 ") unload. Unloading %d leaves."),
				Info,
				*InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info->Leaves.Num());

			if (Info->Leaves.Num() == 0)
			{
				UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources Info[%p]: No more users. Removing GroupValueInfo for key %s"), Info, *Info->Key.GroupValueCookedData->GetDebugString());
				LoadedGroupValueInfo.Remove(InfoKey);
			}
			else
			{
				for (auto UsageCount : Info->Leaves)
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources Info[%p] UsageCount[%p]: Number of GroupValues required for leaf: %d/%d @ %p for %s (-1 in GroupValue)"), Info, &UsageCount.Get(),
						(int)UsageCount->LoadedGroupValues.Num() - 1, UsageCount->Key.GroupValueSet.Num(), &UsageCount->LoadedData, *UsageCount->Key.GetDebugString());

					const auto Result = UsageCount->LoadedGroupValues.Remove(InCookedData);
					UE_CLOG(Result == 0, LogWwiseResourceLoader, Error, TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources Info[%p] UsageCount[%p]: Trying to remove a LoadedGroupValue that is not loaded @ %p for key %s"),
						Info, &UsageCount.Get(), &UsageCount->LoadedData, *UsageCount->Key.GetDebugString());

					FWwiseResourceUnloadPromise UnloadPromise;
					FutureArray.Add(UnloadPromise.GetFuture());
					UnloadSwitchContainerLeafResources(MoveTemp(UnloadPromise), UsageCount);
				}
			}
		}
		else
		{
			UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources Info[%p]: GroupValue %s (%s %" PRIu32 ":%" PRIu32 ") still loaded (Count: %d times)."),
				Info, *InCookedData.DebugName.ToString(), *InCookedData.GetTypeName(), (uint32)InCookedData.GroupId, (uint32)InCookedData.Id, (int)Info->GroupValueCount);
		}

		WaitForFutures(MoveTemp(FutureArray), [&LoadedData, Promise = MoveTemp(Promise)]() mutable
		{
			SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::UnloadGroupValueResources SwitchContainer.Done"));
			--LoadedData.IsProcessing;
			Promise.EmplaceValue();
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadInitBankResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadInitBankResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadInitBankResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadInitBankResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);

	++LoadedData.IsProcessing;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	FCompletionPromise SoundBankPromise;
	auto Future = SoundBankPromise.GetFuture();

	if (LoadedData.bLoaded)
	{
		UnloadSoundBankFile(InCookedData, [&LoadedData, SoundBankPromise = MoveTemp(SoundBankPromise)]() mutable
		{
			LoadedData.bLoaded = false;
			SoundBankPromise.EmplaceValue();
		});
	}
	else
	{
		SoundBankPromise.EmplaceValue();
	}

	Future.Next([this, Promise = MoveTemp(Promise), &LoadedMedia, &InCookedData, &LoadedData](int) mutable
	{
		FCompletionFutureArray FutureArray;
		AddUnloadMediaFutures(FutureArray, LoadedMedia, TEXT("InitBank"), InCookedData.DebugName.ToString(), InCookedData.SoundBankId);
		WaitForFutures(MoveTemp(FutureArray), [Promise = MoveTemp(Promise), &LoadedData]() mutable
		{
			--LoadedData.IsProcessing;
			Promise.EmplaceValue();
		});
	});
}

void FWwiseResourceLoaderImpl::UnloadMediaResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadMediaResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadMediaResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadMediaResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);
	
	if (LoadedData.IsLoaded())
	{
		++LoadedData.IsProcessing;
		UnloadMediaFile(InCookedData, [Promise = MoveTemp(Promise), &LoadedData]() mutable
		{
			LoadedData.bLoaded = false;
			--LoadedData.IsProcessing;
			Promise.EmplaceValue();
		});
	}
	else
	{
		Promise.EmplaceValue();
	}
}

void FWwiseResourceLoaderImpl::UnloadShareSetResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadShareSetResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadShareSetResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadShareSetResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	++LoadedData.IsProcessing;
	FCompletionFutureArray FutureArray;
	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks, TEXT("ShareSet"), InCookedData.DebugName.ToString(), InCookedData.ShareSetId);
	AddUnloadMediaFutures(FutureArray, LoadedMedia, TEXT("ShareSet"), InCookedData.DebugName.ToString(), InCookedData.ShareSetId);
	WaitForFutures(MoveTemp(FutureArray), [Promise = MoveTemp(Promise), &LoadedData]() mutable
	{
		--LoadedData.IsProcessing;
		Promise.EmplaceValue();
	});
}

void FWwiseResourceLoaderImpl::UnloadSoundBankResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadSoundBankResources"));

	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadSoundBankResources IsProcessing"), [this, Promise = MoveTemp(Promise), &LoadedData, &InCookedData]() mutable
		{
			UnloadSoundBankResources(MoveTemp(Promise), LoadedData, InCookedData);
		});
		return;
	}
	
	LogUnloadResources(InCookedData);

	if (LoadedData.IsLoaded())
	{
		++LoadedData.IsProcessing;
		UnloadSoundBankFile(InCookedData, [Promise = MoveTemp(Promise), &LoadedData]() mutable
		{
			LoadedData.bLoaded = false;
			--LoadedData.IsProcessing;
			Promise.EmplaceValue();
		});
	}
	else
	{
		Promise.EmplaceValue();
	}
}

void FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources(FWwiseResourceUnloadPromise&& Promise, TSharedRef<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe> UsageCount)
{
	SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources"));
	check(ExecutionQueue.IsRunningInThisThread());

	auto& LoadedData = UsageCount->LoadedData;
	const auto& CookedData = UsageCount->Key;
	if (LoadedData.IsProcessing)
	{
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources IsProcessing"), [this, Promise = MoveTemp(Promise), UsageCount]() mutable
		{
			UnloadSwitchContainerLeafResources(MoveTemp(Promise), UsageCount);
		});
		return;
	}

	if (UNLIKELY(UsageCount->HaveAllKeys()))
	{
		SCOPED_WWISERESOURCELOADER_EVENT_2(TEXT("FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources LoadedGroupValues.Done"));
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources[%p]: Unloading Switch Container Leaf %s that is still fully in use. Skipping unload."), &UsageCount.Get(), *UsageCount->Key.GetDebugString())
		return Promise.EmplaceValue();
	}
	
	LogUnloadResources(CookedData, &LoadedData);

	auto& LoadedSoundBanks = LoadedData.LoadedSoundBanks;
	auto& LoadedExternalSources = LoadedData.LoadedExternalSources;
	auto& LoadedMedia = LoadedData.LoadedMedia;

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources UsageCount[%p]: Unloading Switch Container Leaf %s"), &UsageCount.Get(), *CookedData.GetDebugString());

	++LoadedData.IsProcessing;
	FCompletionFutureArray FutureArray;
	AddUnloadSoundBankFutures(FutureArray, LoadedSoundBanks, TEXT("Switch Container Leaf"), CookedData.GetDebugString(), 0);
	AddUnloadExternalSourceFutures(FutureArray, LoadedExternalSources, TEXT("Switch Container Leaf"), CookedData.GetDebugString(), 0);
	AddUnloadMediaFutures(FutureArray, LoadedMedia, TEXT("Switch Container Leaf"), CookedData.GetDebugString(), 0);
	WaitForFutures(MoveTemp(FutureArray), [Promise = MoveTemp(Promise), &LoadedData]() mutable
	{
		SCOPED_WWISERESOURCELOADER_EVENT_3(TEXT("FWwiseResourceLoaderImpl::UnloadSwitchContainerLeafResources Done"));
		DEC_DWORD_STAT(STAT_WwiseResourceLoaderSwitchContainerCombinations);
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Done unloading Switch Container Leaf @ %p"), &LoadedData);
		
		--LoadedData.IsProcessing;
		Promise.EmplaceValue();
	});
}

void FWwiseResourceLoaderImpl::DeleteSwitchContainerLeafGroupValueUsageCount(FWwiseResourceUnloadPromise&& Promise,
	TSharedRef<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe>& UsageCount)
{
	if (LIKELY(UsageCount.IsUnique()))
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("FWwiseResourceLoaderImpl::DeleteSwitchContainerLeafGroupValueUsageCount UsageCount[%p]: Destroyed %s"), 
			&UsageCount.Get(), *UsageCount->Key.GetDebugString());
		Promise.EmplaceValue();
	}
	else
	{
		// We need to wait for the user to stop using this. This can happen when a GroupValue is waiting to be unloaded while we want to destroy the SwitchContainerLeaf.
		
		// This makes an unique copy of UsageCount, that is passed as reference to the new instance
		ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::DeleteSwitchContainerLeafGroupValueUsageCount !IsUnique"), [this, Promise = MoveTemp(Promise), UsageCount]() mutable
		{
			DeleteSwitchContainerLeafGroupValueUsageCount(MoveTemp(Promise), UsageCount);
		});
	}
}

void FWwiseResourceLoaderImpl::AttachAuxBusNode(FWwiseLoadedAuxBusPtr AuxBusListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedAuxBusList.AddTail(AuxBusListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderAuxBusses);
}

void FWwiseResourceLoaderImpl::AttachEventNode(FWwiseLoadedEventPtr EventListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedEventList.AddTail(EventListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderEvents);
}

void FWwiseResourceLoaderImpl::AttachExternalSourceNode(FWwiseLoadedExternalSourcePtr ExternalSourceListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedExternalSourceList.AddTail(ExternalSourceListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderExternalSources);
}

void FWwiseResourceLoaderImpl::AttachGroupValueNode(FWwiseLoadedGroupValuePtr GroupValueListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedGroupValueList.AddTail(GroupValueListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderGroupValues);
}

void FWwiseResourceLoaderImpl::AttachInitBankNode(FWwiseLoadedInitBankPtr InitBankListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedInitBankList.AddTail(InitBankListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderInitBanks);
}

void FWwiseResourceLoaderImpl::AttachMediaNode(FWwiseLoadedMediaPtr MediaListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedMediaList.AddTail(MediaListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderMedia);
}

void FWwiseResourceLoaderImpl::AttachShareSetNode(FWwiseLoadedShareSetPtr ShareSetListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedShareSetList.AddTail(ShareSetListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderShareSets);
}

void FWwiseResourceLoaderImpl::AttachSoundBankNode(FWwiseLoadedSoundBankPtr SoundBankListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedSoundBankList.AddTail(SoundBankListNode);
	}
	INC_DWORD_STAT(STAT_WwiseResourceLoaderSoundBanks);
}


void FWwiseResourceLoaderImpl::DetachAuxBusNode(FWwiseLoadedAuxBusPtr AuxBusListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedAuxBusList.RemoveNode(AuxBusListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderAuxBusses);
}

void FWwiseResourceLoaderImpl::DetachEventNode(FWwiseLoadedEventPtr EventListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedEventList.RemoveNode(EventListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderEvents);
}

void FWwiseResourceLoaderImpl::DetachExternalSourceNode(FWwiseLoadedExternalSourcePtr ExternalSourceListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedExternalSourceList.RemoveNode(ExternalSourceListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderExternalSources);
}

void FWwiseResourceLoaderImpl::DetachGroupValueNode(FWwiseLoadedGroupValuePtr GroupValueListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedGroupValueList.RemoveNode(GroupValueListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderGroupValues);
}

void FWwiseResourceLoaderImpl::DetachInitBankNode(FWwiseLoadedInitBankPtr InitBankListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedInitBankList.RemoveNode(InitBankListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderInitBanks);
}

void FWwiseResourceLoaderImpl::DetachMediaNode(FWwiseLoadedMediaPtr MediaListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedMediaList.RemoveNode(MediaListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderMedia);
}

void FWwiseResourceLoaderImpl::DetachShareSetNode(FWwiseLoadedShareSetPtr ShareSetListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedShareSetList.RemoveNode(ShareSetListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderShareSets);
}

void FWwiseResourceLoaderImpl::DetachSoundBankNode(FWwiseLoadedSoundBankPtr SoundBankListNode)
{
	{
		FScopeLock Lock(&ListUpdateCriticalSection);		
		LoadedSoundBankList.RemoveNode(SoundBankListNode, false);
	}
	DEC_DWORD_STAT(STAT_WwiseResourceLoaderSoundBanks);
}


void FWwiseResourceLoaderImpl::AddLoadExternalSourceFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources,
                                                            const TArray<FWwiseExternalSourceCookedData>& InExternalSources, const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const
{
	for (const auto& ExternalSource : InExternalSources)
	{
		TWwisePromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		LoadExternalSourceFile(ExternalSource, [this, &ExternalSource, &LoadedExternalSources, InType, InDebugName, InShortId, Promise = MoveTemp(Promise)](bool bInResult) mutable
		{
			if (UNLIKELY(!bInResult))
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load External Source %s (%" PRIu32 ") for %s %s (%" PRIu32 ")"),
					InType,
					*ExternalSource.DebugName.ToString(), (uint32)ExternalSource.Cookie,
					InType, *InDebugName, (uint32)InShortId);
				Promise.EmplaceValue();
			}
			else
			{
				ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::AddLoadExternalSourceFutures EmplaceValue"), [&ExternalSource, &LoadedExternalSources, Promise = MoveTemp(Promise), InType, InDebugName, InShortId]() mutable
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("AddLoadExternalSourceFutures: Adding SoundBank %s (%" PRIu32 ") to %s %s (%" PRIu32 ")"),
						*ExternalSource.DebugName.ToString(), (uint32)ExternalSource.Cookie,
						InType, *InDebugName, (uint32)InShortId);
						
					LoadedExternalSources.Add(&ExternalSource);
					Promise.EmplaceValue();
				});
			}
		});
	}
}

void FWwiseResourceLoaderImpl::AddUnloadExternalSourceFutures(FCompletionFutureArray& FutureArray,
	TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources,
	const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const
{
	if (LoadedExternalSources.Num() == 0) return;
	const auto ToUnload(MoveTemp(LoadedExternalSources));
	LoadedExternalSources.Empty();

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("AddUnloadExternalSourceFutures: Unloading all %d External Sources from %s %s (%" PRIu32 ")"),
		(int)ToUnload.Num(),
		InType, *InDebugName, (uint32)InShortId);

	for (const auto* ExternalSource : ToUnload)
	{
		TWwisePromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		UnloadExternalSourceFile(*ExternalSource, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
	}
}

void FWwiseResourceLoaderImpl::AddLoadMediaFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseMediaCookedData*>& LoadedMedia,
                                                   const TArray<FWwiseMediaCookedData>& InMedia, const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const
{
	for (const auto& Media : InMedia)
	{
		TWwisePromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		LoadMediaFile(Media, [this, &Media, &LoadedMedia, InType, InDebugName, InShortId, Promise = MoveTemp(Promise)](bool bInResult) mutable
		{
			if (UNLIKELY(!bInResult))
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load Media %s (%" PRIu32 ") for %s %s (%" PRIu32 ")"),
					InType,
					*Media.DebugName.ToString(), (uint32)Media.MediaId,
					InType, *InDebugName, (uint32)InShortId);
				Promise.EmplaceValue();
			}
			else
			{
				if (UNLIKELY(Test::bMockSleepOnMediaLoad))
				{
					FPlatformProcess::Sleep(0.001f);
				}
				ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::AddLoadMediaFutures EmplaceValue"), [&Media, &LoadedMedia, Promise = MoveTemp(Promise), InType, InDebugName, InShortId]() mutable
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("AddLoadMediaFutures: Adding Media %s (%" PRIu32 ") to %s %s (%" PRIu32 ")"),
						*Media.DebugName.ToString(), (uint32)Media.MediaId,
						InType, *InDebugName, (uint32)InShortId);
						
					LoadedMedia.Add(&Media);
					Promise.EmplaceValue();
				});
			}
		});
	}
}

void FWwiseResourceLoaderImpl::AddUnloadMediaFutures(FCompletionFutureArray& FutureArray,
	TArray<const FWwiseMediaCookedData*>& LoadedMedia,
	const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const
{
	if (LoadedMedia.Num() == 0) return;
	const auto ToUnload(MoveTemp(LoadedMedia));
	LoadedMedia.Empty();

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("AddUnloadMediaFutures: Unloading all %d Media from %s %s (%" PRIu32 ")"),
		(int)ToUnload.Num(),
		InType, *InDebugName, (uint32)InShortId);
						
	for (const auto* Media : ToUnload)
	{
		TWwisePromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		UnloadMediaFile(*Media, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
	}
}

void FWwiseResourceLoaderImpl::AddLoadSoundBankFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks,
                                                       const TArray<FWwiseSoundBankCookedData>& InSoundBank, const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const
{
	for (const auto& SoundBank : InSoundBank)
	{
		TWwisePromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		LoadSoundBankFile(SoundBank, [this, &SoundBank, &LoadedSoundBanks, InType, InDebugName, InShortId, Promise = MoveTemp(Promise)](bool bInResult) mutable
		{
			if (UNLIKELY(!bInResult))
			{
				UE_LOG(LogWwiseResourceLoader, Warning, TEXT("Load%sResources: Could not load SoundBank %s (%" PRIu32 ") for %s %s (%" PRIu32 ")"),
					InType,
					*SoundBank.DebugName.ToString(), (uint32)SoundBank.SoundBankId,
					InType, *InDebugName, (uint32)InShortId);
				Promise.EmplaceValue();
			}
			else
			{
				ExecutionQueue.Async(WWISERESOURCELOADER_ASYNC_NAME("FWwiseResourceLoaderImpl::AddLoadSoundBankFutures EmplaceValue"), [&SoundBank, &LoadedSoundBanks, Promise = MoveTemp(Promise), InType, InDebugName, InShortId]() mutable
				{
					UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("AddLoadSoundBankFutures: Adding SoundBank %s (%" PRIu32 ") to %s %s (%" PRIu32 ")"),
						*SoundBank.DebugName.ToString(), (uint32)SoundBank.SoundBankId,
						InType, *InDebugName, (uint32)InShortId);
						
					LoadedSoundBanks.Add(&SoundBank);
					Promise.EmplaceValue();
				});
			}
		});
	}
}

void FWwiseResourceLoaderImpl::AddUnloadSoundBankFutures(FCompletionFutureArray& FutureArray,
                                                         TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks,
                                                         const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const
{
	if (LoadedSoundBanks.Num() == 0) return;
	const auto ToUnload(MoveTemp(LoadedSoundBanks));
	LoadedSoundBanks.Empty();

	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("AddUnloadSoundBankFutures: Unloading all %d SoundBanks from %s %s (%" PRIu32 ")"),
		(int)ToUnload.Num(),
		InType, *InDebugName, (uint32)InShortId);
						
	for (const auto* SoundBank : ToUnload)
	{
		TWwisePromise<void> Promise;
		FutureArray.Add(Promise.GetFuture());
		UnloadSoundBankFile(*SoundBank, [Promise = MoveTemp(Promise)]() mutable
		{
			Promise.EmplaceValue();
		});
	}
}

void FWwiseResourceLoaderImpl::WaitForFutures(FCompletionFutureArray&& FutureArray, FCompletionCallback&& Callback, int NextId) const
{
	{
		SCOPED_WWISERESOURCELOADER_EVENT_4(TEXT("FWwiseResourceLoaderImpl::WaitForFutures"));
		while (FutureArray.Num() > NextId)
		{
			auto Future = MoveTemp(FutureArray[NextId]);
			if (Future.IsReady())
			{
				++NextId;
			}
			else
			{
				Future.Next([this, FutureArray = MoveTemp(FutureArray), Callback = MoveTemp(Callback), NextId = NextId + 1](int) mutable
				{
					WaitForFutures(MoveTemp(FutureArray), MoveTemp(Callback), NextId);
				});
				return;
			}
		}
	}
	return Callback();
}

void FWwiseResourceLoaderImpl::LoadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FLoadFileCallback&& InCallback) const
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadSoundBankAsync: %" PRIu32 "] %s at %s"),
		(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString(), *InSoundBank.SoundBankPathName.ToString());

	if (UNLIKELY(!SoundBankManager))
	{
		SoundBankManager = IWwiseSoundBankManager::Get();
		if (UNLIKELY(!SoundBankManager))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve SoundBank Manager"));
			InCallback(false);
			return;
		}
	}
	SoundBankManager->LoadSoundBank(InSoundBank, GetUnrealPath(), [&InSoundBank, InCallback = MoveTemp(InCallback)](bool bInResult)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadSoundBankAsync: %" PRIu32 "] %s: Done."),
			(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString());
		InCallback(bInResult);
	});
}

void FWwiseResourceLoaderImpl::UnloadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FUnloadFileCallback&& InCallback) const
{
	auto Path = GetUnrealPath(InSoundBank.SoundBankPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadSoundBankAsync: %" PRIu32 "] %s at %s"),
		(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString(), *InSoundBank.SoundBankPathName.ToString());

	if (UNLIKELY(!SoundBankManager))
	{
		UE_CLOG(!IsEngineExitRequested(), LogWwiseResourceLoader, Error, TEXT("Failed to retrieve SoundBank Manager"));
		InCallback();
		return;
	}
	SoundBankManager->UnloadSoundBank(InSoundBank, GetUnrealPath(), [&InSoundBank, InCallback = MoveTemp(InCallback)]()
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadSoundBankAsync: %" PRIu32 "] %s: Done."),
			(uint32)InSoundBank.SoundBankId, *InSoundBank.DebugName.ToString());
		InCallback();
	});
}

void FWwiseResourceLoaderImpl::LoadMediaFile(const FWwiseMediaCookedData& InMedia, FLoadFileCallback&& InCallback) const
{
	auto Path = GetUnrealPath(InMedia.MediaPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadMediaAsync: %" PRIu32 "] %s at %s"),
		(uint32)InMedia.MediaId, *InMedia.DebugName.ToString(), *InMedia.MediaPathName.ToString());

	if (UNLIKELY(!MediaManager))
	{
		MediaManager = IWwiseMediaManager::Get();
		if (UNLIKELY(!MediaManager))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve Media Manager"));
			InCallback(false);
			return;
		}
	}

	MediaManager->LoadMedia(InMedia, GetUnrealPath(), [&InMedia, InCallback = MoveTemp(InCallback)](bool bInResult)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadMediaAsync: %" PRIu32 "] %s: Done."),
			(uint32)InMedia.MediaId, *InMedia.DebugName.ToString());
		InCallback(bInResult);
	});
}

void FWwiseResourceLoaderImpl::UnloadMediaFile(const FWwiseMediaCookedData& InMedia, FUnloadFileCallback&& InCallback) const
{
	auto Path = GetUnrealPath(InMedia.MediaPathName);
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadMediaAsync: %" PRIu32 "] %s at %s"),
		(uint32)InMedia.MediaId, *InMedia.DebugName.ToString(), *InMedia.MediaPathName.ToString());


	if (UNLIKELY(!MediaManager))
	{
		UE_CLOG(!IsEngineExitRequested(), LogWwiseResourceLoader, Error, TEXT("Failed to retrieve Media Manager"));
		InCallback();
		return;
	}

	MediaManager->UnloadMedia(InMedia, GetUnrealPath(), [&InMedia, InCallback = MoveTemp(InCallback)]()
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadMediaAsync: %" PRIu32 "] %s: Done."),
			(uint32)InMedia.MediaId, *InMedia.DebugName.ToString());
		InCallback();
	});
}

void FWwiseResourceLoaderImpl::LoadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FLoadFileCallback&& InCallback) const
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadExternalSourceAsync: %" PRIu32 "] %s"),
		(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());

	if (UNLIKELY(!ExternalSourceManager))
	{
		ExternalSourceManager = IWwiseExternalSourceManager::Get();
		if (UNLIKELY(!ExternalSourceManager))
		{
			UE_LOG(LogWwiseResourceLoader, Error, TEXT("Failed to retrieve External Source Manager"));
			InCallback(false);
			return;
		}
	}

	ExternalSourceManager->LoadExternalSource(InExternalSource, GetUnrealExternalSourcePath(), CurrentLanguage, [&InExternalSource, InCallback = MoveTemp(InCallback)](bool bInResult)
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[LoadExternalSourceAsync: %" PRIu32 "] %s: Done."),
			(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());
		InCallback(bInResult);
	});
}

void FWwiseResourceLoaderImpl::UnloadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FUnloadFileCallback&& InCallback) const
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadExternalSourceAsync: %" PRIu32 "] %s"),
		(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());

	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_CLOG(!IsEngineExitRequested(), LogWwiseResourceLoader, Error, TEXT("Failed to retrieve External Source Manager"));
		InCallback();
		return;
	}

	ExternalSourceManager->UnloadExternalSource(InExternalSource, GetUnrealExternalSourcePath(), CurrentLanguage, [&InExternalSource, InCallback = MoveTemp(InCallback)]()
	{
		UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("[UnloadExternalSourceAsync: %" PRIu32 "] %s: Done."),
			(uint32)InExternalSource.Cookie, *InExternalSource.DebugName.ToString());
		InCallback();
	});
}
