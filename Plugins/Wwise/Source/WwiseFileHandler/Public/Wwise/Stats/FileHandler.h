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

#include "Stats/Stats.h"
#include "Wwise/Stats/NamedEvents.h"

// File Handler
DECLARE_STATS_GROUP(TEXT("File Handler"), STATGROUP_WwiseFileHandler, STATCAT_Wwise);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Created External Source States"), STAT_WwiseFileHandlerCreatedExternalSourceStates, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Known External Source Media"), STAT_WwiseFileHandlerKnownExternalSourceMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Preteched External Source Media"), STAT_WwiseFileHandlerPrefetchedExternalSourceMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Loaded External Source Media"), STAT_WwiseFileHandlerLoadedExternalSourceMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Known Media"), STAT_WwiseFileHandlerKnownMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Prefetched Media"), STAT_WwiseFileHandlerPrefetchedMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Loaded Media"), STAT_WwiseFileHandlerLoadedMedia, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Known SoundBanks"), STAT_WwiseFileHandlerKnownSoundBanks, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Loaded SoundBanks"), STAT_WwiseFileHandlerLoadedSoundBanks, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Total Error Count"), STAT_WwiseFileHandlerTotalErrorCount, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("State Operations Being Processed"), STAT_WwiseFileHandlerStateOperationsBeingProcessed, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("State Operation Latency"), STAT_WwiseFileHandlerStateOperationLatency, STATGROUP_WwiseFileHandler, WWISEFILEHANDLER_API);

// File Handler - Low-Level I/O
DECLARE_STATS_GROUP(TEXT("File Handler - Low-level I/O"), STATGROUP_WwiseFileHandlerLowLevelIO, STATCAT_Wwise);

DECLARE_FLOAT_COUNTER_STAT_EXTERN(TEXT("Streaming KB / Frame"), STAT_WwiseFileHandlerStreamingKB, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Opened Streams"), STAT_WwiseFileHandlerOpenedStreams, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Batched Requests"), STAT_WwiseFileHandlerBatchedRequests, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Pending Requests"), STAT_WwiseFileHandlerPendingRequests, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Total Requests"), STAT_WwiseFileHandlerTotalRequests, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_FLOAT_ACCUMULATOR_STAT_EXTERN(TEXT("Total Streaming MB"), STAT_WwiseFileHandlerTotalStreamedMB, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("IO Request Latency"), STAT_WwiseFileHandlerIORequestLatency, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("File Operation Latency"), STAT_WwiseFileHandlerFileOperationLatency, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("SoundEngine Callback Latency"), STAT_WwiseFileHandlerSoundEngineCallbackLatency, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);

DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Stream Critical Priority"), STAT_WwiseFileHandlerCriticalPriority, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Stream High Priority"), STAT_WwiseFileHandlerHighPriority, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Stream Normal Priority"), STAT_WwiseFileHandlerNormalPriority, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Stream Below Normal Priority"), STAT_WwiseFileHandlerBelowNormalPriority, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Stream Low Priority"), STAT_WwiseFileHandlerLowPriority, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Stream Background Priority"), STAT_WwiseFileHandlerBackgroundPriority, STATGROUP_WwiseFileHandlerLowLevelIO, WWISEFILEHANDLER_API);


WWISEFILEHANDLER_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseFileHandler, Log, All);

#define SCOPED_WWISEFILEHANDLER_EVENT(Text) SCOPED_WWISE_NAMED_EVENT(TEXT("WwiseFileHandler"), Text)
#define SCOPED_WWISEFILEHANDLER_EVENT_2(Text) SCOPED_WWISE_NAMED_EVENT_2(TEXT("WwiseFileHandler"), Text)
#define SCOPED_WWISEFILEHANDLER_EVENT_3(Text) SCOPED_WWISE_NAMED_EVENT_3(TEXT("WwiseFileHandler"), Text)
#define SCOPED_WWISEFILEHANDLER_EVENT_4(Text) SCOPED_WWISE_NAMED_EVENT_4(TEXT("WwiseFileHandler"), Text)
#define SCOPED_WWISEFILEHANDLER_EVENT_F(Format, ...) SCOPED_WWISE_NAMED_EVENT_F(TEXT("WwiseFileHandler"), Format, __VA_ARGS__)
#define SCOPED_WWISEFILEHANDLER_EVENT_F_2(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_2(TEXT("WwiseFileHandler"), Format, __VA_ARGS__)
#define SCOPED_WWISEFILEHANDLER_EVENT_F_3(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_3(TEXT("WwiseFileHandler"), Format, __VA_ARGS__)
#define SCOPED_WWISEFILEHANDLER_EVENT_F_4(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_4(TEXT("WwiseFileHandler"), Format, __VA_ARGS__)

#define WWISEFILEHANDLER_ASYNC_NAME(Text) TEXT("WwiseFileHandler ") TEXT(Text)
