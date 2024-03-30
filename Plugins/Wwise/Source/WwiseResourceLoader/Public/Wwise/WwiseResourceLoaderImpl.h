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

#include "WwiseResourceLoaderModule.h"
#include "Wwise/WwiseExecutionQueue.h"
#include "Wwise/CookedData/WwiseAuxBusCookedData.h"
#include "Wwise/CookedData/WwiseEventCookedData.h"
#include "Wwise/CookedData/WwiseShareSetCookedData.h"
#include "Wwise/Loaded/WwiseLoadedAuxBus.h"
#include "Wwise/Loaded/WwiseLoadedSoundBank.h"
#include "Wwise/Loaded/WwiseLoadedEvent.h"
#include "Wwise/Loaded/WwiseLoadedExternalSource.h"
#include "Wwise/Loaded/WwiseLoadedGroupValue.h"
#include "Wwise/Loaded/WwiseLoadedInitBank.h"
#include "Wwise/Loaded/WwiseLoadedMedia.h"
#include "Wwise/Loaded/WwiseLoadedShareSet.h"

#include "Wwise/WwiseResourceLoaderFuture.h"
#include "Wwise/WwiseSharedGroupValueKey.h"
#include "Wwise/WwiseSharedLanguageId.h"
#include "Wwise/WwiseSharedPlatformId.h"

#include "Wwise/Stats/ResourceLoader.h"

#if WITH_EDITORONLY_DATA
#include "Engine/EngineTypes.h"
#include "UObject/SoftObjectPath.h"
#endif

#include "WwiseResourceLoaderImpl.generated.h"

class IWwiseSoundBankManager;
class IWwiseMediaManager;
class IWwiseExternalSourceManager;
/**
 * @brief What reload strategy should be used for language changes
*/
UENUM()
enum EWwiseReloadLanguage
{
	/// Don't reload anything. The game is fully responsible to reload elements. This doesn't call
	/// any operation on the SoundEngine side, so everything will keep on working as usual.
	Manual,

	/// Reloads immediately without stopping anything. Game is responsible for stopping and restarting
	/// possibly affected sounds or else they might cause audible breaks. This is useful when some
	/// sounds can keep on playing, such as music and ambient sounds, while the dialogues are being
	/// internally reloaded.
	/// 
	/// Depending on the quantity of currently loaded localized banks, the operation can take a long time.
	/// 
	/// \warning Affected events needs to be restarted once the operation is done.
	Immediate,

	/// Stops all sounds first, unloads all the localized banks, and reloads the new language. This will
	/// cause an audible break while the operation is done.
	/// 
	/// Depending on the quantity of currently loaded localized banks, the operation can take a long time.
	/// 
	/// \warning Affected events needs to be restarted once the operation is done.
	Safe
};

/**
 * @brief Whether the WwiseResourceLoader is allowed to load/unload assets
*/
enum class EWwiseResourceLoaderState
{
	/// Do not allow the WwiseResourceLoader to load/unload assets
	AlwaysDisabled,
	/// Allow the WwiseResourceLoader to load/unload assets
	Enabled,
};

struct WWISERESOURCELOADER_API FWwiseSwitchContainerLeafGroupValueUsageCount
{
	/**
	 * @brief SwitchContainer Leaf this structure represents.
	*/
	const FWwiseSwitchContainerLeafCookedData& Key;

	/**
	 * @brief Number of GroupValues present in this key that are already loaded.
	*/
	TSet<FWwiseGroupValueCookedData> LoadedGroupValues;

	/**
	 * @brief Resources represented by the Key that were successfully loaded.
	*/
	struct WWISERESOURCELOADER_API FLoadedData
	{
		FLoadedData();
		TArray<const FWwiseSoundBankCookedData*> LoadedSoundBanks;
		TArray<const FWwiseExternalSourceCookedData*> LoadedExternalSources;
		TArray<const FWwiseMediaCookedData*> LoadedMedia;
		int IsProcessing{0};
		bool IsLoaded() const;
	} LoadedData;

	FWwiseSwitchContainerLeafGroupValueUsageCount(const FWwiseSwitchContainerLeafCookedData& InKey);

	bool HaveAllKeys() const;
};

struct WWISERESOURCELOADER_API FWwiseSwitchContainerLoadedGroupValueInfo
{
	/**
	 * @brief GroupValue key this structure represents.
	*/
	FWwiseSharedGroupValueKey Key;

	/**
	 * @brief Number of times this particular GroupValue got loaded in the currently loaded maps.
	 * 
	 * Any value higher than 0 means the Leaves might be required.
	*/
	int GroupValueCount;

	/**
	 * @brief Leaves that uses this particular GroupValue.
	 * 
	 * @note The ownership of this pointer is uniquely created and discarded during SwitchContainerLeaf loading and unloading.
	*/
	TSet<TSharedRef<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe>> Leaves;

	FWwiseSwitchContainerLoadedGroupValueInfo(const FWwiseSharedGroupValueKey& InKey) :
		Key(InKey),
		GroupValueCount(0),
		Leaves()
	{}

	bool ResourcesAreLoaded() const
	{
		check(GroupValueCount >= 0);

		return GroupValueCount > 0;
	}

	bool operator ==(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key == InRhs.Key;
	}

	bool operator !=(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key != InRhs.Key;
	}

	bool operator <(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key < InRhs.Key;
	}

	bool operator <=(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key <= InRhs.Key;
	}

	bool operator >(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key > InRhs.Key;
	}

	bool operator >=(const FWwiseSwitchContainerLoadedGroupValueInfo& InRhs) const
	{
		return Key >= InRhs.Key;
	}

	bool operator ==(const FWwiseSharedGroupValueKey& InRhs) const
	{
		return Key == InRhs;
	}

	bool operator !=(const FWwiseSharedGroupValueKey& InRhs) const
	{
		return Key != InRhs;
	}
};
inline uint32 GetTypeHash(const FWwiseSwitchContainerLoadedGroupValueInfo& InValue)
{
	return GetTypeHash(InValue.Key);
}

class WWISERESOURCELOADER_API FWwiseResourceLoaderImpl
{
public:
	using FWwiseSetLanguageFuture = TWwiseFuture<void>;
	using FWwiseSetLanguagePromise = TWwisePromise<void>;

	static FWwiseResourceLoaderImpl* Instantiate()
	{
		if (auto* Module = IWwiseResourceLoaderModule::GetModule())
		{
			return Module->InstantiateResourceLoaderImpl();
		}
		return nullptr;
	}

	EWwiseResourceLoaderState WwiseResourceLoaderState = EWwiseResourceLoaderState::Enabled;

	/**
	 * @brief Currently targeted platform for this runtime
	*/
	FWwiseSharedPlatformId CurrentPlatform;

	/**
	 * @brief Currently targeted language for this runtime
	*/
	FWwiseLanguageCookedData CurrentLanguage;

	/**
	 * @brief Location in the staged product where the SoundBank medias are found
	*/
	FString StagePath;

#if WITH_EDITORONLY_DATA
	/**
	 * @brief Location where the Wwise Generated SoundBanks product is found on disk relative to the project
	*/
	FDirectoryPath GeneratedSoundBanksPath;
#endif

	ENamedThreads::Type TaskThread = ENamedThreads::AnyThread;

	/**
	 * Constructor used when creating a default Resource Loader implementation. You should not create or use one yourself,
	 * and you should use the WwiseResourceLoader::Get class accessor to retrieve a valid Resource Loader instance.
	 *
	 * This can be called by derived classes if you want to modify the default Resource Loader behavior.
	 */
	FWwiseResourceLoaderImpl();

	/**
	 * Constructor used for mock testing the Resource Loader. Hard-codes the different managers.
	 * 
	 * @param ExternalSourceManager External Source Manager file handler to be used in this instance
	 * @param MediaManager Media Manager file handler to be used in this instance
	 * @param SoundBankManager  SoundBank Manager file handler to be used in this instance
	 */
	FWwiseResourceLoaderImpl(IWwiseExternalSourceManager& ExternalSourceManager, IWwiseMediaManager& MediaManager, IWwiseSoundBankManager& SoundBankManager);

	virtual ~FWwiseResourceLoaderImpl() {}

	FName GetUnrealExternalSourcePath() const;
	FString GetUnrealPath() const;
	FString GetUnrealPath(const FName& InPath) const { return GetUnrealPath(InPath.ToString()); }
	FString GetUnrealPath(const FString& InPath) const;

	FString GetUnrealStagePath(const FName& InPath) const;
	FString GetUnrealStagePath(const FString& InPath) const;
#if WITH_EDITORONLY_DATA
	FString GetUnrealGeneratedSoundBanksPath(const FName& InPath) const { return GetUnrealGeneratedSoundBanksPath(InPath.ToString());}
	FString GetUnrealGeneratedSoundBanksPath(const FString& InPath) const;
#endif

	virtual EWwiseResourceLoaderState GetResourceLoaderState();
	virtual void SetResourceLoaderState(EWwiseResourceLoaderState State);
	virtual bool IsEnabled();
	virtual void Disable();
	virtual void Enable();

	virtual void SetLanguageAsync(FWwiseSetLanguagePromise&& Promise, const FWwiseLanguageCookedData& InLanguage, EWwiseReloadLanguage InReloadLanguage);
	void SetPlatform(const FWwiseSharedPlatformId& InPlatform);

	virtual FWwiseLoadedAuxBusPtr CreateAuxBusNode(const FWwiseLocalizedAuxBusCookedData& InAuxBusCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadAuxBusAsync(FWwiseLoadedAuxBusPromise&& Promise, FWwiseLoadedAuxBusPtr&& InAuxBusListNode);
	virtual void UnloadAuxBusAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedAuxBusPtr&& InAuxBusListNode);

	virtual FWwiseLoadedEventPtr CreateEventNode(const FWwiseLocalizedEventCookedData& InEventCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadEventAsync(FWwiseLoadedEventPromise&& Promise, FWwiseLoadedEventPtr&& InEventListNode);
	virtual void UnloadEventAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEventPtr&& InEventListNode);

	virtual FWwiseLoadedExternalSourcePtr CreateExternalSourceNode(const FWwiseExternalSourceCookedData& InExternalSourceCookedData);
	virtual void LoadExternalSourceAsync(FWwiseLoadedExternalSourcePromise&& Promise, FWwiseLoadedExternalSourcePtr&& InExternalSourceListNode);
	virtual void UnloadExternalSourceAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedExternalSourcePtr&& InExternalSourceListNode);

	virtual FWwiseLoadedGroupValuePtr CreateGroupValueNode(const FWwiseGroupValueCookedData& InGroupValueCookedData);
	virtual void LoadGroupValueAsync(FWwiseLoadedGroupValuePromise&& Promise, FWwiseLoadedGroupValuePtr&& InGroupValueListNode);
	virtual void UnloadGroupValueAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedGroupValuePtr&& InGroupValueListNode);

	virtual FWwiseLoadedInitBankPtr CreateInitBankNode(const FWwiseInitBankCookedData& InInitBankCookedData);
	virtual void LoadInitBankAsync(FWwiseLoadedInitBankPromise&& Promise, FWwiseLoadedInitBankPtr&& InInitBankListNode);
	virtual void UnloadInitBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedInitBankPtr&& InInitBankListNode);

	virtual FWwiseLoadedMediaPtr CreateMediaNode(const FWwiseMediaCookedData& InMediaCookedData);
	virtual void LoadMediaAsync(FWwiseLoadedMediaPromise&& Promise, FWwiseLoadedMediaPtr&& InMediaListNode);
	virtual void UnloadMediaAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedMediaPtr&& InMediaListNode);

	virtual FWwiseLoadedShareSetPtr CreateShareSetNode(const FWwiseLocalizedShareSetCookedData& InShareSetCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadShareSetAsync(FWwiseLoadedShareSetPromise&& Promise, FWwiseLoadedShareSetPtr&& InShareSetListNode);
	virtual void UnloadShareSetAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedShareSetPtr&& InShareSetListNode);

	virtual FWwiseLoadedSoundBankPtr CreateSoundBankNode(const FWwiseLocalizedSoundBankCookedData& InSoundBankCookedData, const FWwiseLanguageCookedData* InLanguageOverride);
	virtual void LoadSoundBankAsync(FWwiseLoadedSoundBankPromise&& Promise, FWwiseLoadedSoundBankPtr&& InSoundBankListNode);
	virtual void UnloadSoundBankAsync(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedSoundBankPtr&& InSoundBankListNode);

	// Checks whether no element remains pending in the Resource Loader at this point. Used for unit testing. 
	bool IsEmpty() const
	{
		return LoadedAuxBusList.Num() == 0
			&& LoadedEventList.Num() == 0
			&& LoadedExternalSourceList.Num() == 0
			&& LoadedGroupValueList.Num() == 0
			&& LoadedInitBankList.Num() == 0
			&& LoadedMediaList.Num() == 0
			&& LoadedShareSetList.Num() == 0
			&& LoadedSoundBankList.Num() == 0
			&& LoadedGroupValueInfo.Num() == 0;
	}

protected:
	using FLoadFileCallback = TUniqueFunction<void(bool bInResult)>;
	using FUnloadFileCallback = TUniqueFunction<void()>;
	using FCompletionCallback = TUniqueFunction<void()>;
	using FCompletionPromise = TWwisePromise<void>;
	using FCompletionFuture = TWwiseFuture<void>;
	using FCompletionFutureArray = TArray<FCompletionFuture>;

	FCriticalSection ListUpdateCriticalSection;
	
	/**
	 * @brief List of all the loaded Auxiliary Bus Wwise Objects.
	 *
	 * This list is maintained through the LoadAuxBusAsync and UnloadAuxBusAsync operations.
	 *
	 * @note To modify this list, you must call the operation asynchronously through ListExecutionQueue.
	*/
	FWwiseLoadedAuxBusList LoadedAuxBusList;

	/**
	 * @brief List of all the loaded Event Wwise Objects.
	 *
	 * This list is maintained through the LoadEventAsync and UnloadEventAsync operations.
	 *
	 * @note To modify this list, you must call the operation asynchronously through ListExecutionQueue.
	*/
	FWwiseLoadedEventList LoadedEventList;

	/**
	 * @brief List of all the loaded External Source Wwise Objects.
	 *
	 * This list is maintained through the LoadExternalSourceAsync and UnloadExternalSourceAsync operations.
	 *
	 * External Sources are component parts of many other Wwise objects. Events, for example, can contain
	 * their own External Sources. This list only contains the External Sources that were added independently,
	 * not those that are already included in objects such as Events.
	 *
	 * @note To modify this list, you must call the operation asynchronously through ListExecutionQueue.
	*/
	FWwiseLoadedExternalSourceList LoadedExternalSourceList;

	/**
	 * @brief List of all the loaded GroupValue (States, Switches) Wwise Objects.
	 *
	 * This list is maintained through the LoadGroupValueAsync and UnloadGroupValueAsync operations.
	 *
	 * GroupValues are component parts of many other Wwise objects. Events, for example, can contain
	 * their own GroupValues. This list only contains the GroupValues that were added independently,
	 * not those that are already included in objects such as Events.
	 *
	 * @note To modify this list, you must call the operation asynchronously through ListExecutionQueue.
	*/
	FWwiseLoadedGroupValueList LoadedGroupValueList;

	/**
	 * @brief List of all the loaded Init Bank Wwise Objects.
	 *
	 * This list is maintained through the LoadInitBankAsync and UnloadInitBankAsync operations.
	 *
	 * @note In order to modify this list (add or remove), you must call the operation asynchronously through the
	 *       ListExecutionQueue.
	 *
	 * @warning Although this is a list, it can only include a single Init Bank. Each project supports one Init Bank,
	 *          which is used for the duration of the Sound Engine's lifespan.
	*/
	FWwiseLoadedInitBankList LoadedInitBankList;

	/**
	 * @brief List of all the loaded Media Wwise Objects.
	 *
	 * This list is maintained through the LoadMediaAsync and UnloadMediaAsync operations.
	 *
	 * Media objects are component parts of many other Wwise objects. Events, for example, can contain their
	 * own Media objects. This list only contains the Media objects that were added independently, not those
	 * that are already included in objects such as Events.
	 *
	 * @note To modify this list, you must call the operation asynchronously through ListExecutionQueue.
	*/
	FWwiseLoadedMediaList LoadedMediaList;

	/**
	 * @brief List of all the loaded Share Set Wwise Objects.
	 *
	 * This list is maintained through the LoadShareSetAsync and UnloadShareSetAsync operations.
	 *
	 * @note To modify this list, you must call the operation asynchronously through ListExecutionQueue.
	*/
	FWwiseLoadedShareSetList LoadedShareSetList;

	/**
	 * @brief List of all the loaded SoundBank Wwise Objects.
	 *
	 * This list is maintained through the LoadSoundBankAsync and UnloadSoundBankAsync operations.
	 *
	 * SoundBanks are building blocks of multiple other Wwise objects. An Event is included (and thus require) inside
	 * a SoundBank. This list only contains the independently added SoundBanks, as the different objects, such as
	 * Events, are responsible for keeping track of their own required sub-objects.
	 *
	 * @note To modify this list, you must call the operation asynchronously through ListExecutionQueue.
	*/
	FWwiseLoadedSoundBankList LoadedSoundBankList;


	/**
	 * @brief Set of all known GroupValues, each of which contains the list of the Switch Containers that require them.
	 *
	 * @note To modify this list, you must call the operation asynchronously through FileExecutionQueue.
	*/
	TSet<FWwiseSwitchContainerLoadedGroupValueInfo> LoadedGroupValueInfo;

	mutable FWwiseExecutionQueue ExecutionQueue;

	mutable IWwiseExternalSourceManager* ExternalSourceManager{nullptr};
	mutable IWwiseMediaManager* MediaManager{nullptr};
	mutable IWwiseSoundBankManager* SoundBankManager{nullptr};

	virtual void LoadAuxBusResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData);
	virtual void LoadEventResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual void LoadEventSwitchContainerResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual void LoadExternalSourceResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData);
	virtual void LoadGroupValueResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData);
	virtual void LoadInitBankResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData);
	virtual void LoadMediaResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData);
	virtual void LoadShareSetResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData);
	virtual void LoadSoundBankResources(FWwiseResourceLoadPromise&& Promise, FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData);
	virtual void LoadSwitchContainerLeafResources(FCompletionPromise&& Promise, TSharedRef<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe> UsageCount);

	virtual void UnloadAuxBusResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedAuxBusInfo::FLoadedData& LoadedData, const FWwiseAuxBusCookedData& InCookedData);
	virtual void UnloadEventResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual void UnloadEventSwitchContainerResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedEventInfo::FLoadedData& LoadedData, const FWwiseEventCookedData& InCookedData);
	virtual void UnloadExternalSourceResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedExternalSourceInfo::FLoadedData& LoadedData, const FWwiseExternalSourceCookedData& InCookedData);
	virtual void UnloadGroupValueResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedGroupValueInfo::FLoadedData& LoadedData, const FWwiseGroupValueCookedData& InCookedData);
	virtual void UnloadInitBankResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedInitBankInfo::FLoadedData& LoadedData, const FWwiseInitBankCookedData& InCookedData);
	virtual void UnloadMediaResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedMediaInfo::FLoadedData& LoadedData, const FWwiseMediaCookedData& InCookedData);
	virtual void UnloadShareSetResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedShareSetInfo::FLoadedData& LoadedData, const FWwiseShareSetCookedData& InCookedData);
	virtual void UnloadSoundBankResources(FWwiseResourceUnloadPromise&& Promise, FWwiseLoadedSoundBankInfo::FLoadedData& LoadedData, const FWwiseSoundBankCookedData& InCookedData);
	virtual void UnloadSwitchContainerLeafResources(FWwiseResourceUnloadPromise&& Promise, TSharedRef<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe> UsageCount);
	virtual void DeleteSwitchContainerLeafGroupValueUsageCount(FWwiseResourceUnloadPromise&& Promise, TSharedRef<FWwiseSwitchContainerLeafGroupValueUsageCount, ESPMode::ThreadSafe>& UsageCount);

	virtual void AttachAuxBusNode(FWwiseLoadedAuxBusListNode* AuxBusListNode);
	virtual void AttachEventNode(FWwiseLoadedEventListNode* EventListNode);
	virtual void AttachExternalSourceNode(FWwiseLoadedExternalSourceListNode* ExternalSourceListNode);
	virtual void AttachGroupValueNode(FWwiseLoadedGroupValueListNode* GroupValueListNode);
	virtual void AttachInitBankNode(FWwiseLoadedInitBankListNode* InitBankListNode);
	virtual void AttachMediaNode(FWwiseLoadedMediaListNode* MediaListNode);
	virtual void AttachShareSetNode(FWwiseLoadedShareSetListNode* ShareSetListNode);
	virtual void AttachSoundBankNode(FWwiseLoadedSoundBankListNode* SoundBankListNode);

	virtual void DetachAuxBusNode(FWwiseLoadedAuxBusListNode* AuxBusListNode);
	virtual void DetachEventNode(FWwiseLoadedEventListNode* EventListNode);
	virtual void DetachExternalSourceNode(FWwiseLoadedExternalSourceListNode* ExternalSourceListNode);
	virtual void DetachGroupValueNode(FWwiseLoadedGroupValueListNode* GroupValueListNode);
	virtual void DetachInitBankNode(FWwiseLoadedInitBankListNode* InitBankListNode);
	virtual void DetachMediaNode(FWwiseLoadedMediaListNode* MediaListNode);
	virtual void DetachShareSetNode(FWwiseLoadedShareSetListNode* ShareSetListNode);
	virtual void DetachSoundBankNode(FWwiseLoadedSoundBankListNode* SoundBankListNode);

	void AddLoadExternalSourceFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources,
	                                  const TArray<FWwiseExternalSourceCookedData>& InExternalSources, const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const;
	void AddUnloadExternalSourceFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseExternalSourceCookedData*>& LoadedExternalSources,
										const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const;
	void AddLoadMediaFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseMediaCookedData*>& LoadedMedia,
	                         const TArray<FWwiseMediaCookedData>& InMedia, const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const;
	void AddUnloadMediaFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseMediaCookedData*>& LoadedMedia,
							   const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const;
	void AddLoadSoundBankFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks,
	                             const TArray<FWwiseSoundBankCookedData>& InSoundBank, const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const;
	void AddUnloadSoundBankFutures(FCompletionFutureArray& FutureArray, TArray<const FWwiseSoundBankCookedData*>& LoadedSoundBanks,
	                               const TCHAR* InType, const FString& InDebugName, uint32 InShortId) const;
	void WaitForFutures(FCompletionFutureArray&& FutureArray, FCompletionCallback&& Callback, int NextId = 0) const;

	void LoadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FLoadFileCallback&& InCallback) const;
	void UnloadSoundBankFile(const FWwiseSoundBankCookedData& InSoundBank, FUnloadFileCallback&& InCallback) const;
	void LoadMediaFile(const FWwiseMediaCookedData& InMedia, FLoadFileCallback&& InCallback) const;
	void UnloadMediaFile(const FWwiseMediaCookedData& InMedia, FUnloadFileCallback&& InCallback) const;
	void LoadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FLoadFileCallback&& InCallback) const;
	void UnloadExternalSourceFile(const FWwiseExternalSourceCookedData& InExternalSource, FUnloadFileCallback&& InCallback) const;

	template<typename MapValue>
	inline const FWwiseLanguageCookedData* GetLanguageMapKey(const TMap<FWwiseLanguageCookedData, MapValue>& Map, const FWwiseLanguageCookedData* InLanguageOverride, const FName& InDebugName) const;

	template<typename T>
	static inline void LogLoad(const T& Object);
	template<typename T>
	static inline void LogUnload(const T& Object);
	template<typename T>
	static inline void LogLoadResources(const T& Object);
	template<typename T>
	static inline void LogUnloadResources(const T& Object);
	template<typename T>
	static inline void LogLoadResources(const T& Object, void* Ptr);
	template<typename T>
	static inline void LogUnloadResources(const T& Object, void* Ptr);

public:
	struct WWISERESOURCELOADER_API Test
	{
#if defined(WITH_LOW_LEVEL_TESTS) && WITH_LOW_LEVEL_TESTS || defined(WITH_AUTOMATION_TESTS) || (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)
#define WWISE_RESOURCELOADERIMPL_TEST_CONST
#else
#define WWISE_RESOURCELOADERIMPL_TEST_CONST const
#endif
		static WWISE_RESOURCELOADERIMPL_TEST_CONST bool bMockSleepOnMediaLoad;
	};

};

template <typename T>
void FWwiseResourceLoaderImpl::LogLoad(const T& Object)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Loading %s"), *Object.GetDebugString());
}

template <typename T>
void FWwiseResourceLoaderImpl::LogUnload(const T& Object)
{
	UE_LOG(LogWwiseResourceLoader, Verbose, TEXT("Unloading %s"), *Object.GetDebugString());
}

template <typename T>
void FWwiseResourceLoaderImpl::LogLoadResources(const T& Object)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading resources for %s"), *Object.GetDebugString());
}

template <typename T>
void FWwiseResourceLoaderImpl::LogUnloadResources(const T& Object)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading resources for %s"), *Object.GetDebugString());
}

template <typename T>
void FWwiseResourceLoaderImpl::LogLoadResources(const T& Object, void* Ptr)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Loading resources for %s @ %p"), *Object.GetDebugString(), Ptr);
}

template <typename T>
void FWwiseResourceLoaderImpl::LogUnloadResources(const T& Object, void* Ptr)
{
	UE_LOG(LogWwiseResourceLoader, VeryVerbose, TEXT("Unloading resources for %s @ %p"), *Object.GetDebugString(), Ptr);
}

template<typename MapValue>
inline const FWwiseLanguageCookedData* FWwiseResourceLoaderImpl::GetLanguageMapKey(const TMap<FWwiseLanguageCookedData, MapValue>& Map, const FWwiseLanguageCookedData* InLanguageOverride, const FName& InDebugName) const
{
	if (InLanguageOverride)
	{
		if (Map.Find(*InLanguageOverride))
		{
			return InLanguageOverride;
		}
		UE_LOG(LogWwiseResourceLoader, Log, TEXT("GetLanguageMapKey: Could not find overridden language %s while processing asset %s. Defaulting to language %s"),
			*InLanguageOverride->GetLanguageName().ToString(), *InDebugName.ToString(), *CurrentLanguage.GetLanguageName().ToString());
	}

	if (LIKELY(Map.Contains(FWwiseLanguageCookedData::Sfx)))
	{
		return &FWwiseLanguageCookedData::Sfx;
	}

	if (Map.Find(CurrentLanguage))
	{
		return &CurrentLanguage;
	}

	UE_LOG(LogWwiseResourceLoader, Warning, TEXT("GetLanguageMapKey: Could not find language %s while processing asset %s."),
		*CurrentLanguage.GetLanguageName().ToString(), *InDebugName.ToString());
	return nullptr;
}

