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
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/CookedData/WwiseExternalSourceCookedData.h"
#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseFileHandlerBase.h"

class FWwiseExternalSourceFileState;

struct WWISEFILEHANDLER_API FWwiseExternalSourceState : public FWwiseExternalSourceCookedData
{
	FWwiseExternalSourceState(const FWwiseExternalSourceCookedData& InCookedData);
	~FWwiseExternalSourceState();

	TAtomic<int> LoadCount;
	void IncrementLoadCount();
	bool DecrementLoadCount();
};
using FWwiseExternalSourceStateSharedPtr = TSharedPtr<FWwiseExternalSourceState>;

class WWISEFILEHANDLER_API FWwiseExternalSourceManagerImpl : public IWwiseExternalSourceManager, public FWwiseFileHandlerBase
{
public:
	FWwiseExternalSourceManagerImpl();
	~FWwiseExternalSourceManagerImpl();

	virtual const TCHAR* GetManagingTypeName() const override { return TEXT("External Source"); }
	virtual void LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback) override;
	virtual void UnloadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback) override;
	virtual void SetGranularity(AkUInt32 InStreamingGranularity) override;

	IWwiseStreamingManagerHooks& GetStreamingHooks() override final { return *this; }

	virtual TArray<uint32> PrepareExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo,
		const TArray<FWwiseExternalSourceCookedData>&& InCookedData) override;
	virtual void BindPlayingIdToExternalSources(const uint32 InPlayingId, const TArray<uint32>& InMediaIds) override;
	virtual void OnEndOfEvent(const uint32 InPlayingID) override;
	virtual void SetExternalSourceMediaById(const FName& ExternalSourceName, const int32 MediaId) override;
	virtual void SetExternalSourceMediaByName(const FName& ExternalSourceName, const FName& MediaName) override;
	virtual void SetExternalSourceMediaWithIds(const int32 ExternalSourceCookie, const int32 MediaId) override;


#if WITH_EDITORONLY_DATA
	virtual void Cook(FWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
		const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage) override;
#endif

protected:
	/**
	 * @brief Lock on the Cookie to Media Table. Lock as "ReadOnly" for using the tables (Prepare), and as "Write" for modifying the tables.
	*/
	FRWLock CookieToMediaLock;
	TMap<uint32, FWwiseExternalSourceFileState*> CookieToMedia;

	FCriticalSection PlayingIdToMediaIdsLock;
	TMultiMap<uint32, uint32> PlayingIdToMediaIds;

	uint32 StreamingGranularity;
	TMap<uint32, FWwiseExternalSourceStateSharedPtr> ExternalSourceStatesById;

	virtual void LoadExternalSourceImpl(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback);
	virtual void UnloadExternalSourceImpl(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback);
	virtual FWwiseExternalSourceStateSharedPtr CreateExternalSourceState(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath);
	virtual bool CloseExternalSourceState(FWwiseExternalSourceState& InExternalSourceState);

	virtual void LoadExternalSourceMedia(const uint32 InExternalSourceCookie, const FName& InExternalSourceName, const FName& InRootPath, FLoadExternalSourceCallback&& InCallback);
	virtual void UnloadExternalSourceMedia(const uint32 InExternalSourceCookie, const FName& InExternalSourceName, const FName& InRootPath, FUnloadExternalSourceCallback&& InCallback);

	virtual uint32 PrepareExternalSourceInfo(AkExternalSourceInfo& OutInfo, const FWwiseExternalSourceCookedData& InCookedData);
	virtual void OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback) override;
};
