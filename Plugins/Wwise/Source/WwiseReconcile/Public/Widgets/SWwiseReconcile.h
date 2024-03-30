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

#include "Wwise/WwiseReconcile.h"
#include "IWwiseReconcileColumn.h"
#include "Widgets/SWindow.h"

class SWwiseReconcileListView;

class SWwiseReconcile : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWwiseReconcile)
		: _FocusSearchBoxWhenOpened(false)
		, _ShowTreeTitle(true)
		, _ShowSeparator(true)
		, _AllowContextMenu(true)
	{}

	/** Content displayed to the left of the search bar */
	SLATE_NAMED_SLOT(FArguments, SearchContent)

		/** If true, the search box will be focus the frame after construction */
		SLATE_ARGUMENT(bool, FocusSearchBoxWhenOpened)

		/** If true, The tree title will be displayed */
		SLATE_ARGUMENT(bool, ShowTreeTitle)

		/** If true, The tree search bar separator be displayed */
		SLATE_ARGUMENT(bool, ShowSeparator)

		/** If false, the context menu will be suppressed */
		SLATE_ARGUMENT(bool, AllowContextMenu)

		///** The selection mode for the tree view */
		//SLATE_ARGUMENT(ESelectionMode::Type, SelectionMode)
		SLATE_END_ARGS()

	WWISERECONCILE_API void Construct(const FArguments& InArgs, const TArray< FWwiseReconcileItem >& ReconcileItems, TSharedRef<SWindow>& ReconcileWindow);

	WWISERECONCILE_API SWwiseReconcile();

	/** Get the columns to be displayed in this outliner */
	const TMap<FName, TSharedPtr<IWwiseReconcileColumn>>& GetColumns() const
	{
		return Columns;
	}
private:
	FReply ReconcileAssets();

	FReply CloseWindow();

	void SetupColumns(SHeaderRow& HeaderRow);

	/** Generate a row in the tree view */
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FWwiseReconcileItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** The header row of the Reconcile view */
	TSharedPtr< SHeaderRow > HeaderRowWidget;

	/** Items to Reconcile */
	TArray< TSharedPtr<FWwiseReconcileItem> > ReconcileItems;

	TSharedPtr<SWwiseReconcileListView> ReconcileList;

	/** Map of columns that are shown on the Picker. */
	TMap<FName, TSharedPtr<IWwiseReconcileColumn>> Columns;

	TWeakPtr<SWindow> Window;
};