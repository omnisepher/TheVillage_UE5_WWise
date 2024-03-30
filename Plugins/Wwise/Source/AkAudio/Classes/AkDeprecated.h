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
#include "Serialization/BulkData.h"
#include "UObject/Object.h"
#include "AkAudioType.h"

#include "AkDeprecated.generated.h"


//These classes are deprecated but we use them during migration to clean up old assets

UCLASS()
class AKAUDIO_API UAkAssetData : public UObject
{
	GENERATED_BODY()

	FByteBulkData Data;
	void Serialize(FArchive& Ar) override
	{
		Super::Serialize(Ar);
		Data.Serialize(Ar, this);
	}
};


UCLASS()
class AKAUDIO_API UAkAssetPlatformData : public UObject
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY(transient, VisibleAnywhere, Category = "UAkAssetData")
	TMap<FString, UAkAssetData*> AssetDataPerPlatform;
#endif

	UPROPERTY(transient)
	UAkAssetData* CurrentAssetData = nullptr;

	void Serialize(FArchive& Ar) override
	{
		Super::Serialize(Ar);
#if WITH_EDITORONLY_DATA
		Ar << AssetDataPerPlatform;
#endif
	}

};

struct AKAUDIO_API FAkMediaDataChunk
{
	FByteBulkData Data;
	bool IsPrefetch = false;

	void Serialize(FArchive& Ar, UObject* Owner)
	{
		Ar << IsPrefetch;
		Data.Serialize(Ar, Owner);
	}
};


UCLASS()
class AKAUDIO_API UAkMediaAssetData : public UObject
{
	GENERATED_BODY()

	TIndirectArray<FAkMediaDataChunk> DataChunks;

	void Serialize(FArchive& Ar) override
	{
		Super::Serialize(Ar);

		int32 numChunks = DataChunks.Num();
		Ar << numChunks;

		if (Ar.IsLoading())
		{
			DataChunks.Empty();
			for (int32 i = 0; i < numChunks; ++i)
			{
				DataChunks.Add(new FAkMediaDataChunk());
			}
		}

		for (int32 i = 0; i < numChunks; ++i)
		{
			DataChunks[i].Serialize(Ar, this);
		}
	}
};

UCLASS()
class AKAUDIO_API UAkMediaAsset : public UObject
{
	GENERATED_BODY()

	UPROPERTY(transient, VisibleAnywhere, Category = "AkMediaAsset")
	TMap<FString, UAkMediaAssetData*> MediaAssetDataPerPlatform;

	void Serialize(FArchive& Ar) override
	{
		Super::Serialize(Ar);
		Ar << MediaAssetDataPerPlatform;
	}
};

UCLASS()
class AKAUDIO_API UAkLocalizedMediaAsset : public UAkMediaAsset
{
	GENERATED_BODY()
};

UCLASS()
class AKAUDIO_API UAkExternalMediaAsset : public UAkMediaAsset
{
	GENERATED_BODY()

};

UCLASS()
class AKAUDIO_API UAkFolder : public UAkAudioType
{
	GENERATED_BODY()
#if WITH_EDITORONLY_DATA
	virtual FWwiseObjectInfo* GetInfoMutable() override { return nullptr; }
#endif
};
