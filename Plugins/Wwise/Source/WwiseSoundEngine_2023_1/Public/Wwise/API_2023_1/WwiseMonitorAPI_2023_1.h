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

#include "Wwise/API/WwiseMonitorAPI.h"

class WWISESOUNDENGINE_API FWwiseMonitorAPI_2023_1 : public IWwiseMonitorAPI
{
public:
	UE_NONCOPYABLE(FWwiseMonitorAPI_2023_1);
	FWwiseMonitorAPI_2023_1() = default;

	/// Post a monitoring message or error code. This will be displayed in the Wwise capture
	/// log. Since this function doesn't send variable arguments, be sure that the error code you're posting doesn't contain any tag.
	/// Otherwise, there'll be an undefined behavior
	/// \return AK_Success if successful, AK_Fail if there was a problem posting the message.
	///			In optimized mode, this function returns AK_NotCompatible.
	/// \remark This function is provided as a tracking tool only. It does nothing if it is 
	///			called in the optimized/release configuration and return AK_NotCompatible.
	virtual AKRESULT PostCode(
		AK::Monitor::ErrorCode in_eError,		///< Message or error code to be displayed
		AK::Monitor::ErrorLevel in_eErrorLevel,	///< Specifies whether it should be displayed as a message or an error
		AkPlayingID in_playingID = AK_INVALID_PLAYING_ID,   ///< Related Playing ID if applicable
		AkGameObjectID in_gameObjID = AK_INVALID_GAME_OBJECT, ///< Related Game Object ID if applicable, AK_INVALID_GAME_OBJECT otherwise
		AkUniqueID in_audioNodeID = AK_INVALID_UNIQUE_ID, ///< Related Audio Node ID if applicable, AK_INVALID_UNIQUE_ID otherwise
		bool in_bIsBus = false		///< true if in_audioNodeID is a bus
		) override;

	virtual AKRESULT PostCodeVarArg(
		AK::Monitor::ErrorCode in_eError,		///< Error code to be displayed. This code corresponds to a predefined message, that may have parameters that can be passed in the variable arguments. Check the message format at the end of AkMonitorError.h.
		AK::Monitor::ErrorLevel in_eErrorLevel,	///< Specifies whether it should be displayed as a message or an error
		AK::Monitor::MsgContext msgContext,		///< The message context containing the following information : Related Playing ID if applicable, Related Game Object ID if applicable, AK_INVALID_GAME_OBJECT otherwise,  Related Audio Node ID if applicable, AK_INVALID_UNIQUE_ID otherwise and whether if in_audioNodeID is a bus
		...							///< The variable arguments, depending on the ErrorCode posted.
		) override;

	/// Post a monitoring message. This will be displayed in the Wwise capture log.
	/// \return AK_Success if successful, AK_Fail if there was a problem posting the message.
	///			In optimized mode, this function returns AK_NotCompatible.
	/// \remark This function is provided as a tracking tool only. It does nothing if it is 
	///			called in the optimized/release configuration and return AK_NotCompatible.
	virtual AKRESULT PostCodeVaList(
		AK::Monitor::ErrorCode in_eError,		///< Error code to be displayed. This code corresponds to a predefined message, that may have parameters that can be passed in the variable arguments. Check the message format at the end of AkMonitorError.h.
		AK::Monitor::ErrorLevel in_eErrorLevel,	///< Specifies whether it should be displayed as a message or an error
		AK::Monitor::MsgContext msgContext,		///< The message context containing the following information : Related Playing ID if applicable, Related Game Object ID if applicable, AK_INVALID_GAME_OBJECT otherwise,  Related Audio Node ID if applicable, AK_INVALID_UNIQUE_ID otherwise and whether if in_audioNodeID is a bus
		::va_list args				///< The variable arguments, depending on the ErrorCode posted.
		) override;

#ifdef AK_SUPPORT_WCHAR
	/// Post a unicode monitoring message or error string. This will be displayed in the Wwise capture
	/// log.
	/// \return AK_Success if successful, AK_Fail if there was a problem posting the message.
	///			In optimized mode, this function returns AK_NotCompatible.
	/// \remark This function is provided as a tracking tool only. It does nothing if it is 
	///			called in the optimized/release configuration and return AK_NotCompatible.
	virtual AKRESULT PostString(
		const wchar_t* in_pszError,	///< Message or error string to be displayed
		AK::Monitor::ErrorLevel in_eErrorLevel,	///< Specifies whether it should be displayed as a message or an error
		AkPlayingID in_playingID = AK_INVALID_PLAYING_ID,   ///< Related Playing ID if applicable
		AkGameObjectID in_gameObjID = AK_INVALID_GAME_OBJECT, ///< Related Game Object ID if applicable, AK_INVALID_GAME_OBJECT otherwise
		AkUniqueID in_audioNodeID = AK_INVALID_UNIQUE_ID, ///< Related Audio Node ID if applicable, AK_INVALID_UNIQUE_ID otherwise
		bool in_bIsBus = false		///< true if in_audioNodeID is a bus
		) override;

#endif // #ifdef AK_SUPPORT_WCHAR

	/// Post a monitoring message or error string. This will be displayed in the Wwise capture
	/// log.
	/// \return AK_Success if successful, AK_Fail if there was a problem posting the message.
	///			In optimized mode, this function returns AK_NotCompatible.
	/// \remark This function is provided as a tracking tool only. It does nothing if it is 
	///			called in the optimized/release configuration and return AK_NotCompatible.
	virtual AKRESULT PostString(
		const char* in_pszError,	///< Message or error string to be displayed
		AK::Monitor::ErrorLevel in_eErrorLevel,	///< Specifies whether it should be displayed as a message or an error
		AkPlayingID in_playingID = AK_INVALID_PLAYING_ID,   ///< Related Playing ID if applicable
		AkGameObjectID in_gameObjID = AK_INVALID_GAME_OBJECT, ///< Related Game Object ID if applicable, AK_INVALID_GAME_OBJECT otherwise
		AkUniqueID in_audioNodeID = AK_INVALID_UNIQUE_ID, ///< Related Audio Node ID if applicable, AK_INVALID_UNIQUE_ID otherwise
		bool in_bIsBus = false		///< true if in_audioNodeID is a bus
		) override;

	/// Enable/Disable local output of monitoring messages or errors. Pass 0 to disable,
	/// or any combination of ErrorLevel_Message and ErrorLevel_Error to enable. 
	/// \return AK_Success.
	///			In optimized/release configuration, this function returns AK_NotCompatible.
	virtual AKRESULT SetLocalOutput(
		AkUInt32 in_uErrorLevel = AK::Monitor::ErrorLevel_All, ///< ErrorLevel(s) to enable in output. Default parameters enable all.
		AK::Monitor::LocalOutputFunc in_pMonitorFunc = 0 	  ///< Handler for local output. If NULL, the standard platform debug output method is used.
		) override;

	/// Add a translator to the wwiseErrorHandler
	/// The additional translators increase the chance of a monitoring messages or errors
	/// to be successfully translated.
	/// \return AK_Success.
	///	In optimized/release configuration, this function returns AK_NotCompatible.
	virtual AKRESULT AddTranslator(
		AkErrorMessageTranslator* translator,	///< The AkErrorMessageTranslator to add to the WwiseErrorHandler
		bool overridePreviousTranslators = false		///< Whether or not the newly added translator should override all the previous translators. 
														///< In both cases, the default translator will remain
		) override;

	/// Reset the wwiseErrorHandler to only using the default translator
	/// \return AK_Success.
	///	In optimized/release configuration, this function returns AK_NotCompatible.
	virtual AKRESULT ResetTranslator(
		) override;

	/// Get the time stamp shown in the capture log along with monitoring messages.
	/// \return Time stamp in milliseconds.
	///			In optimized/release configuration, this function returns 0.
	virtual AkTimeMs GetTimeStamp() override;

	/// Add the streaming manager settings to the profiler capture.
	virtual void MonitorStreamMgrInit(
		const AkStreamMgrSettings& in_streamMgrSettings
		) override;

	/// Add device settings to the list of active streaming devices.
	/// The list of streaming devices and their settings will be 
	/// sent to the profiler capture when remote connecting from Wwise.
	/// 
	/// \remark \c AK::Monitor::MonitorStreamMgrTerm must be called to
	///			clean-up memory	used to keep track of active streaming devices.
	virtual void MonitorStreamingDeviceInit(
		AkDeviceID in_deviceID,
		const AkDeviceSettings& in_deviceSettings
		) override;

	/// Remove streaming device entry from the list of devices
	/// to send when remote connecting from Wwise.
	virtual void MonitorStreamingDeviceDestroyed(
		AkDeviceID in_deviceID
		) override;

	/// Monitor streaming manager destruction as part of the
	/// profiler capture.
	/// 
	/// \remark This function must be called to clean-up memory	used by
	///			\c AK::Monitor::MonitorStreamingDeviceInit and \c AK::Monitor::MonitorStreamingDeviceTerm
	/// 		to keep track of active streaming devices.
	virtual void MonitorStreamMgrTerm() override;

	/// Add the default, WwiseSDK-provided WAAPI error translator.
	virtual void SetupDefaultWAAPIErrorTranslator(
		const FString& WaapiIP, ///< IP Address of the WAAPI server
		AkUInt32 WaapiPort, ///< Port of the WAAPI server
		AkUInt32 Timeout ///< Maximum time that can be spent resolving the error parameters. Set to INT_MAX to wait infinitely or 0 to disable XML translation entirely.
		) override;

	/// Terminate the default, WwiseSDK-provided WAAPI error translator.
	virtual void TerminateDefaultWAAPIErrorTranslator() override;
private:
#if AK_SUPPORT_WAAPI && WITH_EDITORONLY_DATA && !defined(AK_OPTIMIZED)
	static AkWAAPIErrorMessageTranslator WaapiErrorMessageTranslator;
#endif
};
