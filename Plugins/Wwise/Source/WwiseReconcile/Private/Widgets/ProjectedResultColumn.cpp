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

#include "ProjectedResultColumn.h"

#include "AkAudioStyle.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "Widgets/SWidget.h"
#include "AkUnrealAssetDataHelper.h"
#include "WwiseUnrealHelper.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "AssetViewUtils.h"
#include "Misc/EngineBuildSettings.h"

#define LOCTEXT_NAMESPACE "AkAudio"

const FLinearColor WarningColor = FLinearColor(1.f, 0.33f, 0);
const FLinearColor ErrorColor = FLinearColor(1.f, 0, 0);

FName FProjectedResultColumn::GetColumnId()
{
	return FName("ReconcileProjectedResults");
}

const TSharedRef<SWidget> FProjectedResultColumn::ConstructRowWidget(FWwiseReconcileItem TreeItem,
	const STableRow<TSharedPtr<FWwiseReconcileItem>>& Row)
{
	if (EnumHasAllFlags(TreeItem.OperationRequired, EWwiseReconcileOperationFlags::Delete))
	{
		bool bReferenced = false;
		bool bReferencedByUndo = false;

		auto Asset = TreeItem.Asset.GetAsset();
		if (!IsValid(Asset))
		{
			return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("ReconcileCannotGetAsset", "Asset {0} cannot be found. Please refresh the Browser."), FText::FromString(TreeItem.Asset.AssetName.ToString())))
				.ColorAndOpacity(ErrorColor)
			];
		}

		ObjectTools::GatherObjectReferencersForDeletion(TreeItem.Asset.GetAsset(), bReferenced, bReferencedByUndo);

		if (bReferenced || bReferencedByUndo)
		{
			return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("ReconcileReferenced", "Asset {0} is referenced somewhere. Please delete this asset manually."), FText::FromString(TreeItem.Asset.AssetName.ToString())))
				.ColorAndOpacity(WarningColor)
			];
		}
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ReconcileDelete", "This Asset will be Deleted"))
			];
	}

	auto WwiseRef = TreeItem.WwiseAnyRef.WwiseAnyRef;
	FName AssetName = AkUnrealAssetDataHelper::GetAssetDefaultName(WwiseRef);
	FString AssetPackagePath = IWwiseReconcile::Get()->GetAssetPackagePath(*WwiseRef);
	int PackageLength = AssetViewUtils::GetPackageLengthForCooking(AssetPackagePath / AssetName.ToString(), FEngineBuildSettings::IsInternalBuild());
	int MaxPath = AssetViewUtils::GetMaxCookPathLen();

	if (PackageLength > MaxPath || PackageLength >= NAME_SIZE)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth().
			VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ReconcileTooLong", "Asset path exceeds platform limit"))
				.ColorAndOpacity(WarningColor)
			];
	}

	if(EnumHasAnyFlags(TreeItem.OperationRequired, EWwiseReconcileOperationFlags::Create | EWwiseReconcileOperationFlags::RenameExisting))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
#if UE_5_1_OR_LATER
		FAssetData Asset = AssetRegistryModule.GetRegistry().GetAssetByObjectPath(AssetPackagePath / AkUnrealAssetDataHelper::GetAssetDefaultName(WwiseRef).ToString() + "." + AkUnrealAssetDataHelper::GetAssetDefaultName(WwiseRef).ToString());
#else
		FName AssetPath = FName(AssetPackagePath / AkUnrealAssetDataHelper::GetAssetDefaultName(WwiseRef).ToString() + "." + AkUnrealAssetDataHelper::GetAssetDefaultName(WwiseRef).ToString());
		FAssetData Asset = AssetRegistryModule.GetRegistry().GetAssetByObjectPath(AssetPath);
#endif
		if(Asset.IsValid())
		{
			FString DisplayedAssetPath = AssetPackagePath / AkUnrealAssetDataHelper::GetAssetDefaultName(WwiseRef).ToString();
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth().
				VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("ReconcileExist", "Asset already exists at path {0}"), FText::FromString(DisplayedAssetPath)))
					.ColorAndOpacity(FLinearColor(WarningColor))
				];
		}
	}
	
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 1, 2, 1)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(FAkAudioStyle::GetBrush(TreeItem.WwiseAnyRef.WwiseAnyRef->GetType()))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth().
		VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(AssetPackagePath / AssetName.ToString()))
		];
}

SHeaderRow::FColumn::FArguments FProjectedResultColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments ProjectedResultColumnHeader = SHeaderRow::Column(GetColumnId());
	TAttribute<FText> StatusLabel;
	StatusLabel.Set(FText::FromString("Projected Result"));
	ProjectedResultColumnHeader.DefaultLabel(StatusLabel);
	ProjectedResultColumnHeader.DefaultTooltip(LOCTEXT("ProjectedResult_Tooltip", "The expected result of Reconciling an asset."));
#if UE_5_0_OR_LATER
	ProjectedResultColumnHeader.FillSized(600.f);
#else
	ProjectedResultColumnHeader.ManualWidth(600.f);
#endif
	return ProjectedResultColumnHeader;
}

#undef LOCTEXT_NAMESPACE
