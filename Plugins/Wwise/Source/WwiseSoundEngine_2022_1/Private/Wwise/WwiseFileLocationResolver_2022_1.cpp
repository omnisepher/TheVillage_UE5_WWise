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

#include "Wwise/WwiseFileLocationResolver_2022_1.h"
#include "Wwise/Stats/SoundEngine.h"
#include "WwiseUnrealHelper.h"
#include "Wwise/Stats/NamedEvents.h"
#include "Wwise/WwiseSoundEngineUtils.h"

#include "Misc/ScopeLock.h"

#include <inttypes.h>

void FWwiseFileLocationResolver::FileOpenDataBridge::Callback(AkAsyncFileOpenData* in_rOpenInfo, AKRESULT in_eResult)
{
	if (in_rOpenInfo)
	{
		auto* Data = (FileOpenDataBridge*)in_rOpenInfo->pCookie;
		Data->Result = in_eResult;
		Data->Done->Trigger();
	}	
}

AKRESULT FWwiseFileLocationResolver::Open(AkFileID in_fileID, AkOpenMode in_eOpenMode, AkFileSystemFlags* in_pFlags,
	bool& io_bSyncOpen, AkFileDesc& io_fileDesc)
{
	if (io_bSyncOpen)
	{
		SCOPED_WWISESOUNDENGINE_EVENT_2(TEXT("FWwiseDefaultIOHook::Open SyncOpen"));
		FileOpenDataBridge* Data;
		{
			FScopeLock Lock(&MapLock);
			auto** DataPtr = OpenFileIDMap.Find(in_fileID);
			if (LIKELY(DataPtr))
			{
				Data = *DataPtr;
				auto RemoveCount = OpenFileIDMap.Remove(in_fileID, Data);
				if (UNLIKELY(RemoveCount != 1))
				{
					UE_LOG(LogWwiseSoundEngine, Error, TEXT("FWwiseDefaultIOHook::Open Removed %" PRIi32 " OpenFileNameMap Bridges for file ID %" PRIu32), in_fileID);
				}

				if (UNLIKELY(!Data))
				{
					UE_LOG(LogWwiseSoundEngine, Error, TEXT("FWwiseDefaultIOHook::Open Empty Bridge for file ID %" PRIu32), in_fileID);
					return AK_FileNotFound;
				}
			}
			else
			{
				if (in_eOpenMode != AkOpenMode::AK_OpenModeRead)
				{
					Lock.Unlock();
					UE_LOG(LogWwiseSoundEngine, VeryVerbose, TEXT("FWwiseDefaultIOHook::Open Bridge not yet created. Synchronous write file open of File ID %" PRIu32), in_fileID);
					bool bSyncOpen = false;
					auto Result = Open(in_fileID, in_eOpenMode, in_pFlags, bSyncOpen, io_fileDesc);
					if (Result != AK_Success)
					{
						return Result;
					}
					bSyncOpen = true;
					Result = Open(in_fileID, in_eOpenMode, in_pFlags, bSyncOpen, io_fileDesc);
					return Result;
				}

				// Synchronous loading. Can happen in offline rendering mode
				UE_LOG(LogWwiseSoundEngine, VeryVerbose, TEXT("FWwiseDefaultIOHook::Open Creating Synchronous Bridge for file %" PRIu32), in_fileID);
				Data = new FileOpenDataBridge;
				Data->pszFileName = nullptr;
				Data->fileID = in_fileID;
				Data->pFlags = in_pFlags;
				Data->eOpenMode = in_eOpenMode;
				Data->pCookie = Data;
				Data->pFileDesc = new AkFileDesc(io_fileDesc);
				Data->pCustomData = nullptr;
				Data->pCallback = &FileOpenDataBridge::Callback;
				Open(Data);
			}
		}

		{
			SCOPED_WWISESOUNDENGINE_EVENT_4(TEXT("FWwiseDefaultIOHook::Open SyncOpen Wait"));
			Data->Done->Wait();
		}
		FMemory::Memset(io_fileDesc, 0);
		io_fileDesc.iFileSize = Data->pFileDesc->iFileSize;
		io_fileDesc.pCustomParam = Data->pFileDesc;
		auto Result = Data->Result;
		delete Data;
		UE_LOG(LogWwiseSoundEngine, Verbose, TEXT("FWwiseDefaultIOHook::Opened file ID %" PRIu32 ": (%" PRIu32 ") %s"), in_fileID, Result, WwiseUnrealHelper::GetResultString(Result));
		return Result;
	}
	else
	{
		SCOPED_WWISESOUNDENGINE_EVENT_2(TEXT("FWwiseDefaultIOHook::Open Async Init"));
		UE_LOG(LogWwiseSoundEngine, VeryVerbose, TEXT("FWwiseDefaultIOHook::Open Creating Bridge for file %" PRIu32), in_fileID);
		auto Data = new FileOpenDataBridge;
		Data->pszFileName = nullptr;
		Data->fileID = in_fileID;
		Data->pFlags = in_pFlags;
		Data->eOpenMode = in_eOpenMode;
		Data->pCookie = Data;
		Data->pFileDesc = new AkFileDesc(io_fileDesc);
		Data->pCustomData = nullptr;
		Data->pCallback = &FileOpenDataBridge::Callback;
		{
			FScopeLock Lock(&MapLock);
			OpenFileIDMap.Add(in_fileID, Data);
		}
		return Open(Data);
	}
}

AKRESULT FWwiseFileLocationResolver::Open(const AkOSChar* in_pszFileName, AkOpenMode in_eOpenMode,
	AkFileSystemFlags* in_pFlags, bool& io_bSyncOpen, AkFileDesc& io_fileDesc)
{
	const FString Filename(in_pszFileName);

	if (io_bSyncOpen)
	{
		SCOPED_WWISESOUNDENGINE_EVENT_2(TEXT("FWwiseDefaultIOHook::Open SyncOpen"));
		FileOpenDataBridge* Data;
		{
			FScopeLock Lock(&MapLock);
			auto** DataPtr = OpenFileNameMap.Find(Filename);
			if (LIKELY(DataPtr))
			{
				Data = *DataPtr;
				auto RemoveCount = OpenFileNameMap.Remove(Filename, Data);
				if (UNLIKELY(RemoveCount != 1))
				{
					UE_LOG(LogWwiseSoundEngine, Error, TEXT("FWwiseDefaultIOHook::Open Removed %" PRIi32 " OpenFileNameMap Bridges for file %s"), *Filename);
				}
				if (UNLIKELY(!Data))
				{
					UE_LOG(LogWwiseSoundEngine, Error, TEXT("FWwiseDefaultIOHook::Open Empty Bridge for file %s"), *Filename);
					return AK_FileNotFound;
				}
			}
			else
			{
				if (in_eOpenMode != AkOpenMode::AK_OpenModeRead)
				{
					Lock.Unlock();
					UE_LOG(LogWwiseSoundEngine, VeryVerbose, TEXT("FWwiseDefaultIOHook::Open Bridge not yet created. Synchronous write file open of file %s"), *Filename);
					bool bSyncOpen = false;
					auto Result = Open(in_pszFileName, in_eOpenMode, in_pFlags, bSyncOpen, io_fileDesc);
					if (Result != AK_Success)
					{
						return Result;
					}
					bSyncOpen = true;
					Result = Open(in_pszFileName, in_eOpenMode, in_pFlags, bSyncOpen, io_fileDesc);
					return Result;
				}

				// Synchronous loading. Can happen in offline rendering mode
				UE_LOG(LogWwiseSoundEngine, VeryVerbose, TEXT("FWwiseDefaultIOHook::Open Creating Synchronous Bridge for file %s"), *Filename);
				Data = new FileOpenDataBridge;
				Data->pszFileName = in_pszFileName;
				Data->fileID = 0;
				Data->pFlags = in_pFlags;
				Data->eOpenMode = in_eOpenMode;
				Data->pCookie = Data;
				Data->pFileDesc = nullptr;
				Data->pCustomData = nullptr;
				Data->pCallback = &FileOpenDataBridge::Callback;
				Open(Data);
			}
		}

		{
			SCOPED_WWISESOUNDENGINE_EVENT_4(TEXT("FWwiseDefaultIOHook::Open SyncOpen Wait"));
			Data->Done->Wait();
		}
		FMemory::Memset(io_fileDesc, 0);
		io_fileDesc.iFileSize = Data->pFileDesc->iFileSize;
		io_fileDesc.pCustomParam = Data->pFileDesc;
		auto Result = Data->Result;
		delete Data;
		UE_LOG(LogWwiseSoundEngine, Verbose, TEXT("FWwiseDefaultIOHook::Opened file %s: (%" PRIu32 ") %s"), *Filename, Result, WwiseUnrealHelper::GetResultString(Result));
		return Result;
	}
	else
	{
		SCOPED_WWISESOUNDENGINE_EVENT_2(TEXT("FWwiseDefaultIOHook::Open Async Init"));
		UE_LOG(LogWwiseSoundEngine, VeryVerbose, TEXT("FWwiseDefaultIOHook::Open Creating Bridge for file %s"), *Filename);
		auto Data = new FileOpenDataBridge;
		Data->pszFileName = in_pszFileName;
		Data->fileID = 0;
		Data->pFlags = in_pFlags;
		Data->eOpenMode = in_eOpenMode;
		Data->pCookie = Data;
		Data->pFileDesc = nullptr;
		Data->pCustomData = nullptr;
		Data->pCallback = &FileOpenDataBridge::Callback;
		{
			FScopeLock Lock(&MapLock);
			OpenFileNameMap.Add(Filename, Data);
		}
		return Open(Data);
	}
}
