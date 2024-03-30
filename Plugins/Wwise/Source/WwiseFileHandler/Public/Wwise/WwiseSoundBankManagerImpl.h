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

#include "Wwise/WwiseSoundBankManager.h"
#include "Wwise/CookedData/WwiseSoundBankCookedData.h"
#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseFileHandlerBase.h"

class WWISEFILEHANDLER_API FWwiseSoundBankManagerImpl : public IWwiseSoundBankManager, public FWwiseFileHandlerBase
{
public:
	FWwiseSoundBankManagerImpl();
	~FWwiseSoundBankManagerImpl() override;

	const TCHAR* GetManagingTypeName() const override { return TEXT("SoundBank"); }
	void LoadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FLoadSoundBankCallback&& InCallback) override;
	void UnloadSoundBank(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath, FUnloadSoundBankCallback&& InCallback) override;
	void SetGranularity(AkUInt32 InStreamingGranularity) override;

	IWwiseStreamingManagerHooks& GetStreamingHooks() final { return *this; }

protected:
	uint32 StreamingGranularity;

	virtual FWwiseFileStateSharedPtr CreateOp(const FWwiseSoundBankCookedData& InSoundBankCookedData, const FString& InRootPath);
};
