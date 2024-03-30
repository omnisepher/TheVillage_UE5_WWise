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

#include "Wwise/WwiseIOHook.h"
#include "Wwise/API/WwiseStreamMgrAPI.h"
#include "Wwise/Stats/FileHandler.h"
#include <inttypes.h>

bool FWwiseIOHook::Init(const AkDeviceSettings& InDeviceSettings)
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHook::Init"));
	auto* StreamMgr = IWwiseStreamMgrAPI::Get();
	if (UNLIKELY(!StreamMgr))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("IOHook: Could not retrieve StreamMgr while Init"));
		return false;
	}

	// If the Stream Manager's File Location Resolver was not set yet, set this object as the 
	// File Location Resolver (this I/O hook is also able to resolve file location).
	if (LIKELY(!StreamMgr->GetFileLocationResolver()))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("IOHook: Setting File Location Resolver"));
		StreamMgr->SetFileLocationResolver(GetLocationResolver());
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("IOHook: Existing File Location Resolver. Not updating."));
	}

	// Create a device in the Stream Manager, specifying this as the hook.
	auto Result = StreamMgr->CreateDevice(InDeviceSettings, GetIOHook(), StreamingDevice);
	UE_CLOG(UNLIKELY(Result != AK_Success || StreamingDevice == AK_INVALID_DEVICE_ID), LogWwiseFileHandler, Error, TEXT("IOHook: CreateDevice failed."));
	UE_CLOG(LIKELY(Result == AK_Success && StreamingDevice != AK_INVALID_DEVICE_ID), LogWwiseFileHandler, Verbose, TEXT("IOHook: CreateDevice = %" PRIu32), StreamingDevice);
	return StreamingDevice != AK_INVALID_DEVICE_ID;
}

void FWwiseIOHook::Term()
{
	SCOPED_WWISEFILEHANDLER_EVENT_2(TEXT("FWwiseIOHook::Term"));
	auto* StreamMgr = IWwiseStreamMgrAPI::Get();
	if (UNLIKELY(!StreamMgr))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("IOHook::Term Could not term StreamMgr"));
		return;
	}

	if (LIKELY(StreamMgr->GetFileLocationResolver() == GetLocationResolver()))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("IOHook::Term Resetting File Location Resolver"));
		StreamMgr->SetFileLocationResolver(nullptr);
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("IOHook::Term Different File Location Resolver. Not setting to null."));
	}

	if (LIKELY(StreamingDevice != AK_INVALID_DEVICE_ID))
	{
		StreamMgr->DestroyDevice(StreamingDevice);
		StreamingDevice = AK_INVALID_DEVICE_ID;
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("IOHook::Term Device Destroyed."));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("IOHook::Term No device to destroy."))
	}
}
