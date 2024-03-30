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

#include "WaapiPlaybackTransport.h"
#include "DataSource/WwiseBrowserDataSource.h"
#include "DataSource/WaapiDataSource.h"
#include "Misc/TextFilter.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateBrush.h"
#include "Framework/Views/ITypedTableView.h"
#include "WwiseBrowser/IWwiseBrowserColumn.h"

class FUICommandList;
class STableViewBase;
class ITableRow;
class SHeaderRow;
class FWwiseBrowserDataSource;
class SWwiseBrowserTreeView;
using StringFilter = TTextFilter<const FString&>;

class WwiseBrowserBuilderVisitor;
class FWwiseBrowserDataLoader;

class SWwiseBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SWwiseBrowser )
		: _FocusSearchBoxWhenOpened(false)
		, _ShowTreeTitle(true)
		, _ShowSearchBar(true)
		, _ShowSeparator(true)
		, _AllowContextMenu(true)
		, _SelectionMode( ESelectionMode::Multi )
		{}

		/** Content displayed to the left of the search bar */
		SLATE_NAMED_SLOT( FArguments, SearchContent )

		/** If true, the search box will be focus the frame after construction */
		SLATE_ARGUMENT( bool, FocusSearchBoxWhenOpened )

		/** If true, The tree title will be displayed */
		SLATE_ARGUMENT( bool, ShowTreeTitle )

		/** If true, The tree search bar will be displayed */
		SLATE_ARGUMENT( bool, ShowSearchBar )

		/** If true, The tree search bar separator be displayed */
		SLATE_ARGUMENT( bool, ShowSeparator )

		/** If false, the context menu will be suppressed */
		SLATE_ARGUMENT( bool, AllowContextMenu )

		/** The selection mode for the tree view */
		SLATE_ARGUMENT( ESelectionMode::Type, SelectionMode )
	SLATE_END_ARGS( )

	AUDIOKINETICTOOLS_API void Construct(const FArguments& InArgs);
	SWwiseBrowser(void);
	~SWwiseBrowser();

	AUDIOKINETICTOOLS_API static const FName WwiseBrowserTabName;

	AUDIOKINETICTOOLS_API TSharedRef<SWidget> MakeAddFilterMenu();

	void SoundBankFilterExecute(ESoundBankStatusFilter Filter);
	bool SoundBankFilterIsChecked(ESoundBankStatusFilter Filter);
	void UAssetFilterExecute(EUAssetStatusFilter Filter);
	bool UAssetFilterIsChecked(EUAssetStatusFilter Filter);
	void WwiseTypeFilterExecute(EWwiseTypeFilter Filter);
	bool WwiseTypeFilterIsChecked(EWwiseTypeFilter Filter);
	void SoundBankNotUpToDateExecute();
	void RemoveFiltersExecute();
	void UAssetNotUpToDateExecute();

	AUDIOKINETICTOOLS_API void ForceRefresh();

	AUDIOKINETICTOOLS_API void InitialParse();

	static FReply DoDragDetected(const FPointerEvent& MouseEvent, const TArray<FWwiseTreeItemPtr>& SelectedItems);

	TAttribute<FText> GetFilterHighlightText() const;

	static void ImportWwiseAssets(const TArray<FWwiseTreeItemPtr>& SelectedItems, const FString& PackagePath);

	/** Get the columns to be displayed in this outliner */
	const TMap<FName, TSharedPtr<IWwiseBrowserColumn>>& GetColumns() const
	{
		return Columns;
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	EVisibility IsItemPlaying(FGuid ItemId) const;

	void ExpandItem(FWwiseTreeItemPtr TreeItem, bool bShouldExpand);

	void ExpandItem(FWwiseTreeItemPtr TreeItem);

	bool IsItemExpanded(FWwiseTreeItemPtr TreeItem);

	void UpdateWaapiSelection(const TArray<TSharedPtr<FWwiseTreeItem>>& WaapiTreeItems);

	EWwiseConnectionStatus IsWaapiAvailable() const;

	/** Prevent the selection changed callback from running when filtering */
	bool AllowTreeViewDelegates;

private:

	/** The new tree view widget */
	TSharedPtr<SWwiseBrowserTreeView> TreeViewPtr;

	/** The header row of the browser tree view */
	TSharedPtr< SHeaderRow > HeaderRowWidget;

	/** Filter for the search box */
	TSharedPtr<StringFilter> SearchBoxFilter;

	/** True when the tree is current being filtered */
	bool bIsFilterApplied;

	FSoundBankStatusFilter SoundBankStatusFilter;

	FUAssetStatusFilter UAssetStatusFilter;

	FWwiseTypeFilter WwiseTypeFilter;

	const FSlateBrush* GetFilterBadgeIcon() const;

	void OnTextFilterUpdated();
	void OnFilterUpdated();
	void ApplyFilter();
	bool IsFiltering();
	FText GetFilterText();
	FString GetFilterString();

	/** Root items */
	TArray< FWwiseTreeItemPtr > RootItems;

	/** Remember the selected items. Useful when filtering to preserve selection status. */
	TSet< FString > LastSelectedPaths;

	/** Remember the expanded items. Useful when filtering to preserve expansion status. */
	TSet< FString > LastExpandedPaths;

	//Open the Wwise Integration settings
	FReply OnOpenSettingsClicked();

	/** Ran when the Refresh button is clicked. Parses the Wwise project (using the WwiseWwuParser) and populates the window. */
	FReply OnRefreshClicked();
	
	FReply OnGenerateSoundBanksClicked();

	FReply OnReconcileClicked();
	void OnTreeItemDoubleClicked(FWwiseTreeItemPtr TreeItem);

	/** Populates the browser window only (does not parse the Wwise project) */
	void ConstructTree();

	/** Generate a row in the tree view */
	TSharedRef<ITableRow> GenerateRow( FWwiseTreeItemPtr TreeItem, const TSharedRef<STableViewBase>& OwnerTable );

	/** Get the children of a specific tree element */
	void GetChildrenForTree( FWwiseTreeItemPtr TreeItem, TArray< FWwiseTreeItemPtr >& OutChildren );

	/** Commands handled by this widget */
	TSharedRef<FUICommandList> CommandList;

	/** Handle Drag & Drop */
	virtual FReply OnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent) override;

	void ExpandFirstLevel();
	void ExpandParents(FWwiseTreeItemPtr Item);

	FText GetProjectName() const;
	FText GetConnectedWwiseProjectName() const;

	EVisibility IsWarningVisible() const;
	EVisibility IsWarningNotVisible() const;
	FText GetWarningText() const;

	FText GetConnectionStatusText() const;
	FText GetSoundBanksLocationText() const;
	FSlateColor GetSoundBanksLocationTextColor() const;

	/** Used by the search filter */
	void PopulateSearchStrings( const FString& FolderName, OUT TArray< FString >& OutSearchStrings ) const;
	void OnSearchBoxChanged( const FText& InSearchText );
	void SetItemVisibility(FWwiseTreeItemPtr Item, bool IsVisible);
	void SaveCurrentTreeExpansion();
	void RestoreTreeExpansion(const TArray< FWwiseTreeItemPtr >& Items);

	/** Handler for tree view selection changes */
	void TreeSelectionChanged( FWwiseTreeItemPtr TreeItem, ESelectInfo::Type SelectInfo );

	/** Handler for tree view expansion changes */
	void TreeExpansionChanged( FWwiseTreeItemPtr TreeItem, bool bIsExpanded );

	/** Builds the command list for the context menu on Wwise Browser items. */
	void CreateWwiseBrowserCommands();

	/** Callback for creating a context menu for the Wwise items list. */
	TSharedPtr<SWidget> MakeWwiseBrowserContextMenu();

	/** Callback to execute the play command from the context menu. Only available with a WAAPI connection. */
	void HandlePlayWwiseItemCommandExecute();
	bool HandlePlayOrStopWwiseItemCanExecute();

	/** Callback to execute the stop command from the context menu. Only available with a WAAPI connection. */
	void HandleStopWwiseItemCommandExecute();

	/** Callback to execute the stop all command from the context menu. Only available with a WAAPI connection. */
	void HandleStopAllWwiseItemCommandExecute();

	/** Finds the selected item's containing workunit in Explorer. Only available with a WAAPI connection */
	void HandleExploreWwiseItemCommandExecute();
	bool HandleExploreWwiseItemCanExecute();

	/** Finds the selected item in the Wwise Project Explorer. Only available with a WAAPI connection */
	void HandleFindInProjectExplorerWwiseItemCommandExecute();
	bool HandleFindInProjectExplorerWwiseItemCanExecute();

	/** Finds the selected item in the Content browser. */
	void HandleFindInContentBrowserCommandExecute();
	bool HandleFindInContentBrowserCanExecute();

	void HandleRefreshWwiseBrowserCommandExecute();

	/** Callback to import a Wwise item into the project's Contents*/
	void HandleImportWwiseItemCommandExecute() const;

	/** Callback to reconcile a Wwise item*/
	void HandleReconcileWwiseItemCommandExecute() const;

	/** Set up the columns required for this outliner */
	void SetupColumns(SHeaderRow& HeaderRow);

	void GetTreeItemsFromWaapi(const TArray<TSharedPtr<FWwiseTreeItem>>& WaapiTreeItems, TArray<TSharedPtr<FWwiseTreeItem>>& TreeItems);

	FWwiseTreeItemPtr GetTreeItemFromWaapiItem(FWwiseTreeItemPtr WaapiTreeItem);

	void ExpandItems(const TArray< FWwiseTreeItemPtr >& Items);

	void CreateReconcileTab() const;

private:
	/** Map of columns that are shown on the Browser. */
	TMap<FName, TSharedPtr<IWwiseBrowserColumn>> Columns;

	/** DataSource responsible for loading all of the Browser's data through WAAPI, ProjectDB, Unreal assets */
	TUniquePtr<FWwiseBrowserDataSource> DataSource;

	/** Transport for handling playback of Wwise Items*/
	TUniquePtr<WaapiPlaybackTransport> Transport;
};