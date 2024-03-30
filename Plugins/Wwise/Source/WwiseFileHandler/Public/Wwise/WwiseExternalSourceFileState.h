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

#include "Wwise/WwiseFileState.h"
#include "Wwise/WwiseStreamableFileStateInfo.h"

class FWwiseFileCacheHandle;

class WWISEFILEHANDLER_API FWwiseExternalSourceFileState : public FWwiseFileState, public AkExternalSourceInfo
{
public:
	const uint32 MemoryAlignment;
	const bool bDeviceMemory;

	const uint32 MediaId;
	const FName MediaPathName;
	const FName RootPath;

	TAtomic<int> PlayCount;

protected:
	FWwiseExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory, 
		uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId);
	~FWwiseExternalSourceFileState() override;

public:
	bool CanDelete() const override { return FWwiseFileState::CanDelete() && PlayCount.Load() == 0; }

	virtual bool GetExternalSourceInfo(AkExternalSourceInfo& OutInfo);
	void IncrementPlayCount();
	bool DecrementPlayCount();

protected:
	bool CanUnloadFromSoundEngine() const override { return FWwiseFileState::CanUnloadFromSoundEngine() && PlayCount == 0; }

	const TCHAR* GetManagingTypeName() const override final { return TEXT("External Source"); }
	uint32 GetShortId() const override final { return MediaId; }
};

class WWISEFILEHANDLER_API FWwiseInMemoryExternalSourceFileState : public FWwiseExternalSourceFileState
{
public:
	const uint8* Ptr;
	IMappedFileHandle* MappedHandle;
	IMappedFileRegion* MappedRegion;

	FWwiseInMemoryExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory, 
		uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId);
	~FWwiseInMemoryExternalSourceFileState() override { Term(); }

	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	void CloseFile(FCloseFileCallback&& InCallback) override;
};

class WWISEFILEHANDLER_API FWwiseStreamedExternalSourceFileState : public FWwiseExternalSourceFileState, public FWwiseStreamableFileStateInfo, public AkSourceSettings
{
public:
	const uint32 PrefetchSize;
	const uint32 StreamingGranularity;

	FWwiseFileCacheHandle* StreamedFile;

	FWwiseStreamedExternalSourceFileState(uint32 InMemoryAlignment, bool bInDeviceMemory,
		uint32 InPrefetchSize, uint32 InStreamingGranularity,
		uint32 InMediaId, const FName& InMediaPathName, const FName& InRootPath, int32 InCodecId);
	~FWwiseStreamedExternalSourceFileState() override { Term(); }

	void CloseStreaming() override;
	FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() override { return this; }
	const FWwiseStreamableFileStateInfo* GetStreamableFileStateInfo() const override { return this; }

	void OpenFile(FOpenFileCallback&& InCallback) override;
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override;
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override;
	void CloseFile(FCloseFileCallback&& InCallback) override;

	bool CanProcessFileOp() const override;
	AKRESULT ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseAkFileOperationDone&& InFileOpDoneCallback) override;
};
