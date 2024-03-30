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


#include "SWwiseBrowserTreeView.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "WwiseUEFeatures.h"
#include "IAudiokineticTools.h"
#include "SWwiseBrowser.h"
#include "SlateOptMacros.h"
#include "WwiseBrowser/WwiseAssetDragDropOp.h"

#if UE_5_0_OR_LATER
#include "WwiseUEFeatures.h"
#endif

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply HandleOnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent, TWeakPtr<SWwiseBrowserTreeView> Table)
{
	if (!MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Unhandled();
	}

	auto TablePtr = Table.Pin();

	TArray<FWwiseTreeItemPtr> SelectedItems;
	if (TablePtr.IsValid() && MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		SelectedItems = TablePtr->GetSelectedItems();

		if (SelectedItems.Num() == 0)
		{
			return FReply::Unhandled();
		}
	}

	UE_LOG(LogAudiokineticTools, Verbose, TEXT("SWwiseBrowser::OnDragDetected: User drag operation started."));

	auto AkSettings = GetMutableDefault<UAkSettings>();
	const FString DefaultPath = AkSettings->DefaultAssetCreationPath;

	TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> DragAssets;
	TSet<FGuid> SeenGuids;

	bool bAllItemsCanBeCreated = true;
	for (auto& WwiseTreeItem : SelectedItems)
	{
		if (!WwiseTreeItem->ItemId.IsValid() && !WwiseTreeItem->IsFolder())
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Cannot drag selected Wwise asset: %s does not have a valid ID"), *(WwiseTreeItem->FolderPath));
			continue;
		}

		if (!WwiseBrowserHelpers::CanCreateAsset(WwiseTreeItem))
		{
			bAllItemsCanBeCreated = false;
		}

		WwiseBrowserHelpers::FindOrCreateAssetsRecursive(WwiseTreeItem, DragAssets,SeenGuids, WwiseBrowserHelpers::EAssetCreationMode::Transient);
	}

	if (DragAssets.Num() == 0 && bAllItemsCanBeCreated)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to find or create Wwise asset '%s'%s in Browser operation"),
			*(SelectedItems[0]->FolderPath), SelectedItems.Num() > 1 ? TEXT(" and others"): TEXT(""));
		return FReply::Unhandled();
	}

	return FReply::Handled().BeginDragDrop(FWwiseAssetDragDropOp::New(DragAssets));
}

void SWwiseBrowserTreeView::Construct(const FArguments& InArgs, TSharedRef<SWwiseBrowser> Owner)
{
	WwiseBrowserWeak = Owner;
	STreeView::Construct(InArgs);
}

void SWwiseBrowserTreeView::SetSelectedItems(TArray<TSharedPtr<FWwiseTreeItem>> Items)
{
	SelectedItems.Reset();
	for(auto Item : Items)
	{
		SelectedItems.Add(Item);
	}

	if (OnSelectionChanged.IsBound() && SelectedItems.Num() > 0)
	{
		NullableItemType SelectedItem = (*typename TItemSet::TIterator(SelectedItems));

		OnSelectionChanged.ExecuteIfBound(SelectedItem, ESelectInfo::Direct);
	}
}

TSharedRef<SWidget> SWwiseBrowserTreeRow::GenerateWidgetForColumn(const FName& InColumnId)
{
	auto ItemPtr = Item.Pin();
	if (!ItemPtr.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	auto WwiseBrowser = WwiseBrowserWeak.Pin();
	check(WwiseBrowser.IsValid());

	// Create the widget for this item
	TSharedRef<SWidget> NewItemWidget = SNullWidget::NullWidget;

	auto Column = WwiseBrowser->GetColumns().FindRef(InColumnId);
	if (Column.IsValid())
	{
		NewItemWidget = Column->ConstructRowWidget(ItemPtr.ToSharedRef(), *this);
	}

	if( InColumnId == WwiseBrowserHelpers::WwiseBrowserColumnId)
	{
		// The first column gets the tree expansion arrow for this row
		return SNew(SBox)
			.MinDesiredHeight(20)
			[
				SNew( SHorizontalBox )

				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(6, 0, 0, 0)
				[
					SNew( SExpanderArrow, SharedThis(this) ).IndentAmount(12)
				]

				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					NewItemWidget
				]
			];
	}
	else
	{
		// Other columns just get widget content -- no expansion arrow needed
		return NewItemWidget;
	}
}

void SWwiseBrowserTreeRow::Construct(
	const FArguments& InArgs,
	const TSharedRef<SWwiseBrowserTreeView>& BrowserTreeView,
	TSharedRef<SWwiseBrowser> WwiseBrowser)
{
	Item = InArgs._Item->AsShared();
	WwiseBrowserWeak = WwiseBrowser;

	auto Args = FSuperRowType::FArguments();

	Args.OnDragDetected_Static(HandleOnDragDetected, TWeakPtr<SWwiseBrowserTreeView>(BrowserTreeView));

	auto ItemPtr = Item.Pin();
	auto WwiseBrowserPtr = WwiseBrowserWeak.Pin();

	SetVisibility(ItemPtr->IsVisible ? EVisibility::Visible : EVisibility::Collapsed);

	if (!WwiseBrowserPtr.IsValid())
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Wwise Browser is invalid."));
		return;
	}

	if (!ItemPtr.IsValid())
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Wwise Item is invalid."));
		return;
	}

	SMultiColumnTableRow<FWwiseTreeItemPtr>::Construct(Args, BrowserTreeView);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
