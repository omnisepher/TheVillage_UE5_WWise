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

#include "WwiseBrowserForwards.h"
#include "Templates/SharedPointer.h"
#include "AssetRegistry/AssetData.h"
#include "WaapiPicker/WwiseTreeItem.h"

#include "WwiseBrowserHelpers.generated.h"

class UAkAssetFactory;
class UObject;
class FString;

UCLASS()
class UAkDragDropBlocker : public UObject
{
	GENERATED_BODY()
};

namespace WwiseBrowserHelpers
{
	struct WwiseBrowserAssetPayload
	{
		FAssetData CreatedAsset;
		TArray<FAssetData> ExistingAssets;
		FString RelativePackagePath;
		FString Name;
		FGuid WwiseObjectGuid;
	};

	enum EAssetCreationMode
	{
		Transient,
		InPackage
	};

	enum EAssetDuplicationMode
	{
		DoDuplication,
		NoDuplication
	};

	/**
	* Recursively iterates over a WwiseTreeItem and its children
	* Found assets are added to the ExistingAssets array in the payload
	* If no assets are found, a new asset is created and the payload's CreatedAsset field is set
	*
  	* @param WwiseTreeItem			  Current WwiseTreeItem we want to create assets from
  	* @param InOutBrowserAssetPayloads The recursively filled array of WwiseBrowserAssetPayloads
  	* @param InOutKnownGuids		  List of encountered Guids so the same asset payload is not created twice
  	*								  This can happen when both an asset and its parent are selected in the browser
  	* @param AssetCreationMode        Create assets in the transient package or directly in the content directory
  	* @param PackagePath			  Root path where the assets should be saved when AssetCreationMode is 'InPackage'
  	* @param CurrentRelativePath      Recursively built path of the assets relative to the root tree item
  	*/
	void FindOrCreateAssetsRecursive(
		const FWwiseTreeItemPtr& WwiseTreeItem, 
		TArray<WwiseBrowserAssetPayload>& InOutBrowserAssetPayloads, 
		TSet<FGuid>& InOutKnownGuids, 
		const EAssetCreationMode AssetCreationMode,
		const FString& PackagePath = "",
		const FString& CurrentRelativePath = ""
	);

	FAssetData CreateBrowserAsset(const FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass, const EAssetCreationMode AssetCreationMode, const FString& PackagePath);
	FAssetData CreateBrowserAssetTask(const FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass, const EAssetCreationMode AssetCreationMode, const FString& PackagePath);

	FAssetData CreateTransientAsset(const ::FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass);
	FAssetData CreateAssetInPackage(const ::FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, const FString& PackagePath, UClass* AssetClass);
	FAssetData CreateAsset(const ::FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass, UPackage* Pkg);

	/**
	* Save, rename, duplicate assets produced by FindOrCreateAssetsRecursive
	*
  	* @param Assets			      Array of WwiseBrowserAssetPayloads created to handle
  	* @param RootPackagePath      Root path where renamed and duplicated assets will be copied to
  	* @param AssetCreationMode	  Must match the AssetCreationMode passed to FindOrCreateAssetsRecursive
  	* @param AssetDuplicationMode Whether existing assets should be duplicated into a new asset placed relative to RootPackagePath
  	*/
	void SaveSelectedAssets(const TArray<WwiseBrowserAssetPayload> Assets, const FString& RootPackagePath, const EAssetCreationMode AssetCreationMode, const EAssetDuplicationMode AssetDuplicationMode);
	void SaveSelectedAssetsTask(const TArray<WwiseBrowserAssetPayload> Assets, const FString& RootPackagePath, const EAssetCreationMode AssetCreationMode, const EAssetDuplicationMode AssetDuplicationMode);

	UAkAssetFactory* GetAssetFactory(const FWwiseTreeItemPtr& WwiseTreeItem);

	EWwiseItemType::Type GetTypeFromClass(UClass* Class);
	bool CanCreateAsset(const FWwiseTreeItemPtr& Item);
	FLinearColor GetTextColor(bool UpToDate);

	const FName WwiseBrowserColumnId = TEXT("WwiseItem");
}
