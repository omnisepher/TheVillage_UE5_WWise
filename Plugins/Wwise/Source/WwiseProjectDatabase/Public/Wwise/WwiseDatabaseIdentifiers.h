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

#include "CoreMinimal.h"

#include "WwiseDatabaseIdentifiers.generated.h"

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseMediaIdKey
{
	GENERATED_BODY()

	UPROPERTY() uint32 MediaId = 0;
	UPROPERTY() uint32 SoundBankId = 0;

	FWwiseDatabaseMediaIdKey()
	{}
	FWwiseDatabaseMediaIdKey(uint32 InMediaId, uint32 InSoundBankId) :
		MediaId(InMediaId),
		SoundBankId(InSoundBankId)
	{}
	bool operator==(const FWwiseDatabaseMediaIdKey& Rhs) const
	{
		return MediaId == Rhs.MediaId
			&& SoundBankId == Rhs.SoundBankId;
	}
	bool operator<(const FWwiseDatabaseMediaIdKey& Rhs) const
	{
		return (MediaId < Rhs.MediaId)
			|| (MediaId == Rhs.MediaId && SoundBankId < Rhs.SoundBankId);
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableIdKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = 0;

	UPROPERTY() uint32 Id = 0;
	UPROPERTY() uint32 LanguageId = 0;
	UPROPERTY() uint32 SoundBankId = 0;

	FWwiseDatabaseLocalizableIdKey()
	{}
	FWwiseDatabaseLocalizableIdKey(uint32 InId, uint32 InLanguageId) :
		Id(InId),
		LanguageId(InLanguageId)
	{}
	FWwiseDatabaseLocalizableIdKey(uint32 InId, uint32 InLanguageId, uint32 InSoundBankId) :
		Id(InId),
		LanguageId(InLanguageId),
		SoundBankId(InSoundBankId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableIdKey& Rhs) const
	{
		return Id == Rhs.Id
			&& LanguageId == Rhs.LanguageId
			&& SoundBankId == Rhs.SoundBankId;
	}
	bool operator<(const FWwiseDatabaseLocalizableIdKey& Rhs) const
	{
		return (Id < Rhs.Id)
			|| (Id == Rhs.Id && LanguageId < Rhs.LanguageId)
			|| (Id == Rhs.Id && LanguageId == Rhs.LanguageId && SoundBankId < Rhs.SoundBankId);
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseGroupValueKey
{
	GENERATED_BODY()

	UPROPERTY() uint32 GroupId = 0;
	UPROPERTY() uint32 Id = 0;

	FWwiseDatabaseGroupValueKey()
	{}
	FWwiseDatabaseGroupValueKey(uint32 InGroupId, uint32 InId) :
		GroupId(InGroupId),
		Id(InId)
	{}
	bool operator==(const FWwiseDatabaseGroupValueKey& Rhs) const
	{
		return GroupId == Rhs.GroupId
			&& Id == Rhs.Id;
	}
	bool operator<(const FWwiseDatabaseGroupValueKey& Rhs) const
	{
		return (GroupId < Rhs.GroupId)
			|| (GroupId == Rhs.GroupId && Id < Rhs.Id);
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableGroupValueKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = 0;

	UPROPERTY() FWwiseDatabaseGroupValueKey GroupValue;
	UPROPERTY() uint32 LanguageId = 0;

	FWwiseDatabaseLocalizableGroupValueKey()
	{}
	FWwiseDatabaseLocalizableGroupValueKey(uint32 InGroup, uint32 InId, uint32 InLanguageId) :
		GroupValue(InGroup, InId),
		LanguageId(InLanguageId)
	{}
	FWwiseDatabaseLocalizableGroupValueKey(FWwiseDatabaseGroupValueKey InGroupValue, uint32 InLanguageId) :
		GroupValue(InGroupValue),
		LanguageId(InLanguageId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableGroupValueKey& Rhs) const
	{
		return GroupValue == Rhs.GroupValue
			&& LanguageId == Rhs.LanguageId;
	}
	bool operator<(const FWwiseDatabaseLocalizableGroupValueKey& Rhs) const
	{
		return (GroupValue < Rhs.GroupValue)
			|| (GroupValue == Rhs.GroupValue && LanguageId < Rhs.LanguageId);
	}
};


USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableGuidKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE;

	UPROPERTY() FGuid Guid;
	UPROPERTY() uint32 LanguageId = 0;		// 0 if no Language

	FWwiseDatabaseLocalizableGuidKey()
	{}
	FWwiseDatabaseLocalizableGuidKey(FGuid InGuid, uint32 InLanguageId) :
		Guid(InGuid),
		LanguageId(InLanguageId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableGuidKey& Rhs) const
	{
		return Guid == Rhs.Guid
			&& LanguageId == Rhs.LanguageId;
	}
	bool operator<(const FWwiseDatabaseLocalizableGuidKey& Rhs) const
	{
		return (Guid < Rhs.Guid)
			|| (Guid == Rhs.Guid && LanguageId < Rhs.LanguageId);
	}
};

USTRUCT()
struct WWISEPROJECTDATABASE_API FWwiseDatabaseLocalizableNameKey
{
	GENERATED_BODY()

	static constexpr uint32 GENERIC_LANGUAGE = FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE;

	UPROPERTY() FName Name;
	UPROPERTY() uint32 LanguageId = 0;		// 0 if no Language

	FWwiseDatabaseLocalizableNameKey()
	{}
	FWwiseDatabaseLocalizableNameKey(FName InName, uint32 InLanguageId) :
		Name(InName),
		LanguageId(InLanguageId)
	{}
	bool operator==(const FWwiseDatabaseLocalizableNameKey& Rhs) const
	{
		return Name == Rhs.Name
			&& LanguageId == Rhs.LanguageId;
	}
	bool operator<(const FWwiseDatabaseLocalizableNameKey& Rhs) const
	{
		return (Name.FastLess(Rhs.Name))
			|| (Name == Rhs.Name && LanguageId < Rhs.LanguageId);
	}
};

uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseMediaIdKey& FileId);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableIdKey& LocalizableId);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseGroupValueKey& LocalizableGroupValue);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableGroupValueKey& LocalizableGroupValue);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableIdKey& EventId);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableGuidKey& LocalizableGuid);
uint32 WWISEPROJECTDATABASE_API GetTypeHash(const FWwiseDatabaseLocalizableNameKey& LocalizableName);
