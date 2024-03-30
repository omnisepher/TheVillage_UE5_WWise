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

#include "Wwise/Ref/WwiseRefState.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataStateGroup.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataState.h"

const TCHAR* const FWwiseRefState::NAME = TEXT("State");

const FWwiseMetadataState* FWwiseRefState::GetState() const
{
	const auto* StateGroup = GetStateGroup();
	if (UNLIKELY(!StateGroup))
	{
		return nullptr;
	}
	const auto& States = StateGroup->States;
	if (States.IsValidIndex(StateIndex))
	{
		return &States[StateIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get State index #%zu"), StateIndex);
		return nullptr;
	}
}

uint32 FWwiseRefState::StateId() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return 0;
	}
	return State->Id;
}

FGuid FWwiseRefState::StateGuid() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return {};
	}
	return State->GUID;
}

FName FWwiseRefState::StateName() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return {};
	}
	return State->Name;
}

FName FWwiseRefState::StateObjectPath() const
{
	const auto* State = GetState();
	if (UNLIKELY(!State))
	{
		return {};
	}
	return State->ObjectPath;
}

uint32 FWwiseRefState::Hash() const
{
	auto Result = FWwiseRefStateGroup::Hash();
	Result = HashCombine(Result, GetTypeHash(StateIndex));
	return Result;
}
