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

// Allows usage of the Atomics and Windows macro-based types.
// 
// In some circumstances, under some platforms, the definitions for Windows macros are
// already done. This code makes sure we safeguard what is already set up, before including
// the official Allow header.
//
// This code should be used before any #include <AK/...> operation, and the attached Post
// operation should be done afterwards.

#if defined(AK_PRESOUNDENGINEINCLUDE_GUARD)
	#error PreSoundEngineInclude is not reentrant. Please include "Wwise/PostSoundEngineInclude.h" after including your <AK/...> files
#endif
#define AK_PRESOUNDENGINEINCLUDE_GUARD 1

#include "CoreTypes.h"

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT

#if defined(InterlockedIncrement)
#define AK_KEEP_InterlockedIncrement 1
#endif
#if defined(InterlockedDecrement)
#define AK_KEEP_InterlockedDecrement 1
#endif
#if defined(InterlockedAdd)
#define AK_KEEP_InterlockedAdd 1
#endif
#if defined(InterlockedExchange)
#define AK_KEEP_InterlockedExchange 1
#endif
#if defined(InterlockedExchangeAdd)
#define AK_KEEP_InterlockedExchangeAdd 1
#endif
#if defined(InterlockedCompareExchange)
#define AK_KEEP_InterlockedCompareExchange 1
#endif
#if defined(InterlockedCompareExchangePointer)
#define AK_KEEP_InterlockedCompareExchangePointer 1
#endif
#if defined(InterlockedExchange64)
#define AK_KEEP_InterlockedExchange64 1
#endif
#if defined(InterlockedExchangeAdd64)
#define AK_KEEP_InterlockedExchangeAdd64 1
#endif
#if defined(InterlockedCompareExchange64)
#define AK_KEEP_InterlockedCompareExchange64 1
#endif
#if defined(InterlockedIncrement64)
#define AK_KEEP_InterlockedIncrement64 1
#endif
#if defined(InterlockedDecrement64)
#define AK_KEEP_InterlockedDecrement64 1
#endif
#if defined(InterlockedAnd)
#define AK_KEEP_InterlockedAnd 1
#endif
#if defined(InterlockedOr)
#define AK_KEEP_InterlockedOr 1
#endif
#if defined(InterlockedXor)
#define AK_KEEP_InterlockedXor 1
#endif
#if defined(INT)
#define AK_KEEP_INT 1
#endif
#if defined(UINT)
#define AK_KEEP_UINT 1
#endif
#if defined(DWORD)
#define AK_KEEP_DWORD 1
#endif
#if defined(FLOAT)
#define AK_KEEP_FLOAT 1
#endif
#if defined(TRUE)
#define AK_KEEP_TRUE 1
#endif
#if defined(FALSE)
#define AK_KEEP_FALSE 1
#endif

#include "Microsoft/AllowMicrosoftPlatformTypes.h"
#include "Microsoft/AllowMicrosoftPlatformAtomics.h"

#endif

#ifdef THIRD_PARTY_INCLUDES_START
THIRD_PARTY_INCLUDES_START
#endif

#ifdef PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
#endif