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
#include "Logging/LogMacros.h"

#include "Wwise/Stats/NamedEvents.h"

DECLARE_STATS_GROUP(TEXT("Resource Loader"), STATGROUP_WwiseResourceLoader, STATCAT_Wwise);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Aux Busses"), STAT_WwiseResourceLoaderAuxBusses, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Events"), STAT_WwiseResourceLoaderEvents, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced External Sources"), STAT_WwiseResourceLoaderExternalSources, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Group Values"), STAT_WwiseResourceLoaderGroupValues, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Init Banks"), STAT_WwiseResourceLoaderInitBanks, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Media"), STAT_WwiseResourceLoaderMedia, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced ShareSets"), STAT_WwiseResourceLoaderShareSets, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced SoundBanks"), STAT_WwiseResourceLoaderSoundBanks, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Referenced Switch Container Combinations"), STAT_WwiseResourceLoaderSwitchContainerCombinations, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Resource Loading"), STAT_WwiseResourceLoaderTiming, STATGROUP_WwiseResourceLoader, WWISERESOURCELOADER_API);

WWISERESOURCELOADER_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseResourceLoader, Log, All);

#define SCOPED_WWISERESOURCELOADER_EVENT(Text) SCOPED_WWISE_NAMED_EVENT(TEXT("WwiseResourceLoader"), Text)
#define SCOPED_WWISERESOURCELOADER_EVENT_2(Text) SCOPED_WWISE_NAMED_EVENT_2(TEXT("WwiseResourceLoader"), Text)
#define SCOPED_WWISERESOURCELOADER_EVENT_3(Text) SCOPED_WWISE_NAMED_EVENT_3(TEXT("WwiseResourceLoader"), Text)
#define SCOPED_WWISERESOURCELOADER_EVENT_4(Text) SCOPED_WWISE_NAMED_EVENT_4(TEXT("WwiseResourceLoader"), Text)
#define SCOPED_WWISERESOURCELOADER_EVENT_F(Format, ...) SCOPED_WWISE_NAMED_EVENT_F(TEXT("WwiseResourceLoader"), Format, __VA_ARGS__)
#define SCOPED_WWISERESOURCELOADER_EVENT_F_2(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_2(TEXT("WwiseResourceLoader"), Format, __VA_ARGS__)
#define SCOPED_WWISERESOURCELOADER_EVENT_F_3(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_3(TEXT("WwiseResourceLoader"), Format, __VA_ARGS__)
#define SCOPED_WWISERESOURCELOADER_EVENT_F_4(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_4(TEXT("WwiseResourceLoader"), Format, __VA_ARGS__)

#define WWISERESOURCELOADER_ASYNC_NAME(Text) TEXT("WwiseResourceLoader ") TEXT(Text)
