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

#include "Wwise/API/Platforms/Windows/WindowsAPI.h"

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS && (!defined(PLATFORM_WINGDK) || !PLATFORM_WINGDK)

class FWwisePlatformAPI_2023_1_Windows : public IWwisePlatformAPI
{
public:
	UE_NONCOPYABLE(FWwisePlatformAPI_2023_1_Windows);
	FWwisePlatformAPI_2023_1_Windows() = default;
	virtual ~FWwisePlatformAPI_2023_1_Windows() override {}

	/// Finds the device ID for particular Audio Endpoint. 
	/// \note CoInitialize must have been called for the calling thread.  See Microsoft's documentation about CoInitialize for more details.
	/// \return A device ID to use with AkPlatformInitSettings.idAudioDevice
	virtual AkUInt32 GetDeviceID(IMMDevice* in_pDevice) override;

	/// Finds an audio endpoint that matches the token in the device name or device ID and returns an ID.  
	/// This is a helper function that searches in the device ID (as returned by IMMDevice->GetId) and 
	/// in the property PKEY_Device_FriendlyName.  The token parameter is case-sensitive.  If you need to do matching on different conditions, use IMMDeviceEnumerator directly and AK::GetDeviceID.
	/// \note CoInitialize must have been called for the calling thread.  See Microsoft's documentation about CoInitialize for more details.
	/// \return The device ID as returned by IMMDevice->GetId, hashed by AK::SoundEngine::GetIDFromName()
	virtual AkUInt32 GetDeviceIDFromName(wchar_t* in_szToken) override;

	/// Get the user-friendly name for the specified device.  Call repeatedly with index starting at 0 and increasing to get all available devices, including disabled and unplugged devices, until the returned string is null and out_uDeviceID is AK_INVALID_DEVICE_ID.
	/// The number of indexable devices for the given uDeviceStateMask can be fetched by calling AK::SoundEngine::GetWindowsDeviceCount().
	/// You can also get the default device information by specifying index=-1.  The default device is the one with a green checkmark in the Audio Playback Device panel in Windows.
	/// The returned out_uDeviceID parameter is the Device ID to use with Wwise.  Use it to specify the main device in AkPlatformInitSettings.idAudioDevice. 
	/// \note CoInitialize must have been called for the calling thread.  See Microsoft's documentation about CoInitialize for more details.
	/// \return The name of the device at the "index" specified.  The pointer is valid until the next call to GetWindowsDeviceName.
	virtual const wchar_t* GetWindowsDeviceName(
		AkInt32 index,			 ///< Index of the device in the array.  -1 to get information on the default device.
		AkUInt32 &out_uDeviceID, ///< Device ID for Wwise.  This is the same as what is returned from AK::GetDeviceID and AK::GetDeviceIDFromName.  Use it to specify the main device in AkPlatformInitSettings.idAudioDevice. 
		AkAudioDeviceState uDeviceStateMask = AkDeviceState_All ///< Optional bitmask used to filter the device based on their state.
		) override;

	/// Get the number of Audio Endpoints available for the specified device state mask.
	/// \note CoInitialize must have been called for the calling thread.  See Microsoft's documentation about CoInitialize for more details.
	/// \return The number of Audio Endpoints available for the specified device state mask.
	virtual AkUInt32 GetWindowsDeviceCount(
		AkAudioDeviceState uDeviceStateMask = AkDeviceState_All ///< Optional bitmask used to filter the device based on their state.
		) override;

	/// Get the Audio Endpoint for the specified device index.  Call repeatedly with index starting at 0 and increasing to get all available devices, including disabled and unplugged devices, until the false is returned.
	/// You can also get the default device information by specifying index=-1.  The default device is the one with a green checkmark in the Audio Playback Device panel in Windows.
	/// The returned out_uDeviceID parameter is the Device ID to use with Wwise.  Use it to specify the main device in AkPlatformInitSettings.idAudioDevice. 
	/// The returned out_ppDevice is a pointer to a pointer variable to which the method writes the address of the IMMDevice. out_ppDevice is optional; if it is null, then no action is taken. 
	/// If the method returns false, *out_ppDevice is null and out_uDeviceID is AK_INVALID_DEVICE_ID. If the method successed, *out_ppDevice will be a counted reference to the interface, and the caller is responsible for releasing the interface when it is no longer needed, by calling Release(), or encapsulating the device in a COM Smart Pointer.
	/// \note CoInitialize must have been called for the calling thread.  See Microsoft's documentation about CoInitialize for more details.
	/// \return Whether or not a device was found at the given index.
	virtual bool GetWindowsDevice(
		AkInt32 in_index,			///< Index of the device in the array.  -1 to get information on the default device.
		AkUInt32& out_uDeviceID,	///< Device ID for Wwise.  This is the same as what is returned from AK::GetDeviceID and AK::GetDeviceIDFromName.  Use it to specify the main device in AkPlatformInitSettings.idAudioDevice. 
		IMMDevice** out_ppDevice,	///< pointer to a pointer variable to which the method writes the address of the IMMDevice in question.
		AkAudioDeviceState uDeviceStateMask = AkDeviceState_All ///< Optional bitmask used to filter the device based on their state.
		) override;
};

using FWwisePlatformAPI = FWwisePlatformAPI_2023_1_Windows;

#endif
