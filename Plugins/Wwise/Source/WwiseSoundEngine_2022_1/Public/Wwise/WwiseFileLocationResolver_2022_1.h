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

#include "Containers/Map.h"
#include "HAL/CriticalSection.h"
#include "HAL/Event.h"

class WWISESOUNDENGINE_API FWwiseFileLocationResolver : public AK::StreamMgr::IAkFileLocationResolver
{
private:
	struct FileOpenDataBridge : public AkAsyncFileOpenData
	{
		AKRESULT Result;
		FEventRef Done;

		static void Callback(AkAsyncFileOpenData* in_pOpenInfo, AKRESULT in_eResult);
	};

public:
	virtual AKRESULT Open(
		AkAsyncFileOpenData* io_pOpenData
	) = 0;

private:
	FCriticalSection MapLock;
	TMultiMap<AkFileID, FileOpenDataBridge*> OpenFileIDMap;
	virtual AKRESULT Open(
		AkFileID				in_fileID,
		AkOpenMode				in_eOpenMode,
		AkFileSystemFlags* in_pFlags,
		bool& io_bSyncOpen,
		AkFileDesc& io_fileDesc
	) override final;

	TMultiMap<FString, FileOpenDataBridge*> OpenFileNameMap;
	virtual AKRESULT Open(
		const AkOSChar* in_pszFileName,
		AkOpenMode				in_eOpenMode,
		AkFileSystemFlags* in_pFlags,
		bool& io_bSyncOpen,
		AkFileDesc& io_fileDesc
	) override final;
};
