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

#include "SoundBankStatusColumn.h"

#include "WwiseBrowserHelpers.h"
#include "Widgets/SWidget.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FText FSoundBankStatusColumn::GetDisplayedName(FWwiseTreeItemPtr TreeItem)
{
	if(!TreeItem->ShouldDisplayInfo())
	{
		return LOCTEXT("WwiseBrowserEmpty", "");
	}
	if(TreeItem->IsUAssetUpToDate())
	{
		return LOCTEXT("UAssetStatusUpToDate", "UAsset Up to Date");
	}
	else if(TreeItem->IsUAssetOutOfDate())
	{
		return LOCTEXT("UAssetOutOfDate", "UAsset Needs Update");
	}
	else if(TreeItem->IsRenamedInSoundBank())
	{
		return LOCTEXT("UAssetStatusRename", "Renamed in SoundBank");
	}
	else if(TreeItem->IsUAssetOrphaned())
	{
		return LOCTEXT("UAssetStatusOrphaned", "UAsset Orphaned");
	}
	else if(TreeItem->HasMultipleUAssets())
	{
		return LOCTEXT("UAssetStatusMultiple", "Multiple UAssets");
	}
	else if(TreeItem->IsUAssetMissing())
	{
		return LOCTEXT("UAssetStatusMissing", "UAsset Missing");
	}
	return LOCTEXT("UAssetStatusNotInSoundBank", "Not in SoundBank or Unreal");
}

FName FSoundBankStatusColumn::GetColumnId()
{
	return FName("SoundBankStatus");
}

const TSharedRef<SWidget> FSoundBankStatusColumn::ConstructRowWidget(FWwiseTreeItemPtr TreeItem,
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
		.AutoWidth().
		VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(GetDisplayedName(TreeItem))
			.ColorAndOpacity(WwiseBrowserHelpers::GetTextColor(TreeItem->IsUAssetUpToDate()))
		];
}

SHeaderRow::FColumn::FArguments FSoundBankStatusColumn::ConstructHeaderRowColumn()
{
	auto UEStatusColumnHeader = SHeaderRow::Column(GetColumnId());
	TAttribute<FText> UEStatusLabel;
	UEStatusLabel.Set(FText::FromString("SoundBanks vs UAssets"));
	UEStatusColumnHeader.DefaultLabel(UEStatusLabel);
	UEStatusColumnHeader.DefaultTooltip(LOCTEXT("SoundBankColumn_Tooltip", "The status of the UAssets compared to the information in the Generated SoundBanks."));
	return UEStatusColumnHeader;
}

#undef LOCTEXT_NAMESPACE
