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

#include "AkInclude.h"
#include "WwiseMockFileState.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/Stats/FileHandler.h"

class FWwiseMockExternalSourceManager : public IWwiseExternalSourceManager, public FWwiseFileHandlerBase
{
public:
	FWwiseMockExternalSourceManager() {}
	~FWwiseMockExternalSourceManager() override {}

	const TCHAR* GetManagingTypeName() const override { return TEXT("MockExternalSource"); }

	virtual void LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
	                                const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback) override
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMockExternalSourceManager::LoadExternalSource"))

		// NOTE We are assuming a correspondence between Cookie and MediaId. This need not be the case, see the WwiseSimpleExtSrcManager for an example implementation of CookieMediaId maps
		IncrementFileStateUseAsync(InExternalSourceCookedData.Cookie, EWwiseFileStateOperationOrigin::Loading, [this, InExternalSourceCookedData, InRootPath]() mutable 
		{
			return CreateOp(InExternalSourceCookedData, InRootPath);
		}, [InCallback = MoveTemp(InCallback)](const FWwiseFileStateSharedPtr, bool bInResult)
		{
			InCallback(bInResult);
		});
	}
	
	virtual void UnloadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback) override
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMockExternalSourceManager::UnLoadExternalSource"))
		DecrementFileStateUseAsync(InExternalSourceCookedData.Cookie, nullptr, EWwiseFileStateOperationOrigin::Loading, MoveTemp(InCallback));
	}
	
	virtual void SetGranularity(AkUInt32 InStreamingGranularity) override {}

	IWwiseStreamingManagerHooks& GetStreamingHooks() override final { return *this; }

	virtual TArray<uint32> PrepareExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo,
		const TArray<FWwiseExternalSourceCookedData>&& InCookedData) override { return {}; }
	virtual void BindPlayingIdToExternalSources(const uint32 InPlayingId, const TArray<uint32>& InMediaIds) override {}
	virtual void OnEndOfEvent(const uint32 InPlayingID) override {}
	virtual void SetExternalSourceMediaById(const FName& ExternalSourceName, const int32 MediaId) override {}
	virtual void SetExternalSourceMediaByName(const FName& ExternalSourceName, const FName& MediaName) override {}
	virtual void SetExternalSourceMediaWithIds(const int32 ExternalSourceCookie, const int32 MediaId) override {}


#if WITH_EDITORONLY_DATA
	virtual void Cook(FWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
		const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage) override {}
#endif

	bool IsEmpty()
	{
		FRWScopeLock Lock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		return FileStatesById.Num() == 0;
	}

protected:
	virtual void OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback) override
	{
		FWwiseFileHandlerBase::OnDeleteState(InShortId, InFileState, InOperationOrigin, MoveTemp(InCallback));
	}
	
	FWwiseFileStateSharedPtr CreateOp(FWwiseExternalSourceCookedData InExternalSourceCookedData, FName Name)
	{
		auto* FileState = new FWwiseMockFileState(InExternalSourceCookedData.Cookie);
		return FWwiseFileStateSharedPtr(FileState);
	}
};
