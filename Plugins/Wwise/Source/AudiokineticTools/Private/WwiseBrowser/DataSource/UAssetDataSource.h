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

#include "WwiseItemType.h"
#include "GenericPlatform/GenericPlatform.h"
#include "Templates/TypeHash.h"

struct UAssetDataSourceId
{
	FGuid ItemId;
	uint32 ShortId = 0;
	FName Name;
	inline const bool operator==(const UAssetDataSourceId& other) const
	{
		if (other.ItemId == ItemId && ItemId.IsValid())
		{
			return true;
		}
		if (ShortId == other.ShortId && ShortId > 0)
		{
			return true;
		}
		return Name == other.Name;
	}
};

struct UAssetDataSourceInformation
{
	EWwiseItemType::Type Type;
	UAssetDataSourceId Id;
	TArray<FAssetData> AssetsData;
	FName AssetName;
	inline const bool operator==(const UAssetDataSourceId& other) const
	{
		return Id == other;
	}

	inline const bool operator==(const UAssetDataSourceInformation& other) const
	{
		return Id == other.Id;
	}
};

class FUAssetDataSource
{
	// UAssets that have a valid Guid that exists in the Project Database
	TMap<FGuid, UAssetDataSourceInformation> UsedItems;
	// UAssets with invalid Guid will use the ShortId to sync with the Project Database.
	TMap<uint32, UAssetDataSourceInformation> UAssetWithoutGuid;
	// UAssets with invalid Guid and ShortId will use the AssetName to sync with the Project Database. Also includes "None" States.
	TMap<FName, UAssetDataSourceInformation> UAssetWithoutShortId;

	UAssetDataSourceInformation CreateUAssetInfo(const UAssetDataSourceId& Id, const FAssetData& Asset);

	bool GuidExistsInProjectDatabase(const FGuid ItemId);

public:
	void ConstructItems();
	void GetAssetsInfo(FGuid ItemId, uint32 ShortId, FString Name, EWwiseItemType::Type& ItemType, FName& AssetName, TArray<FAssetData>& Assets);
	void GetOrphanAssets(TArray<UAssetDataSourceInformation>& OrphanAssets) const;
};