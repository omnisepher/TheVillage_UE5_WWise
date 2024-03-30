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

#include "WwiseStatusColumn.h"

#include "WwiseBrowserForwards.h"
#include "WwiseBrowserHelpers.h"
#include "DataSource/WaapiDataSource.h"
#include "Widgets/SWidget.h"
#include "Widgets/Layout/SSpacer.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FWwiseStatusColumn::FWwiseStatusColumn(SWwiseBrowser& WwiseBrowser)
	: WwiseBrowserWeak(StaticCastSharedRef<SWwiseBrowser>(WwiseBrowser.AsShared()))
{
}

FText FWwiseStatusColumn::GetDisplayedName(FWwiseTreeItemPtr TreeItem)
{
	if(!TreeItem->ShouldDisplayInfo() || WwiseBrowserWeak.Pin()->IsWaapiAvailable() != EWwiseConnectionStatus::Connected)
	{
		return LOCTEXT("WwiseBrowserEmpty", "");
	}
	if(TreeItem->IsSoundBankUpToDate())
	{
		return LOCTEXT("WwiseStatusColumnUpToDate", "SoundBank Up to Date");
	}
	else if (TreeItem->IsRenamedInWwise())
	{
		return LOCTEXT("WwiseStatusColumnRenamed", "Renamed in Wwise");
	}
	else if(TreeItem->IsNewInWwise())
	{
		return LOCTEXT("WwiseStatusColumnNewInWwise", "New In Wwise");
	}
	else if(TreeItem->IsDeletedInWwise())
	{
		return LOCTEXT("WwiseStatusColumnNewInWwise", "Deleted In Wwise");
	}
	else if (TreeItem->IsMovedInWwise())
	{
		return LOCTEXT("WwiseStatusColumnMovedInWwise", "Moved In Wwise");
	}
	return LOCTEXT("WwiseStatusColumnNotInWwise", "Not in Wwise or SoundBank");
}

FName FWwiseStatusColumn::GetColumnId()
{
	return FName("WwiseStatus");
}

const TSharedRef<SWidget> FWwiseStatusColumn::ConstructRowWidget(FWwiseTreeItemPtr TreeItem,
	const STableRow<FWwiseTreeItemPtr>& Row)
{

	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth().
	VAlign(VAlign_Center)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(8.f)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(GetDisplayedName(TreeItem))
			.ColorAndOpacity((WwiseBrowserWeak.Pin()->IsWaapiAvailable() != EWwiseConnectionStatus::Connected) ? FLinearColor::Gray : WwiseBrowserHelpers::GetTextColor(TreeItem->IsSoundBankUpToDate()))
		]
	];
}

SHeaderRow::FColumn::FArguments FWwiseStatusColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments WwiseStatusColumnHeader = SHeaderRow::Column(GetColumnId());
	TAttribute<FText> StatusLabel;
	StatusLabel.Set(FText::FromString("Wwise Project vs SoundBanks"));
	WwiseStatusColumnHeader.DefaultLabel(StatusLabel);
	WwiseStatusColumnHeader.DefaultTooltip(LOCTEXT("WwiseColumn_Tooltip", "The status of the name and ID in the Generated SoundBanks compared to the name and ID in the Wwise Project. The Wwise Project must be open for this column to display information."));
	return WwiseStatusColumnHeader;
}

#undef LOCTEXT_NAMESPACE
