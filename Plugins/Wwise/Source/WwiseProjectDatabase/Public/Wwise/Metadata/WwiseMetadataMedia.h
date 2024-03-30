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

#include "Wwise/Metadata/WwiseMetadataLoadable.h"

struct WWISEPROJECTDATABASE_API FWwiseMetadataMediaReference : public FWwiseMetadataLoadable
{
	uint32 Id;

	FWwiseMetadataMediaReference(FWwiseMetadataLoader& Loader);
};

inline uint32 GetTypeHash(const FWwiseMetadataMediaReference& Media)
{
	return GetTypeHash(Media.Id);
}
inline bool operator ==(const FWwiseMetadataMediaReference& Lhs, const FWwiseMetadataMediaReference& Rhs)
{
	return Lhs.Id == Rhs.Id;
}
inline bool operator <(const FWwiseMetadataMediaReference& Lhs, const FWwiseMetadataMediaReference& Rhs)
{
	return Lhs.Id < Rhs.Id;
}

enum class EWwiseMetadataMediaLocation : uint32
{
	Memory,
	Loose,
	OtherBank,

	Unknown = (uint32)-1
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataMediaAttributes : public FWwiseMetadataMediaReference
{
	FName Language;
	bool bStreaming;
	EWwiseMetadataMediaLocation Location;
	bool bUsingReferenceLanguage;
	uint32 Align;
	bool bDeviceMemory;

	FWwiseMetadataMediaAttributes(FWwiseMetadataLoader& Loader);

private:
	static EWwiseMetadataMediaLocation LocationFromString(const FName& LocationString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataMedia : public FWwiseMetadataMediaAttributes
{
	FName ShortName;
	FName Path;
	FName CachePath;
	uint32 PrefetchSize;

	FWwiseMetadataMedia(FWwiseMetadataLoader& Loader);
};
