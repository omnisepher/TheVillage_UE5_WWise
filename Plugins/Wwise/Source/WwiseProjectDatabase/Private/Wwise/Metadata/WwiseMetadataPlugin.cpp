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

#include "Wwise/Metadata/WwiseMetadataPlugin.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataPluginReference::FWwiseMetadataPluginReference(FWwiseMetadataLoader& Loader) :
	Id(Loader.GetUint32(this, TEXT("Id")))
{
	Loader.LogParsed(TEXT("PluginReference"), Id);
}

FWwiseMetadataPluginAttributes::FWwiseMetadataPluginAttributes(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader),
	LibName(Loader.GetString(this, TEXT("LibName"))),
	LibId(Loader.GetUint32(this, TEXT("LibId")))
{
	Loader.LogParsed(TEXT("PluginAttributes"), Id, Name);
}

FWwiseMetadataPlugin::FWwiseMetadataPlugin(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataPluginAttributes(Loader),
	MediaRefs(Loader.GetArray<FWwiseMetadataMediaReference>(this, TEXT("MediaRefs"))),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs")))
{
	Loader.LogParsed(TEXT("Plugin"), Id, Name);
}
