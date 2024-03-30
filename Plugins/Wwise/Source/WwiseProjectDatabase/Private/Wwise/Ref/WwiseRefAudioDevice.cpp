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

#include "Wwise/Ref/WwiseRefAudioDevice.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefAudioDevice::NAME = TEXT("AudioDevice");

const FWwiseMetadataPlugin* FWwiseRefAudioDevice::GetPlugin() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank || !SoundBank->Plugins))
	{
		return nullptr;
	}

	const auto& Plugins = SoundBank->Plugins->AudioDevices;
	if (Plugins.IsValidIndex(AudioDeviceIndex))
	{
		return &Plugins[AudioDeviceIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Audio Device index #%zu"), AudioDeviceIndex);
		return nullptr;
	}
}

WwiseMediaIdsMap FWwiseRefAudioDevice::GetPluginMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const
{
	const auto* AudioDevice = GetPlugin();
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!AudioDevice || !SoundBank))
	{
		return {};
	}
	const auto& Media = AudioDevice->MediaRefs;

	WwiseMediaIdsMap Result;
	Result.Empty(Media.Num());
	for (const auto& Elem : Media)
	{
		FWwiseDatabaseMediaIdKey Id(Elem.Id, SoundBank->Id);

		const auto* MediaInGlobalMap = GlobalMap.Find(Id);
		if (MediaInGlobalMap)
		{
			Result.Add(Elem.Id, *MediaInGlobalMap);
		}
	}
	return Result;
}

uint32 FWwiseRefAudioDevice::AudioDeviceId() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return 0;
	}
	return AudioDevice->Id;
}

FGuid FWwiseRefAudioDevice::AudioDeviceGuid() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return {};
	}
	return AudioDevice->GUID;
}

FName FWwiseRefAudioDevice::AudioDeviceName() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return {};
	}
	return AudioDevice->Name;
}

FName FWwiseRefAudioDevice::AudioDeviceObjectPath() const
{
	const auto* AudioDevice = GetPlugin();
	if (UNLIKELY(!AudioDevice))
	{
		return {};
	}
	return AudioDevice->ObjectPath;
}

uint32 FWwiseRefAudioDevice::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(AudioDeviceIndex));
	return Result;
}
