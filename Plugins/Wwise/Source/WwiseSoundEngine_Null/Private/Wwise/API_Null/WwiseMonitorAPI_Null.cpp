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

#include "Wwise/API_Null/WwiseMonitorAPI_Null.h"
#include "Wwise/Stats/SoundEngine_Null.h"

AKRESULT FWwiseMonitorAPI_Null::PostCode(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseMonitorAPI_Null::PostCodeVarArg(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AK::Monitor::MsgContext msgContext,
	...
)
{
	va_list Args;
	va_start(Args, msgContext);
	auto Result = PostCodeVaList(in_eError, in_eErrorLevel, msgContext, Args);
	va_end(Args);
	return Result;
}

AKRESULT FWwiseMonitorAPI_Null::PostCodeVaList(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AK::Monitor::MsgContext msgContext,
	::va_list args
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseMonitorAPI_Null::PostString(
	const wchar_t* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}
#endif // #ifdef AK_SUPPORT_WCHAR

AKRESULT FWwiseMonitorAPI_Null::PostString(
	const char* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseMonitorAPI_Null::SetLocalOutput(
	AkUInt32 in_uErrorLevel,
	AK::Monitor::LocalOutputFunc in_pMonitorFunc
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseMonitorAPI_Null::AddTranslator(
	AkErrorMessageTranslator* translator,
	bool overridePreviousTranslators
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseMonitorAPI_Null::ResetTranslator(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AkTimeMs FWwiseMonitorAPI_Null::GetTimeStamp()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return 0;
}

void FWwiseMonitorAPI_Null::MonitorStreamMgrInit(
	const AkStreamMgrSettings& in_streamMgrSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMonitorAPI_Null::MonitorStreamingDeviceInit(
	AkDeviceID in_deviceID,
	const AkDeviceSettings& in_deviceSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMonitorAPI_Null::MonitorStreamingDeviceDestroyed(
	AkDeviceID in_deviceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMonitorAPI_Null::MonitorStreamMgrTerm()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMonitorAPI_Null::SetupDefaultWAAPIErrorTranslator(const FString& WaapiIP, AkUInt32 WaapiPort, AkUInt32 Timeout)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

void FWwiseMonitorAPI_Null::TerminateDefaultWAAPIErrorTranslator()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

