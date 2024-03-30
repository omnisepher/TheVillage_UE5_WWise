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
#include "Wwise/WwiseFileHandlerBase.h"
#include "Wwise/WwiseSoundBankManager.h"
#include "Wwise/CookedData/WwiseSoundBankCookedData.h"
#include "Wwise/Stats/FileHandler.h"

class FWwiseMockSoundBankManager : public IWwiseSoundBankManager, public FWwiseFileHandlerBase
{
public:
	FWwiseMockSoundBankManager() {}
	~FWwiseMockSoundBankManager() override {}

	const TCHAR* GetManagingTypeName() const override { return TEXT("MockSoundBank"); }

	void LoadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FLoadSoundBankCallback&& InCallback)
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMockSoundBankManager::LoadSoundBank"));
		IncrementFileStateUseAsync(InSoundBankCookedData.SoundBankId, EWwiseFileStateOperationOrigin::Loading, [this, InSoundBankCookedData, InRootPath]() mutable
		{
			return CreateOp(InSoundBankCookedData, InRootPath);
		}, [InCallback = MoveTemp(InCallback)](const FWwiseFileStateSharedPtr, bool bInResult)
		{
			InCallback(bInResult);
		});
	}

	void UnloadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FUnloadSoundBankCallback&& InCallback)
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMockSoundBankManager::UnloadSoundBank"));
		DecrementFileStateUseAsync(InSoundBankCookedData.SoundBankId, nullptr, EWwiseFileStateOperationOrigin::Loading, MoveTemp(InCallback));
	}
	void SetGranularity(AkUInt32 InStreamingGranularity) override {}

	IWwiseStreamingManagerHooks& GetStreamingHooks() override final { return *this; }


	bool IsEmpty()
	{
		FRWScopeLock Lock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
		return FileStatesById.Num() == 0;
	}

protected:
	virtual FWwiseFileStateSharedPtr CreateOp(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath)
	{
		auto* FileState = new FWwiseMockFileState(InSoundBankCookedData.SoundBankId);
		FileState->bIsStreamedState = FWwiseMockFileState::OptionalBool::False;
		return FWwiseFileStateSharedPtr(FileState);
	}
};
