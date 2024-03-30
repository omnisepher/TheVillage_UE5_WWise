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

#pragma once

#include "Dom/JsonObject.h"
#include "Wwise/Metadata/WwiseMetadataLoadable.h"
#include "Wwise/Metadata/WwiseMetadataGameParameter.h"

enum class EWwiseRequiredMetadata
{
	Optional,
	Mandatory
};

struct FWwiseMetadataLoader
{
	bool bResult;
	const TSharedRef<FJsonObject>& JsonObject;

	FWwiseMetadataLoader(const TSharedRef<FJsonObject>& InJsonObject) :
		bResult(true),
		JsonObject(InJsonObject)
	{
	}

	void Fail(const TCHAR* FieldName);
	void LogParsed(const TCHAR* FieldName, const uint32 Id = 0, const FName Name = FName());

	bool GetBool(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required = EWwiseRequiredMetadata::Mandatory);
	float GetFloat(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required = EWwiseRequiredMetadata::Mandatory);
	FGuid GetGuid(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required = EWwiseRequiredMetadata::Mandatory);
	FName GetString(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required = EWwiseRequiredMetadata::Mandatory);
	uint32 GetUint32(FWwiseMetadataLoadable* Object, const FString& FieldName, EWwiseRequiredMetadata Required = EWwiseRequiredMetadata::Mandatory);

	template <typename T>
	T GetObject(FWwiseMetadataLoadable* Object, const FString& FieldName);

	template <typename T>
	T* GetObjectPtr(FWwiseMetadataLoadable* Object, const FString& FieldName);

	template<typename T>
	TArray<T> GetArray(FWwiseMetadataLoadable* Object, const FString& FieldName);

	template<typename T>
	void GetPropertyArray(T* Object, const TMap<FName, size_t>& FloatProperties);
};

template<typename T>
T FWwiseMetadataLoader::GetObject(FWwiseMetadataLoadable* Object, const FString& FieldName)
{
	check(Object);
	Object->AddRequestedValue(TEXT("object"), FieldName);

	const TSharedPtr<FJsonObject>* InnerObject;
	if (!JsonObject->TryGetObjectField(FieldName, InnerObject))
	{
		Fail(*FieldName);
		return T{};
	}
	auto SharedRef(InnerObject->ToSharedRef());
	FWwiseMetadataLoader ObjectLoader(SharedRef);
	T Result(ObjectLoader);
	if (ObjectLoader.bResult)
	{
		Result.CheckRequestedValues(SharedRef);
	}
	else
	{
		bResult = false;
		LogParsed(*FieldName);
	}

	return Result;
}


template <typename T>
T* FWwiseMetadataLoader::GetObjectPtr(FWwiseMetadataLoadable* Object, const FString& FieldName)
{
	check(Object);
	Object->AddRequestedValue(TEXT("optional object"), FieldName);

	const TSharedPtr<FJsonObject>* InnerObject;
	if (!JsonObject->TryGetObjectField(FieldName, InnerObject))
	{
		return nullptr;
	}

	auto SharedRef(InnerObject->ToSharedRef());
	FWwiseMetadataLoader ObjectLoader(SharedRef);
	T* Result = new T(ObjectLoader);
	if (ObjectLoader.bResult)
	{
		if (Result)
		{
			Result->CheckRequestedValues(SharedRef);
		}
	}
	else
	{
		bResult = false;
		LogParsed(*FieldName);
		delete Result;
		return nullptr;
	}

	return Result;
}

template <typename T>
TArray<T> FWwiseMetadataLoader::GetArray(FWwiseMetadataLoadable* Object, const FString& FieldName)
{
	check(Object);
	Object->AddRequestedValue(TEXT("array"), FieldName);

	const TArray< TSharedPtr<FJsonValue> >* Array;
	if (!JsonObject->TryGetArrayField(FieldName, Array))
	{
		// No data. Not a fail, valid!
		Object->IncLoadedSize(sizeof(TArray<T>));
		return TArray<T>{};
	}

	TArray<T> Result;
	Result.Empty(Array->Num());

	for (auto& InnerObject : *Array)
	{
		const TSharedPtr<FJsonObject>* InnerJsonObjectPtr;
		if (!InnerObject->TryGetObject(InnerJsonObjectPtr))
		{
			LogParsed(*FieldName);
			continue;
		}
		
		auto SharedRef(InnerJsonObjectPtr->ToSharedRef());
		FWwiseMetadataLoader ArrayLoader(SharedRef);
		T ResultObject(ArrayLoader);

		if (ArrayLoader.bResult)
		{
			ResultObject.CheckRequestedValues(SharedRef);
		}
		else
		{
			bResult = false;
			ArrayLoader.LogParsed(*FieldName);
			Result.Empty();
			break;
		}

		Result.Add(MoveTemp(ResultObject));
	}

	Object->IncLoadedSize(sizeof(TArray<T>));
	return Result;
}

template <typename T>
void FWwiseMetadataLoader::GetPropertyArray(T* Object, const TMap<FName, size_t>& FloatProperties)
{
	check(Object);
	Object->AddRequestedValue(TEXT("propertyarray"), TEXT("Properties"));

	Object->IncLoadedSize(FloatProperties.Num() * sizeof(float));

	const TArray< TSharedPtr<FJsonValue> >* Array;
	if (!JsonObject->TryGetArrayField(TEXT("Properties"), Array))
	{
		// No data. Not a fail, valid!
		return;
	}

	for (auto& InnerObject : *Array)
	{
		const TSharedPtr<FJsonObject>* InnerJsonObjectPtr;
		if (!InnerObject->TryGetObject(InnerJsonObjectPtr))
		{
			continue;
		}

		const auto SharedRef(InnerJsonObjectPtr->ToSharedRef());
		FString Name;
		if (!SharedRef->TryGetStringField(TEXT("Name"), Name))
		{
			Fail(TEXT("Property::Name"));
			continue;
		}
		FString Type;
		if (!SharedRef->TryGetStringField(TEXT("Type"), Type) || Type != TEXT("Real32"))
		{
			Fail(TEXT("Property::Type"));
			continue;
		}
		double Value;
		if (!SharedRef->TryGetNumberField(TEXT("Value"), Value))
		{
			Fail(TEXT("Property::Value"));
			continue;
		}
		if (const auto* Property = FloatProperties.Find(FName(Name)))
		{
			*(float*)((intptr_t)Object + *Property) = Value;
		}
		else
		{
			Fail(*Name);
			continue;
		}
	}
}
