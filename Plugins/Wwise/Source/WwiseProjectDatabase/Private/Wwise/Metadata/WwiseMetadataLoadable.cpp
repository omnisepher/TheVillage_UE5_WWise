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

#include "Wwise/Metadata/WwiseMetadataLoadable.h"
#include "Wwise/Stats/ProjectDatabase.h"

#include "Dom/JsonObject.h"

void FWwiseMetadataLoadable::AddRequestedValue(const FString& Type, const FString& Value)
{
	bool IsAlreadySet = false;
	RequestedValues.Add(Value, &IsAlreadySet);
	if (UNLIKELY(IsAlreadySet))
	{
		UE_LOG(LogWwiseProjectDatabase, Fatal, TEXT("Trying to load the same %s field twice: %s"), *Type, *Value);
	}
}

void FWwiseMetadataLoadable::CheckRequestedValues(TSharedRef<FJsonObject>& JsonObject)
{
	TArray<FString> Keys;
	JsonObject->Values.GetKeys(Keys);
	auto Diff = TSet<FString>(Keys).Difference(RequestedValues);
	for (const auto& Key : Diff)
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("Unknown Json field: %s"), *Key);
	}
}

void FWwiseMetadataLoadable::IncLoadedSize(size_t Size)
{
	INC_DWORD_STAT_BY(STAT_WwiseProjectDatabaseMemory, Size);
	LoadedSize += Size;
}

void FWwiseMetadataLoadable::DecLoadedSize(size_t Size)
{
	DEC_DWORD_STAT_BY(STAT_WwiseProjectDatabaseMemory, Size);
	LoadedSize -= Size;
}

void FWwiseMetadataLoadable::UnloadLoadedSize()
{
	DEC_DWORD_STAT_BY(STAT_WwiseProjectDatabaseMemory, LoadedSize);
	LoadedSize = 0;
}
