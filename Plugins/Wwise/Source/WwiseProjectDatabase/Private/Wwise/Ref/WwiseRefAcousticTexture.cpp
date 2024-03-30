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

#include "Wwise/Ref/WwiseRefAcousticTexture.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

#include "Wwise/Metadata/WwiseMetadataAcousticTexture.h"
#include "Wwise/Metadata/WwiseMetadataSoundBank.h"
#include "Wwise/Stats/ProjectDatabase.h"

const TCHAR* const FWwiseRefAcousticTexture::NAME = TEXT("AcousticTexture");

const FWwiseMetadataAcousticTexture* FWwiseRefAcousticTexture::GetAcousticTexture() const
{
	const auto* SoundBank = GetSoundBank();
	if (UNLIKELY(!SoundBank))
	{
		return nullptr;
	}
	const auto& AcousticTextures = SoundBank->AcousticTextures;
	if (AcousticTextures.IsValidIndex(AcousticTextureIndex))
	{
		return &AcousticTextures[AcousticTextureIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Acoustic Texture index #%zu"), AcousticTextureIndex);
		return nullptr;
	}
}

uint32 FWwiseRefAcousticTexture::AcousticTextureId() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return 0;
	}
	return AcousticTexture->Id;
}

FGuid FWwiseRefAcousticTexture::AcousticTextureGuid() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return {};
	}
	return AcousticTexture->GUID;
}

FName FWwiseRefAcousticTexture::AcousticTextureName() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return {};
	}
	return AcousticTexture->Name;
}

FName FWwiseRefAcousticTexture::AcousticTextureObjectPath() const
{
	const auto* AcousticTexture = GetAcousticTexture();
	if (UNLIKELY(!AcousticTexture))
	{
		return {};
	}
	return AcousticTexture->ObjectPath;
}

uint32 FWwiseRefAcousticTexture::Hash() const
{
	auto Result = FWwiseRefSoundBank::Hash();
	Result = HashCombine(Result, GetTypeHash(AcousticTextureIndex));
	return Result;
}
