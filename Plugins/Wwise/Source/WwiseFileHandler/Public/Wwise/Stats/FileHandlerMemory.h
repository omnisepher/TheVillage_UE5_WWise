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

#include "Wwise/Stats/Memory.h"

DECLARE_MEMORY_STAT_EXTERN(TEXT("ExtSrc"), STAT_WwiseMemoryExtSrc, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryExtSrc_FName GET_STATFNAME(STAT_WwiseMemoryExtSrc)
#if AK_SUPPORT_DEVICE_MEMORY
DECLARE_MEMORY_STAT_EXTERN(TEXT("ExtSrc Device"), STAT_WwiseMemoryExtSrcDevice, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryExtSrcDevice_FName GET_STATFNAME(STAT_WwiseMemoryExtSrcDevice)
#else
#define STAT_WwiseMemoryExtSrcDevice_FName NAME_None
#endif

DECLARE_MEMORY_STAT_EXTERN(TEXT("ExtSrc Prefetch"), STAT_WwiseMemoryExtSrcPrefetch, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryExtSrcPrefetch_FName GET_STATFNAME(STAT_WwiseMemoryExtSrcPrefetch)
#if AK_SUPPORT_DEVICE_MEMORY
DECLARE_MEMORY_STAT_EXTERN(TEXT("ExtSrc Prefetch Device"), STAT_WwiseMemoryExtSrcPrefetchDevice, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryExtSrcPrefetchDevice_FName GET_STATFNAME(STAT_WwiseMemoryExtSrcPrefetchDevice)
#else
#define STAT_WwiseMemoryExtSrcPrefetchDevice_FName NAME_None
#endif

DECLARE_MEMORY_STAT_EXTERN(TEXT("Media"), STAT_WwiseMemoryMedia, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryMedia_FName GET_STATFNAME(STAT_WwiseMemoryMedia)
#if AK_SUPPORT_DEVICE_MEMORY
DECLARE_MEMORY_STAT_EXTERN(TEXT("Media Device"), STAT_WwiseMemoryMediaDevice, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryMediaDevice_FName GET_STATFNAME(STAT_WwiseMemoryMediaDevice)
#else
#define STAT_WwiseMemoryMediaDevice_FName NAME_None
#endif

DECLARE_MEMORY_STAT_EXTERN(TEXT("Media Prefetch"), STAT_WwiseMemoryMediaPrefetch, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryMediaPrefetch_FName GET_STATFNAME(STAT_WwiseMemoryMediaPrefetch)
#if AK_SUPPORT_DEVICE_MEMORY
DECLARE_MEMORY_STAT_EXTERN(TEXT("Media Prefetch Device"), STAT_WwiseMemoryMediaPrefetchDevice, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemoryMediaPrefetchDevice_FName GET_STATFNAME(STAT_WwiseMemoryMediaPrefetchDevice)
#else
#define STAT_WwiseMemoryMediaPrefetchDevice_FName NAME_None
#endif

DECLARE_MEMORY_STAT_EXTERN(TEXT("SoundBank"), STAT_WwiseMemorySoundBank, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemorySoundBank_FName GET_STATFNAME(STAT_WwiseMemorySoundBank)
DECLARE_MEMORY_STAT_EXTERN(TEXT("SoundBank Mapped"), STAT_WwiseMemorySoundBankMapped, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemorySoundBankMapped_FName GET_STATFNAME(STAT_WwiseMemorySoundBankMapped)
#if AK_SUPPORT_DEVICE_MEMORY
DECLARE_MEMORY_STAT_EXTERN(TEXT("SoundBank Device"), STAT_WwiseMemorySoundBankDevice, STATGROUP_WwiseMemory, WWISEFILEHANDLER_API);
#define STAT_WwiseMemorySoundBankDevice_FName GET_STATFNAME(STAT_WwiseMemorySoundBankDevice)
#else
#define STAT_WwiseMemorySoundBankDevice_FName NAME_None
#endif
