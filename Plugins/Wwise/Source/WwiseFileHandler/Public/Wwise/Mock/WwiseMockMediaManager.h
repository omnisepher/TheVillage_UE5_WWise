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
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/CookedData/WwiseMediaCookedData.h"
#include "Wwise/Stats/FileHandler.h"

class FWwiseMockMediaManager : public IWwiseMediaManager, public FWwiseFileHandlerBase
{
public:
	FWwiseMockMediaManager() {}
	~FWwiseMockMediaManager() override {}

	const TCHAR* GetManagingTypeName() const override { return TEXT("MockMedia"); }

	void LoadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FLoadMediaCallback&& InCallback) override
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMockMediaManager::LoadMedia"));
		IncrementFileStateUseAsync(InMediaCookedData.MediaId, EWwiseFileStateOperationOrigin::Loading, [this, InMediaCookedData, InRootPath]() mutable
		{
			return CreateOp(InMediaCookedData, InRootPath);
		}, [InCallback = MoveTemp(InCallback)](const FWwiseFileStateSharedPtr, bool bInResult)
		{
			InCallback(bInResult);
		});
	}
	void UnloadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FUnloadMediaCallback&& InCallback) override
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMockMediaManager::UnloadMedia"));
		DecrementFileStateUseAsync(InMediaCookedData.MediaId, nullptr, EWwiseFileStateOperationOrigin::Loading, MoveTemp(InCallback));
	}
	void SetGranularity(AkUInt32 InStreamingGranularity) override {}

	IWwiseStreamingManagerHooks& GetStreamingHooks() override final { return *this; }

	bool IsMediaLoaded(int32 mediaId)
	{
		for (const auto& pair : FileStatesById) {
			const FWwiseFileStateSharedPtr& fileState = pair.Value;
			if (fileState->GetShortId() == mediaId) {
				return true;
			}
		}
		return false;
	}

	bool IsEmpty()
	{
		FRWScopeLock Lock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		return FileStatesById.Num() == 0;
	}

protected:
	virtual FWwiseFileStateSharedPtr CreateOp(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath)
	{
		auto* FileState = new FWwiseMockFileState(InMediaCookedData.MediaId);
		if (InMediaCookedData.bStreaming)
		{
			FileState->bIsStreamedState = FWwiseMockFileState::OptionalBool::True;
		}
		else
		{
			FileState->bIsStreamedState = FWwiseMockFileState::OptionalBool::False;
		}
		return FWwiseFileStateSharedPtr(FileState);
	}
};
