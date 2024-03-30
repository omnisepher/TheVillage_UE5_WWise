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

#include "ISoundBankInfoCache.h"
#include "Containers/Map.h"
#include "Misc/Guid.h"

class FArchive;

class WwiseSoundBankInfoCache : public ISoundBankInfoCache
{
public:
	bool Load(const FString& Path);

	bool IsSoundBankUpToUpdate(const FGuid& Id, const FString& Platform, const FString& Language, const uint32 Hash) const override;

private:
	void readString(FArchive& ar, FString& Value);
	void readGuid(FArchive& Ar, FGuid& Value);
	static void readBool(FArchive& Ar, bool& Value);

private:
	enum class CacheType : uint8
	{
		XML,
		JSON,
		Bank,
		Count
	};

	enum class SerializeState : uint8
	{
		None,
		Platform,
		Language,
		BankInfo,
		InfoFile,
		InfoFileType,
		EndOfData
	};

	struct FileInfo
	{
		uint32 Hash = 0;
		int64 Timestamp = 0;
		bool Updated = false;
	};

	struct MemoryStats
	{
		int64 Timestamp;
		uint32 DataSize;
		uint32 FileSize;
		uint32 DecodedSize;
		uint32 SFXPreFetchSize;
		uint32 SFXInMemorySize;
		uint32 SFXMissingFiles;
		uint32 MusicPreFetchSize;
		uint32 MusicInMemorySize;
		uint32 MusicMissingFiles;
		uint32 VoicePreFetchSize;
		uint32 VoiceInMemorySize;
		uint32 VoiceMissingFiles;
		uint32 ReplacedFiles;
	};

	struct BankInfo : public FileInfo
	{
		MemoryStats Stats;
		bool IsTemporary = false;
	};

	struct CacheKey
	{
		CacheKey() = default;
		CacheKey(const FString& InName, const FGuid& InPlatform, uint32 InLanguage)
			: Name(InName), Platform(InPlatform), Language(InLanguage)
		{}


		FString Name;
		FGuid Platform;
		uint32 Language;

		bool operator==(const CacheKey& Right) const
		{
			return Name == Right.Name
				&& Platform == Right.Platform
				&& Language == Right.Language
				;
		}
	};

	friend FArchive& operator<<(FArchive& Ar, WwiseSoundBankInfoCache::FileInfo& Value);
	friend FArchive& operator<<(FArchive& Ar, WwiseSoundBankInfoCache::BankInfo& Value);
	friend FArchive& operator<<(FArchive& Ar, WwiseSoundBankInfoCache::MemoryStats& Value);
	friend uint32 GetTypeHash(const CacheKey& Key);

private:
	TMap<FString, FGuid> platformNameToGuidMap;
	TMap<CacheKey, BankInfo> bankInfoMap;
};

FArchive& operator<<(FArchive& Ar, WwiseSoundBankInfoCache::FileInfo& Value);
FArchive& operator<<(FArchive& Ar, WwiseSoundBankInfoCache::BankInfo& Value);
FArchive& operator<<(FArchive& Ar, WwiseSoundBankInfoCache::MemoryStats& Value);
uint32 GetTypeHash(const WwiseSoundBankInfoCache::CacheKey& Key);