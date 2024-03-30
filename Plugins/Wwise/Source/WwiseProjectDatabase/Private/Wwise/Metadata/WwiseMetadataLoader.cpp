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

#include "Wwise/Metadata/WwiseMetadataLoader.h"

#include "Wwise/Metadata/WwiseMetadataLoadable.h"
#include "Wwise/Stats/ProjectDatabase.h"

#include "Dom/JsonObject.h"

#include <inttypes.h>

void FWwiseMetadataLoader::Fail(const TCHAR* FieldName)
{
	UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not retrieve field %s"), FieldName);
	bResult = false;
}

void FWwiseMetadataLoader::LogParsed(const TCHAR* FieldName, const uint32 Id, const FName Name)
{
	if (bResult)
	{
		if (Id && !Name.IsNone())
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s [%" PRIu32 "] %s"), FieldName, Id, *Name.ToString());
		}
		else if (Id)
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s [%" PRIu32 "]"), FieldName, Id);
		}
		else if (!Name.IsNone())
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s: %s"), FieldName, *Name.ToString());
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Parsed %s"), FieldName);
		}
	}
	else 
	{
		if (Id && !Name.IsNone())
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s [%" PRIu32 "] %s"), FieldName, Id, *Name.ToString());
		}
		else if (Id)
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s [%" PRIu32 "]"), FieldName, Id);
		}
		else if (!Name.IsNone())
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s: %s"), FieldName, *Name.ToString());
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("... while parsing %s"), FieldName);
		}
	}
}

bool FWwiseMetadataLoader::GetBool(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("bool"), FieldName);

	bool Value = false;

	if (!JsonObject->TryGetBoolField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value));
	return Value;
}

float FWwiseMetadataLoader::GetFloat(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("float"), FieldName);

	double Value{};

	if (!JsonObject->TryGetNumberField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value));
	return float(Value);
}

FGuid FWwiseMetadataLoader::GetGuid(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("guid"), FieldName);

	FGuid Value{};

	FString ValueAsString;
	if (!JsonObject->TryGetStringField(FieldName, ValueAsString))
	{
		if (Required == EWwiseRequiredMetadata::Mandatory)
		{
			Fail(*FieldName);
		}
	}
	else if (ValueAsString.Len() != 38)
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Invalid GUID %s: %s"), *FieldName, *ValueAsString);
		Fail(*FieldName);
	}
	else
	{
		if (!FGuid::ParseExact(ValueAsString, EGuidFormats::DigitsWithHyphensInBraces, Value))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not decode GUID %s: %s"), *FieldName, *ValueAsString);
			Fail(*FieldName);
		}
	}

	Object->IncLoadedSize(sizeof(Value));
	return Value;
}

FName FWwiseMetadataLoader::GetString(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("string"), FieldName);

	FString Value{};

	if (!JsonObject->TryGetStringField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value) + Value.GetAllocatedSize());
	return FName(Value);
}

uint32 FWwiseMetadataLoader::GetUint32(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required)
{
	check(Object);
	Object->AddRequestedValue(TEXT("uint32"), FieldName);

	uint32 Value{};

	if (!JsonObject->TryGetNumberField(FieldName, Value) && Required == EWwiseRequiredMetadata::Mandatory)
	{
		Fail(*FieldName);
	}

	Object->IncLoadedSize(sizeof(Value));
	return Value;
}



