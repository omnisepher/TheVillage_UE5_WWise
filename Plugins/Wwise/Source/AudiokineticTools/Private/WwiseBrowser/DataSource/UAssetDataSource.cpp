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

#include "UAssetDataSource.h"

#include "AkUnrealAssetDataHelper.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/Stats/AudiokineticTools.h"
#include "WwiseBrowser/WwiseBrowserHelpers.h"

UAssetDataSourceInformation FUAssetDataSource::CreateUAssetInfo(const UAssetDataSourceId& Id, const FAssetData& Asset)
{
	auto AssetInfo = UAssetDataSourceInformation();
	AssetInfo.AssetName = Asset.AssetName;
	AssetInfo.Id = Id;
	AssetInfo.Type = WwiseBrowserHelpers::GetTypeFromClass(Asset.GetClass());
	AssetInfo.AssetsData.Add(Asset);
	return AssetInfo;
}

bool FUAssetDataSource::GuidExistsInProjectDatabase(const FGuid ItemId)
{
	SCOPED_AUDIOKINETICTOOLS_EVENT_3(TEXT("FUAssetDataSource::GuidExistsInProjectDatabase"))
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (ProjectDatabase)
	{
		const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
		const FWwiseRefPlatform Platform = DataStructure.GetPlatform(ProjectDatabase->GetCurrentPlatform());
		const auto* PlatformData = DataStructure.GetCurrentPlatformData();

		if (UNLIKELY(!PlatformData))
		{
			return false;
		}

		FWwiseDatabaseLocalizableGuidKey Key = FWwiseDatabaseLocalizableGuidKey(ItemId, FWwiseDatabaseLocalizableIdKey::GENERIC_LANGUAGE);
		return PlatformData->Guids.Find(Key) != nullptr;
	}
	return false;
}

void FUAssetDataSource::ConstructItems()
{
	SCOPED_AUDIOKINETICTOOLS_EVENT_2(TEXT("FUAssetDataSource::ConstructItems"))
	UsedItems.Empty();
	UAssetWithoutGuid.Empty();
	UAssetWithoutShortId.Empty();

	TArray<FAssetData> AllAssets;
	AkAssetDatabase::Get().FindAllAssets(AllAssets);

	for (auto& Asset : AllAssets)
	{
		auto AssetType = WwiseBrowserHelpers::GetTypeFromClass(Asset.GetClass());
		
		if (AssetType == EWwiseItemType::InitBank)
		{
			continue;
		}

		auto GuidValue = Asset.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseGuid));
		auto ShortIdValue = Asset.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseShortId));

		UAssetDataSourceId Id;
		Id.Name = Asset.AssetName;

		if (GuidValue.IsSet())
		{
			FString GuidAsString = GuidValue.GetValue();
			FGuid Guid = FGuid(GuidAsString);
			Id.ItemId = Guid;
		}

		if (ShortIdValue.IsSet())
		{
			Id.ShortId = FCString::Strtoui64(*ShortIdValue.GetValue(), NULL, 10);
		}
		
		// States always share a state called None, and therefore always share the same ShortId. Do not list by ShortId in that case.
		bool bStoreByShortId = Id.ShortId != AK_INVALID_UNIQUE_ID &&
							   !(AssetType == EWwiseItemType::State && Asset.AssetName.ToString().EndsWith("None"));

		if (Id.ItemId.IsValid() && GuidExistsInProjectDatabase(Id.ItemId))
		{
			if (auto UAssetInfo = UsedItems.Find(Id.ItemId))
			{
				UAssetInfo->AssetsData.Add(Asset);
			}
			else
			{
				auto AssetInfo = CreateUAssetInfo(Id, Asset);
				UsedItems.Add(Id.ItemId, AssetInfo);
			}
		}

		else if (bStoreByShortId)
		{
			if (auto UAsset = UAssetWithoutGuid.Find(Id.ShortId))
			{
				UAsset->AssetsData.Add(Asset);
			}
			else
			{
				auto AssetInfo = CreateUAssetInfo(Id, Asset);
				UAssetWithoutGuid.Add(Id.ShortId, AssetInfo);
			}
		}

		else
		{
			if (auto UAsset = UAssetWithoutShortId.Find(Id.Name))
			{
				UAsset->AssetsData.Add(Asset);
			}
			else
			{
				auto AssetInfo = CreateUAssetInfo(Id, Asset);
				UAssetWithoutShortId.Add(Id.Name, AssetInfo);
			}
		}
	}
}

void FUAssetDataSource::GetAssetsInfo(FGuid ItemId, uint32 ShortId, FString Name, EWwiseItemType::Type& ItemType, FName& AssetName, TArray<FAssetData>& Assets)
{
	SCOPED_AUDIOKINETICTOOLS_EVENT_2(TEXT("FUAssetDataSource::GetAssetsInfo"))
	UAssetDataSourceId Id;
	Id.ItemId = ItemId;
	Id.ShortId = ShortId;
	Id.Name = FName(*Name);

	if (auto Item = UsedItems.Find(Id.ItemId))
	{
		Assets = Item->AssetsData;
		ItemType = Item->Type;
		AssetName = Item->AssetName;
	}

	if (auto Item = UAssetWithoutGuid.Find(Id.ShortId))
	{
		auto GuidItem = UsedItems.Find(Id.ItemId);
		for(auto Asset : Item->AssetsData)
		{
			if (!AkUnrealAssetDataHelper::IsSameType(Asset, Item->Type))
			{
				continue;
			}
			Assets.Add(Asset);
			if(AssetName == FName())
			{
				AssetName = Item->AssetName;
				ItemType = Item->Type;
			}
			if(GuidItem)
			{
				GuidItem->AssetsData.Add(Asset);
			}
		}
		UAssetWithoutGuid.Remove(ShortId);
	}

	if (auto Item = UAssetWithoutShortId.Find(FName(*Name)))
	{
		auto GuidItem = UsedItems.Find(Id.ItemId);
		for(auto Asset : Item->AssetsData)
		{
			if(!AkUnrealAssetDataHelper::IsSameType(Asset, Item->Type))
			{
				continue;
			}
			Assets.Add(Asset);
			if(AssetName == FName())
			{
				AssetName = Item->AssetName;
				ItemType = Item->Type;
			}
			if(GuidItem)
			{
				GuidItem->AssetsData.Add(Asset);
			}
			UAssetWithoutShortId.Remove(FName(*Name));
		}
	}
}

void FUAssetDataSource::GetOrphanAssets(TArray<UAssetDataSourceInformation>& OrphanAssets) const
{
	for(auto ShortIdItem : UAssetWithoutGuid)
	{
		OrphanAssets.Add(ShortIdItem.Value);
	}

	for(auto NameItem : UAssetWithoutShortId)
	{
		OrphanAssets.Add(NameItem.Value);
	}
}
