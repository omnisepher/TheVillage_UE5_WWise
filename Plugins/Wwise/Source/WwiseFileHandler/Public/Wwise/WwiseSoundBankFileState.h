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

class WWISEFILEHANDLER_API FWwiseSoundBankFileState : public FWwiseFileState, public FWwiseSoundBankCookedData
{
public:
	const FString RootPath;

protected:
	FWwiseSoundBankFileState(const FWwiseSoundBankCookedData& InCookedData, const FString& InRootPath);

public:
	~FWwiseSoundBankFileState() override;

	const TCHAR* GetManagingTypeName() const override final { return TEXT("SoundBank"); }
	uint32 GetShortId() const override final { return SoundBankId; }
};

class WWISEFILEHANDLER_API FWwiseInMemorySoundBankFileState : public FWwiseSoundBankFileState
{
public:
	const uint8* Ptr;
	int64 FileSize;
	IMappedFileHandle* MappedHandle;
	IMappedFileRegion* MappedRegion;

	FWwiseInMemorySoundBankFileState(const FWwiseSoundBankCookedData& InCookedData, const FString& InRootPath);
	~FWwiseInMemorySoundBankFileState() override { Term(); }

	bool LoadAsMemoryMapped() const;
	bool LoadAsMemoryView() const;

	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	bool CanCloseFile() const override;
	void CloseFile(FCloseFileCallback&& InCallback) override;

private:
	void FreeMemoryIfNeeded();

	struct BankLoadCookie
	{
		FWwiseInMemorySoundBankFileState* BankFileState;
		FLoadInSoundEngineCallback Callback;

		BankLoadCookie(FWwiseInMemorySoundBankFileState* InBankFileState)
			: BankFileState(InBankFileState)
		{}

		BankLoadCookie(BankLoadCookie* InOther);
	};

	struct BankUnloadCookie
	{
		FWwiseInMemorySoundBankFileState* BankFileState;
		FUnloadFromSoundEngineCallback Callback;

		BankUnloadCookie(FWwiseInMemorySoundBankFileState* InBankFileState)
			: BankFileState(InBankFileState)
		{}

		BankUnloadCookie(BankUnloadCookie* InOther)
		{
			if (InOther)
			{
				BankFileState = InOther->BankFileState;
				Callback = MoveTemp(InOther->Callback);
			}
		}
	};

	static void BankLoadCallback(
		AkUInt32	InBankID,
		const void* InMemoryBankPtr,
		AKRESULT	InLoadResult,
		void*		InCookie
	);

	static void BankUnloadCallback(
		AkUInt32	InBankID,
		const void* InMemoryBankPtr,
		AKRESULT	InUnloadResult,
		void*		InCookie
	);
};
