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

#include "WwiseBrowserTreeColumn.h"

#include "AkAudioStyle.h"
#include "SWwiseBrowser.h"
#include "WwiseBrowserHelpers.h"
#include "Widgets/SWidget.h"
#include "WaapiPicker/WwiseTreeItem.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FWwiseBrowserTreeColumn::FWwiseBrowserTreeColumn(SWwiseBrowser& WwiseBrowser)
	: WwiseBrowserWeak( StaticCastSharedRef<SWwiseBrowser>(WwiseBrowser.AsShared()) )
{
}

FName FWwiseBrowserTreeColumn::GetColumnId()
{
	return FName("WwiseObjects");
}

const TSharedRef<SWidget> FWwiseBrowserTreeColumn::ConstructRowWidget(FWwiseTreeItemPtr TreeItem,
	const STableRow<FWwiseTreeItemPtr>& Row)
{
	auto WwiseBrowser = WwiseBrowserWeak.Pin();

	bool bItemUpToDate = WwiseBrowser->IsWaapiAvailable() == EWwiseConnectionStatus::Connected ? TreeItem->IsItemUpToDate() : TreeItem->IsUAssetUpToDate();

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 1, 2, 1)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(FAkAudioStyle::GetBrush(TreeItem->ItemType))
		]

	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TreeItem->DisplayName))
			.ColorAndOpacity(WwiseBrowserHelpers::GetTextColor(bItemUpToDate || !TreeItem->ShouldDisplayInfo()))
			.HighlightText(WwiseBrowser->GetFilterHighlightText())
		];
}

SHeaderRow::FColumn::FArguments FWwiseBrowserTreeColumn::ConstructHeaderRowColumn()
{
	auto NameColumnHeader = SHeaderRow::Column(GetColumnId());
	NameColumnHeader.ColumnId(WwiseBrowserHelpers::WwiseBrowserColumnId);
	TAttribute<FText> Label;
	Label.Set(FText::FromString("Name"));
	NameColumnHeader.DefaultLabel(Label);
	NameColumnHeader.DefaultTooltip(LOCTEXT("WwiseBrowserColumn_Tooltip", "The names of the items in their hierarchy. The hierarchy uses the information from the Generated SoundBanks first."));
	return NameColumnHeader;
}

#undef LOCTEXT_NAMESPACE
