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

#include "AkMigrationWidgets.h" 
#include "AkSettingsPerUser.h"
#include "WwiseUEFeatures.h"
#include "AkUnrealEditorHelper.h"
#include "WwiseUnrealHelper.h"
#include "AkWaapiClient.h"

#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDesktopPlatform.h"
#include "Interfaces/IMainFrameModule.h"

#include "Widgets/Layout/SHeader.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AkAudio"

namespace MigrationDialogUtils
{
	inline const FSlateBrush* GetExpandableAreaBorderImage(const SExpandableArea& Area)
	{
		if (Area.IsTitleHovered())
		{
			return Area.IsExpanded() ? FAkAppStyle::Get().GetBrush("DetailsView.CategoryTop_Hovered") : FAkAppStyle::Get().GetBrush("DetailsView.CollapsedCategory_Hovered");
		}
		return Area.IsExpanded() ? FAkAppStyle::Get().GetBrush("DetailsView.CategoryTop") : FAkAppStyle::Get().GetBrush("DetailsView.CollapsedCategory");
	}
}

void SMigrationWidget::Construct(const FArguments& InArgs)
{
	bShowBankTransfer = InArgs._ShowBankTransfer;
	bShowMediaCleanup = InArgs._ShowDeprecatedAssetCleanup;
	bShowAssetMigration = InArgs._ShowAssetMigration;
	bShowProjectMigration = InArgs._ShowProjectMigration;
	Dialog = InArgs._Dialog;

	ChildSlot
	[
		SNew(SBox)
		.MaxDesiredWidth(800)
		[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[

			SNew(SBorder)
			.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)

				/// Intro title
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MigrationWelcome",
						"Migrating to Single Source of Truth\n"
					))
					.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
				]

				/// Intro text
				+ SVerticalBox::Slot()
				.Padding(0, 0)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MigrationIntroText",
						"We recommend performing all necessary migration steps as soon as possible, in a single operation. Since the migration operation has an impact on Unreal assets, ensure to checkout all Wwise-related assets from Source control. "
						"Migration will also potentially update settings in your Wwise project. It is important to first check-out, open, and migrate the Wwise Project in Wwise Authoring BEFORE pressing 'Continue'.\n"
					))
					.AutoWrapText(true)
				]

				/// More info hyperlink
				+ SVerticalBox::Slot()
				.Padding(0, 0)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.0f)
					.AutoWidth()
					[
						SNew(SHyperlink)
						.Text(LOCTEXT("MigrationNotesLink", "Please refer to Wwise 2023.1 migration notes for more information about the migration process."))
						.OnNavigate_Lambda([=]{ FPlatformProcess::LaunchURL(TEXT("https://www.audiokinetic.com/library/edge/?source=UE4&id=pg_important_migration_notes_2023_1_0.html"), nullptr, nullptr); })

					]
					+ SHorizontalBox::Slot()
					[
						SNew(SSpacer)
					]
				]

				/// Necessary steps text:
				+ SVerticalBox::Slot()
				.Padding(0, 0)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MigrationIntroNecessarySteps",
						"\nBelow are the necessary migration steps for your project:\n"
					))
					.AutoWrapText(true)
				]

				/// Transfer SoundBanks option
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(BankTransferWidget, SBankTransferWidget)
					.Visibility(this, &SMigrationWidget::GetBankTransferWidgetVisibility)
				]

				/// Deleting deprecated assets
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SAssignNew(DeprecatedAssetCleanupWidget, SDeprecatedAssetCleanupWidget)
					.Visibility(this, &SMigrationWidget::GetMediaCleanupWidgetVisibility)
					.NumDeprecatedAssets(InArgs._NumDeprecatedAssets)
				]

				/// Migrating assets
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SAssignNew(AssetMigrationWidget, SAssetMigrationWidget)
					.Visibility(this, &SMigrationWidget::GetAssetMigrationWidgetVisibility)
				]

				// Migrating Wwise Project options
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SAssignNew(ProjectMigrationWidget, SProjectMigrationWidget)
					.Visibility(this, &SMigrationWidget::GetProjectMigrationWidgetVisibility)
				]
				+ SVerticalBox::Slot()
				.Padding(0, 5)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GeneralInfo",
						"If you choose the cancel option or a subset of operations to perform, you can open this dialog at a later time from the Build Menu under Audiokinetic > Finish Project Migration.\n"
					))
					.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
					.AutoWrapText(true)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(2.0f)
						.AutoWidth()
						[
							SNew(SButton)
							.ButtonStyle(FAkAppStyle::Get(), "FlatButton.Success")
							.ForegroundColor(FLinearColor::White)
							.OnClicked(this, &SMigrationWidget::OnContinueClicked)
							.IsEnabled(this, &SMigrationWidget::CanClickContinue)
							.ToolTipText(this, &SMigrationWidget::GetContinueToolTip)
							[
								SNew(STextBlock)
								.TextStyle(FAkAppStyle::Get(), "FlatButton.DefaultTextStyle")
								.Text(FText::FromString("Continue"))
							]
						]

						+ SHorizontalBox::Slot()
						.Padding(2.0f)
						.AutoWidth()
						[
							SNew(SButton)
							.ButtonStyle(FAkAppStyle::Get(), "FlatButton.Default")
							.ForegroundColor(FLinearColor::White)
							.OnClicked(this, &SMigrationWidget::OnCancelClicked)
							.ToolTipText(FText::FromString("Cancel"))
							[
								SNew(STextBlock)
								.TextStyle(FAkAppStyle::Get(), "FlatButton.DefaultTextStyle")
								.Text(FText::FromString("Cancel"))
							]
						]
					]
				]
			]
		]
		]
	];
}

bool SMigrationWidget::CanClickContinue() const
{
	if (bShowBankTransfer && BankTransferWidget->SoundBankTransferCheckBox->IsChecked())
	{
		if (BankTransferWidget->BankTransferMethod == AkAssetMigration::EBankTransferMode::NoTransfer)
		{
			return false;	
		}
		if (BankTransferWidget->BankTransferMethod == AkAssetMigration::EBankTransferMode::DefinitionFile && BankTransferWidget->SoundBankDefinitionFilePath.IsEmpty())
		{
			return false;
		}
	}
	return true;
}

FText SMigrationWidget::GetContinueToolTip() const
{
	if (bShowBankTransfer && BankTransferWidget->SoundBankTransferCheckBox->IsChecked())
	{
		if (BankTransferWidget->BankTransferMethod == AkAssetMigration::EBankTransferMode::NoTransfer)
		{
			return FText::FromString("Please choose a SoundBank transfer method first");
		}
		if (BankTransferWidget->BankTransferMethod == AkAssetMigration::EBankTransferMode::DefinitionFile && BankTransferWidget->SoundBankDefinitionFilePath.IsEmpty())
		{
			return FText::FromString("Please choose a SoundBank Definition file path first");
		}
	}
	return  FText::FromString("Continue");
}

FReply SMigrationWidget::OnContinueClicked()
{
	FSlateApplication::Get().RequestDestroyWindow(Dialog.ToSharedRef());
	return FReply::Handled();
}

FReply SMigrationWidget::OnCancelClicked()
{
	BankTransferWidget->BankTransferMethod = AkAssetMigration::EBankTransferMode::NoTransfer;
	BankTransferWidget->DeleteSoundBanksCheckBox->SetIsChecked(false);
	BankTransferWidget->TransferAutoLoadCheckBox->SetIsChecked(false);
	DeprecatedAssetCleanupWidget->DeleteAssetsCheckBox->SetIsChecked(false);
	AssetMigrationWidget->MigrateAssetsCheckBox->SetIsChecked(false);
	bCancel = true;

	FSlateApplication::Get().RequestDestroyWindow(Dialog.ToSharedRef());
	return FReply::Handled();
}

EVisibility SMigrationWidget::GetBankTransferWidgetVisibility() const
{
	return bShowBankTransfer ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SMigrationWidget::GetMediaCleanupWidgetVisibility() const
{
	return bShowMediaCleanup ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SMigrationWidget::GetAssetMigrationWidgetVisibility() const
{
	return bShowAssetMigration ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SMigrationWidget::GetProjectMigrationWidgetVisibility() const
{
	return bShowProjectMigration ? EVisibility::Visible : EVisibility::Collapsed;
}

void SBankTransferWidget::Construct(const FArguments& InArgs)
{
	SoundBankDefinitionFilePath = WwiseUnrealHelper::GetProjectDirectory() + "SoundBankDefinition.tsv";

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SounbankMigrationHeader", "Soundbank Migration"))
				.Font(FAkAppStyle::Get().GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.05f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("BankMigrationMessageBegin",
								"AkAudioBanks assets are no longer used in the integration and must be deleted.\n"
								"You can optionally transfer your SoundBank structures defined in Unreal back to Wwise Authoring (before deleting them).\n"
								"The 'Auto Load' property on AkAudioBank assets can also optionally be transferred to the assets that were grouped in them.\n"
								
							))
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.FillHeight(0.10f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.FillHeight(0.10f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 2.0f)
							[
								SAssignNew(SoundBankTransferCheckBox, SCheckBox)
								.IsChecked(ECheckBoxState::Checked)
								.OnCheckStateChanged(this, &SBankTransferWidget::OnCheckedTransferBanks)
								.Content()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("DoTransferMessage", "Transfer SoundBanks To Wwise"))
									.ToolTipText(LOCTEXT("TransferAkAudioBankToolTip", "Create SoundBanks in the Wwise project matching the AkAudioBank assets in Unreal.\nThey will contain the same Events and Aux Busses that were grouped into the Unreal assets."))
									.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
								]
							]

							+ SHorizontalBox::Slot()
							.HAlign(EHorizontalAlignment::HAlign_Left)
							.AutoWidth()
							.Padding(8.0f, 2.0f)
							[
								SNew(SBorder)
								.BorderBackgroundColor(this, &SBankTransferWidget::GetDropDownBorderColour)
								.ColorAndOpacity(this, &SBankTransferWidget::GetDropDownColour)
								.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
								[
									SNew(SComboButton)
									.ToolTipText(LOCTEXT("ComboButtonTip", "Choose a transfer method from the drop-down menu."))
									.OnGetMenuContent(this, &SBankTransferWidget::OnGetTransferMethodMenu)
									.ContentPadding(FMargin(0))
									.Visibility(this, &SBankTransferWidget::GetTransferMethodVisibility)
									.ButtonContent()
									[
										SNew(STextBlock)
										.Text(this, &SBankTransferWidget::GetTransferMethodText)
									]
								]
							]
						]
						+ SVerticalBox::Slot()
						.Padding(0, 5)
						.AutoHeight()
						[
							SAssignNew(SoundBankDefinitionFilePathPicker, SDefinitionFilePicker)
							.FilePath(SoundBankDefinitionFilePath)
							.OnFileChanged(this, &SBankTransferWidget::SetDefinitionFilePath)
							.Visibility(this, &SBankTransferWidget::GetDefinitionFilePathVisibility)
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(TransferAutoLoadCheckBox, SCheckBox)
								.IsChecked(ECheckBoxState::Checked)
								.ToolTipText(LOCTEXT("TransferAutoloadTooltip", "Transfer SoundBank Auto Load property from deprecated AkAudioBank assets to the AkAudioType assets that were grouped in them."))
								.Content()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("TransferAutoloadMessage", "Transfer Auto Load property from AkAudioBank assets to AkAudioType assets"))
									.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
								]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(DeleteSoundBanksCheckBox, SCheckBox)
							.IsChecked(ECheckBoxState::Checked)
							.Content()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("DeleteAkAudioBankMessage", " Delete AkAudioBank Assets"))
								.ToolTipText(LOCTEXT("DeleteAkAudioBankToolTip", "Delete all AkAudioBank assets in the unreal project."))
								.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
							]
						]
					]
				]
			]
		]
	];
}

EVisibility SBankTransferWidget::GetDefinitionFilePathVisibility() const
{
	return SoundBankTransferCheckBox->IsChecked() && BankTransferMethod == AkAssetMigration::EBankTransferMode::DefinitionFile ? EVisibility::Visible : EVisibility::Collapsed;
}

void SBankTransferWidget::SetDefinitionFilePath(const FString& PickedPath)
{
	SoundBankDefinitionFilePath = PickedPath;
}

void SBankTransferWidget::SetTransferMethod(AkAssetMigration::EBankTransferMode TransferMethod)
{
	BankTransferMethod = TransferMethod;
}

void SBankTransferWidget::OnCheckedTransferBanks(ECheckBoxState NewState)
{
	if (NewState != ECheckBoxState::Checked)
	{
		BankTransferMethod = AkAssetMigration::EBankTransferMode::NoTransfer;
	}
}

EVisibility SBankTransferWidget::GetTransferMethodVisibility() const
{
	return SoundBankTransferCheckBox->IsChecked() ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SBankTransferWidget::GetTransferMethodText() const
{
	if (BankTransferMethod == AkAssetMigration::EBankTransferMode::DefinitionFile)
	{
		return 	LOCTEXT("SoundBankDefinition", "Create SoundBank Definition File");
	}
	else if (BankTransferMethod == AkAssetMigration::EBankTransferMode::WAAPI)
	{
		return LOCTEXT("WaapiTransfer", "WAAPI");
	}

	return LOCTEXT("NoTransferMethodSet", "Choose a transfer method...");
}

FSlateColor SBankTransferWidget::GetDropDownBorderColour() const
{
	if (BankTransferMethod ==  AkAssetMigration::EBankTransferMode::NoTransfer && SoundBankTransferCheckBox->IsChecked())
	{
		return FLinearColor::Red;
	}

	return FLinearColor::White;
}

FLinearColor SBankTransferWidget::GetDropDownColour() const
{
	if (BankTransferMethod ==  AkAssetMigration::EBankTransferMode::NoTransfer && SoundBankTransferCheckBox->IsChecked())
	{
		return FLinearColor::Red;
	}

	return FLinearColor::White;
}

TSharedRef<SWidget> SBankTransferWidget::OnGetTransferMethodMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("SoundBankDefinition", "Create SoundBank Definition File"),
		FText(),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SBankTransferWidget::SetTransferMethod, AkAssetMigration::EBankTransferMode::DefinitionFile)
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("WaapiTransferMenuItemText","WAAPI"),
		FText(),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &SBankTransferWidget::SetTransferMethod, AkAssetMigration::EBankTransferMode::WAAPI),
			FCanExecuteAction::CreateSP(this, &SBankTransferWidget::CheckWaapiConnection)
		)
	);
	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

bool SBankTransferWidget::CheckWaapiConnection() const
{
	bool bWaapiConnected = false;
	if (auto UserSettings =  GetDefault<UAkSettingsPerUser>())
	{
		if (!UserSettings->bAutoConnectToWAAPI)
		{
			LOCTEXT("WaapiTransferMenuItemText","WAAPI (Auto Connect to WAAPI disabled in user settings)");
		}
		else
		{
			FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
			bWaapiConnected = WaapiClient && WaapiClient->IsConnected();
			if (!bWaapiConnected)
			{
				LOCTEXT("WaapiTransferMenuItemText","WAAPI (WAAPI connection not established)");
			}
		}
	}

	if (bWaapiConnected)
	{
		LOCTEXT("WaapiTransferMenuItemText","WAAPI");
	}
	return bWaapiConnected;
}

void SDeprecatedAssetCleanupWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MediaCleanupHeader", "Delete deprecated assets"))
				.Font(FAkAppStyle::Get().GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
				SNew(SBorder)
				.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.Padding(4.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.0f)
					.HAlign(EHorizontalAlignment::HAlign_Left)
					[
						SNew(STextBlock)
						.Text(FText::FormatOrdered(LOCTEXT("AssetCleanupMessageBegin",
							"AkMediaAsset, AkFolder and AkPlatformAssetData have been deprecated, all assets of this type will be deleted from the project. "
							"The project currently contains {0} such assets.\n"), FText::FromString(FString::FromInt(InArgs._NumDeprecatedAssets))
						))
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 2.0f)
					.HAlign(EHorizontalAlignment::HAlign_Left)
					[
						SAssignNew(DeleteAssetsCheckBox, SCheckBox)
						.IsChecked(ECheckBoxState::Checked)
						.Content()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DeleteAssetMessage", " Delete deprecated assets"))
							.ToolTipText(LOCTEXT("DeleteMediaToolTip", "Delete all deprecated assets that are still in the project"))
							.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
							]
						]
					]
				]
			]
		]
	];
}

void SAssetMigrationWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UpdateAssetsHeader", "Migrate Wwise Assets"))
				.Font(FAkAppStyle::Get().GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.05f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AssetMigrationMessageBegin",
								"Wwise asset properties have changed and they no longer serialize SoundBank or media binary data.\n"
								"Wwise asset properties will be updated and reserialized to the disk."
							))
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(MigrateAssetsCheckBox, SCheckBox)
							.IsChecked(ECheckBoxState::Checked)
							.Content()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("MigrateAssetsMessage", " Migrate Wwise assets"))
								.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
								.ToolTipText(LOCTEXT("MigrateAssetsTooltip", "Dirty and save all Wwise assets"))
							]
						]
					]
				]
			]
		]
	];
}

void SProjectMigrationWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ExpandableSection, SExpandableArea)
			.InitiallyCollapsed(false)
			.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
			.BorderImage_Lambda([this]() { return MigrationDialogUtils::GetExpandableAreaBorderImage(*ExpandableSection); })
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ProjectChangesHeader", "Update Wwise and Unreal project settings"))
				.Font(FAkAppStyle::Get().GetFontStyle("DetailsView.CategoryFontStyle"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(0.05f)
						[
							SNew(SSpacer)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ProjectSettingsMigrationMessageBegin",
								"SoundBank generation settings in your Wwise project, and the Wwise Integration settings in your Unreal project, need to be updated.\n"
							))
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SAssignNew(AutoMigrateCheckbox, SCheckBox)
							.IsChecked(ECheckBoxState::Checked)
							.Content()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("UpdateProjectMessage", "Update project settings"))
								.Font(FAkAppStyle::Get().GetFontStyle("StandardDialog.LargeFont"))
								.ToolTipText(LOCTEXT("UpdateProjectTooltip", "Update Unreal project settings"))
							]
						]
						+ SVerticalBox::Slot()
						.Padding(0, 5)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AkGeneratedSoundBanksFolderMigration",
							"\nGeneratedSoundBanks folder location:"
							))
							.Visibility(this, &SProjectMigrationWidget::GetPathVisibility)

						]
						+ SVerticalBox::Slot()
						.Padding(0, 5)
						.AutoHeight()
						[
							SAssignNew(GeneratedSoundBanksFolderPickerWidget, SDirectoryPicker)
							.Directory(WwiseUnrealHelper::GetSoundBankDirectory())
							.Visibility(this, &SProjectMigrationWidget::GetPathVisibility)
						]
					]
				]
			]
		]
	];
}

EVisibility SProjectMigrationWidget::GetPathVisibility() const
{
	return AutoMigrateCheckbox->IsChecked() ? EVisibility::Visible : EVisibility::Collapsed;
}

void SBankMigrationFailureWidget::Construct(const FArguments& InArgs)
{
	Dialog = InArgs._Dialog;
	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(4.0f)

				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BankMigrationFailureHeader", "Errors encountered while transfering banks"))
					.Font(FAkAppStyle::Get().GetFontStyle("DetailsView.CategoryFontStyle"))
					.ShadowOffset(FVector2D(1.0f, 1.0f))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SBorder)
					.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.MaxHeight(600)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(SMultiLineEditableTextBox)
							.Text(InArgs._ErrorMessage)
							.AutoWrapText(true)
							.IsReadOnly(true)
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Padding(4.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.0f)
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FAkAppStyle::Get(), "FlatButton.Default")
						.ForegroundColor(FLinearColor::White)
						.OnClicked(this, &SBankMigrationFailureWidget::OnIgnoreClicked)
						.ToolTipText(FText::FromString("Ignore warnings and continue migration"))
						[
							SNew(STextBlock)
							.TextStyle(FAkAppStyle::Get(), "FlatButton.DefaultTextStyle")
							.Text(FText::FromString("Ignore Errors and Continue"))
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(2.0f)
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FAkAppStyle::Get(), "FlatButton.Default")
						.ForegroundColor(FLinearColor::White)
						.OnClicked(this, &SBankMigrationFailureWidget::OnCancelClicked)
						.ToolTipText(FText::FromString("Cancel the migration process to re-attempt at a later time"))
						[
							SNew(STextBlock)
							.TextStyle(FAkAppStyle::Get(), "FlatButton.DefaultTextStyle")
							.Text(FText::FromString("Cancel Migration"))
						]
					]
				]
			]
		];
}

FReply SBankMigrationFailureWidget::OnCancelClicked()
{
	bCancel = true;
	FSlateApplication::Get().RequestDestroyWindow(Dialog.ToSharedRef());
	return FReply::Handled();
}

FReply SBankMigrationFailureWidget::OnIgnoreClicked()
{
	bCancel = false;
	FSlateApplication::Get().RequestDestroyWindow(Dialog.ToSharedRef());
	return FReply::Handled();
}

void SDefinitionFilePicker::Construct( const FArguments& InArgs )
{
	OnFileChanged = InArgs._OnFileChanged;

	FilePath = InArgs._FilePath;
	Message = InArgs._Message;

	TSharedPtr<SButton> OpenButton;
	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SAssignNew(EditableTextBox, SEditableTextBox)
			.Text(this, &SDefinitionFilePicker::GetFilePathText)
			.OnTextChanged(this, &SDefinitionFilePicker::OnFileTextChanged)
			.OnTextCommitted(this, &SDefinitionFilePicker::OnFileTextCommitted)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f, 0.0f, 0.0f)
		[
			SAssignNew(OpenButton, SButton)
			.ToolTipText(Message)
			.OnClicked(this, &SDefinitionFilePicker::BrowseForFile)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("...")))
			]
		]
	];

	OpenButton->SetEnabled(InArgs._IsEnabled);
}

void SDefinitionFilePicker::OnFileTextChanged(const FText& InFilePath)
{
	FilePath = InFilePath.ToString();
}

void SDefinitionFilePicker::OnFileTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FilePath = InText.ToString();
	OnFileChanged.ExecuteIfBound(FilePath);
}
	
FString SDefinitionFilePicker::GetFilePath() const
{
	return FilePath;
}

FText SDefinitionFilePicker::GetFilePathText() const
{
	return FText::FromString(GetFilePath());
}

bool SDefinitionFilePicker::OpenDefinitionFilePicker(FString& OutFilePath, const FString& DefaultFilePath)
{
	TArray<FString> OpenFileNames;
	bool bOpened = false;

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = NULL;

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		bool bAllowMultiSelect = false;
		FString FileTypes = TEXT("TSV Files (*.tsv)|*.tsv");
		FString DefaultFolder = FPaths::GetPath(DefaultFilePath);
		FString DefaultFile =  FPaths::GetCleanFilename(DefaultFilePath);
		const FString Title = TEXT("Choose location for definition file");
		bOpened = DesktopPlatform->SaveFileDialog(
			ParentWindowWindowHandle,
			Title,
			*DefaultFolder,
			*DefaultFile,
			FileTypes,
			bAllowMultiSelect,
			OpenFileNames
		);
	}
	if (bOpened)
	{
		OutFilePath = FPaths::ConvertRelativePathToFull(OpenFileNames[0]);
	}

	return bOpened;
}

FReply SDefinitionFilePicker::BrowseForFile()
{
	if ( OpenDefinitionFilePicker(FilePath, FilePath) )
	{
		OnFileChanged.ExecuteIfBound(FilePath);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
