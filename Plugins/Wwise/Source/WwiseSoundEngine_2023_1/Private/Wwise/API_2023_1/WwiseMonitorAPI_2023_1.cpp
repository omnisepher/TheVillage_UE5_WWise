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

#include "Wwise/API_2023_1/WwiseMonitorAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

#if AK_SUPPORT_WAAPI && WITH_EDITORONLY_DATA && !defined(AK_OPTIMIZED)
AkWAAPIErrorMessageTranslator FWwiseMonitorAPI_2023_1::WaapiErrorMessageTranslator;
#endif

AKRESULT FWwiseMonitorAPI_2023_1::PostCode(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::PostCode(in_eError, in_eErrorLevel, in_playingID, in_gameObjID, in_audioNodeID, in_bIsBus);
}

AKRESULT FWwiseMonitorAPI_2023_1::PostCodeVarArg(
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

AKRESULT FWwiseMonitorAPI_2023_1::PostCodeVaList(
	AK::Monitor::ErrorCode in_eError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AK::Monitor::MsgContext msgContext,
	::va_list args
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::PostCodeVaList(in_eError, in_eErrorLevel, msgContext, args);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseMonitorAPI_2023_1::PostString(
	const wchar_t* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::PostString(in_pszError, in_eErrorLevel, in_playingID, in_gameObjID, in_audioNodeID, in_bIsBus);
}
#endif // #ifdef AK_SUPPORT_WCHAR

AKRESULT FWwiseMonitorAPI_2023_1::PostString(
	const char* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID,
	AkUniqueID in_audioNodeID,
	bool in_bIsBus
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::PostString(in_pszError, in_eErrorLevel, in_playingID, in_gameObjID, in_audioNodeID, in_bIsBus);
}

AKRESULT FWwiseMonitorAPI_2023_1::SetLocalOutput(
	AkUInt32 in_uErrorLevel,
	AK::Monitor::LocalOutputFunc in_pMonitorFunc
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::SetLocalOutput(in_uErrorLevel, in_pMonitorFunc);
}

AKRESULT FWwiseMonitorAPI_2023_1::AddTranslator(
	AkErrorMessageTranslator* translator,
	bool overridePreviousTranslators
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::AddTranslator(translator, overridePreviousTranslators);
}

AKRESULT FWwiseMonitorAPI_2023_1::ResetTranslator(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::ResetTranslator();
}

AkTimeMs FWwiseMonitorAPI_2023_1::GetTimeStamp()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::Monitor::GetTimeStamp();
}

void FWwiseMonitorAPI_2023_1::MonitorStreamMgrInit(
	const AkStreamMgrSettings& in_streamMgrSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::Monitor::MonitorStreamMgrInit(in_streamMgrSettings);
}

void FWwiseMonitorAPI_2023_1::MonitorStreamingDeviceInit(
	AkDeviceID in_deviceID,
	const AkDeviceSettings& in_deviceSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::Monitor::MonitorStreamingDeviceInit(in_deviceID, in_deviceSettings);
}

void FWwiseMonitorAPI_2023_1::MonitorStreamingDeviceDestroyed(
	AkDeviceID in_deviceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::Monitor::MonitorStreamingDeviceDestroyed(in_deviceID);
}

void FWwiseMonitorAPI_2023_1::MonitorStreamMgrTerm()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::Monitor::MonitorStreamMgrTerm();
}

void FWwiseMonitorAPI_2023_1::SetupDefaultWAAPIErrorTranslator(const FString& WaapiIP, AkUInt32 WaapiPort, AkUInt32 Timeout)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_SUPPORT_WAAPI && WITH_EDITORONLY_DATA && !defined(AK_OPTIMIZED)
	WaapiErrorMessageTranslator.SetConnectionIP(TCHAR_TO_ANSI(*WaapiIP), WaapiPort, Timeout);
	AddTranslator(&WaapiErrorMessageTranslator);
#endif
}

void FWwiseMonitorAPI_2023_1::TerminateDefaultWAAPIErrorTranslator()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if AK_SUPPORT_WAAPI && WITH_EDITORONLY_DATA && !defined(AK_OPTIMIZED)
	WaapiErrorMessageTranslator.Term();
#endif
}

