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

#include "AkInclude.h"

#if PLATFORM_ANDROID
#include "AkPlatform_Android/AkAndroidPlatform.h"
#elif PLATFORM_TVOS
#include "AkPlatform_tvOS/AkTVOSPlatform.h"
#elif PLATFORM_IOS && !PLATFORM_TVOS
#include "AkPlatform_iOS/AkIOSPlatform.h"
#elif (defined(PLATFORM_LINUXARM64) && PLATFORM_LINUXARM64) || (defined(PLATFORM_LINUXAARCH64) && PLATFORM_LINUXAARCH64)
#include "AkPlatform_LinuxArm64/AkLinuxArm64Platform.h"
#elif PLATFORM_LINUX
#include "AkPlatform_Linux/AkLinuxPlatform.h"
#elif PLATFORM_MAC
#include "AkPlatform_Mac/AkMacPlatform.h"
#elif defined(PLATFORM_PS4) && PLATFORM_PS4
#include "AkPlatform_PS4/AkPS4Platform.h"
#elif defined(AK_PS5)
#include "AkPlatform_PS5/AkPS5Platform.h"
#elif defined(PLATFORM_SWITCH) && PLATFORM_SWITCH
#include "AkPlatform_Switch/AkSwitchPlatform.h"
#elif defined(AK_WINDOWSGC)
#include "AkPlatform_WinGC/AkWinGCPlatform.h"
#elif PLATFORM_WINDOWS
#include "AkPlatform_Windows/AkWindowsPlatform.h"
#elif ((defined(PLATFORM_XBOXONEGDK) && PLATFORM_XBOXONEGDK) || (defined(PLATFORM_XB1) && PLATFORM_XB1))
#include "AkPlatform_XboxOneGC/AkXboxOneGDKPlatform.h"
#elif defined(PLATFORM_XSX) && PLATFORM_XSX
#include "AkPlatform_XboxSeriesX/AkXboxSeriesXPlatform.h"
#else
#error "The Wwise plug-in does not support the current build platform."
#endif

namespace AkUnrealPlatformHelper
{
	AKAUDIO_API TSet<FString> GetAllSupportedUnrealPlatforms();
	AKAUDIO_API TSet<FString> GetAllSupportedUnrealPlatformsForProject();
	AKAUDIO_API TArray<TSharedPtr<FString> > GetAllSupportedWwisePlatforms(bool ProjectScope = false);
	AKAUDIO_API bool IsEditorPlatform(FString Platform);
}