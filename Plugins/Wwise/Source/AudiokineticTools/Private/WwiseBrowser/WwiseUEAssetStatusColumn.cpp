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

#include "WwiseUEAssetStatusColumn.h"

#include "AkAudioStyle.h"
#include "WwiseBrowserForwards.h"
#include "WwiseBrowserHelpers.h"
#include "Widgets/SWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FText FWwiseUEAssetStatusColumn::GetDisplayedName(FWwiseTreeItemPtr TreeItem)
{
	if(!TreeItem->UEAssetExists() || !TreeItem->ShouldDisplayInfo())
	{
		return LOCTEXT("WwiseBrowserEmpty", "");
	}
	if(TreeItem->HasUniqueUAsset())
	{
		return FText::FromName(TreeItem->UAssetName);
	}
	return FText::Format(LOCTEXT("UEAssetsColumnMultiple", "Multiple UAssets ({0})"), TreeItem->Assets.Num());
}

FName FWwiseUEAssetStatusColumn::GetColumnId()
{
	return FName("UEStatus");
}

const TSharedRef<SWidget> FWwiseUEAssetStatusColumn::ConstructRowWidget(FWwiseTreeItemPtr TreeItem,
	const STableRow<FWwiseTreeItemPtr>& Row)
{
	return SNew(SHorizontalBox)

	+ SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(SSpacer)
		.Size(8.f)
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(0, 1, 2, 1)
	.VAlign(VAlign_Center)
	[
		SNew(SImage)
		.Image(FAkAudioStyle::GetBrush(TreeItem->ItemType))
		.Visibility((!TreeItem->HasUniqueUAsset() || TreeItem->IsFolder()) ? EVisibility::Collapsed : EVisibility::Visible)
	]

	+ SHorizontalBox::Slot()
	.AutoWidth().
	VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(GetDisplayedName(TreeItem))
	];
}

SHeaderRow::FColumn::FArguments FWwiseUEAssetStatusColumn::ConstructHeaderRowColumn()
{
	auto UEStatusColumnHeader = SHeaderRow::Column(GetColumnId());
	TAttribute<FText> UEStatusLabel;
	UEStatusLabel.Set(FText::FromString("Wwise UAssets Status"));
	UEStatusColumnHeader.DefaultLabel(UEStatusLabel);
	UEStatusColumnHeader.DefaultTooltip(LOCTEXT("UAssetColumn_Tooltip", "The number of UAssets that exist for this Item."));
	return UEStatusColumnHeader;
}

#undef LOCTEXT_NAMESPACE
