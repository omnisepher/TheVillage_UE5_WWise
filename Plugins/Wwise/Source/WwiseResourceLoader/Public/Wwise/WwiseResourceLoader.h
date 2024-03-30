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

#include "Wwise/WwiseResourceLoaderImpl.h"
#include "Wwise/WwiseResourceLoaderModule.h"

class IWwiseSoundBankManager;
class IWwiseExternalSourceManager;
class IWwiseMediaManager;
/**
 * @brief Operations available to manage and handle Wwise SoundBanks in Unreal.
*/
class WWISERESOURCELOADER_API FWwiseResourceLoader
{
public:
	using FWwiseSetLanguageFuture = TWwiseFuture<void>;
	using FWwiseSetLanguagePromise = TWwisePromise<void>;

	inline static FWwiseResourceLoader* Get()
	{
		if (auto* Module = IWwiseResourceLoaderModule::GetModule())
		{
			return Module->GetResourceLoader();
		}
		return nullptr;
	}
	static FWwiseResourceLoader* Instantiate()
	{
		if (auto* Module = IWwiseResourceLoaderModule::GetModule())
		{
			return Module->InstantiateResourceLoader();
		}
		return nullptr;
	}

	virtual bool IsEnabled() const;
	virtual void Enable();
	virtual void Disable();

	FWwiseResourceLoader();
	virtual ~FWwiseResourceLoader() {}

	//
	// User-facing operations
	//

	FWwiseLanguageCookedData GetCurrentLanguage() const;
	FWwiseSharedPlatformId GetCurrentPlatform() const;

	/**
	 * @brief Returns the actual Unreal file path needed in order to retrieve this particular Wwise Path.
	 * 
	 * This method acts differently depending on usage in ResourceLoaderImpl or Editor. In Editor, this will return
	 * the full path to the Generated SoundBanks folder. In a packaged game, this will return the full
	 * path to the staged file.
	 * 
	 * @param WwisePath Requested file path, as found in SoundBanksInfo.
	 * @return The corresponding Unreal path.
	*/
	virtual FString GetUnrealPath(const FName& InPath) const { return GetUnrealPath(InPath.ToString()); }
	virtual FString GetUnrealPath(const FString& InPath) const;
	virtual FName GetUnrealExternalSourcePath() const;

	virtual FString GetUnrealStagePath(const FName& InPath) const { return GetUnrealStagePath(InPath.ToString()); }
	virtual FString GetUnrealStagePath(const FString& InPath) const;
#if WITH_EDITORONLY_DATA
	virtual FString GetUnrealGeneratedSoundBanksPath(const FName& InPath) const { return GetUnrealGeneratedSoundBanksPath(InPath.ToString()); }
	virtual FString GetUnrealGeneratedSoundBanksPath(const FString& InPath) const;

	virtual void SetUnrealGeneratedSoundBanksPath(const FDirectoryPath& DirectoryPath);
	virtual const FDirectoryPath& GetUnrealGeneratedSoundBanksPath();
#endif

	/**
	 * @brief Sets the language for the current runtime, optionally reloading all affected assets immediately
	 * @param LanguageId The current language being processed, or 0 if none
	 * @param ReloadLanguage What reload strategy should be used for language changes 
	*/
	virtual void SetLanguage(FWwiseLanguageCookedData InLanguage, EWwiseReloadLanguage InReloadLanguage);
	virtual void SetPlatform(const FWwiseSharedPlatformId& InPlatform);

	virtual FWwiseLoadedAuxBusPtr LoadAuxBus(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual void UnloadAuxBus(FWwiseLoadedAuxBusPtr&& InAuxBus);

	virtual FWwiseLoadedEventPtr LoadEvent(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual void UnloadEvent(FWwiseLoadedEventPtr&& InEvent);

	virtual FWwiseLoadedExternalSourcePtr LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData);
	virtual void UnloadExternalSource(FWwiseLoadedExternalSourcePtr&& InExternalSource);

	virtual FWwiseLoadedGroupValuePtr LoadGroupValue(const FWwiseGroupValueCookedData& InGroupValueCookedData);
	virtual void UnloadGroupValue(FWwiseLoadedGroupValuePtr&& InGroupValue);

	virtual FWwiseLoadedInitBankPtr LoadInitBank(const FWwiseInitBankCookedData& InInitBankCookedData);
	virtual void UnloadInitBank(FWwiseLoadedInitBankPtr&& InInitBank);

	virtual FWwiseLoadedMediaPtr LoadMedia(const FWwiseMediaCookedData& InMediaCookedData);
	virtual void UnloadMedia(FWwiseLoadedMediaPtr&& InMedia);

	virtual FWwiseLoadedShareSetPtr LoadShareSet(const FWwiseLocalizedShareSetCookedData& InShareSetCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual void UnloadShareSet(FWwiseLoadedShareSetPtr&& InShareSet);

	virtual FWwiseLoadedSoundBankPtr LoadSoundBank(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual void UnloadSoundBank(FWwiseLoadedSoundBankPtr&& InSoundBank);

	virtual FWwiseSetLanguageFuture SetLanguageAsync(FWwiseLanguageCookedData InLanguage, EWwiseReloadLanguage InReloadLanguage);

	virtual FWwiseLoadedAuxBusFuture LoadAuxBusAsync(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual FWwiseResourceUnloadFuture UnloadAuxBusAsync(FWwiseLoadedAuxBusFuture&& InAuxBus);

	virtual FWwiseLoadedEventFuture LoadEventAsync(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual FWwiseResourceUnloadFuture UnloadEventAsync(FWwiseLoadedEventFuture&& InEvent);

	virtual FWwiseLoadedExternalSourceFuture LoadExternalSourceAsync(const FWwiseExternalSourceCookedData& InExternalSourceCookedData);
	virtual FWwiseResourceUnloadFuture UnloadExternalSourceAsync(FWwiseLoadedExternalSourceFuture&& InExternalSource);

	virtual FWwiseLoadedGroupValueFuture LoadGroupValueAsync(const FWwiseGroupValueCookedData& InGroupValueCookedData);
	virtual FWwiseResourceUnloadFuture UnloadGroupValueAsync(FWwiseLoadedGroupValueFuture&& InGroupValue);

	virtual FWwiseLoadedInitBankFuture LoadInitBankAsync(const FWwiseInitBankCookedData& InInitBankCookedData);
	virtual FWwiseResourceUnloadFuture UnloadInitBankAsync(FWwiseLoadedInitBankFuture&& InInitBank);

	virtual FWwiseLoadedMediaFuture LoadMediaAsync(const FWwiseMediaCookedData& InMediaCookedData);
	virtual FWwiseResourceUnloadFuture UnloadMediaAsync(FWwiseLoadedMediaFuture&& InMedia);

	virtual FWwiseLoadedShareSetFuture LoadShareSetAsync(const FWwiseLocalizedShareSetCookedData& InShareSetCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual FWwiseResourceUnloadFuture UnloadShareSetAsync(FWwiseLoadedShareSetFuture&& InShareSet);

	virtual FWwiseLoadedSoundBankFuture LoadSoundBankAsync(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride = nullptr);
	virtual FWwiseResourceUnloadFuture UnloadSoundBankAsync(FWwiseLoadedSoundBankFuture&& InSoundBank);

	virtual FWwiseSharedPlatformId SystemPlatform() const;
	virtual FWwiseLanguageCookedData SystemLanguage() const;

	TUniquePtr<FWwiseResourceLoaderImpl> ResourceLoaderImpl;
};
