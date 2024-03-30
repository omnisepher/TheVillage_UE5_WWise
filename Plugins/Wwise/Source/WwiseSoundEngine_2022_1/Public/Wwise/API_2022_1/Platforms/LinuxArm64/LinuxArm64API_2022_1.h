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

#include "Wwise/API/Platforms/LinuxArm64/LinuxArm64API.h"

#if defined(PLATFORM_LINUXARM64) && PLATFORM_LINUXARM64

class FWwisePlatformAPI_2022_1_LinuxArm64 : public IWwisePlatformAPI
{
public:
	UE_NONCOPYABLE(FWwisePlatformAPI_2022_1_LinuxArm64);
	FWwisePlatformAPI_2022_1_LinuxArm64() = default;
	virtual ~FWwisePlatformAPI_2022_1_LinuxArm64() override {}
};

using FWwisePlatformAPI = FWwisePlatformAPI_2022_1_LinuxArm64;

#endif
