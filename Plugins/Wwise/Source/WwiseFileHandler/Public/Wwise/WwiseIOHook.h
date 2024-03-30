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

#include "Wwise/WwiseLowLevelIOHook.h"
#include "Wwise/WwiseFileLocationResolver.h"

class WWISEFILEHANDLER_API FWwiseIOHook
{
public:
	virtual ~FWwiseIOHook() {}

	virtual bool Init(const AkDeviceSettings& InDeviceSettings);
	virtual void Term();
	virtual AK::StreamMgr::IAkLowLevelIOHook* GetIOHook() = 0;
	virtual AK::StreamMgr::IAkFileLocationResolver* GetLocationResolver() = 0;

protected:
	FWwiseIOHook() :
		StreamingDevice(AK_INVALID_DEVICE_ID)
	{}

	AkDeviceID StreamingDevice;
};

class WWISEFILEHANDLER_API FWwiseDefaultIOHook :
	public FWwiseIOHook,
	public FWwiseLowLevelIOHook,
	public FWwiseFileLocationResolver
{
public:
	AK::StreamMgr::IAkLowLevelIOHook* GetIOHook() final
	{
		return this;
	}
	AK::StreamMgr::IAkFileLocationResolver* GetLocationResolver() final
	{
		return this;
	}

protected:
	FWwiseDefaultIOHook() {}
};