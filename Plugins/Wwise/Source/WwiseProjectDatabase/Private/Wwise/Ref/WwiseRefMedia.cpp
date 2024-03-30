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

#include "Wwise/Ref/WwiseRefMedia.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataMedia.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"

const TCHAR* const FWwiseRefMedia::NAME = TEXT("Media");

const FWwiseMetadataMedia* FWwiseRefMedia::GetMedia() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}

	const auto& Media = SoundBank->Media;
	if (Media.IsValidIndex(MediaIndex))
	{
		return &Media[MediaIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Media index #%zu"), MediaIndex);
		return nullptr;
	}
}

uint32 FWwiseRefMedia::MediaId() const
{
	const auto* Media = GetMedia();
	if (UNLIKELY(!Media))
	{
		return 0;
	}
	return Media->Id;
}

FName FWwiseRefMedia::MediaShortName() const
{
	const auto* Media = GetMedia();
	if (UNLIKELY(!Media))
	{
		return {};
	}
	return Media->ShortName;
}

FName FWwiseRefMedia::MediaPath() const
{
	const auto* Media = GetMedia();
	if (UNLIKELY(!Media))
	{
		return {};
	}
	return Media->Path;
}

uint32 FWwiseRefMedia::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(MediaIndex));
	return Result;
}
