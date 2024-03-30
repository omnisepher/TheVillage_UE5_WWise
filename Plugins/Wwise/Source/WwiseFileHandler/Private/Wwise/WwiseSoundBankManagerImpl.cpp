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

#include "Wwise/WwiseSoundBankManagerImpl.h"
#include "Wwise/WwiseSoundBankFileState.h"
#include "Wwise/Stats/FileHandler.h"

FWwiseSoundBankManagerImpl::FWwiseSoundBankManagerImpl() :
	StreamingGranularity(0)
{
}

FWwiseSoundBankManagerImpl::~FWwiseSoundBankManagerImpl()
{
}

void FWwiseSoundBankManagerImpl::LoadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FLoadSoundBankCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseSoundBankManagerImpl::LoadSoundBank"));
	IncrementFileStateUseAsync(InSoundBankCookedData.SoundBankId, EWwiseFileStateOperationOrigin::Loading, [this, InSoundBankCookedData, InRootPath]() mutable
	{
		return CreateOp(InSoundBankCookedData, InRootPath);
	}, [InCallback = MoveTemp(InCallback)](const FWwiseFileStateSharedPtr, bool bInResult)
	{
		InCallback(bInResult);
	});
}

void FWwiseSoundBankManagerImpl::UnloadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FUnloadSoundBankCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseSoundBankManagerImpl::UnloadSoundBank"));
	DecrementFileStateUseAsync(InSoundBankCookedData.SoundBankId, nullptr, EWwiseFileStateOperationOrigin::Loading, MoveTemp(InCallback));
}

void FWwiseSoundBankManagerImpl::SetGranularity(AkUInt32 InStreamingGranularity)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseSoundBankManagerImpl::SetGranularity"));
	StreamingGranularity = InStreamingGranularity;
}

FWwiseFileStateSharedPtr FWwiseSoundBankManagerImpl::CreateOp(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath)
{
	return FWwiseFileStateSharedPtr(new FWwiseInMemorySoundBankFileState(InSoundBankCookedData, InRootPath));
}
