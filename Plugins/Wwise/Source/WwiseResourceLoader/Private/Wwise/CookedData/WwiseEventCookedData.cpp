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

#include "Wwise/CookedData/WwiseEventCookedData.h"

#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>

FWwiseEventCookedData::FWwiseEventCookedData():
	EventId(0),
	SoundBanks(),
	Media(),
	ExternalSources(),
	SwitchContainerLeaves(),
	RequiredGroupValueSet(),
	DestroyOptions(EWwiseEventDestroyOptions::WaitForEventEnd)
{}

void FWwiseEventCookedData::Serialize(FArchive& Ar)
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

FString FWwiseEventCookedData::GetDebugString() const
{
	bool bFirst = true;
	auto Result = FString::Printf(TEXT("Event %s (%" PRIu32 ")"), *DebugName.ToString(), EventId);
	if (SoundBanks.Num() > 0)
	{
		if (bFirst)
		{
			Result += TEXT(" with ");
			bFirst = false;
		}
		else
		{
			Result += TEXT(", ");
		}
		Result += FString::Printf(TEXT("%d SoundBank%s"), SoundBanks.Num(), SoundBanks.Num() > 1 ? TEXT("s") : TEXT(""));
	}
	if (Media.Num() > 0)
	{
		if (bFirst)
		{
			Result += TEXT(" with ");
			bFirst = false;
		}
		else
		{
			Result += TEXT(", ");
		}
		Result += FString::Printf(TEXT("%d Media"), Media.Num());
	}
	if (ExternalSources.Num() > 0)
	{
		if (bFirst)
		{
			Result += TEXT(" with ");
			bFirst = false;
		}
		else
		{
			Result += TEXT(", ");
		}
		Result += FString::Printf(TEXT("%d ExternalSource%s"), ExternalSources.Num(), ExternalSources.Num() > 1 ? TEXT("s") : TEXT(""));
	}
	if (SwitchContainerLeaves.Num() > 0)
	{
		if (bFirst)
		{
			Result += TEXT(" with ");
			bFirst = false;
		}
		else
		{
			Result += TEXT(", ");
		}
		Result += FString::Printf(TEXT("%d SwitchContainer Lea%s"), SwitchContainerLeaves.Num(), SwitchContainerLeaves.Num() > 1 ? TEXT("ves") : TEXT("f"));
	}
	if (RequiredGroupValueSet.Num() > 0)
	{
		if (bFirst)
		{
			Result += TEXT(" with ");
			bFirst = false;
		}
		else
		{
			Result += TEXT(", ");
		}
		Result += FString::Printf(TEXT("%d GroupValue%s"), RequiredGroupValueSet.Num(), RequiredGroupValueSet.Num() > 1 ? TEXT("s") : TEXT(""));
	}

	Result += FString::Printf(TEXT(" (%s)"), DestroyOptions == EWwiseEventDestroyOptions::StopEventOnDestroy ? TEXT("sod") : TEXT("wfe"));
	return Result;
}
