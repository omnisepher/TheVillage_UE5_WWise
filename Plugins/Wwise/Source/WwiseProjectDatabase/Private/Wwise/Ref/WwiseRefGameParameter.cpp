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

#include "Wwise/Ref/WwiseRefGameParameter.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataGameParameter.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"

const TCHAR* const FWwiseRefGameParameter::NAME = TEXT("GameParameter");

const FWwiseMetadataGameParameter* FWwiseRefGameParameter::GetGameParameter() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& GameParameters = SoundBank->GameParameters;
	if (GameParameters.IsValidIndex(GameParameterIndex))
	{
		return &GameParameters[GameParameterIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get GameParameter index #%zu"), GameParameterIndex);
		return nullptr;
	}
}

uint32 FWwiseRefGameParameter::GameParameterId() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return 0;
	}
	return GameParameter->Id;
}

FGuid FWwiseRefGameParameter::GameParameterGuid() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return {};
	}
	return GameParameter->GUID;
}

FName FWwiseRefGameParameter::GameParameterName() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return {};
	}
	return GameParameter->Name;
}

FName FWwiseRefGameParameter::GameParameterObjectPath() const
{
	const auto* GameParameter = GetGameParameter();
	if (UNLIKELY(!GameParameter))
	{
		return {};
	}
	return GameParameter->ObjectPath;
}

uint32 FWwiseRefGameParameter::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(GameParameterIndex));
	return Result;
}
