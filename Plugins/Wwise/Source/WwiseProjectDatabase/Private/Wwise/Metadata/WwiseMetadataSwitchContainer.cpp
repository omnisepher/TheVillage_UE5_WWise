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

#include "Wwise/Metadata/WwiseMetadataSwitchContainer.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataSwitchContainer::FWwiseMetadataSwitchContainer(FWwiseMetadataLoader& Loader) :
	SwitchValue(Loader.GetObject<FWwiseMetadataSwitchValue>(this, TEXT("SwitchValue"))),
	MediaRefs(Loader.GetArray<FWwiseMetadataMediaReference>(this, TEXT("MediaRefs"))),
	ExternalSourceRefs(Loader.GetArray<FWwiseMetadataExternalSourceReference>(this, TEXT("ExternalSourceRefs"))),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs"))),
	Children(Loader.GetArray<FWwiseMetadataSwitchContainer>(this, TEXT("Children")))
{
	Loader.LogParsed(TEXT("SwitchContainer"));
}

TSet<FWwiseMetadataMediaReference> FWwiseMetadataSwitchContainer::GetAllMedia() const
{
	TSet<FWwiseMetadataMediaReference> Result(MediaRefs);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllMedia());
	}
	return Result;
}

TSet<FWwiseMetadataExternalSourceReference> FWwiseMetadataSwitchContainer::GetAllExternalSources() const
{
	TSet<FWwiseMetadataExternalSourceReference> Result(ExternalSourceRefs);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllExternalSources());
	}
	return Result;
}

TSet<FWwiseMetadataPluginReference> FWwiseMetadataSwitchContainer::GetAllCustomPlugins() const
{
	if (!PluginRefs)
	{
		return {};
	}
	TSet<FWwiseMetadataPluginReference> Result(PluginRefs->Custom);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllCustomPlugins());
	}
	return Result;
}

TSet<FWwiseMetadataPluginReference> FWwiseMetadataSwitchContainer::GetAllPluginShareSets() const
{
	if (!PluginRefs)
	{
		return {};
	}
	TSet<FWwiseMetadataPluginReference> Result(PluginRefs->ShareSets);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllPluginShareSets());
	}
	return Result;
}

TSet<FWwiseMetadataPluginReference> FWwiseMetadataSwitchContainer::GetAllAudioDevices() const
{
	if (!PluginRefs)
	{
		return {};
	}
	TSet<FWwiseMetadataPluginReference> Result(PluginRefs->AudioDevices);
	for (const auto& Child : Children)
	{
		Result.Append(Child.GetAllAudioDevices());
	}
	return Result;
}
