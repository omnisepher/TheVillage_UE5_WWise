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

#include "Wwise/API/WwiseStreamMgrAPI.h"

class WWISESOUNDENGINE_API FWwiseStreamMgrAPI_2023_1 : public IWwiseStreamMgrAPI
{
public:
	UE_NONCOPYABLE(FWwiseStreamMgrAPI_2023_1);
	FWwiseStreamMgrAPI_2023_1() = default;

	AK::IAkStreamMgr* GetAkStreamMgrInstance() override;

	/// \name Audiokinetic implementation-specific Stream Manager factory.
	//@{
	/// Stream Manager factory.
	/// \remarks 
	/// - In order for the Stream Manager to work properly, you also need to create 
	/// at least one streaming device (and implement its I/O hook), and register the 
	/// File Location Resolver with AK::StreamMgr::SetFileLocationResolver().
	/// - Use AK::StreamMgr::GetDefaultSettings(), then modify the settings you want,
	/// then feed this function with them.
	/// \sa 
	/// - AK::IAkStreamMgr
	/// - AK::StreamMgr::SetFileLocationResolver()
	/// - AK::StreamMgr::GetDefaultSettings()
	virtual AK::IAkStreamMgr* Create(
		const AkStreamMgrSettings& in_settings		///< Stream manager initialization settings.
		) override;

	/// Get the default values for the Stream Manager's settings.
	/// \sa 
	/// - AK::StreamMgr::Create()
	/// - AkStreamMgrSettings
	/// - \ref streamingmanager_settings
	virtual void GetDefaultSettings(
		AkStreamMgrSettings& out_settings	///< Returned AkStreamMgrSettings structure with default values.
		) override;

	/// Get the one and only File Location Resolver registered to the Stream Manager.
	/// \sa
	/// - AK::StreamMgr::IAkFileLocationResolver
	/// - AK::StreamMgr::SetFileLocationResolver()
	virtual AK::StreamMgr::IAkFileLocationResolver* GetFileLocationResolver() override;

	/// Register the one and only File Location Resolver to the Stream Manager.
	/// \sa 
	/// - AK::StreamMgr::IAkFileLocationResolver
	virtual void SetFileLocationResolver(
		AK::StreamMgr::IAkFileLocationResolver* in_pFileLocationResolver ///< Interface to your File Location Resolver
		) override;

	//@}

	/// \name Stream Manager: High-level I/O devices management.
	//@{
	/// Streaming device creation.
	/// Creates a high-level device, with specific settings. 
	/// You need to provide the associated low-level I/O hook, implemented on your side.
	/// \return The device ID. AK_INVALID_DEVICE_ID if there was an error and it could not be created.
	/// \warning 
	/// - This function is not thread-safe.
	/// \remarks 
	/// - You may use AK::StreamMgr::GetDefaultDeviceSettings() first to get default values for the 
	/// settings, change those you want, then feed the structure to this function.
	/// - The returned device ID should be kept by the Low-Level IO, to assign it to file descriptors
	/// in AK::StreamMgr::IAkLowLevelIOHook::BatchOpen().
	/// \return
	/// - AK_Success: Device was added to the system properly
	/// - AK_InsufficientMemory: Not enough memory to complete the operation
	/// - AK_InvalidParameter: One of the settings in AkDeviceSettings is out of range. Check asserts or debug console.
	/// \sa
	/// - AK::StreamMgr::IAkLowLevelIOHook
	/// - AK::StreamMgr::GetDefaultDeviceSettings()
	/// - \ref streamingmanager_settings
	virtual AKRESULT CreateDevice(
		const AkDeviceSettings& in_settings,		///< Device settings.
		AK::StreamMgr::IAkLowLevelIOHook* in_pLowLevelHook,		///< Associated low-level I/O hook. Pass either a IAkLowLevelIOHook interface, consistent with the type of the scheduler.
		AkDeviceID& out_idDevice					///< Assigned unique device id to use in all other functions of this interface.
	) override;
	
	/// Streaming device destruction.
	/// \return AK_Success if the device was successfully destroyed.
	/// \warning This function is not thread-safe. No stream should exist for that device when it is destroyed.
	virtual AKRESULT DestroyDevice(
		AkDeviceID					in_deviceID         ///< Device ID of the device to destroy.
		) override;
		
	/// Execute pending I/O operations on all created I/O devices.
	/// This should only be called in single-threaded environments where an I/O device cannot spawn a thread.
	/// \return AK_Success when called from an appropriate environment, AK_NotCompatible otherwise.
	/// \sa 
	/// - AK::StreamMgr::CreateDevice()
	virtual AKRESULT PerformIO() override;

	/// Get the default values for the streaming device's settings. Recommended usage
	/// is to call this function first, then pass the settings to AK::StreamMgr::CreateDevice().
	/// \sa 
	/// - AK::StreamMgr::CreateDevice()
	/// - AkDeviceSettings
	/// - \ref streamingmanager_settings
	virtual void GetDefaultDeviceSettings(
		AkDeviceSettings& out_settings		///< Returned AkDeviceSettings structure with default values.
		) override;
	//@}

	/// \name Language management.
	//@{
	/// Set the current language once and only once, here. The language name is stored in a static buffer 
	/// inside the Stream Manager. In order to resolve localized (language-specific) file location,
	/// AK::StreamMgr::IAkFileLocationResolver implementations query this string. They may use it to 
	/// construct a file path (for e.g. SDK/samples/SoundEngine/Common/AkFileLocationBase.cpp), or to
	/// find a language-specific file within a look-up table (for e.g. SDK/samples/SoundEngine/Common/AkFilePackageLUT.cpp).
	/// Pass a valid null-terminated string, without a trailing slash or backslash. Empty strings are accepted.
	/// You may register for language changes (see RegisterToLanguageChangeNotification()). 
	/// After changing the current language, all observers are notified.
	/// \return AK_Success if successful (if language string has less than AK_MAX_LANGUAGE_NAME_SIZE characters). AK_Fail otherwise.
	/// \warning Not multithread safe.
	/// \sa 
	/// - AK::StreamMgr::GetCurrentLanguage()
	/// - AK::StreamMgr::AddLanguageChangeObserver()
	virtual AKRESULT SetCurrentLanguage(
		const AkOSChar* in_pszLanguageName			///< Language name.
		) override;

	/// Get the current language. The language name is stored in a static buffer inside the Stream Manager, 
	/// with AK::StreamMgr::SetCurrentLanguage(). In order to resolve localized (language-specific) file location,
	/// AK::StreamMgr::IAkFileLocationResolver implementations query this string. They may use it to 
	/// construct a file path (for e.g. SDK/samples/SoundEngine/Common/AkFileLocationBase.cpp), or to
	/// find a language-specific file within a look-up table (for e.g. SDK/samples/SoundEngine/Common/AkFilePackageLUT.cpp).
	/// \return Current language.
	/// \sa AK::StreamMgr::SetCurrentLanguage()
	virtual const AkOSChar* GetCurrentLanguage() override;

	/// Register to language change notifications.
	/// \return AK_Success if successful, AK_Fail otherwise (no memory or no cookie).
	/// \warning Not multithread safe.
	/// \sa 
	/// - AK::StreamMgr::SetCurrentLanguage()
	/// - AK::StreamMgr::RemoveLanguageChangeObserver()
	virtual AKRESULT AddLanguageChangeObserver(
		AK::StreamMgr::AkLanguageChangeHandler in_handler,	///< Callback function.
		void* in_pCookie					///< Cookie, passed back to AkLanguageChangeHandler. Must set.
		) override;

	/// Unregister to language change notifications. Use the cookie you have passed to 
	/// AddLanguageChangeObserver() to identify the observer.
	/// \warning Not multithread safe.
	/// \sa 
	/// - AK::StreamMgr::SetCurrentLanguage()
	/// - AK::StreamMgr::AddLanguageChangeObserver()
	virtual void RemoveLanguageChangeObserver(
		void* in_pCookie					///< Cookie that was passed to AddLanguageChangeObserver().
		) override;

	/// \name Stream Manager: Cache management.
	//@{
	/// Flush cache of all devices. This function has no effect for devices where
	/// AkDeviceSettings::bUseStreamCache was set to false (no caching).
	/// \sa
	/// - \ref streamingmanager_settings
	virtual void FlushAllCaches() override;

	//@}
};
