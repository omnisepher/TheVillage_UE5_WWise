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

#include "GenericPlatform/GenericPlatformFile.h"
#include "AkInclude.h"

#include "Wwise/WwiseExecutionQueue.h"

class FWwiseAsyncCycleCounter;
class FWwiseFileCacheHandle;

class IAsyncReadRequest;

using FWwiseFileOperationDone = TUniqueFunction<void(bool bResult)>;
using FWwiseAkFileOperationDone = TUniqueFunction<void(AkAsyncIOTransferInfo* TransferInfo, AKRESULT AkResult)>;

/**
 * Wwise File Cache manager.
 *
 * This is a simple Wwise version of Unreal's complex FFileCache.
 *
 * WwiseFileHandler module already opens any file only once, so we don't need a global cache.
 *
 * Compared to Unreal's FFileCache, we want to process everything asynchronously,
 * including file opening in the unlikely possibility the file is not accessible or present.
 * This allows for a fully asynchronous process.
 */
class WWISEFILEHANDLER_API FWwiseFileCache
{
public:
	static FWwiseFileCache* Get();

	FWwiseFileCache();
	virtual ~FWwiseFileCache();
	virtual void CreateFileCacheHandle(FWwiseFileCacheHandle*& OutHandle, const FString& Pathname, FWwiseFileOperationDone&& OnDone);

	FWwiseExecutionQueue OpenQueue;
	FWwiseExecutionQueue DeleteRequestQueue;
};

class WWISEFILEHANDLER_API FWwiseFileCacheHandle
{
public:
	FWwiseFileCacheHandle(const FString& Pathname);
	virtual ~FWwiseFileCacheHandle();
	virtual void CloseAndDelete();

	virtual void Open(FWwiseFileOperationDone&& OnDone);
		
	virtual void ReadData(uint8* OutBuffer, int64 Offset, int64 BytesToRead, EAsyncIOPriorityAndFlags Priority, FWwiseFileOperationDone&& OnDone);
	void ReadAkData(uint8* OutBuffer, int64 Offset, int64 BytesToRead, int8 AkPriority, FWwiseFileOperationDone&& OnDone);
	void ReadAkData(const AkIoHeuristics& Heuristics, AkAsyncIOTransferInfo& TransferInfo, FWwiseAkFileOperationDone&& Callback);

	const FString& GetPathname() const { return Pathname; }
	int64 GetFileSize() const { return FileSize; }

protected:
	FString Pathname;

	IAsyncReadFileHandle* FileHandle;
	int64 FileSize;

	FWwiseFileOperationDone InitializationDone;
	FWwiseAsyncCycleCounter* InitializationStat;

	std::atomic<int32> RequestsInFlight { 0 };

	void DeleteRequest(IAsyncReadRequest* Request);
	virtual void OnDeleteRequest(IAsyncReadRequest* Request);
	virtual void RemoveRequestInFlight();
	virtual void OnCloseAndDelete();
	virtual void OnSizeRequestDone(bool bWasCancelled, IAsyncReadRequest* Request);
	virtual void OnReadDataDone(bool bWasCancelled, IAsyncReadRequest* Request, FWwiseFileOperationDone&& OnDone);
	virtual void OnReadDataDone(bool bResult, FWwiseFileOperationDone&& OnDone);
	virtual void CallDone(bool bResult, FWwiseFileOperationDone&& OnDone);
};
