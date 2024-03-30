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

#include "Wwise/Metadata/WwiseMetadataSwitchValue.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataSwitchValueAttributes::FWwiseMetadataSwitchValueAttributes()
{
}

FWwiseMetadataSwitchValueAttributes::FWwiseMetadataSwitchValueAttributes(FWwiseMetadataLoader& Loader) :
	GroupType(GroupTypeFromString(Loader.GetString(this, TEXT("GroupType")))),
	GroupId(Loader.GetUint32(this, TEXT("GroupId"))),
	Id(Loader.GetUint32(this, TEXT("Id"))),
	GUID(Loader.GetGuid(this, TEXT("GUID"))),
	bDefault(Loader.GetBool(this, TEXT("Default"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("SwitchValueAttributes"));
}

EWwiseMetadataSwitchValueGroupType FWwiseMetadataSwitchValueAttributes::GroupTypeFromString(const FName& TypeString)
{
	if (TypeString == "Switch")
	{
		return EWwiseMetadataSwitchValueGroupType::Switch;
	}
	else if (TypeString == "State")
	{
		return EWwiseMetadataSwitchValueGroupType::State;
	}
	UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataSwitchValueAttributes: Unknown GroupType: %s"), *TypeString.ToString());
	return EWwiseMetadataSwitchValueGroupType::Unknown;
}

FWwiseMetadataSwitchValue::FWwiseMetadataSwitchValue()
{
}

FWwiseMetadataSwitchValue::FWwiseMetadataSwitchValue(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataSwitchValueAttributes(Loader)
{
	Loader.LogParsed(TEXT("SwitchValue"));
}
