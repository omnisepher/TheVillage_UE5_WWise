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

#include "Platforms/AkPlatformBase.h"
#include "AkAudioDevice.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "WwiseUnrealHelper.h"

FString FAkPlatformBase::GetWwisePluginDirectory()
{
	return FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("Wwise"))->GetBaseDir());
}

FString FAkPlatformBase::GetDSPPluginsDirectory(const FString& PlatformArchitecture)
{
#ifdef WWISE_DSP_DIR
	static constexpr const auto* DspDir = TEXT(WWISE_DSP_DIR);
#elif UE_BUILD_SHIPPING
	static constexpr const auto* DspDir = TEXT("Release");
#elif UE_BUILD_DEBUG
	static constexpr const auto* DspDir = TEXT("Debug");
#else
	static constexpr const auto* DspDir = TEXT("Profile");
#endif

	return WwiseUnrealHelper::GetThirdPartyDirectory() / PlatformArchitecture / DspDir / "bin" / "";
}
