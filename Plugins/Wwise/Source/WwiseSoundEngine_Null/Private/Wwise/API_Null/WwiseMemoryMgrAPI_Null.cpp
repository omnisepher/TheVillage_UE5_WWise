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

#include "Wwise/API_Null/WwiseMemoryMgrAPI_Null.h"
#include "Wwise/Stats/SoundEngine_Null.h"

AKRESULT FWwiseMemoryMgrAPI_Null::Init(
	AkMemSettings* in_pSettings
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseMemoryMgrAPI_Null::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

void FWwiseMemoryMgrAPI_Null::GetDefaultSettings(
	AkMemSettings& out_pMemSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

bool FWwiseMemoryMgrAPI_Null::IsInitialized()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return false;
}

void FWwiseMemoryMgrAPI_Null::Term()
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseMemoryMgrAPI_Null::Term"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMemoryMgrAPI_Null::InitForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMemoryMgrAPI_Null::TermForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMemoryMgrAPI_Null::TrimForThread()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void* FWwiseMemoryMgrAPI_Null::dMalloc(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void* FWwiseMemoryMgrAPI_Null::Malloc(
	AkMemPoolId in_poolId,
	size_t		in_uSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void* FWwiseMemoryMgrAPI_Null::dRealloc(
	AkMemPoolId	in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void* FWwiseMemoryMgrAPI_Null::Realloc(
	AkMemPoolId in_poolId,
	void* in_pAlloc,
	size_t		in_uSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void* FWwiseMemoryMgrAPI_Null::dReallocAligned(
	AkMemPoolId	in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	AkUInt32	in_uAlignment,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void* FWwiseMemoryMgrAPI_Null::ReallocAligned(
	AkMemPoolId in_poolId,
	void* in_pAlloc,
	size_t		in_uSize,
	AkUInt32	in_uAlignment
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void FWwiseMemoryMgrAPI_Null::Free(
	AkMemPoolId in_poolId,
	void* in_pMemAddress
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void* FWwiseMemoryMgrAPI_Null::dMalign(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	AkUInt32	in_uAlignment,
	const char* in_pszFile,
	AkUInt32	in_uLine
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void* FWwiseMemoryMgrAPI_Null::Malign(
	AkMemPoolId in_poolId,
	size_t		in_uSize,
	AkUInt32	in_uAlignment
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void FWwiseMemoryMgrAPI_Null::GetCategoryStats(
	AkMemPoolId	in_poolId,
	AK::MemoryMgr::CategoryStats& out_poolStats
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMemoryMgrAPI_Null::GetGlobalStats(
	AK::MemoryMgr::GlobalStats& out_stats
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMemoryMgrAPI_Null::StartProfileThreadUsage(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

AkUInt64 FWwiseMemoryMgrAPI_Null::StopProfileThreadUsage(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return 0;
}

void FWwiseMemoryMgrAPI_Null::DumpToFile(
	const AkOSChar* pszFilename
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

