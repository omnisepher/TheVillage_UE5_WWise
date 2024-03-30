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

#include "Wwise/API_2023_1/WwiseSoundEngineAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

#include "CoreTypes.h"

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT
#pragma pack(push)
#pragma warning(push)
#pragma warning(disable: 4103)		// alignment changed after including header, may be due to missing #pragma pack(pop)
#endif // PLATFORM_MICROSOFT

#include "Wwise/PreSoundEngineInclude.h"
#include <AK/Plugin/AkVorbisDecoderFactory.h>
#include <AK/Plugin/AkMeterFXFactory.h>
#include <AK/Plugin/AkAudioInputSourceFactory.h>

#if AK_SUPPORT_OPUS
#include <AK/Plugin/AkOpusDecoderFactory.h>
#endif // AK_SUPPORT_OPUS

#if PLATFORM_IOS && !PLATFORM_TVOS
#include "Generated/AkiOSPlugins.h"
#endif

#if PLATFORM_SWITCH
#include "Generated/AkSwitchPlugins.h"
#if AK_SUPPORT_OPUS
#include <AK/Plugin/AkOpusNXFactory.h>
#endif
#endif

#if PLATFORM_TVOS
#include "Generated/AkTVOSPlugins.h"
#endif
#include "Wwise/PostSoundEngineInclude.h"

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT
#pragma warning(pop)
#pragma pack(pop)
#endif // PLATFORM_MICROSOFT

FWwiseSoundEngineAPI_2023_1::FWwiseSoundEngineAPI_2023_1():
	IWwiseSoundEngineAPI(new FQuery, new FAudioInputPlugin)
{}

bool FWwiseSoundEngineAPI_2023_1::IsInitialized()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::IsInitialized();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::Init(
	AkInitSettings* in_pSettings,
	AkPlatformInitSettings* in_pPlatformSettings
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseSoundEngineAPI_2023_1::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Init(in_pSettings, in_pPlatformSettings);
}

void FWwiseSoundEngineAPI_2023_1::GetDefaultInitSettings(
	AkInitSettings& out_settings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::GetDefaultInitSettings(out_settings);
}

void FWwiseSoundEngineAPI_2023_1::GetDefaultPlatformInitSettings(
	AkPlatformInitSettings& out_platformSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::GetDefaultPlatformInitSettings(out_platformSettings);
}

void FWwiseSoundEngineAPI_2023_1::Term()
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseSoundEngineAPI_2023_1::Term"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::Term();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetAudioSettings(
	AkAudioSettings& out_audioSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetAudioSettings(out_audioSettings);
}

AkChannelConfig FWwiseSoundEngineAPI_2023_1::GetSpeakerConfiguration(
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetSpeakerConfiguration(in_idOutput);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetOutputDeviceConfiguration(
	AkOutputDeviceID in_idOutput,
	AkChannelConfig& io_channelConfig,
	Ak3DAudioSinkCapabilities& io_capabilities
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetOutputDeviceConfiguration(in_idOutput, io_channelConfig, io_capabilities);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetPanningRule(
	AkPanningRule& out_ePanningRule,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetPanningRule(out_ePanningRule, in_idOutput);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetPanningRule(
	AkPanningRule		in_ePanningRule,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetPanningRule(in_ePanningRule, in_idOutput);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetSpeakerAngles(
	AkReal32* io_pfSpeakerAngles,
	AkUInt32& io_uNumAngles,
	AkReal32& out_fHeightAngle,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetSpeakerAngles(io_pfSpeakerAngles, io_uNumAngles, out_fHeightAngle, in_idOutput);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetSpeakerAngles(
	const AkReal32* in_pfSpeakerAngles,
	AkUInt32			in_uNumAngles,
	AkReal32 			in_fHeightAngle,
	AkOutputDeviceID	in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetSpeakerAngles(in_pfSpeakerAngles, in_uNumAngles, in_fHeightAngle, in_idOutput);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetVolumeThreshold(
	AkReal32 in_fVolumeThresholdDB
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetVolumeThreshold(in_fVolumeThresholdDB);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetMaxNumVoicesLimit(
	AkUInt16 in_maxNumberVoices
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMaxNumVoicesLimit(in_maxNumberVoices);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetJobMgrMaxActiveWorkers( 
	AkJobType in_jobType,
	AkUInt32 in_uNewMaxActiveWorkers
	)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetJobMgrMaxActiveWorkers(in_jobType, in_uNewMaxActiveWorkers);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RenderAudio(
	bool in_bAllowSyncRender
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RenderAudio(in_bAllowSyncRender);
}

AK::IAkGlobalPluginContext* FWwiseSoundEngineAPI_2023_1::GetGlobalPluginContext()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetGlobalPluginContext();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterPlugin(
	AkPluginType in_eType,
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulPluginID,
	AkCreatePluginCallback in_pCreateFunc,
	AkCreateParamCallback in_pCreateParamFunc,
	AkGetDeviceListCallback in_pGetDeviceList
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterPlugin(in_eType, in_ulCompanyID, in_ulPluginID, in_pCreateFunc, in_pCreateParamFunc, in_pGetDeviceList);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterPluginDLL(
	const AkOSChar* in_DllName,
	const AkOSChar* in_DllPath
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterPluginDLL(in_DllName, in_DllPath);
}

bool FWwiseSoundEngineAPI_2023_1::IsPluginRegistered(
	AkPluginType in_eType,
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulPluginID
	)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::IsPluginRegistered(in_eType, in_ulCompanyID, in_ulPluginID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterCodec(
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulCodecID,
	AkCreateFileSourceCallback in_pFileCreateFunc,
	AkCreateBankSourceCallback in_pBankCreateFunc
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterCodec(in_ulCompanyID, in_ulCodecID, in_pFileCreateFunc, in_pBankCreateFunc);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterGlobalCallback(
	AkGlobalCallbackFunc in_pCallback,
	AkUInt32 in_eLocation,
	void* in_pCookie,
	AkPluginType in_eType,
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulPluginID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterGlobalCallback(in_pCallback, in_eLocation, in_pCookie, in_eType, in_ulCompanyID, in_ulPluginID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnregisterGlobalCallback(
	AkGlobalCallbackFunc in_pCallback,
	AkUInt32 in_eLocation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnregisterGlobalCallback(in_pCallback, in_eLocation);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterResourceMonitorCallback(
	AkResourceMonitorCallbackFunc in_pCallback
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterResourceMonitorCallback(in_pCallback);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnregisterResourceMonitorCallback(
	AkResourceMonitorCallbackFunc in_pCallback
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnregisterResourceMonitorCallback(in_pCallback);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterAudioDeviceStatusCallback(
	AK::AkDeviceStatusCallbackFunc in_pCallback
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterAudioDeviceStatusCallback(in_pCallback);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnregisterAudioDeviceStatusCallback()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnregisterAudioDeviceStatusCallback();
}

#ifdef AK_SUPPORT_WCHAR
AkUInt32 FWwiseSoundEngineAPI_2023_1::GetIDFromString(const wchar_t* in_pszString)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetIDFromString(in_pszString);
}
#endif //AK_SUPPORT_WCHAR

AkUInt32 FWwiseSoundEngineAPI_2023_1::GetIDFromString(const char* in_pszString)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetIDFromString(in_pszString);
}

AkPlayingID FWwiseSoundEngineAPI_2023_1::PostEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkUInt32 in_cExternals,
	AkExternalSourceInfo* in_pExternalSources,
	AkPlayingID	in_PlayingID
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseSoundEngineAPI_2023_1::PostEvent"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PostEvent(in_eventID, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AkPlayingID FWwiseSoundEngineAPI_2023_1::PostEvent(
	const wchar_t* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkUInt32 in_cExternals,
	AkExternalSourceInfo* in_pExternalSources,
	AkPlayingID	in_PlayingID
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseSoundEngineAPI_2023_1::PostEvent"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PostEvent(in_pszEventName, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AkPlayingID FWwiseSoundEngineAPI_2023_1::PostEvent(
	const char* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkUInt32 in_cExternals,
	AkExternalSourceInfo* in_pExternalSources,
	AkPlayingID	in_PlayingID
)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseSoundEngineAPI_2023_1::PostEvent"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PostEvent(in_pszEventName, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::ExecuteActionOnEvent(
	AkUniqueID in_eventID,
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ExecuteActionOnEvent(in_eventID, in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::ExecuteActionOnEvent(
	const wchar_t* in_pszEventName,
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ExecuteActionOnEvent(in_pszEventName, in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::ExecuteActionOnEvent(
	const char* in_pszEventName,
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ExecuteActionOnEvent(in_pszEventName, in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID);
}

AkPlayingID FWwiseSoundEngineAPI_2023_1::PostMIDIOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkMIDIPost* in_pPosts,
	AkUInt16 in_uNumPosts,
	bool in_bAbsoluteOffsets,
	AkUInt32 in_uFlags,
	AkCallbackFunc in_pfnCallback,
	void* in_pCookie,
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PostMIDIOnEvent(in_eventID, in_gameObjectID, in_pPosts, in_uNumPosts, in_bAbsoluteOffsets, in_uFlags, in_pfnCallback, in_pCookie, in_playingID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::StopMIDIOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::StopMIDIOnEvent(in_eventID, in_gameObjectID, in_playingID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PinEventInStreamCache(
	AkUniqueID in_eventID,
	AkPriority in_uActivePriority,
	AkPriority in_uInactivePriority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PinEventInStreamCache(in_eventID, in_uActivePriority, in_uInactivePriority);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PinEventInStreamCache(
	const wchar_t* in_pszEventName,
	AkPriority in_uActivePriority,
	AkPriority in_uInactivePriority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PinEventInStreamCache(in_pszEventName, in_uActivePriority, in_uInactivePriority);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PinEventInStreamCache(
	const char* in_pszEventName,
	AkPriority in_uActivePriority,
	AkPriority in_uInactivePriority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PinEventInStreamCache(in_pszEventName, in_uActivePriority, in_uInactivePriority);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnpinEventInStreamCache(
	AkUniqueID in_eventID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnpinEventInStreamCache(in_eventID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::UnpinEventInStreamCache(
	const wchar_t* in_pszEventName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnpinEventInStreamCache(in_pszEventName);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::UnpinEventInStreamCache(
	const char* in_pszEventName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnpinEventInStreamCache(in_pszEventName);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetBufferStatusForPinnedEvent(
	AkUniqueID in_eventID,
	AkReal32& out_fPercentBuffered,
	bool& out_bCachePinnedMemoryFull
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetBufferStatusForPinnedEvent(in_eventID, out_fPercentBuffered, out_bCachePinnedMemoryFull);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetBufferStatusForPinnedEvent(
	const char* in_pszEventName,
	AkReal32& out_fPercentBuffered,
	bool& out_bCachePinnedMemoryFull
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetBufferStatusForPinnedEvent(in_pszEventName, out_fPercentBuffered, out_bCachePinnedMemoryFull);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::GetBufferStatusForPinnedEvent(
	const wchar_t* in_pszEventName,
	AkReal32& out_fPercentBuffered,
	bool& out_bCachePinnedMemoryFull
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetBufferStatusForPinnedEvent(in_pszEventName, out_fPercentBuffered, out_bCachePinnedMemoryFull);
}
#endif

AKRESULT FWwiseSoundEngineAPI_2023_1::SeekOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_iPosition,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SeekOnEvent(in_eventID, in_gameObjectID, in_iPosition, in_bSeekToNearestMarker, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SeekOnEvent(
	const wchar_t* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_iPosition,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_iPosition, in_bSeekToNearestMarker, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SeekOnEvent(
	const char* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_iPosition,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_iPosition, in_bSeekToNearestMarker, in_PlayingID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SeekOnEvent(
	AkUniqueID in_eventID,
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fPercent,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SeekOnEvent(in_eventID, in_gameObjectID, in_fPercent, in_bSeekToNearestMarker, in_PlayingID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SeekOnEvent(
	const wchar_t* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fPercent,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_fPercent, in_bSeekToNearestMarker, in_PlayingID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SeekOnEvent(
	const char* in_pszEventName,
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fPercent,
	bool in_bSeekToNearestMarker,
	AkPlayingID in_PlayingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SeekOnEvent(in_pszEventName, in_gameObjectID, in_fPercent, in_bSeekToNearestMarker, in_PlayingID);
}

void FWwiseSoundEngineAPI_2023_1::CancelEventCallbackCookie(
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::CancelEventCallbackCookie(in_pCookie);
}

void FWwiseSoundEngineAPI_2023_1::CancelEventCallbackGameObject(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::CancelEventCallbackGameObject(in_gameObjectID);
}

void FWwiseSoundEngineAPI_2023_1::CancelEventCallback(
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::CancelEventCallback(in_playingID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetSourcePlayPosition(
	AkPlayingID		in_PlayingID,
	AkTimeMs* out_puPosition,
	bool			in_bExtrapolate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetSourcePlayPosition(in_PlayingID, out_puPosition, in_bExtrapolate);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetSourcePlayPositions(
	AkPlayingID		in_PlayingID,
	AkSourcePosition* out_puPositions,
	AkUInt32* io_pcPositions,
	bool			in_bExtrapolate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetSourcePlayPositions(in_PlayingID, out_puPositions, io_pcPositions, in_bExtrapolate);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetSourceStreamBuffering(
	AkPlayingID		in_PlayingID,
	AkTimeMs& out_buffering,
	bool& out_bIsBuffering
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetSourceStreamBuffering(in_PlayingID, out_buffering, out_bIsBuffering);
}

void FWwiseSoundEngineAPI_2023_1::StopAll(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::StopAll(in_gameObjectID);
}

void FWwiseSoundEngineAPI_2023_1::StopPlayingID(
	AkPlayingID in_playingID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::StopPlayingID(in_playingID, in_uTransitionDuration, in_eFadeCurve);
}

void FWwiseSoundEngineAPI_2023_1::ExecuteActionOnPlayingID(
	AK::SoundEngine::AkActionOnEventType in_ActionType,
	AkPlayingID in_playingID,
	AkTimeMs in_uTransitionDuration,
	AkCurveInterpolation in_eFadeCurve
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::ExecuteActionOnPlayingID(in_ActionType, in_playingID, in_uTransitionDuration, in_eFadeCurve);
}

void FWwiseSoundEngineAPI_2023_1::SetRandomSeed(
	AkUInt32 in_uSeed
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::SetRandomSeed(in_uSeed);
}

void FWwiseSoundEngineAPI_2023_1::MuteBackgroundMusic(
	bool in_bMute
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::MuteBackgroundMusic(in_bMute);
}

bool FWwiseSoundEngineAPI_2023_1::GetBackgroundMusicMute()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetBackgroundMusicMute();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SendPluginCustomGameData(
	AkUniqueID in_busID,
	AkGameObjectID in_busObjectID,
	AkPluginType in_eType,
	AkUInt32 in_uCompanyID,
	AkUInt32 in_uPluginID,
	const void* in_pData,
	AkUInt32 in_uSizeInBytes
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SendPluginCustomGameData(in_busID, in_busObjectID, in_eType, in_uCompanyID, in_uPluginID, in_pData, in_uSizeInBytes);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterGameObj(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterGameObj(in_gameObjectID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterGameObj(
	AkGameObjectID in_gameObjectID,
	const char* in_pszObjName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterGameObj(in_gameObjectID, in_pszObjName);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnregisterGameObj(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnregisterGameObj(in_gameObjectID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnregisterAllGameObj(
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnregisterAllGameObj();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetPosition(
	AkGameObjectID in_GameObjectID,
	const AkSoundPosition& in_Position,
	AkSetPositionFlags in_eFlags
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetPosition(in_GameObjectID, in_Position, in_eFlags);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetMultiplePositions(
	AkGameObjectID in_GameObjectID,
	const AkSoundPosition* in_pPositions,
	AkUInt16 in_NumPositions,
	AK::SoundEngine::MultiPositionType in_eMultiPositionType,
	AkSetPositionFlags in_eFlags
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMultiplePositions(in_GameObjectID, in_pPositions, in_NumPositions, in_eMultiPositionType, in_eFlags);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetMultiplePositions(
	AkGameObjectID in_GameObjectID,
	const AkChannelEmitter* in_pPositions,
	AkUInt16 in_NumPositions,
	AK::SoundEngine::MultiPositionType in_eMultiPositionType,
	AkSetPositionFlags in_eFlags
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMultiplePositions(in_GameObjectID, in_pPositions, in_NumPositions, in_eMultiPositionType, in_eFlags);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetScalingFactor(
	AkGameObjectID in_GameObjectID,
	AkReal32 in_fAttenuationScalingFactor
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetScalingFactor(in_GameObjectID, in_fAttenuationScalingFactor);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetDistanceProbe(
	AkGameObjectID in_listenerGameObjectID,
	AkGameObjectID in_distanceProbeGameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetDistanceProbe(in_listenerGameObjectID, in_distanceProbeGameObjectID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::ClearBanks()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ClearBanks();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetBankLoadIOSettings(
	AkReal32            in_fThroughput,
	AkPriority          in_priority
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBankLoadIOSettings(in_fThroughput, in_priority);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBank(
	const wchar_t* in_pszString,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBank(in_pszString, out_bankID, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBank(
	const char* in_pszString,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBank(in_pszString, out_bankID, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBank(
	AkBankID			in_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBank(in_bankID, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryView(
	const void* in_pInMemoryBankPtr,
	AkUInt32 in_uInMemoryBankSize,
	AkBankID& out_bankID)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBankMemoryView(in_pInMemoryBankPtr, in_uInMemoryBankSize, out_bankID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryView(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBankMemoryView(in_pInMemoryBankPtr, in_uInMemoryBankSize, out_bankID, out_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryCopy(
	const void* in_pInMemoryBankPtr,
	AkUInt32 in_uInMemoryBankSize,
	AkBankID& out_bankID)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBankMemoryCopy(in_pInMemoryBankPtr, in_uInMemoryBankSize, out_bankID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryCopy(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBankMemoryCopy(in_pInMemoryBankPtr, in_uInMemoryBankSize, out_bankID, out_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::DecodeBank(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkMemPoolId			in_uPoolForDecodedBank,
	void*& out_pDecodedBankPtr,
	AkUInt32& out_uDecodedBankSize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::DecodeBank(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_uPoolForDecodedBank, out_pDecodedBankPtr, out_uDecodedBankSize);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBank(
	const wchar_t* in_pszString,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBank(in_pszString, in_pfnBankCallback, in_pCookie, out_bankID, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBank(
	const char* in_pszString,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBank(in_pszString, in_pfnBankCallback, in_pCookie, out_bankID, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBank(
	AkBankID			in_bankID,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBank(in_bankID, in_pfnBankCallback, in_pCookie, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryView(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBankMemoryView(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_pfnBankCallback, in_pCookie, out_bankID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryView(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBankMemoryView(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_pfnBankCallback, in_pCookie, out_bankID, out_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryCopy(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID
)
{
	AkBankType bankType;
	
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	auto Result = AK::SoundEngine::LoadBankMemoryCopy(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_pfnBankCallback, in_pCookie, out_bankID, bankType);

	return LIKELY(Result == AK_Success && bankType == AkBankType_User) ? AK_Success :
		LIKELY(Result != AK_Success) ? Result : AK_InvalidBankType;
}

AKRESULT FWwiseSoundEngineAPI_2023_1::LoadBankMemoryCopy(
	const void* in_pInMemoryBankPtr,
	AkUInt32			in_uInMemoryBankSize,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankID& out_bankID,
	AkBankType& out_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::LoadBankMemoryCopy(in_pInMemoryBankPtr, in_uInMemoryBankSize, in_pfnBankCallback, in_pCookie, out_bankID, out_bankType);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::UnloadBank(
	const wchar_t* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::UnloadBank(
	const char* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnloadBank(
	AkBankID            in_bankID,
	const void* in_pInMemoryBankPtr,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnloadBank(in_bankID, in_pInMemoryBankPtr, in_bankType);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::UnloadBank(
	const wchar_t* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_pfnBankCallback, in_pCookie, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::UnloadBank(
	const char* in_pszString,
	const void* in_pInMemoryBankPtr,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnloadBank(in_pszString, in_pInMemoryBankPtr, in_pfnBankCallback, in_pCookie, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnloadBank(
	AkBankID            in_bankID,
	const void* in_pInMemoryBankPtr,
	AkBankCallbackFunc  in_pfnBankCallback,
	void* in_pCookie,
	AkBankType			in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnloadBank(in_bankID, in_pInMemoryBankPtr, in_pfnBankCallback, in_pCookie, in_bankType);
}

void FWwiseSoundEngineAPI_2023_1::CancelBankCallbackCookie(
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::CancelBankCallbackCookie(in_pCookie);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const wchar_t* in_pszString,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_uFlags, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const char* in_pszString,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_uFlags, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkBankID							in_bankID,
	AK::SoundEngine::AkBankContent		in_uFlags,
	AkBankType							in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_bankID, in_uFlags, in_bankType);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const wchar_t* in_pszString,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_pfnBankCallback, in_pCookie, in_uFlags, in_bankType);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareBank(
	AK::SoundEngine::PreparationType	in_PreparationType,
	const char* in_pszString,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_pszString, in_pfnBankCallback, in_pCookie, in_uFlags, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareBank(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkBankID			in_bankID,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie,
	AK::SoundEngine::AkBankContent	in_uFlags,
	AkBankType						in_bankType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareBank(in_PreparationType, in_bankID, in_pfnBankCallback, in_pCookie, in_uFlags, in_bankType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::ClearPreparedEvents()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ClearPreparedEvents();
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const wchar_t** in_ppszString,
	AkUInt32			in_uNumEvent
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const char** in_ppszString,
	AkUInt32			in_uNumEvent
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkUniqueID* in_pEventID,
	AkUInt32			in_uNumEvent
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_pEventID, in_uNumEvent);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const wchar_t** in_ppszString,
	AkUInt32			in_uNumEvent,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent, in_pfnBankCallback, in_pCookie);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	const char** in_ppszString,
	AkUInt32			in_uNumEvent,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_ppszString, in_uNumEvent, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareEvent(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkUniqueID* in_pEventID,
	AkUInt32			in_uNumEvent,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareEvent(in_PreparationType, in_pEventID, in_uNumEvent, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetMedia(
	AkSourceSettings* in_pSourceSettings,
	AkUInt32			in_uNumSourceSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMedia(in_pSourceSettings, in_uNumSourceSettings);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnsetMedia(
	AkSourceSettings* in_pSourceSettings,
	AkUInt32			in_uNumSourceSettings
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnsetMedia(in_pSourceSettings, in_uNumSourceSettings);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::TryUnsetMedia(
	AkSourceSettings* in_pSourceSettings,
	AkUInt32          in_uNumSourceSettings,
	AKRESULT* out_pUnsetResults
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::TryUnsetMedia(in_pSourceSettings, in_uNumSourceSettings, out_pUnsetResults);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareGameSyncs(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkGroupType		in_eGameSyncType,
	const wchar_t* in_pszGroupName,
	const wchar_t** in_ppszGameSyncName,
	AkUInt32		in_uNumGameSyncs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareGameSyncs(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkGroupType		in_eGameSyncType,
	const char* in_pszGroupName,
	const char** in_ppszGameSyncName,
	AkUInt32		in_uNumGameSyncs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareGameSyncs(
	AK::SoundEngine::PreparationType	in_PreparationType,
	AkGroupType		in_eGameSyncType,
	AkUInt32		in_GroupID,
	AkUInt32* in_paGameSyncID,
	AkUInt32		in_uNumGameSyncs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_GroupID, in_paGameSyncID, in_uNumGameSyncs);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareGameSyncs(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkGroupType			in_eGameSyncType,
	const wchar_t* in_pszGroupName,
	const wchar_t** in_ppszGameSyncName,
	AkUInt32			in_uNumGameSyncs,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs, in_pfnBankCallback, in_pCookie);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareGameSyncs(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkGroupType			in_eGameSyncType,
	const char* in_pszGroupName,
	const char** in_ppszGameSyncName,
	AkUInt32			in_uNumGameSyncs,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_pszGroupName, in_ppszGameSyncName, in_uNumGameSyncs, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PrepareGameSyncs(
	AK::SoundEngine::PreparationType		in_PreparationType,
	AkGroupType			in_eGameSyncType,
	AkUInt32			in_GroupID,
	AkUInt32* in_paGameSyncID,
	AkUInt32			in_uNumGameSyncs,
	AkBankCallbackFunc	in_pfnBankCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PrepareGameSyncs(in_PreparationType, in_eGameSyncType, in_GroupID, in_paGameSyncID, in_uNumGameSyncs, in_pfnBankCallback, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetListeners(
	AkGameObjectID in_emitterGameObj,
	const AkGameObjectID* in_pListenerGameObjs,
	AkUInt32 in_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetListeners(in_emitterGameObj, in_pListenerGameObjs, in_uNumListeners);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::AddListener(
	AkGameObjectID in_emitterGameObj,
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::AddListener(in_emitterGameObj, in_listenerGameObj);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RemoveListener(
	AkGameObjectID in_emitterGameObj,
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RemoveListener(in_emitterGameObj, in_listenerGameObj);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetDefaultListeners(
	const AkGameObjectID* in_pListenerObjs,
	AkUInt32 in_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetDefaultListeners(in_pListenerObjs, in_uNumListeners);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::AddDefaultListener(
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::AddDefaultListener(in_listenerGameObj);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RemoveDefaultListener(
	AkGameObjectID in_listenerGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RemoveDefaultListener(in_listenerGameObj);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::ResetListenersToDefault(
	AkGameObjectID in_emitterGameObj
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ResetListenersToDefault(in_emitterGameObj);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetListenerSpatialization(
	AkGameObjectID in_uListenerID,
	bool in_bSpatialized,
	AkChannelConfig in_channelConfig,
	AK::SpeakerVolumes::VectorPtr in_pVolumeOffsets
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetListenerSpatialization(in_uListenerID, in_bSpatialized, in_channelConfig, in_pVolumeOffsets);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetRTPCValue(
	AkRtpcID in_rtpcID,
	AkRtpcValue in_value,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetRTPCValue(in_rtpcID, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetRTPCValue(
	const wchar_t* in_pszRtpcName,
	AkRtpcValue in_value,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetRTPCValue(in_pszRtpcName, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SetRTPCValue(
	const char* in_pszRtpcName,
	AkRtpcValue in_value,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetRTPCValue(in_pszRtpcName, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetRTPCValueByPlayingID(
	AkRtpcID in_rtpcID,
	AkRtpcValue in_value,
	AkPlayingID in_playingID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetRTPCValueByPlayingID(in_rtpcID, in_value, in_playingID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetRTPCValueByPlayingID(
	const wchar_t* in_pszRtpcName,
	AkRtpcValue in_value,
	AkPlayingID in_playingID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetRTPCValueByPlayingID(in_pszRtpcName, in_value, in_playingID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SetRTPCValueByPlayingID(
	const char* in_pszRtpcName,
	AkRtpcValue in_value,
	AkPlayingID in_playingID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetRTPCValueByPlayingID(in_pszRtpcName, in_value, in_playingID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::ResetRTPCValue(
	AkRtpcID in_rtpcID,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ResetRTPCValue(in_rtpcID, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::ResetRTPCValue(
	const wchar_t* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ResetRTPCValue(in_pszRtpcName, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::ResetRTPCValue(
	const char* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkTimeMs in_uValueChangeDuration,
	AkCurveInterpolation in_eFadeCurve,
	bool in_bBypassInternalValueInterpolation
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ResetRTPCValue(in_pszRtpcName, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve, in_bBypassInternalValueInterpolation);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetSwitch(
	AkSwitchGroupID in_switchGroup,
	AkSwitchStateID in_switchState,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetSwitch(in_switchGroup, in_switchState, in_gameObjectID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetSwitch(
	const wchar_t* in_pszSwitchGroup,
	const wchar_t* in_pszSwitchState,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetSwitch(in_pszSwitchGroup, in_pszSwitchState, in_gameObjectID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SetSwitch(
	const char* in_pszSwitchGroup,
	const char* in_pszSwitchState,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetSwitch(in_pszSwitchGroup, in_pszSwitchState, in_gameObjectID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::PostTrigger(
	AkTriggerID 	in_triggerID,
	AkGameObjectID 	in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PostTrigger(in_triggerID, in_gameObjectID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::PostTrigger(
	const wchar_t* in_pszTrigger,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PostTrigger(in_pszTrigger, in_gameObjectID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::PostTrigger(
	const char* in_pszTrigger,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::PostTrigger(in_pszTrigger, in_gameObjectID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetState(
	AkStateGroupID in_stateGroup,
	AkStateID in_state
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetState(in_stateGroup, in_state);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetState(
	const wchar_t* in_pszStateGroup,
	const wchar_t* in_pszState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetState(in_pszStateGroup, in_pszState);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SetState(
	const char* in_pszStateGroup,
	const char* in_pszState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetState(in_pszStateGroup, in_pszState);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetGameObjectAuxSendValues(
	AkGameObjectID		in_gameObjectID,
	AkAuxSendValue* in_aAuxSendValues,
	AkUInt32			in_uNumSendValues
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetGameObjectAuxSendValues(in_gameObjectID, in_aAuxSendValues, in_uNumSendValues);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterBusVolumeCallback(
	AkUniqueID in_busID,
	AkBusCallbackFunc in_pfnCallback,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterBusVolumeCallback(in_busID, in_pfnCallback, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterBusMeteringCallback(
	AkUniqueID in_busID,
	AkBusMeteringCallbackFunc in_pfnCallback,
	AkMeteringFlags in_eMeteringFlags,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterBusMeteringCallback(in_busID, in_pfnCallback, in_eMeteringFlags, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterOutputDeviceMeteringCallback(
	AkOutputDeviceID in_idOutput,
	AkOutputDeviceMeteringCallbackFunc in_pfnCallback,
	AkMeteringFlags in_eMeteringFlags,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterOutputDeviceMeteringCallback(in_idOutput, in_pfnCallback, in_eMeteringFlags, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetGameObjectOutputBusVolume(
	AkGameObjectID		in_emitterObjID,
	AkGameObjectID		in_listenerObjID,
	AkReal32			in_fControlValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetGameObjectOutputBusVolume(in_emitterObjID, in_listenerObjID, in_fControlValue);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetActorMixerEffect(
	AkUniqueID in_audioNodeID,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetActorMixerEffect(in_audioNodeID, in_uFXIndex, in_shareSetID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusEffect(
	AkUniqueID in_audioNodeID,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusEffect(in_audioNodeID, in_uFXIndex, in_shareSetID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusEffect(
	const wchar_t* in_pszBusName,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusEffect(in_pszBusName, in_uFXIndex, in_shareSetID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusEffect(
	const char* in_pszBusName,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusEffect(in_pszBusName, in_uFXIndex, in_shareSetID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetOutputDeviceEffect(
	AkOutputDeviceID in_outputDeviceID,
	AkUInt32 in_uFXIndex,
	AkUniqueID in_FXShareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetOutputDeviceEffect(in_outputDeviceID, in_uFXIndex, in_FXShareSetID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetMixer(
	AkUniqueID in_audioNodeID,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMixer(in_audioNodeID, in_shareSetID);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetMixer(
	const wchar_t* in_pszBusName,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMixer(in_pszBusName, in_shareSetID);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SetMixer(
	const char* in_pszBusName,
	AkUniqueID in_shareSetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMixer(in_pszBusName, in_shareSetID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusConfig(
	AkUniqueID in_audioNodeID,
	AkChannelConfig in_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusConfig(in_audioNodeID, in_channelConfig);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusConfig(
	const wchar_t* in_pszBusName,
	AkChannelConfig in_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusConfig(in_pszBusName, in_channelConfig);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusConfig(
	const char* in_pszBusName,
	AkChannelConfig in_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusConfig(in_pszBusName, in_channelConfig);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetObjectObstructionAndOcclusion(
	AkGameObjectID in_EmitterID,
	AkGameObjectID in_ListenerID,
	AkReal32 in_fObstructionLevel,
	AkReal32 in_fOcclusionLevel
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetObjectObstructionAndOcclusion(in_EmitterID, in_ListenerID, in_fObstructionLevel, in_fOcclusionLevel);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetMultipleObstructionAndOcclusion(
	AkGameObjectID in_EmitterID,
	AkGameObjectID in_uListenerID,
	AkObstructionOcclusionValues* in_fObstructionOcclusionValues,
	AkUInt32 in_uNumOcclusionObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetMultipleObstructionAndOcclusion(in_EmitterID, in_uListenerID, in_fObstructionOcclusionValues, in_uNumOcclusionObstruction);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetContainerHistory(
	AK::IWriteBytes* in_pBytes
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetContainerHistory(in_pBytes);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetContainerHistory(
	AK::IReadBytes* in_pBytes
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetContainerHistory(in_pBytes);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::StartOutputCapture(
	const AkOSChar* in_CaptureFileName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::StartOutputCapture(in_CaptureFileName);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::StopOutputCapture()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::StopOutputCapture();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::AddOutputCaptureMarker(
	const char* in_MarkerText
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::AddOutputCaptureMarker(in_MarkerText);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::AddOutputCaptureBinaryMarker(
	void* in_pMarkerData,
	AkUInt32 in_uMarkerDataSize
	)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::AddOutputCaptureBinaryMarker(in_pMarkerData, in_uMarkerDataSize);
}

AkUInt32 FWwiseSoundEngineAPI_2023_1::GetSampleRate()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetSampleRate();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RegisterCaptureCallback(
	AkCaptureCallbackFunc in_pfnCallback,
	AkOutputDeviceID in_idOutput,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RegisterCaptureCallback(in_pfnCallback, in_idOutput, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::UnregisterCaptureCallback(
	AkCaptureCallbackFunc in_pfnCallback,
	AkOutputDeviceID in_idOutput,
	void* in_pCookie
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::UnregisterCaptureCallback(in_pfnCallback, in_idOutput, in_pCookie);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::StartProfilerCapture(
	const AkOSChar* in_CaptureFileName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::StartProfilerCapture(in_CaptureFileName);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::StopProfilerCapture()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::StopProfilerCapture();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetOfflineRenderingFrameTime(
	AkReal32 in_fFrameTimeInSeconds
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetOfflineRenderingFrameTime(in_fFrameTimeInSeconds);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetOfflineRendering(
	bool in_bEnableOfflineRendering
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetOfflineRendering(in_bEnableOfflineRendering);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::AddOutput(
	const AkOutputSettings& in_Settings,
	AkOutputDeviceID* out_pDeviceID,
	const AkGameObjectID* in_pListenerIDs,
	AkUInt32 in_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::AddOutput(in_Settings, out_pDeviceID, in_pListenerIDs, in_uNumListeners);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::RemoveOutput(
	AkOutputDeviceID in_idOutput
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::RemoveOutput(in_idOutput);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::ReplaceOutput(
	const AkOutputSettings& in_Settings,
	AkOutputDeviceID in_outputDeviceId,
	AkOutputDeviceID* out_pOutputDeviceId
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::ReplaceOutput(in_Settings, in_outputDeviceId, out_pOutputDeviceId);
}

AkOutputDeviceID FWwiseSoundEngineAPI_2023_1::GetOutputID(
	AkUniqueID in_idShareSet,
	AkUInt32 in_idDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetOutputID(in_idShareSet, in_idDevice);
}

AkOutputDeviceID FWwiseSoundEngineAPI_2023_1::GetOutputID(
	const char* in_szShareSet,
	AkUInt32 in_idDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetOutputID(in_szShareSet, in_idDevice);
}

#ifdef AK_SUPPORT_WCHAR
AkOutputDeviceID FWwiseSoundEngineAPI_2023_1::GetOutputID(
	const wchar_t* in_szShareSet,
	AkUInt32 in_idDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetOutputID(in_szShareSet, in_idDevice);
}
#endif

AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusDevice(
	AkUniqueID in_idBus,
	AkUniqueID in_idNewDevice
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusDevice(in_idBus, in_idNewDevice);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusDevice(
	const char* in_BusName,
	const char* in_DeviceName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusDevice(in_BusName, in_DeviceName);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::SetBusDevice(
	const wchar_t* in_BusName,
	const wchar_t* in_DeviceName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetBusDevice(in_BusName, in_DeviceName);
}
#endif

AKRESULT FWwiseSoundEngineAPI_2023_1::GetDeviceList(
	AkUInt32 in_ulCompanyID,
	AkUInt32 in_ulPluginID,
	AkUInt32& io_maxNumDevices,
	AkDeviceDescription* out_deviceDescriptions
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetDeviceList(in_ulCompanyID, in_ulPluginID, io_maxNumDevices, out_deviceDescriptions);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetDeviceList(
	AkUniqueID in_audioDeviceShareSetID,
	AkUInt32& io_maxNumDevices,
	AkDeviceDescription* out_deviceDescriptions
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetDeviceList(in_audioDeviceShareSetID, io_maxNumDevices, out_deviceDescriptions);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::SetOutputVolume(
	AkOutputDeviceID in_idOutput,
	AkReal32 in_fVolume
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::SetOutputVolume(in_idOutput, in_fVolume);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::GetDeviceSpatialAudioSupport(
	AkUInt32 in_idDevice)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetDeviceSpatialAudioSupport(in_idDevice);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::Suspend(
	bool in_bRenderAnyway,
	bool in_bFadeOut
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Suspend(in_bRenderAnyway, in_bFadeOut);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::WakeupFromSuspend(
	AkUInt32 in_uDelayMs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::WakeupFromSuspend(in_uDelayMs);
}

AkUInt32 FWwiseSoundEngineAPI_2023_1::GetBufferTick()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetBufferTick();
}

AkUInt64 FWwiseSoundEngineAPI_2023_1::GetSampleTick()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::GetSampleTick();
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetPosition(
	AkGameObjectID in_GameObjectID,
	AkSoundPosition& out_rPosition
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetPosition(in_GameObjectID, out_rPosition);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetListeners(
	AkGameObjectID in_GameObjectID,
	AkGameObjectID* out_ListenerObjectIDs,
	AkUInt32& oi_uNumListeners
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetListeners(in_GameObjectID, out_ListenerObjectIDs, oi_uNumListeners);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetListenerPosition(
	AkGameObjectID in_uListenerID,
	AkListenerPosition& out_rPosition
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetListenerPosition(in_uListenerID, out_rPosition);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetListenerSpatialization(
	AkGameObjectID in_uListenerID,				///< Listener game object ID.
	bool& out_rbSpatialized,
	AK::SpeakerVolumes::VectorPtr& out_pVolumeOffsets,
	AkChannelConfig& out_channelConfig
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetListenerSpatialization(in_uListenerID, out_rbSpatialized, out_pVolumeOffsets, out_channelConfig);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetRTPCValue(
	AkRtpcID in_rtpcID,
	AkGameObjectID in_gameObjectID,
	AkPlayingID	in_playingID,
	AkRtpcValue& out_rValue,
	AK::SoundEngine::Query::RTPCValue_type& io_rValueType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetRTPCValue(in_rtpcID, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
}

#ifdef AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetRTPCValue(
	const wchar_t* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkPlayingID	in_playingID,
	AkRtpcValue& out_rValue,
	AK::SoundEngine::Query::RTPCValue_type& io_rValueType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetRTPCValue(in_pszRtpcName, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
}

#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetRTPCValue(
	const char* in_pszRtpcName,
	AkGameObjectID in_gameObjectID,
	AkPlayingID	in_playingID,
	AkRtpcValue& out_rValue,
	AK::SoundEngine::Query::RTPCValue_type& io_rValueType
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetRTPCValue(in_pszRtpcName, in_gameObjectID, in_playingID, out_rValue, io_rValueType);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetSwitch(
	AkSwitchGroupID in_switchGroup,
	AkGameObjectID  in_gameObjectID,
	AkSwitchStateID& out_rSwitchState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetSwitch(in_switchGroup, in_gameObjectID, out_rSwitchState);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetSwitch(
	const wchar_t* in_pstrSwitchGroupName,
	AkGameObjectID in_GameObj,
	AkSwitchStateID& out_rSwitchState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetSwitch(in_pstrSwitchGroupName, in_GameObj, out_rSwitchState);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetSwitch(
	const char* in_pstrSwitchGroupName,
	AkGameObjectID in_GameObj,
	AkSwitchStateID& out_rSwitchState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetSwitch(in_pstrSwitchGroupName, in_GameObj, out_rSwitchState);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetState(
	AkStateGroupID in_stateGroup,
	AkStateID& out_rState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetState(in_stateGroup, out_rState);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetState(
	const wchar_t* in_pstrStateGroupName,
	AkStateID& out_rState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetState(in_pstrStateGroupName, out_rState);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetState(
	const char* in_pstrStateGroupName,
	AkStateID& out_rState
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetState(in_pstrStateGroupName, out_rState);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetGameObjectAuxSendValues(
	AkGameObjectID		in_gameObjectID,
	AkAuxSendValue* out_paAuxSendValues,
	AkUInt32& io_ruNumSendValues
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetGameObjectAuxSendValues(in_gameObjectID, out_paAuxSendValues, io_ruNumSendValues);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetGameObjectDryLevelValue(
	AkGameObjectID		in_EmitterID,
	AkGameObjectID		in_ListenerID,
	AkReal32& out_rfControlValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetGameObjectDryLevelValue(in_EmitterID, in_ListenerID, out_rfControlValue);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetObjectObstructionAndOcclusion(
	AkGameObjectID in_EmitterID,
	AkGameObjectID in_ListenerID,
	AkReal32& out_rfObstructionLevel,
	AkReal32& out_rfOcclusionLevel
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetObjectObstructionAndOcclusion(in_EmitterID, in_ListenerID, out_rfObstructionLevel, out_rfOcclusionLevel);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::QueryAudioObjectIDs(
	AkUniqueID in_eventID,
	AkUInt32& io_ruNumItems,
	AkObjectInfo* out_aObjectInfos
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::QueryAudioObjectIDs(in_eventID, io_ruNumItems, out_aObjectInfos);
}

#ifdef AK_SUPPORT_WCHAR
AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::QueryAudioObjectIDs(
	const wchar_t* in_pszEventName,
	AkUInt32& io_ruNumItems,
	AkObjectInfo* out_aObjectInfos
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::QueryAudioObjectIDs(in_pszEventName, io_ruNumItems, out_aObjectInfos);
}
#endif //AK_SUPPORT_WCHAR

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::QueryAudioObjectIDs(
	const char* in_pszEventName,
	AkUInt32& io_ruNumItems,
	AkObjectInfo* out_aObjectInfos
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::QueryAudioObjectIDs(in_pszEventName, io_ruNumItems, out_aObjectInfos);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetPositioningInfo(
	AkUniqueID in_ObjectID,
	AkPositioningInfo& out_rPositioningInfo
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetPositioningInfo(in_ObjectID, out_rPositioningInfo);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetActiveGameObjects(
	FAkGameObjectsList& io_GameObjectList
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::Query::AkGameObjectsList GameObjectsList;
	auto Result = AK::SoundEngine::Query::GetActiveGameObjects(GameObjectsList);
	if (UNLIKELY(Result != AK_Success) || GameObjectsList.IsEmpty())
	{
		return Result;
	}
	io_GameObjectList.Reserve(GameObjectsList.Length());
	for (auto It = GameObjectsList.Begin(); It != GameObjectsList.End(); ++It)
	{
		io_GameObjectList.Add(*It);
	}
	GameObjectsList.Term();
	return Result;
}

bool FWwiseSoundEngineAPI_2023_1::FQuery::GetIsGameObjectActive(
	AkGameObjectID in_GameObjId
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetIsGameObjectActive(in_GameObjId);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetMaxRadius(
	FAkRadiusList& io_RadiusList
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SoundEngine::Query::AkRadiusList RadiusList;
	auto Result = AK::SoundEngine::Query::GetMaxRadius(RadiusList);
	if (UNLIKELY(Result != AK_Success) || RadiusList.IsEmpty())
	{
		return Result;
	}
	io_RadiusList.Reserve(RadiusList.Length());
	for (auto It = RadiusList.Begin(); It != RadiusList.End(); ++It)
	{
		io_RadiusList.Add(*It);
	}
	return Result;
}

AkReal32 FWwiseSoundEngineAPI_2023_1::FQuery::GetMaxRadius(
	AkGameObjectID in_GameObjId
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetMaxRadius(in_GameObjId);
}

AkUniqueID FWwiseSoundEngineAPI_2023_1::FQuery::GetEventIDFromPlayingID(
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetEventIDFromPlayingID(in_playingID);
}

AkGameObjectID FWwiseSoundEngineAPI_2023_1::FQuery::GetGameObjectFromPlayingID(
	AkPlayingID in_playingID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetGameObjectFromPlayingID(in_playingID);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetPlayingIDsFromGameObject(
	AkGameObjectID in_GameObjId,
	AkUInt32& io_ruNumIDs,
	AkPlayingID* out_aPlayingIDs
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetPlayingIDsFromGameObject(in_GameObjId, io_ruNumIDs, out_aPlayingIDs);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetCustomPropertyValue(
	AkUniqueID in_ObjectID,
	AkUInt32 in_uPropID,
	AkInt32& out_iValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetCustomPropertyValue(in_ObjectID, in_uPropID, out_iValue);
}

AKRESULT FWwiseSoundEngineAPI_2023_1::FQuery::GetCustomPropertyValue(
	AkUniqueID in_ObjectID,
	AkUInt32 in_uPropID,
	AkReal32& out_fValue
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SoundEngine::Query::GetCustomPropertyValue(in_ObjectID, in_uPropID, out_fValue);
}

void FWwiseSoundEngineAPI_2023_1::FAudioInputPlugin::SetAudioInputCallbacks(
	AkAudioInputPluginExecuteCallbackFunc in_pfnExecCallback,
	AkAudioInputPluginGetFormatCallbackFunc in_pfnGetFormatCallback /*= nullptr */,
	AkAudioInputPluginGetGainCallbackFunc in_pfnGetGainCallback /*= nullptr*/
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	::SetAudioInputCallbacks(in_pfnExecCallback, in_pfnGetFormatCallback, in_pfnGetGainCallback);
}


#if WITH_EDITORONLY_DATA
FWwiseSoundEngineAPI_2023_1::FErrorTranslator::FErrorTranslator(FGetInfoErrorMessageTranslatorFunction InMessageTranslatorFunction) :
	GetInfoErrorMessageTranslatorFunction(InMessageTranslatorFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
}

bool FWwiseSoundEngineAPI_2023_1::FErrorTranslator::GetInfo(TagInformation* in_pTagList, AkUInt32 in_uCount,
                                              AkUInt32& out_uTranslated)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	if (LIKELY(GetInfoErrorMessageTranslatorFunction))
	{
		return GetInfoErrorMessageTranslatorFunction(in_pTagList, in_uCount, out_uTranslated);
	}
	else
	{
		return false;
	}
}
#endif

AkErrorMessageTranslator* FWwiseSoundEngineAPI_2023_1::NewErrorMessageTranslator(FGetInfoErrorMessageTranslatorFunction InMessageTranslatorFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
#if WITH_EDITORONLY_DATA
	return new FErrorTranslator(InMessageTranslatorFunction);
#else
	return nullptr;
#endif
}
