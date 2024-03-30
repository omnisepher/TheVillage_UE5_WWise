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

#include "Wwise/Metadata/WwiseMetadataBasicReference.h"
#include "Wwise/Metadata/WwiseMetadataMedia.h"

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginReference : public FWwiseMetadataLoadable
{
	uint32 Id;

	FWwiseMetadataPluginReference(FWwiseMetadataLoader& Loader);
};

inline uint32 GetTypeHash(const FWwiseMetadataPluginReference& Plugin)
{
	return GetTypeHash(Plugin.Id);
}
inline bool operator ==(const FWwiseMetadataPluginReference& Lhs, const FWwiseMetadataPluginReference& Rhs)
{
	return Lhs.Id == Rhs.Id;
}
inline bool operator <(const FWwiseMetadataPluginReference& Lhs, const FWwiseMetadataPluginReference& Rhs)
{
	return Lhs.Id < Rhs.Id;
}

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginAttributes : public FWwiseMetadataBasicReference
{
	FName LibName;
	uint32 LibId;

	FWwiseMetadataPluginAttributes(FWwiseMetadataLoader& Loader);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPlugin : public FWwiseMetadataPluginAttributes
{
	TArray<FWwiseMetadataMediaReference> MediaRefs;
	FWwiseMetadataPluginReferenceGroup* PluginRefs;

	FWwiseMetadataPlugin(FWwiseMetadataLoader& Loader);
};
