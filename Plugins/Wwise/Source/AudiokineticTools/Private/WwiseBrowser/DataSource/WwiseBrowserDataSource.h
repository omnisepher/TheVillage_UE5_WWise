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
#include "UAssetDataSource.h"

#include "WaapiDataSource.h"
#include "../WwiseBrowserForwards.h"
#include "WwiseItemType.h"
#include "WwiseProjectDatabaseSource.h"
#include "WwiseBrowser/WwiseAssetDragDropOp.h"

struct FWwiseTreeItem;

enum ESoundBankStatusFilter
{
	NewInWwise,
	DeletedInWwise,
	RenamedInWwise,
	NotInWwise,
	MovedInWwise,
	UpToDate,
	NumberOfSoundBankStatus
};

enum EUAssetStatusFilter
{
	UAssetMissing,
	NotInSoundBankOrUnreal,
	UAssetOrphaned,
	RenamedInSoundBank,
	MultipleUAssets,
	UAssetNeedsUpdate,
	UAssetUpToDate,
	NumberOfUAssetStatus
};

enum EWwiseTypeFilter
{
	AcousticTexture,
	Effects,
	Events,
	GameParameters,
	MasterMixerHierarchy,
	State,
	Switch,
	Trigger,
	NumberOfWwiseTypes
};

struct FSoundBankStatusFilter
{
	bool bFilters[NumberOfSoundBankStatus] = { false };
	bool IsKeptInBrowser(FWwiseTreeItemPtr& Item) const;
	bool AreFiltersOff() const;
};

struct FUAssetStatusFilter
{
	bool bFilters[NumberOfUAssetStatus] = { false };
	bool IsKeptInBrowser(FWwiseTreeItemPtr& Item) const;
	bool AreFiltersOff() const;
};

struct FWwiseTypeFilter
{
	bool bFilters[NumberOfWwiseTypes] = { false };
	bool IsKeptInBrowser(FWwiseTreeItemPtr& Item) const;
	bool AreFiltersOff() const;
private:
	EWwiseItemType::Type GetExpectedType(EWwiseTypeFilter Filter) const;
};

class FWwiseBrowserDataSource
{
public:
	DECLARE_DELEGATE(FOnWwiseBrowserDataSourceRefreshed)

	FWwiseBrowserDataSource();

	~FWwiseBrowserDataSource();

	void ConstructTree();
	bool AreFiltersOff();

	void ApplyTextFilter(TSharedPtr<StringFilter> FilterText);

	void ApplyFilter(FSoundBankStatusFilter SoundBankStatusFilter,
					FUAssetStatusFilter UAssetStatusFilter,
					FWwiseTypeFilter WwiseTypeFilter);

	void ApplyFilter(FWwiseTreeItemPtr Item, FWwiseTreeItemPtr& OutItem);

	void ClearFilter();

	bool IsKeptInBrowser(FWwiseTreeItemPtr Item);

	FWwiseTreeItemPtr GetTreeRootForType(EWwiseItemType::Type ItemType, const FString& FilterText={});

	FText GetProjectName();

	FText GetConnectedWwiseProjectName();

	int32 LoadChildren(FWwiseTreeItemPtr TreeItem, TArray<FWwiseTreeItemPtr>& OutChildren);

	// Clean placeholder children
	void ClearEmptyChildren(FWwiseTreeItemPtr TreeItem);

	EWwiseConnectionStatus GetWaapiConnectionStatus() const;

	FString GetItemWorkUnitPath(FWwiseTreeItemPtr InTreeItem);

	void SelectInWwiseProjectExplorer(TArray<FWwiseTreeItemPtr>& InTreeItem);

	FOnWwiseBrowserDataSourceRefreshed WwiseBrowserDataSourceRefreshed;

	FOnWaapiSelectionChange WwiseSelectionChange;
	FOnWaapiSelectionChange WwiseExpansionChange;

	FDelegateHandle PostEditorTickHandle;

	void HandleFindWwiseItemInProjectExplorerCommandExecute(const TArray<FWwiseTreeItemPtr>& SelectedItems) const;

	void CreateProjectDBItem(const FWwiseTreeItemPtr& TreeItemRootSoundBank, FWwiseTreeItemPtr& TreeItemRootDst);
	void CreateWaapiExclusiveItem(const FWwiseTreeItemPtr& WaapiItem, FWwiseTreeItemPtr& TreeItemRootDst);
	void CreateWaapiItem(const FWwiseTreeItemPtr& TreeItemRootWwise, FWwiseTreeItemPtr& TreeItemRootDst);
private:

	void MergeDataSources(bool bGenerateUAssetsInfo = true);

	bool bIsDirty = false;

	float AssetUpdateTimer = 0.f;

	const float AssetTimerRefresh = 0.1f;

	TUniquePtr<FWaapiDataSource> WaapiDataSource;

	TUniquePtr<FWwiseProjectDatabaseDataSource> ProjectDBDataSource;

	TUniquePtr<FUAssetDataSource> UAssetDataSource;

	/** Root items, one for each type of Wwise object */
	FCriticalSection RootItemsLock;
	TArray< FWwiseTreeItemPtr > RootItems;

	TArray< FWwiseTreeItemPtr > RootItemsUnfiltered;

	TSharedPtr<StringFilter> CurrentFilterText;

	FSoundBankStatusFilter SoundBankStatusFilter;

	FUAssetStatusFilter UAssetStatusFilter;

	FWwiseTypeFilter WwiseTypeFilter;

	// Merges TreeItemRootSrc and TreeItemRootDst, with the resulting tree in TreeItemRootDst. New TreeItems will be created if they do not exist in TreeItemRootDst
	void CreateUnifiedTree(const FWwiseTreeItemPtr& TreeItemRootSoundBank, const FWwiseTreeItemPtr& TreeItemRootWaapi, FWwiseTreeItemPtr& TreeItemRootDst);

	void OnWaapiDataSourceRefreshed();

	void OnProjectDBDataSourceRefreshed();

	void SetupAssetCallbacks();
	void OnFilesFullyLoaded();
	void OnTimerTick(float DeltaSeconds);
	
	void OnUAssetSourceRefresh(const FAssetData& RemovedAssetData);

	void OnUAssetSourceRefresh(const FAssetData& RemovedAssetData, const FString& OldPath);

	void OnWwiseSelectionChange(const TArray<TSharedPtr<FWwiseTreeItem>>& Items);
	void OnWwiseExpansionChange(const TArray<TSharedPtr<FWwiseTreeItem>>& Items);

	FDelegateHandle OnAssetAdded;
	FDelegateHandle OnAssetRemoved;
	FDelegateHandle OnAssetRenamed;
	FDelegateHandle OnAssetUpdated;
	FDelegateHandle OnFilesLoaded;
};

