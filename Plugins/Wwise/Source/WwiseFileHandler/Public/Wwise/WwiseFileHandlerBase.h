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

#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/WwiseExecutionQueue.h"
#include "Wwise/WwiseFileState.h"

class WWISEFILEHANDLER_API FWwiseFileHandlerBase : protected IWwiseStreamingManagerHooks
{
protected:
	FWwiseFileHandlerBase();
	~FWwiseFileHandlerBase() override;

	void OpenStreaming(AkAsyncFileOpenData* io_pOpenData) override;
	void CloseStreaming(uint32 InShortId, FWwiseFileState& InFileState) override;
	AKRESULT GetOpenStreamingResult(AkAsyncFileOpenData* io_pOpenData);

	using FCreateStateFunction = TUniqueFunction<FWwiseFileStateSharedPtr()>;
	using FIncrementStateCallback = TUniqueFunction<void(const FWwiseFileStateSharedPtr&, bool bResult)>;
	using FDecrementStateCallback = TUniqueFunction<void()>;
	void IncrementFileStateUseAsync(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin, FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback);
	void DecrementFileStateUseAsync(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback);

	virtual void IncrementFileStateUse(uint32 InShortId, EWwiseFileStateOperationOrigin InOperationOrigin, FCreateStateFunction&& InCreate, FIncrementStateCallback&& InCallback);
	virtual void DecrementFileStateUse(uint32 InShortId, FWwiseFileState* InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback);
	virtual void OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState, EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback);
	void OnIncrementCountAsyncDone(bool bInResult, EWwiseFileStateOperationOrigin InOperationOrigin, FWwiseFileStateSharedPtr State, FIncrementStateCallback&& InCallback);

	virtual const TCHAR* GetManagingTypeName() const { return TEXT("UNKNOWN"); }

	FWwiseExecutionQueue FileHandlerExecutionQueue;

	FRWLock FileStatesByIdLock;
	TMap<uint32, FWwiseFileStateSharedPtr> FileStatesById;
};
