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

class WWISEFILEHANDLER_API FWwiseWriteFileState : public FWwiseFileState, protected FWwiseStreamableFileStateInfo
{
public:
	FWwiseWriteFileState(IFileHandle* InFileHandle, const FString& InFilePathName);
	virtual ~FWwiseWriteFileState() override;

	void CloseStreaming() override;

	bool CanProcessFileOp() const override;
	AKRESULT ProcessWrite(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseAkFileOperationDone&& InFileOpDoneCallback) override;
	using FWwiseStreamableFileStateInfo::GetFileDesc;

protected:
	IFileHandle* FileHandle;
	FString FilePathName;

	const TCHAR* GetManagingTypeName() const override { return TEXT("Write"); }
	uint32 GetShortId() const override { return 0; }

	void OpenFile(FOpenFileCallback&& InCallback) override { OpenFileSucceeded(MoveTemp(InCallback)); }
	void LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback) override { LoadInSoundEngineSucceeded(MoveTemp(InCallback)); }
	void UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback) override { UnloadFromSoundEngineDone(MoveTemp(InCallback)); }
	void CloseFile(FCloseFileCallback&& InCallback) override { CloseFileDone(MoveTemp(InCallback)); }
};