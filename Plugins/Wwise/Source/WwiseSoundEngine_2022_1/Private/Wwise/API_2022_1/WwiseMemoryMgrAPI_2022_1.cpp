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

#include "Wwise/API_2022_1/WwiseMemoryMgrAPI_2022_1.h"
#include "Wwise/Stats/SoundEngine_2022_1.h"

AKRESULT FWwiseMemoryMgrAPI_2022_1::Init(
	AkMemSettings* in_pSettings
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseMemoryMgrAPI_2022_1::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	return AK::MemoryMgr::Init(in_pSettings);
}

void FWwiseMemoryMgrAPI_2022_1::GetDefaultSettings(
	AkMemSettings& out_pMemSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::GetDefaultSettings(out_pMemSettings);
}

bool FWwiseMemoryMgrAPI_2022_1::IsInitialized()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	return AK::MemoryMgr::IsInitialized();
}

void FWwiseMemoryMgrAPI_2022_1::Term()
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseMemoryMgrAPI_2022_1::Term"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::Term();
}

void FWwiseMemoryMgrAPI_2022_1::InitForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::InitForThread();
}

void FWwiseMemoryMgrAPI_2022_1::TermForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::TermForThread();
}

void FWwiseMemoryMgrAPI_2022_1::TrimForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#if AK_WWISESDK_VERSION_MAJOR == 2022 && AK_WWISESDK_VERSION_MINOR == 1 && AK_WWISESDK_VERSION_SUBMINOR >= 2
	AK::MemoryMgr::TrimForThread();
#endif
}

void* FWwiseMemoryMgrAPI_2022_1::dMalloc(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dMalloc(in_poolId, in_uSize, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::Malloc(in_poolId, in_uSize);
#endif
}

void* FWwiseMemoryMgrAPI_2022_1::Malloc(
	AkMemPoolId in_poolId,
	size_t		in_uSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	return AK::MemoryMgr::Malloc(in_poolId, in_uSize);
}

void* FWwiseMemoryMgrAPI_2022_1::dRealloc(
	AkMemPoolId	in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dRealloc(in_poolId, in_pAlloc, in_uSize, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::Realloc(in_poolId, in_pAlloc, in_uSize);
#endif
}

void* FWwiseMemoryMgrAPI_2022_1::Realloc(
	AkMemPoolId in_poolId,
	void* in_pAlloc,
	size_t		in_uSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	return AK::MemoryMgr::Realloc(in_poolId, in_pAlloc, in_uSize);
}

void* FWwiseMemoryMgrAPI_2022_1::dReallocAligned(
	AkMemPoolId	in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	AkUInt32	in_uAlignment,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dReallocAligned(in_poolId, in_pAlloc, in_uSize, in_uAlignment, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::ReallocAligned(in_poolId, in_pAlloc, in_uSize, in_uAlignment);
#endif
}

void* FWwiseMemoryMgrAPI_2022_1::ReallocAligned(
	AkMemPoolId in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	AkUInt32	in_uAlignment
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	return AK::MemoryMgr::ReallocAligned(in_poolId, in_pAlloc, in_uSize, in_uAlignment);
}

void FWwiseMemoryMgrAPI_2022_1::Free(
	AkMemPoolId in_poolId,
	void* in_pMemAddress
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::Free(in_poolId, in_pMemAddress);
}

void* FWwiseMemoryMgrAPI_2022_1::dMalign(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	AkUInt32	in_uAlignment,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
#ifdef AK_MEMDEBUG
	return AK::MemoryMgr::dMalign(in_poolId, in_uSize, in_uAlignment, in_pszFile, in_uLine);
#else
	return AK::MemoryMgr::Malign(in_poolId, in_uSize, in_uAlignment);
#endif
}

void* FWwiseMemoryMgrAPI_2022_1::Malign(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	AkUInt32	in_uAlignment
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	return AK::MemoryMgr::Malign(in_poolId, in_uSize, in_uAlignment);
}

void FWwiseMemoryMgrAPI_2022_1::GetCategoryStats(
	AkMemPoolId	in_poolId,
	AK::MemoryMgr::CategoryStats& out_poolStats
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::GetCategoryStats(in_poolId, out_poolStats);
}

void FWwiseMemoryMgrAPI_2022_1::GetGlobalStats(
	AK::MemoryMgr::GlobalStats& out_stats
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::GetGlobalStats(out_stats);
}

void FWwiseMemoryMgrAPI_2022_1::StartProfileThreadUsage(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::StartProfileThreadUsage();
}

AkUInt64 FWwiseMemoryMgrAPI_2022_1::StopProfileThreadUsage(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	return AK::MemoryMgr::StopProfileThreadUsage();
}

void FWwiseMemoryMgrAPI_2022_1::DumpToFile(
	const AkOSChar* pszFilename
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2022_1);
	AK::MemoryMgr::DumpToFile(pszFilename);
}

