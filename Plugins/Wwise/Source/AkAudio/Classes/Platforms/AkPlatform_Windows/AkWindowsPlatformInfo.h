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

#include "Platforms/AkPlatformInfo.h"
#include "AkWindowsPlatformInfo.generated.h"

UCLASS()
class UAkWin32PlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkWin32PlatformInfo()
	{
		WwisePlatform = "Windows";

#ifdef AK_WINDOWS_VS_VERSION
		Architecture = "Win32_" AK_WINDOWS_VS_VERSION;
#else
		Architecture = "Win32_vc160";
#endif

		LibraryFileNameFormat = "{0}.dll";
		DebugFileNameFormat = "{0}.pdb";
#if WITH_EDITORONLY_DATA
		UAkPlatformInfo::UnrealNameToPlatformInfo.Add("Win32", this);
#endif
	}
};

UCLASS()
class UAkWin64PlatformInfo : public UAkPlatformInfo
{
	GENERATED_BODY()

public:
	UAkWin64PlatformInfo()
	{
		WwisePlatform = "Windows";

#ifdef AK_WINDOWS_VS_VERSION
		Architecture = "x64_" AK_WINDOWS_VS_VERSION;
#else
		Architecture = "x64_vc160";
#endif

		LibraryFileNameFormat = "{0}.dll";
		DebugFileNameFormat = "{0}.pdb";

#if WITH_EDITORONLY_DATA
		UAkPlatformInfo::UnrealNameToPlatformInfo.Add("Win64", this);
#endif
	}
};

UCLASS()
class UAkWindowsPlatformInfo : public UAkWin64PlatformInfo
{
	GENERATED_BODY()
	UAkWindowsPlatformInfo()
	{
#if WITH_EDITORONLY_DATA
		UAkPlatformInfo::UnrealNameToPlatformInfo.Add("Windows", this);
#endif
	}
};
