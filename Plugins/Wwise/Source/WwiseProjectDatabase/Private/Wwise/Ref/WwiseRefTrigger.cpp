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

#include "Wwise/Ref/WwiseRefTrigger.h"

#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/FileHandler.h"
#include "Wwise/Metadata/WwiseMetadataTrigger.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefTrigger::NAME = TEXT("Trigger");

const FWwiseMetadataTrigger* FWwiseRefTrigger::GetTrigger() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& Triggers = SoundBank->Triggers;
	if (Triggers.IsValidIndex(TriggerIndex))
	{
		return &Triggers[TriggerIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Trigger index #%zu"), TriggerIndex);
		return nullptr;
	}
}

uint32 FWwiseRefTrigger::TriggerId() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return 0;
	}
	return Trigger->Id;
}

FGuid FWwiseRefTrigger::TriggerGuid() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return {};
	}
	return Trigger->GUID;
}

FName FWwiseRefTrigger::TriggerName() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return {};
	}
	return Trigger->Name;
}

FName FWwiseRefTrigger::TriggerObjectPath() const
{
	const auto* Trigger = GetTrigger();
	if (UNLIKELY(!Trigger))
	{
		return {};
	}
	return Trigger->ObjectPath;
}

uint32 FWwiseRefTrigger::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(TriggerIndex));
	return Result;
}
