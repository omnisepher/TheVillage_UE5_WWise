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

#include "Wwise/WwiseExternalSourceManagerImpl.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"

#include <inttypes.h>

#include "Wwise/WwiseExternalSourceFileState.h"

FWwiseExternalSourceState::FWwiseExternalSourceState(const FWwiseExternalSourceCookedData& InCookedData) :
	FWwiseExternalSourceCookedData(InCookedData),
	LoadCount(0)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerCreatedExternalSourceStates);
}

FWwiseExternalSourceState::~FWwiseExternalSourceState()
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerCreatedExternalSourceStates);
}

void FWwiseExternalSourceState::IncrementLoadCount()
{
	const auto NewLoadCount = LoadCount.IncrementExchange() + 1;
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource State %" PRIu32 " (%s): ++LoadCount=%d"), Cookie, *DebugName.ToString(), NewLoadCount);
}

bool FWwiseExternalSourceState::DecrementLoadCount()
{
	const auto NewLoadCount = LoadCount.DecrementExchange() - 1;
	const bool bResult = (NewLoadCount == 0);
	if (UNLIKELY(NewLoadCount < 0))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("ExternalSource State %" PRIu32 " (%s): --LoadCount=%d!"), Cookie, *DebugName.ToString(), NewLoadCount);
	}
	else if (bResult)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource State %" PRIu32 " (%s): --LoadCount=%d. Deleting."), Cookie, *DebugName.ToString(), NewLoadCount);
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource State %" PRIu32 " (%s): --LoadCount=%d"), Cookie, *DebugName.ToString(), NewLoadCount);
	}
	return bResult;
}


FWwiseExternalSourceManagerImpl::FWwiseExternalSourceManagerImpl() :
	StreamingGranularity(0)
{
}

FWwiseExternalSourceManagerImpl::~FWwiseExternalSourceManagerImpl()
{
}

void FWwiseExternalSourceManagerImpl::LoadExternalSource(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
	const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseExternalSourceManagerImpl::LoadExternalSource"));
	FileHandlerExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseExternalSourceManagerImpl::LoadExternalSource"), [this, InExternalSourceCookedData, InRootPath, InLanguage, InCallback = MoveTemp(InCallback)]() mutable
	{
		LoadExternalSourceImpl(InExternalSourceCookedData, InRootPath, InLanguage, MoveTemp(InCallback));
	});
}

void FWwiseExternalSourceManagerImpl::UnloadExternalSource(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
	const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseExternalSourceManagerImpl::UnloadExternalSource"));
	FileHandlerExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseExternalSourceManagerImpl::UnloadExternalSource"), [this, InExternalSourceCookedData, InRootPath, InLanguage, InCallback = MoveTemp(InCallback)]() mutable
	{
		UnloadExternalSourceImpl(InExternalSourceCookedData, InRootPath, InLanguage, MoveTemp(InCallback));
	});
}

void FWwiseExternalSourceManagerImpl::SetGranularity(AkUInt32 InStreamingGranularity)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseExternalSourceManagerImpl::SetGranularity"));
	StreamingGranularity = InStreamingGranularity;
}

TArray<uint32> FWwiseExternalSourceManagerImpl::PrepareExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo,
                                                                           const TArray<FWwiseExternalSourceCookedData>
                                                                           &&
                                                                           InCookedData)
{
	SCOPED_WWISEFILEHANDLER_EVENT(TEXT("FWwiseExternalSourceManagerImpl::PrepareExternalSourceInfos"));
	if (InCookedData.Num() == 0)
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("PrepareExternalSourceInfos: No External Sources to process"));
		return {};
	}

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("GetExternalSourceInfos: Preparing %d external sources."), InCookedData.Num());
	TArray<uint32> Result;
	OutInfo.Reset();
	OutInfo.Reserve(InCookedData.Num());
	Result.Reserve(InCookedData.Num());
	{
		FRWScopeLock Lock(CookieToMediaLock, FRWScopeLockType::SLT_ReadOnly);
		for (const auto& Data : InCookedData)
		{
			AkExternalSourceInfo Info;
			const auto MediaId = PrepareExternalSourceInfo(Info, Data);
			if (LIKELY(MediaId != AK_INVALID_UNIQUE_ID))
			{
				OutInfo.Add(MoveTemp(Info));
				Result.Add(MediaId);
			}
		}
	}
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("GetExternalSourceInfos: Successfuly retrieved requested %d of %d external sources."), OutInfo.Num(), InCookedData.Num());
	return Result;
}

#if WITH_EDITORONLY_DATA
void FWwiseExternalSourceManagerImpl::Cook(FWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData,
	TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
	const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseExternalSourceManagerImpl::Cook: External Source manager needs to be overridden."));
}
#endif

void FWwiseExternalSourceManagerImpl::LoadExternalSourceImpl(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath, const FWwiseLanguageCookedData& InLanguage,
	FLoadExternalSourceCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseExternalSourceManagerImpl::LoadExternalSourceImpl"));
	FWwiseExternalSourceStateSharedPtr State;
	if (const auto* StatePtr = ExternalSourceStatesById.Find(InExternalSourceCookedData.Cookie))
	{
		State = *StatePtr;
		State->IncrementLoadCount();
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Creating new State for %s %" PRIu32), GetManagingTypeName(), InExternalSourceCookedData.Cookie);
		State = CreateExternalSourceState(InExternalSourceCookedData, InRootPath);
		if (UNLIKELY(!State.IsValid()))
		{
			SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseExternalSourceManagerImpl::LoadExternalSourceImpl Callback"));
			InCallback(false);
			return;
		}
		else
		{
			State->IncrementLoadCount();
			ExternalSourceStatesById.Add(InExternalSourceCookedData.Cookie, State);
		}
	}
	LoadExternalSourceMedia(InExternalSourceCookedData.Cookie, InExternalSourceCookedData.DebugName, InRootPath, MoveTemp(InCallback));
}

void FWwiseExternalSourceManagerImpl::UnloadExternalSourceImpl(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath, const FWwiseLanguageCookedData& InLanguage,
	FUnloadExternalSourceCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseExternalSourceManagerImpl::UnloadExternalSourceImpl"));
	FWwiseExternalSourceStateSharedPtr State;
	if (const auto* StatePtr = ExternalSourceStatesById.Find(InExternalSourceCookedData.Cookie))
	{
		State = *StatePtr;
	}

	if (UNLIKELY(!State.IsValid()))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("ExternalSource %" PRIu32 " (%s): Unloading an unknown External Source"), InExternalSourceCookedData.Cookie, *InExternalSourceCookedData.DebugName.ToString());
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseExternalSourceManagerImpl::UnloadExternalSourceImpl Callback"));
		InCallback();
	}
	else
	{
		FWwiseExternalSourceState* ExternalSourceState = State.Get();
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("ExternalSource %" PRIu32 " (%s): Closing State instance"), InExternalSourceCookedData.Cookie, *InExternalSourceCookedData.DebugName.ToString());
		if (CloseExternalSourceState(*State) && InExternalSourceCookedData.Cookie != 0)
		{
			ExternalSourceStatesById.Remove(InExternalSourceCookedData.Cookie);
			State.Reset();
		}
		if (LIKELY(InExternalSourceCookedData.Cookie != 0))
		{
			UnloadExternalSourceMedia(InExternalSourceCookedData.Cookie, InExternalSourceCookedData.DebugName, InRootPath, MoveTemp(InCallback));
		}
		else
		{
			SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseExternalSourceManagerImpl::UnloadExternalSourceImpl Callback"));
			InCallback();
		}
	}
}

FWwiseExternalSourceStateSharedPtr FWwiseExternalSourceManagerImpl::CreateExternalSourceState(
	const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath)
{
	return FWwiseExternalSourceStateSharedPtr(new FWwiseExternalSourceState(InExternalSourceCookedData));
}

bool FWwiseExternalSourceManagerImpl::CloseExternalSourceState(FWwiseExternalSourceState& InExternalSourceState)
{
	return InExternalSourceState.DecrementLoadCount();
}


void FWwiseExternalSourceManagerImpl::LoadExternalSourceMedia(const uint32 InExternalSourceCookie,
	const FName& InExternalSourceName, const FName& InRootPath, FLoadExternalSourceCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
	InCallback(false);
}

void FWwiseExternalSourceManagerImpl::UnloadExternalSourceMedia(const uint32 InExternalSourceCookie,
	const FName& InExternalSourceName, const FName& InRootPath, FUnloadExternalSourceCallback&& InCallback)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
	InCallback();
}

uint32 FWwiseExternalSourceManagerImpl::PrepareExternalSourceInfo(AkExternalSourceInfo& OutInfo,
                                                                  const FWwiseExternalSourceCookedData& InCookedData)
{
	const auto* ExternalSourceFileStatePtr = CookieToMedia.Find(InCookedData.Cookie);
	if (UNLIKELY(!ExternalSourceFileStatePtr))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("PrepareExternalSourceInfo %" PRIu32 " (%s): CookieToMedia not defined"), InCookedData.Cookie, *InCookedData.DebugName.ToString());
		return AK_INVALID_UNIQUE_ID;
	}
	auto* ExternalSourceFileState = *ExternalSourceFileStatePtr;

	if (UNLIKELY(!ExternalSourceFileState->GetExternalSourceInfo(OutInfo)))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("Getting external source %" PRIu32 " (%s): AkExternalSourceInfo not initialized"), InCookedData.Cookie, *InCookedData.DebugName.ToString());
		return AK_INVALID_UNIQUE_ID;
	}

	ExternalSourceFileState->IncrementPlayCount();

	OutInfo.iExternalSrcCookie = InCookedData.Cookie;
	UE_CLOG(OutInfo.idFile != 0, LogWwiseFileHandler, VeryVerbose, TEXT("Getting external source %" PRIu32 " (%s): Using file %" PRIu32), InCookedData.Cookie, *InCookedData.DebugName.ToString(), OutInfo.idFile);
	UE_CLOG(OutInfo.idFile == 0, LogWwiseFileHandler, VeryVerbose, TEXT("Getting external source %" PRIu32 " (%s): Using memory file"), InCookedData.Cookie, *InCookedData.DebugName.ToString());
	return ExternalSourceFileState->MediaId;
}

void FWwiseExternalSourceManagerImpl::BindPlayingIdToExternalSources(const uint32 InPlayingId,
                                                                     const TArray<uint32>& InMediaIds)
{
	if (InMediaIds.Num() == 0)
	{
		return;
	}

	SCOPED_WWISEFILEHANDLER_EVENT(TEXT("FWwiseExternalSourceManagerImpl::BindPlayingIdToExternalSources"));
	if (UNLIKELY(InPlayingId == AK_INVALID_PLAYING_ID))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("BindPlayingIdToExternalSources: Failed PostEvent. Unpreparing %d Media."), InMediaIds.Num());

		FRWScopeLock Lock(CookieToMediaLock, FRWScopeLockType::SLT_ReadOnly);
		for (const auto MediaId : InMediaIds)
		{
			FWwiseFileStateSharedPtr State;
			{
				FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
				const auto* StatePtr = FileStatesById.Find(MediaId);
				if (UNLIKELY(!StatePtr || !StatePtr->IsValid()))
				{
					UE_LOG(LogWwiseFileHandler, Warning, TEXT("BindPlayingIdToExternalSources: Getting external source media state %" PRIu32 " failed to decrement after failed PostEvent."), MediaId);
					continue;
				}
				State = *StatePtr;
			}
			auto* ExternalSourceFileState = State->GetStateAs<FWwiseExternalSourceFileState>();
			if (UNLIKELY(!ExternalSourceFileState))
			{
				UE_LOG(LogWwiseFileHandler, Error, TEXT("BindPlayingIdToExternalSources: Getting external source media %" PRIu32 ": Could not cast to ExternalSourceState"), MediaId);
				continue;
			}

			FileHandlerExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseExternalSourceManagerImpl::BindPlayingIdToExternalSources decrement"), [this, MediaId, ExternalSourceFileState]() mutable
			{
				// This type is safe as long as we don't decrement its usage
				if (ExternalSourceFileState->DecrementPlayCount() && ExternalSourceFileState->CanDelete())
				{
					OnDeleteState(MediaId, *ExternalSourceFileState, EWwiseFileStateOperationOrigin::Loading, []{});
				}
			});
		}
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("BindPlayingIdToExternalSources: Binding %d ExtSrc Media to Playing ID %" PRIu32 "."), InMediaIds.Num(), InPlayingId);
		if (InMediaIds.Num())
		{
			FScopeLock Lock(&PlayingIdToMediaIdsLock);
			for (const auto MediaId : InMediaIds)
			{
				PlayingIdToMediaIds.AddUnique(InPlayingId, MediaId);
			}
			
		}
	}
}

void FWwiseExternalSourceManagerImpl::OnEndOfEvent(const uint32 InPlayingId)
{
	SCOPED_WWISEFILEHANDLER_EVENT(TEXT("FWwiseExternalSourceManagerImpl::OnEndOfEvent"));
	TArray<uint32> MediaIds;
	{
		FScopeLock Lock(&PlayingIdToMediaIdsLock);
		if (!PlayingIdToMediaIds.Contains(InPlayingId))
		{
			return;
		}
		PlayingIdToMediaIds.MultiFind(InPlayingId, MediaIds);
		PlayingIdToMediaIds.Remove(InPlayingId);
	}
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("OnEndOfEvent: Unbinding %d ExtSrc Media from Playing ID %" PRIu32 "."), MediaIds.Num(), InPlayingId);

	FRWScopeLock Lock(CookieToMediaLock, FRWScopeLockType::SLT_ReadOnly);

	for (const auto MediaId : MediaIds)
	{
		FWwiseFileStateSharedPtr State;
		{
			FRWScopeLock StateLock(FileStatesByIdLock, FRWScopeLockType::SLT_ReadOnly);
			const auto* StatePtr = FileStatesById.Find(MediaId);
			if (UNLIKELY(!StatePtr || !StatePtr->IsValid()))
			{
				UE_LOG(LogWwiseFileHandler, Warning, TEXT("OnEndOfEvent: Getting external source media state %" PRIu32 " failed to decrement after failed PostEvent."), MediaId);
				continue;
			}
			State = *StatePtr;
		}
		auto* ExternalSourceFileState = State->GetStateAs<FWwiseExternalSourceFileState>();
		if (UNLIKELY(!ExternalSourceFileState))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("OnEndOfEvent: Getting external source media %" PRIu32 ": Could not cast to ExternalSourceState"), MediaId);
			continue;
		}
		FileHandlerExecutionQueue.Async(WWISEFILEHANDLER_ASYNC_NAME("FWwiseExternalSourceManagerImpl::OnEndOfEvent decrement"), [this, MediaId, ExternalSourceFileState]() mutable
		{
			// This type is safe as long as we don't decrement its usage
			if (ExternalSourceFileState->DecrementPlayCount() && ExternalSourceFileState->CanDelete())
			{
				OnDeleteState(MediaId, *ExternalSourceFileState, EWwiseFileStateOperationOrigin::Loading, []{});
			}
		});
	}
}

void FWwiseExternalSourceManagerImpl::OnDeleteState(uint32 InShortId, FWwiseFileState& InFileState,
	EWwiseFileStateOperationOrigin InOperationOrigin, FDecrementStateCallback&& InCallback)
{
	if (InFileState.CanDelete())
	{
		FRWScopeLock Lock(CookieToMediaLock, FRWScopeLockType::SLT_Write);
		TArray<uint32> CookiesToRemove;
		for (const auto Item : CookieToMedia)
		{
			if (Item.Value == &InFileState)
			{
				CookiesToRemove.Add(Item.Key);
			}
		}
		for (const auto Cookie : CookiesToRemove)
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("Removing Cookie %" PRIu32 " binding to media %" PRIu32 "."), Cookie, InFileState.GetShortId());
			CookieToMedia.Remove(Cookie);
		}
	}
	FWwiseFileHandlerBase::OnDeleteState(InShortId, InFileState, InOperationOrigin, MoveTemp(InCallback));
}

void FWwiseExternalSourceManagerImpl::SetExternalSourceMediaById(const FName& ExternalSourceName, const int32 MediaId)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
}

void FWwiseExternalSourceManagerImpl::SetExternalSourceMediaByName(const FName& ExternalSourceName,
	const FName& MediaName)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
}

void FWwiseExternalSourceManagerImpl::SetExternalSourceMediaWithIds(const int32 ExternalSourceCookie,
	const int32 MediaId)
{
	UE_LOG(LogWwiseFileHandler, Error, TEXT("External Source manager needs to be overridden."));
}
