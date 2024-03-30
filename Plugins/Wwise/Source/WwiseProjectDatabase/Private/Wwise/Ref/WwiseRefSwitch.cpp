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

#include "Wwise/Ref/WwiseRefSwitch.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataSwitch.h"

const TCHAR* const FWwiseRefSwitch::NAME = TEXT("Switch");

const FWwiseMetadataSwitch* FWwiseRefSwitch::GetSwitch() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return nullptr;
	}
	const auto& Switches = SwitchGroup->Switches;
	if (Switches.IsValidIndex(SwitchIndex))
	{
		return &Switches[SwitchIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Switch index #%zu"), SwitchIndex);
		return nullptr;
	}
}

uint32 FWwiseRefSwitch::SwitchId() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return 0;
	}
	return Switch->Id;
}

FGuid FWwiseRefSwitch::SwitchGuid() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return {};
	}
	return Switch->GUID;
}

FName FWwiseRefSwitch::SwitchName() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return {};
	}
	return Switch->Name;
}

FName FWwiseRefSwitch::SwitchObjectPath() const
{
	const auto* Switch = GetSwitch();
	if (UNLIKELY(!Switch))
	{
		return {};
	}
	return Switch->ObjectPath;
}

uint32 FWwiseRefSwitch::Hash() const
{
	auto Result = FWwiseRefSwitchGroup::Hash();
	Result = HashCombine(Result, GetTypeHash(SwitchIndex));
	return Result;
}
