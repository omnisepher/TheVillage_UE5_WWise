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

#include "ReconcileOperationColumn.h"

#include "AkAudioStyle.h"
#include "Widgets/SWidget.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "Widgets/Text/STextBlock.h"
#include "Wwise/WwiseReconcile.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FText FReconcileOperationColumn::GetOperationText(FWwiseReconcileItem TreeItem)
{
	
	if (EnumHasAllFlags(TreeItem.OperationRequired, 
		EWwiseReconcileOperationFlags::UpdateExisting | EWwiseReconcileOperationFlags::RenameExisting))
	{
		return LOCTEXT("WwiseReconcileUpdateRename", "Update and Rename");
	}
	switch (TreeItem.OperationRequired)
	{
	case EWwiseReconcileOperationFlags::Create:
		return LOCTEXT("WwiseReconcileCreate", "Create");
	case EWwiseReconcileOperationFlags::UpdateExisting:
		return LOCTEXT("WwiseReconcileUpdate", "Update");
	case EWwiseReconcileOperationFlags::RenameExisting:
		return LOCTEXT("WwiseReconcileRename", "Rename");
	case EWwiseReconcileOperationFlags::Delete:
		return LOCTEXT("WwiseReconcileDelete", "Delete");
	}
	return LOCTEXT("WwiseReconcileEmpty", "");
}

FName FReconcileOperationColumn::GetColumnId()
{
	return FName("ReconcileOperations");
}

const TSharedRef<SWidget> FReconcileOperationColumn::ConstructRowWidget(FWwiseReconcileItem TreeItem,
	const STableRow<TSharedPtr<FWwiseReconcileItem>>& Row)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(GetOperationText(TreeItem))
		];
}

SHeaderRow::FColumn::FArguments FReconcileOperationColumn::ConstructHeaderRowColumn()
{
	auto NameColumnHeader = SHeaderRow::Column(GetColumnId());
	NameColumnHeader.ColumnId(TEXT("WwiseItem"));
	NameColumnHeader.FixedWidth(125.f);
	TAttribute<FText> Label;
	Label.Set(FText::FromString("UAsset Operation"));
	NameColumnHeader.DefaultLabel(Label);
	NameColumnHeader.DefaultTooltip(LOCTEXT("WwiseReconcileOperationColumn_Tooltip", "The operation that will happen on the asset upon Reconciling."));
	return NameColumnHeader;
}
