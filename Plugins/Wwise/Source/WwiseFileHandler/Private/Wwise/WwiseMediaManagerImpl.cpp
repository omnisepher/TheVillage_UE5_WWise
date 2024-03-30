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

#include "Wwise/WwiseMediaManagerImpl.h"
#include "Wwise/WwiseMediaFileState.h"
#include "Wwise/Stats/FileHandler.h"

FWwiseMediaManagerImpl::FWwiseMediaManagerImpl()
{
}

FWwiseMediaManagerImpl::~FWwiseMediaManagerImpl()
{
}

void FWwiseMediaManagerImpl::LoadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FLoadMediaCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMediaManagerImpl::LoadMedia"));
	IncrementFileStateUseAsync(InMediaCookedData.MediaId, EWwiseFileStateOperationOrigin::Loading, [this, InMediaCookedData, InRootPath]() mutable
	{
		return CreateOp(InMediaCookedData, InRootPath);
	}, [InCallback = MoveTemp(InCallback)](const FWwiseFileStateSharedPtr, bool bInResult)
	{
		InCallback(bInResult);
	});
}

void FWwiseMediaManagerImpl::UnloadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FUnloadMediaCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMediaManagerImpl::UnloadMedia"));
	DecrementFileStateUseAsync(InMediaCookedData.MediaId, nullptr, EWwiseFileStateOperationOrigin::Loading, MoveTemp(InCallback));
}

void FWwiseMediaManagerImpl::SetGranularity(AkUInt32 InStreamingGranularity)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseMediaManagerImpl::SetGranularity"));
	StreamingGranularity = InStreamingGranularity;
}

FWwiseFileStateSharedPtr FWwiseMediaManagerImpl::CreateOp(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath)
{
	if (InMediaCookedData.bStreaming)
	{
		return FWwiseFileStateSharedPtr(new FWwiseStreamedMediaFileState(InMediaCookedData, InRootPath, StreamingGranularity));
	}
	else
	{
		return FWwiseFileStateSharedPtr(new FWwiseInMemoryMediaFileState(InMediaCookedData, InRootPath));
	}
}
