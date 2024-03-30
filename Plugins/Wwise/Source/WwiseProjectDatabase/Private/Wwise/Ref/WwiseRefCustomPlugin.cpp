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

#include "Wwise/Ref/WwiseRefCustomPlugin.h"

#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefCustomPlugin::NAME = TEXT("CustomPlugin");

const FWwiseMetadataPlugin* FWwiseRefCustomPlugin::GetPlugin() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank || !SoundBank->Plugins))
	{
		return nullptr;
	}

	const auto& Plugins = SoundBank->Plugins->Custom;
	if (Plugins.IsValidIndex(CustomPluginIndex))
	{
		return &Plugins[CustomPluginIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Custom Plugin index #%zu"), CustomPluginIndex);
		return nullptr;
	}
}

WwiseMediaIdsMap FWwiseRefCustomPlugin::GetPluginMedia(const WwiseMediaGlobalIdsMap& GlobalMap) const
{
	const auto* CustomPlugin = GetPlugin();
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!CustomPlugin || !SoundBank))
	{
		return {};
	}
	const auto& Media = CustomPlugin->MediaRefs;

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

uint32 FWwiseRefCustomPlugin::CustomPluginId() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return 0;
	}
	return CustomPlugin->Id;
}

FGuid FWwiseRefCustomPlugin::CustomPluginGuid() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return {};
	}
	return CustomPlugin->GUID;
}

FName FWwiseRefCustomPlugin::CustomPluginName() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return {};
	}
	return CustomPlugin->Name;
}

FName FWwiseRefCustomPlugin::CustomPluginObjectPath() const
{
	const auto* CustomPlugin = GetPlugin();
	if (UNLIKELY(!CustomPlugin))
	{
		return {};
	}
	return CustomPlugin->ObjectPath;
}

uint32 FWwiseRefCustomPlugin::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(CustomPluginIndex));
	return Result;
}
