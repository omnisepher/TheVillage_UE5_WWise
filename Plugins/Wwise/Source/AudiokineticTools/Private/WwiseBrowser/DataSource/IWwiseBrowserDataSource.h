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

#include "WwiseItemType.h"
#include "Misc/TextFilter.h"
#include "WwiseBrowser/WwiseBrowserForwards.h"

typedef TTextFilter< const FString& > StringFilter;

class IWwiseBrowserDataSource
{
public:
	virtual ~IWwiseBrowserDataSource() = default;

	virtual bool Init() = 0;

	virtual void ConstructTree(bool bShouldRefresh) = 0;

	// Constructs the top-level root for the given type, along with its direct children
	virtual FWwiseTreeItemPtr ConstructTreeRoot(EWwiseItemType::Type Type) = 0;

	// Loads children for the item matching first InParentId, then InParentPath
	virtual int32 LoadChildren(const FGuid& InParentId, const FString& InParentPath, TArray<FWwiseTreeItemPtr>& OutChildren) = 0;

	// Loads children for the item
	virtual int32 LoadChildren(FWwiseTreeItemPtr ParentTreeItem) = 0;

	virtual int32 GetChildItemCount(const FWwiseTreeItemPtr& InParentItem) = 0;

	virtual FWwiseTreeItemPtr GetRootItem(EWwiseItemType::Type RootType) = 0;

	virtual FWwiseTreeItemPtr LoadFilteredRootItem(EWwiseItemType::Type Type, TSharedPtr<StringFilter> CurrentFilterText) = 0;
};
