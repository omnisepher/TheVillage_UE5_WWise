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

#include "Wwise/Ref/WwiseRefStateGroup.h"

#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefStateGroup::NAME = TEXT("StateGroup");

const FWwiseMetadataStateGroup* FWwiseRefStateGroup::GetStateGroup() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& StateGroups = SoundBank->StateGroups;
	if (StateGroups.IsValidIndex(StateGroupIndex))
	{
		return &StateGroups[StateGroupIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get State Group index #%zu"), StateGroupIndex);
		return nullptr;
	}
}

uint32 FWwiseRefStateGroup::StateGroupId() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return 0;
	}
	return StateGroup->Id;
}

FGuid FWwiseRefStateGroup::StateGroupGuid() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return {};
	}
	return StateGroup->GUID;
}

FName FWwiseRefStateGroup::StateGroupName() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return {};
	}
	return StateGroup->Name;
}

FName FWwiseRefStateGroup::StateGroupObjectPath() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return {};
	}
	return StateGroup->ObjectPath;
}

uint32 FWwiseRefStateGroup::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(StateGroupIndex));
	return Result;
}
