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

#include "Wwise/Ref/WwiseRefSwitchGroup.h"

#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataSwitchGroup.h"

const TCHAR* const FWwiseRefSwitchGroup::NAME = TEXT("SwitchGroup");

const FWwiseMetadataSwitchGroup* FWwiseRefSwitchGroup::GetSwitchGroup() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& SwitchGroups = SoundBank->SwitchGroups;
	if (SwitchGroups.IsValidIndex(SwitchGroupIndex))
	{
		return &SwitchGroups[SwitchGroupIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Switch Group index #%zu"), SwitchGroupIndex);
		return nullptr;
	}
}

bool FWwiseRefSwitchGroup::IsControlledByGameParameter() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (!SwitchGroup)
	{
		return false;
	}

	return SwitchGroup->GameParameterRef != nullptr;
}

uint32 FWwiseRefSwitchGroup::SwitchGroupId() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return 0;
	}
	return SwitchGroup->Id;
}

FGuid FWwiseRefSwitchGroup::SwitchGroupGuid() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return {};
	}
	return SwitchGroup->GUID;
}

FName FWwiseRefSwitchGroup::SwitchGroupName() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return {};
	}
	return SwitchGroup->Name;
}

FName FWwiseRefSwitchGroup::SwitchGroupObjectPath() const
{
	const auto* SwitchGroup = GetSwitchGroup();
	if (UNLIKELY(!SwitchGroup))
	{
		return {};
	}
	return SwitchGroup->ObjectPath;
}

uint32 FWwiseRefSwitchGroup::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(SwitchGroupIndex));
	return Result;
}
