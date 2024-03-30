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

#include "Wwise/API_Null/WwiseStreamMgrAPI_Null.h"
#include "Wwise/Stats/SoundEngine_Null.h"

AK::IAkStreamMgr* FWwiseStreamMgrAPI_Null::GetAkStreamMgrInstance()
{
	IWwiseSoundEngineModule::ForceLoadModule();
	return nullptr;
}

AK::IAkStreamMgr* FWwiseStreamMgrAPI_Null::Create(
	const AkStreamMgrSettings& in_settings
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseStreamMgrAPI_Null::Create"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void FWwiseStreamMgrAPI_Null::GetDefaultSettings(
	AkStreamMgrSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

AK::StreamMgr::IAkFileLocationResolver* FWwiseStreamMgrAPI_Null::GetFileLocationResolver()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

void FWwiseStreamMgrAPI_Null::SetFileLocationResolver(
	AK::StreamMgr::IAkFileLocationResolver* in_pFileLocationResolver
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

AKRESULT FWwiseStreamMgrAPI_Null::CreateDevice(
	const AkDeviceSettings& in_settings,		///< Device settings.
	AK::StreamMgr::IAkLowLevelIOHook* in_pLowLevelHook,	///< Associated low-level I/O hook. Pass either a IAkLowLevelIOHook interface, consistent with the type of the scheduler.
	AkDeviceID& out_idDevice		///< Assigned unique device id to use in all other functions of this interface.
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseStreamMgrAPI_Null::DestroyDevice(
	AkDeviceID					in_deviceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseStreamMgrAPI_Null::PerformIO()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

void FWwiseStreamMgrAPI_Null::GetDefaultDeviceSettings(
	AkDeviceSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

AKRESULT FWwiseStreamMgrAPI_Null::SetCurrentLanguage(
	const AkOSChar* in_pszLanguageName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

const AkOSChar* FWwiseStreamMgrAPI_Null::GetCurrentLanguage()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return nullptr;
}

AKRESULT FWwiseStreamMgrAPI_Null::AddLanguageChangeObserver(
	AK::StreamMgr::AkLanguageChangeHandler in_handler,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

void FWwiseStreamMgrAPI_Null::RemoveLanguageChangeObserver(
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseStreamMgrAPI_Null::FlushAllCaches()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

