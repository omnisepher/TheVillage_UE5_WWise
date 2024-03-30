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

#include "IWwiseBrowserDataSource.h"
#include "WwiseItemType.h"
#include "../WwiseBrowserForwards.h"
#include "WaapiPicker/SWaapiPicker.h"

class FJsonValue;
class FJsonObject;

DECLARE_DELEGATE_OneParam(FOnWaapiSelectionChange, const TArray<TSharedPtr<FWwiseTreeItem>>&)

enum EWwiseConnectionStatus
{
	Connected,
	SettingDisabled,
	WrongProjectOpened,
	WwiseNotOpen
};

struct WaapiTransformStringField
{
	const FString keyArg;
	const TArray<FString> valueStringArgs;
	const TArray<int32> valueNumberArgs;
};

struct WWiseWaapiItem
{
	FGuid Guid;
	FName Name;
	FName FullPath;
};

class FWaapiDataSource : IWwiseBrowserDataSource
{
public :

	DECLARE_DELEGATE(FOnWaapiDataSourceRefreshed)

	// IWwiseBrowserDataSource

	virtual ~FWaapiDataSource() override;

	// Sets up the connection to WAAPI
	virtual bool Init() override;

	virtual void ConstructTree(bool bShouldRefresh) override;

	// Constructs the top-level root for the given type, along with its direct children
	virtual FWwiseTreeItemPtr ConstructTreeRoot(EWwiseItemType::Type Type) override;

	virtual int32 LoadChildren(const FGuid& InParentId, const FString& InParentPath, TArray<FWwiseTreeItemPtr>& OutChildren) override;

	virtual int32 LoadChildren(FWwiseTreeItemPtr ParentTreeItem) override;

	virtual int32 GetChildItemCount(const FWwiseTreeItemPtr& InParentItem) override;

	virtual FWwiseTreeItemPtr GetRootItem(EWwiseItemType::Type RootType) override;

	FWwiseTreeItemPtr GetRootItem(const FString& InFullPath);


	virtual FWwiseTreeItemPtr LoadFilteredRootItem(EWwiseItemType::Type RootType, TSharedPtr<StringFilter> CurrentFilterText) override;

	// FWaapiDataSource

	bool TearDown();

	FWwiseTreeItemPtr ConstructWwiseTreeItem(const TSharedPtr<FJsonObject>& InItemInfoObj);

	FWwiseTreeItemPtr ConstructWwiseTreeItem(const TSharedPtr<FJsonValue>& InJsonItem);

	FWwiseTreeItemPtr FindItemFromPath(const FWwiseTreeItemPtr& InParentItem, const FString& InCurrentItemPath);

	FWwiseTreeItemPtr FindItemFromPath(const FString& InCurrentItemPath);

	FWwiseTreeItemPtr FindOrConstructTreeItemFromJsonObject(const TSharedPtr<FJsonObject>& ObjectJson);

	void FindAndCreateItems(FWwiseTreeItemPtr CurrentItem);

	FString GetItemWorkUnitPath(FWwiseTreeItemPtr InItem);

	bool LoadWaapiInfo(const FString& InFromField, const FString& InFromString, TSharedPtr<FJsonObject>& OutJsonResult, const TArray<WaapiTransformStringField>& TransformFields);

	bool IsTreeDirty();

	void Tick(const double InCurrentTime, const float InDeltaTime);

	FString LoadProjectName();

	EWwiseConnectionStatus IsProjectLoaded();

	void SelectInProjectExplorer(TArray<FWwiseTreeItemPtr>& InTreeItems);

	/**
	* Call WAAPI to get information about an object form the path or the id of the object (inFrom).
	*	
	* @param inFromField	 The path or the id from which the data will be get.
	* @param outJsonResult   A JSON object that contains useful information about the call process, gets the object infos or gets an error infos in case the call failed.
	* @return			     A boolean to ensure that the call was successfully done.
	*/
	static bool CallWaapiGetInfoFrom(const FString& inFromField, const FString& inFromString, TSharedPtr<FJsonObject>& outJsonResult, const TArray<TransformStringField>& TransformFields);

	void HandleFindWwiseItemInProjectExplorerCommandExecute(const TArray<FWwiseTreeItemPtr>& SelectedItems) const;

	FOnWaapiDataSourceRefreshed WaapiDataSourceRefreshed;

	FOnWaapiSelectionChange WwiseSelectionChange;
	FOnWaapiSelectionChange WwiseExpansionChange;

private:

	/** Root items, one for each type of Wwise object */
	FCriticalSection WaapiRootItemsLock;
	TArray< FWwiseTreeItemPtr > RootItems;

	TArray<FGuid> ItemsToCreate;

	// Container paths along the Browser Tree
	TMap<FString, FWwiseTreeItemPtr> NodesByPath;

	FDelegateHandle ProjectLoadedHandle;
	FDelegateHandle ConnectionLostHandle;
	FDelegateHandle ClientBeginDestroyHandle;

	// WAAPI Callbacks

	void OnProjectLoadedCallback();

	void OnConnectionLostCallback();

	void OnWaapiClientBeginDestroyCallback();

	void OnWaapiRenamed(uint64_t Id, TSharedPtr<FJsonObject> Response);
	void OnWaapiChildAdded(uint64_t Id, TSharedPtr<FJsonObject> Response);
	void OnWaapiChildRemoved(uint64_t Id, TSharedPtr<FJsonObject> Response);
	void OnWwiseSelectionChanged(uint64_t Id, TSharedPtr<FJsonObject> Response);

	void SubscribeWaapiCallbacks();

	void UnsubscribeWaapiCallbacks();
	void RemoveClientCallbacks();

	struct FWaapiSubscriptionIds
	{
		uint64 Renamed = 0;
		uint64 ChildAdded = 0;
		uint64 ChildRemoved = 0;
		uint64 SelectionChanged = 0;
	} WaapiSubscriptionIds;


};
