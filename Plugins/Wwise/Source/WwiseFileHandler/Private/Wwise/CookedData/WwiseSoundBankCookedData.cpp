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

#include "Wwise/CookedData/WwiseSoundBankCookedData.h"

#include "Wwise/Stats/FileHandler.h"
#include <inttypes.h>

FWwiseSoundBankCookedData::FWwiseSoundBankCookedData() :
	SoundBankId(0),
	SoundBankPathName(),
	MemoryAlignment(0),
	bDeviceMemory(false),
	bContainsMedia(false),
	SoundBankType(EWwiseSoundBankType::User),
	DebugName()
{}

void FWwiseSoundBankCookedData::Serialize(FArchive& Ar)
{
	UStruct* Struct = StaticStruct();
	UE_CLOG(UNLIKELY(!Struct), LogWwiseFileHandler, Fatal, TEXT("SoundBankCookedData Serialize: No StaticStruct."));

	if (Ar.WantBinaryPropertySerialization())
	{
		UE_CLOG(Ar.IsSaving(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing to binary archive %s SoundBankCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), SoundBankId, *DebugName.ToString());
		Struct->SerializeBin(Ar, this);
		UE_CLOG(Ar.IsLoading(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing from binary archive %s SoundBankCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), SoundBankId, *DebugName.ToString());
	}
	else
	{
		UE_CLOG(Ar.IsSaving(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing to tagged archive %s SoundBankCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), SoundBankId, *DebugName.ToString());
		Struct->SerializeTaggedProperties(Ar, reinterpret_cast<uint8*>(this), Struct, nullptr);
		UE_CLOG(Ar.IsLoading(), LogWwiseFileHandler, VeryVerbose, TEXT("Serializing from tagged archive %s SoundBankCookedData %" PRIu32 " %s"), *Ar.GetArchiveName(), SoundBankId, *DebugName.ToString());
	}
}

FString FWwiseSoundBankCookedData::GetDebugString() const
{
	return FString::Printf(TEXT("SoundBank %s (%" PRIu32 ") @ %s (ma:%" PRIi32 " %sdm %smedia %suser)"),
		*DebugName.ToString(), SoundBankId, *SoundBankPathName.ToString(), MemoryAlignment,
		bDeviceMemory ? TEXT("") : TEXT("!"), bContainsMedia ? TEXT("") : TEXT("!"),
		SoundBankType == EWwiseSoundBankType::User ? TEXT("") : TEXT("!"));
};
