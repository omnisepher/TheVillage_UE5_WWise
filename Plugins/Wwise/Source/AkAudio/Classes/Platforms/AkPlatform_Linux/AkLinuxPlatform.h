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

#if PLATFORM_LINUX && (!defined(PLATFORM_LINUXARM64) || !PLATFORM_LINUXARM64) && (!defined(PLATFORM_LINUXAARCH64) || !PLATFORM_LINUXAARCH64)

#include "Platforms/AkPlatformBase.h"
#include "AkLinuxInitializationSettings.h"

#define TCHAR_TO_AK(Text) (const ANSICHAR*)(TCHAR_TO_ANSI(Text))

using UAkInitializationSettings = UAkLinuxInitializationSettings;

struct AKAUDIO_API FAkLinuxPlatform : FAkPlatformBase
{
	static const UAkInitializationSettings* GetInitializationSettings()
	{
		return GetDefault<UAkLinuxInitializationSettings>();
	}

	static const FString GetPlatformBasePath()
	{
		return FString("Linux");
	}
};

using FAkPlatform = FAkLinuxPlatform;

#endif
