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

#include "Wwise/WwiseFileStateTools.h"
#include "Wwise/Stats/FileHandler.h"

#include "AkInclude.h"
#include "WwiseDefines.h"
#include "Wwise/WwiseFileCache.h"

class WWISEFILEHANDLER_API FWwiseStreamableFileStateInfo: protected AkFileDesc
{
public:
	static FWwiseStreamableFileStateInfo* GetFromFileDesc(AkFileDesc& InFileDesc)
	{
#if WWISE_2023_1_OR_LATER
		return static_cast<FWwiseStreamableFileStateInfo*>(&InFileDesc);
#else
		return static_cast<FWwiseStreamableFileStateInfo*>(static_cast<AkFileDesc*>(InFileDesc.pCustomParam));
#endif
	}

	AkFileDesc* GetFileDesc()
	{
		return this;
	}

	virtual ~FWwiseStreamableFileStateInfo() {}
	virtual bool CanProcessFileOp() const
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot process read on a non-streaming asset"));
		return false;
	}
	virtual AKRESULT ProcessRead(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseAkFileOperationDone&& InFileOpDoneCallback)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot process read on a non-streaming asset"));
		return AK_Fail;
	}
	virtual AKRESULT ProcessWrite(AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, AkAsyncIOTransferInfo& OutTransferInfo, FWwiseAkFileOperationDone&& InFileOpDoneCallback)
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("Cannot process write on a non-writable asset"));
		return AK_Fail;
	}
	virtual void CloseStreaming() {}	
	
protected:
	FWwiseStreamableFileStateInfo()
	{
		FMemory::Memset(*static_cast<AkFileDesc*>(this), 0);
	}
};
