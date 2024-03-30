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

#include "ReconcileUEAssetStatusColumn.h"

#include "AkAudioStyle.h"
#include "Widgets/SWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FText FReconcileUEAssetStatusColumn::GetDisplayedName(FWwiseReconcileItem TreeItem)
{
	if(!TreeItem.Asset.IsValid())
	{
		return LOCTEXT("None", "None");
	}
	return FText::FromName(TreeItem.Asset.AssetName);
}

FName FReconcileUEAssetStatusColumn::GetColumnId()
{
	return FName("ReconcileUEStatus");
}

const TSharedRef<SWidget> FReconcileUEAssetStatusColumn::ConstructRowWidget(FWwiseReconcileItem TreeItem,
	const STableRow<TSharedPtr<FWwiseReconcileItem>>& Row)
{
	if(!TreeItem.Asset.IsValid())
	{
		return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth().
		VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReconcileNoAsset", "None"))
		];
	}
	
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(0, 1, 2, 1)
	.VAlign(VAlign_Center)
	[
		SNew(SImage)
		.Image(FAkAudioStyle::GetBrush(TreeItem.Asset.GetClass()))
	]
	+ SHorizontalBox::Slot()
	.AutoWidth().
	VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(FText::FromName(TreeItem.Asset.AssetName))
	];
}

SHeaderRow::FColumn::FArguments FReconcileUEAssetStatusColumn::ConstructHeaderRowColumn()
{
	auto UEStatusColumnHeader = SHeaderRow::Column(GetColumnId());
	TAttribute<FText> UEStatusLabel;
	UEStatusLabel.Set(FText::FromString("Current UAsset"));
	UEStatusColumnHeader.DefaultLabel(UEStatusLabel);
	UEStatusColumnHeader.DefaultTooltip(LOCTEXT("CurrentUAssetColumn_Tooltip", "The current UAsset for that item."));
	return UEStatusColumnHeader;
}

#undef LOCTEXT_NAMESPACE
