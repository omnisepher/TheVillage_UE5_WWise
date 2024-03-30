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

#include "Wwise/CookedData/WwiseSwitchContainerLeafCookedData.h"

#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>

FWwiseSwitchContainerLeafCookedData::FWwiseSwitchContainerLeafCookedData():
	GroupValueSet(),
	SoundBanks(),
	Media(),
	ExternalSources()
{}

void FWwiseSwitchContainerLeafCookedData::Serialize(FArchive& Ar)
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

bool FWwiseSwitchContainerLeafCookedData::operator==(const FWwiseSwitchContainerLeafCookedData& Rhs) const
{
	if (GroupValueSet.Num() != Rhs.GroupValueSet.Num() ||
		Media.Num() != Rhs.Media.Num() ||
		ExternalSources.Num() != Rhs.ExternalSources.Num() ||
		SoundBanks.Num() != Rhs.SoundBanks.Num() ||
		GroupValueSet.Difference(Rhs.GroupValueSet).Num() != 0)
	{
		return false;
	}
	
	for (int i = 0; i < Media.Num(); ++i)
	{
		if (Media[i] != Rhs.Media[i])
		{
			return false;
		}
	}
	for (int i = 0;i < ExternalSources.Num(); ++i)
	{
		if (ExternalSources[i] != Rhs.ExternalSources[i])
		{
			return false;
		}
	}
	for (int i = 0; i < SoundBanks.Num(); ++i)
	{
		if (SoundBanks[i] != Rhs.SoundBanks[i])
		{
			return false;
		}
	}
	return true;
}

FString FWwiseSwitchContainerLeafCookedData::GetDebugString() const
{
	FString Result = TEXT("[");
	bool Add = false;
	for (const auto& GroupValue : GroupValueSet)
	{
		if (Add)
		{
			Result += TEXT(",");
		}
		else
		{
			Add = true;
		}
		Result += GroupValue.GetDebugString();
	}
	Result += TEXT("]");
	return Result;
}
