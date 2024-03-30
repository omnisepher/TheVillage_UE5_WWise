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

#include "Wwise/Ref/WwiseRefExternalSource.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataExternalSource.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"

const TCHAR* const FWwiseRefExternalSource::NAME = TEXT("ExternalSource");

const FWwiseMetadataExternalSource* FWwiseRefExternalSource::GetExternalSource() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& ExternalSources = SoundBank->ExternalSources;
	if (ExternalSources.IsValidIndex(ExternalSourceIndex))
	{
		return &ExternalSources[ExternalSourceIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get External Source index #%zu"), ExternalSourceIndex);
		return nullptr;
	}
}

uint32 FWwiseRefExternalSource::ExternalSourceCookie() const
{
	const auto* ExternalSource = GetExternalSource();
	if (UNLIKELY(!ExternalSource))
	{
		return {};
	}
	return ExternalSource->Cookie;
}

FGuid FWwiseRefExternalSource::ExternalSourceGuid() const
{
	const auto* ExternalSource = GetExternalSource();
	if (UNLIKELY(!ExternalSource))
	{
		return {};
	}
	return ExternalSource->GUID;
}

FName FWwiseRefExternalSource::ExternalSourceName() const
{
	const auto* ExternalSource = GetExternalSource();
	if (UNLIKELY(!ExternalSource))
	{
		return {};
	}
	return ExternalSource->Name;
}

FName FWwiseRefExternalSource::ExternalSourceObjectPath() const
{
	const auto* ExternalSource = GetExternalSource();
	if (UNLIKELY(!ExternalSource))
	{
		return {};
	}
	return ExternalSource->ObjectPath;
}

uint32 FWwiseRefExternalSource::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(ExternalSourceIndex));
	return Result;
}
