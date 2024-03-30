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

#include "Wwise/CookedData/WwiseGroupValueCookedData.h"

#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>

FWwiseGroupValueCookedData::FWwiseGroupValueCookedData():
	Type(EWwiseGroupType::Unknown),
	GroupId(0),
	Id(0)
{}

const FString& FWwiseGroupValueCookedData::GetTypeName() const
{
	static const FString StateName = TEXT("State");
	static const FString SwitchName = TEXT("Switch");
	static const FString UnknownName = TEXT("Unknown");

	switch(Type)
	{
	case EWwiseGroupType::State:
		return StateName;
	case EWwiseGroupType::Switch:
		return SwitchName;
	case EWwiseGroupType::Unknown:
	default:
		return UnknownName;
	}
}

void FWwiseGroupValueCookedData::Serialize(FArchive& Ar)
{
	UStruct* Struct = StaticStruct();
	check(Struct);

	if (Ar.WantBinaryPropertySerialization())
	{
		Struct->SerializeBin(Ar, this);
	}
	else
	{
		Struct->SerializeTaggedProperties(Ar, (uint8*)this, Struct, nullptr);
	}
}

FString FWwiseGroupValueCookedData::GetDebugString() const
{
	return FString::Printf(TEXT("%s %s (%" PRIu32 ":%" PRIu32 ")"), *GetTypeName(), *DebugName.ToString(), GroupId, Id);
}
