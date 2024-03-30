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

struct WWISEPROJECTDATABASE_API FWwiseMetadataBasicReference : public FWwiseMetadataLoadable
{
	uint32 Id;
	FName Name;
	FName ObjectPath;
	FGuid GUID;

	FWwiseMetadataBasicReference();
	FWwiseMetadataBasicReference(uint32 InId, FName&& InName, FName&& InObjectPath, FGuid&& InGUID) :
		Id(MoveTemp(InId)),
		Name(MoveTemp(InName)),
		ObjectPath(MoveTemp(InObjectPath)),
		GUID(MoveTemp(InGUID))
	{}
	FWwiseMetadataBasicReference(uint32 InId, const FName& InName, const FName& InObjectPath, const FGuid& InGUID) :
		Id(InId),
		Name(InName),
		ObjectPath(InObjectPath),
		GUID(InGUID)
	{}
	FWwiseMetadataBasicReference(FWwiseMetadataLoader& Loader);
};

inline uint32 GetTypeHash(const FWwiseMetadataBasicReference& Ref)
{
	return GetTypeHash(Ref.Id);
}
inline bool operator==(const FWwiseMetadataBasicReference& Lhs, const FWwiseMetadataBasicReference& Rhs)
{
	return Lhs.Id == Rhs.Id;
}
inline bool operator<(const FWwiseMetadataBasicReference& Lhs, const FWwiseMetadataBasicReference& Rhs)
{
	return Lhs.Id < Rhs.Id;
}
