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
// This code should be used after any #include <AK/...> operation, and the attached Pre
// operation should be done prior to the includes.

#if !defined(AK_PRESOUNDENGINEINCLUDE_GUARD)
	#error PostSoundEngineInclude is not reentrant. Please include "Wwise/PreSoundEngineInclude.h" before including your <AK/...> files
#endif
#undef AK_PRESOUNDENGINEINCLUDE_GUARD

#ifdef PRAGMA_POP_PLATFORM_DEFAULT_PACKING
PRAGMA_POP_PLATFORM_DEFAULT_PACKING
#endif

#ifdef THIRD_PARTY_INCLUDES_END
THIRD_PARTY_INCLUDES_END
#endif

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT

#include "Microsoft/HideMicrosoftPlatformTypes.h"
#include "Microsoft/HideMicrosoftPlatformAtomics.h"

#if defined(AK_KEEP_InterlockedIncrement)
#undef InterlockedIncrement
#define InterlockedIncrement _InterlockedIncrement
#undef AK_KEEP_InterlockedIncrement
#endif
#if defined(AK_KEEP_InterlockedDecrement)
#undef InterlockedDecrement
#define InterlockedDecrement _InterlockedDecrement
#undef AK_KEEP_InterlockedDecrement
#endif
#if defined(AK_KEEP_InterlockedAdd)
#undef InterlockedAdd
#define InterlockedAdd _InterlockedAdd
#undef AK_KEEP_InterlockedAdd
#endif
#if defined(AK_KEEP_InterlockedExchange)
#undef InterlockedExchange
#define InterlockedExchange _InterlockedExchange
#undef AK_KEEP_InterlockedExchange
#endif
#if defined(AK_KEEP_InterlockedExchangeAdd)
#undef InterlockedExchangeAdd
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#undef AK_KEEP_InterlockedExchangeAdd
#endif
#if defined(AK_KEEP_InterlockedCompareExchange)
#undef InterlockedCompareExchange
#define InterlockedCompareExchange _InterlockedCompareExchange 
#undef AK_KEEP_InterlockedCompareExchange
#endif
#if defined(AK_KEEP_InterlockedCompareExchangePointer)
#undef InterlockedCompareExchangePointer
#if PLATFORM_64BITS
	#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer
#else
	#define InterlockedCompareExchangePointer __InlineInterlockedCompareExchangePointer
#endif
#undef AK_KEEP_InterlockedCompareExchangePointer
#endif
#if defined(AK_KEEP_InterlockedExchange64)
#undef InterlockedExchange64
#define InterlockedExchange64 _InterlockedExchange64
#undef AK_KEEP_InterlockedExchange64
#endif
#if defined(AK_KEEP_InterlockedExchangeAdd64)
#undef InterlockedExchangeAdd64
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#undef AK_KEEP_InterlockedExchangeAdd64
#endif
#if defined(AK_KEEP_InterlockedCompareExchange64)
#undef InterlockedCompareExchange64
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#undef AK_KEEP_InterlockedCompareExchange64
#endif
#if defined(AK_KEEP_InterlockedIncrement64)
#undef InterlockedIncrement64
#define InterlockedIncrement64 _InterlockedIncrement64
#undef AK_KEEP_InterlockedIncrement64
#endif
#if defined(AK_KEEP_InterlockedDecrement64)
#undef InterlockedDecrement64
#define InterlockedDecrement64 _InterlockedDecrement64
#undef AK_KEEP_InterlockedDecrement64
#endif
#if defined(AK_KEEP_InterlockedAnd)
#undef InterlockedAnd
#define InterlockedAnd _InterlockedAnd
#undef AK_KEEP_InterlockedAnd
#endif
#if defined(AK_KEEP_InterlockedOr)
#undef InterlockedOr
#define InterlockedOr _InterlockedOr
#undef AK_KEEP_InterlockedOr
#endif
#if defined(AK_KEEP_InterlockedXor)
#undef InterlockedXor
#define InterlockedXor _InterlockedXor
#undef AK_KEEP_InterlockedXor
#endif
#if defined(AK_KEEP_INT)
#undef INT
#define INT ::INT
#undef AK_KEEP_INT
#endif
#if defined(AK_KEEP_UINT)
#undef UINT
#define UINT ::UINT
#undef AK_KEEP_UINT
#endif
#if defined(AK_KEEP_DWORD)
#undef DWORD
#define DWORD ::DWORD
#undef AK_KEEP_DWORD
#endif
#if defined(AK_KEEP_FLOAT)
#undef FLOAT
#define FLOAT ::FLOAT
#undef AK_KEEP_FLOAT
#endif
#if defined(AK_KEEP_TRUE)
#undef TRUE
#define TRUE 1
#undef AK_KEEP_TRUE
#endif
#if defined(AK_KEEP_FALSE)
#undef FALSE
#define FALSE 0
#undef AK_KEEP_FALSE
#endif

#endif