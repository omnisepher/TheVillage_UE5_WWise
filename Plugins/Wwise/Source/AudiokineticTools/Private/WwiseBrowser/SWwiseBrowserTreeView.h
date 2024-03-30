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
#include "WaapiPicker/WwiseTreeItem.h"
#include "Widgets/Views/STreeView.h"
#include "WwiseBrowserForwards.h"

/**
 * 
 */
class SWwiseBrowserTreeView : public STreeView< FWwiseTreeItemPtr >
{
public:

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedRef<SWwiseBrowser> Owner);

	/** Weak reference to the browser widget that owns this list */
	TWeakPtr<SWwiseBrowser> WwiseBrowserWeak;

	void SetSelectedItems(TArray<TSharedPtr<FWwiseTreeItem>> Items);
};

/** Widget that represents a row in the browser's tree control.  Generates widgets for each column on demand. */
class SWwiseBrowserTreeRow
	: public SMultiColumnTableRow< FWwiseTreeItemPtr >
{
public:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnId) override;

	/** The item associated with this row of data */
	TWeakPtr<FWwiseTreeItem> Item;

	SLATE_BEGIN_ARGS(SWwiseBrowserTreeRow) {}

		/** The list item for this row */
		SLATE_ARGUMENT( FWwiseTreeItemPtr, Item )

	SLATE_END_ARGS()

	/** Construct function for this widget */
	void Construct(const FArguments& InArgs, const
	TSharedRef<SWwiseBrowserTreeView>& BrowserTreeView, TSharedRef<SWwiseBrowser>
	WwiseBrowser);

	/** Weak reference to the browser widget that owns our list */
private:
	TWeakPtr<SWwiseBrowser> WwiseBrowserWeak;

};
