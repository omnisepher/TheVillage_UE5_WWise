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

#include "AkSettingsDetailsCustomization.h"
#include "AkSettings.h"
#include "WwiseUEFeatures.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IStructureDetailsView.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "AudiokineticTools"

/** Dialog widget used to display function properties */
class SDecayKeyEntryDialog : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SDecayKeyEntryDialog) {}
	SLATE_END_ARGS()

	// We call this from a delegate that is called when the window has correctly been added to the slate application.
	// See FAkSettingsDetailsCustomization::InsertKeyModal.
	void FocusNumericEntryBox()
	{
		if (numberBox != nullptr)
		{
			FSlateApplication::Get().ClearAllUserFocus();
			FSlateApplication::Get().SetAllUserFocus(numberBox);
			FSlateApplication::Get().ClearKeyboardFocus();
			FSlateApplication::Get().SetKeyboardFocus(numberBox);
		}
	}

	void Construct(const FArguments& InArgs, TWeakPtr<SWindow> InParentWindow, float& decay)
	{
		bCommitted = false;
		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(2.0f)
					.AutoHeight()
					[
						SAssignNew(numberBox, SNumericEntryBox<float>)
						.MinValue(0.0f)
						.MaxValue(100.0f)
						.Value_Lambda([&decay]() {return decay;})
						.OnValueCommitted_Lambda([this, InParentWindow, &decay](const float& value, ETextCommit::Type commitType)
						{
							if (commitType == ETextCommit::Type::OnCleared)
								decay = 0.0f;
							else
								decay = value;

							if (commitType == ETextCommit::OnEnter)
							{
								bCommitted = true;
								if (InParentWindow.IsValid())
								{
									InParentWindow.Pin()->RequestDestroyWindow();
								}
							}
						})
						.OnValueChanged_Lambda([&decay](const float& value)
						{
							decay = value;
						})
					]
					+ SVerticalBox::Slot()
					.Padding(2.0f)
					.AutoHeight()
					[
						SNew(SButton)
						.ForegroundColor(FLinearColor::White)
						.OnClicked_Lambda([this, InParentWindow, InArgs]()
						{
							if (InParentWindow.IsValid())
							{
								InParentWindow.Pin()->RequestDestroyWindow();
							}
							bCommitted = true;
							return FReply::Handled();
							})
						.ToolTipText(FText::FromString("Insert given key value"))
						[
							SNew(STextBlock)
							.TextStyle(FAkAppStyle::Get(), "FlatButton.DefaultTextStyle")
							.Text(FText::FromString("Insert"))
						]
					]
				]
			]
		];
	}

	bool bCommitted = false;
	TSharedPtr<SNumericEntryBox<float>> numberBox = nullptr;
};

/** Dialog widget used to display function properties */
class SClearWarningDialog : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SClearWarningDialog) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakPtr<SWindow> InParentWindow)
	{
		bOKPressed = false;
		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(2.0f)
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Warning: This will remove all aux bus values in the map. Do you want to continue?"))
					]
				]
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
						.OnClicked_Lambda([this, InParentWindow, InArgs]()
						{
							if (InParentWindow.IsValid())
							{
								InParentWindow.Pin()->RequestDestroyWindow();
							}
							bOKPressed = true;
							return FReply::Handled();
						})
						.ToolTipText(FText::FromString("Clear aux bus assignment map"))
						[
							SNew(STextBlock)
							.TextStyle(FAkAppStyle::Get(), "FlatButton.DefaultTextStyle")
							.Text(FText::FromString("Clear"))
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(2.0f)
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FAkAppStyle::Get(), "FlatButton.Default")
						.ForegroundColor(FLinearColor::White)
						.OnClicked_Lambda([this, InParentWindow, InArgs]()
						{
							if (InParentWindow.IsValid())
							{
								InParentWindow.Pin()->RequestDestroyWindow();
							}
							return FReply::Handled();
						})
						.ToolTipText(FText::FromString("Cancel"))
						[
							SNew(STextBlock)
							.TextStyle(FAkAppStyle::Get(), "FlatButton.DefaultTextStyle")
							.Text(FText::FromString("Cancel"))
						]
					]
				]
			]
		];
	}

	bool bOKPressed;
};

TSharedRef<IDetailCustomization> FAkSettingsDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FAkSettingsDetailsCustomization);
}

void FAkSettingsDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	IDetailCategoryBuilder& GeometryCategoryBuilder = DetailLayout.EditCategory("Geometry Surface Properties", FText::GetEmpty(), ECategoryPriority::Uncommon);
	GeometryCategoryBuilder.AddCustomRow(FText::FromString("Update Geometry Map")).WholeRowContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Verify and Update"))
				.ToolTipText(FText::FromString("Verify each row of the Geometry Surface Properties Table below and remove rows with an invalid Physical Material."))
				.OnClicked_Raw(this, &FAkSettingsDetailsCustomization::VerifyAndUpdateGeometrySurfacePropertiesTable)
			]
			+ SHorizontalBox::Slot().FillWidth(8)
		]
	];
}

FReply FAkSettingsDetailsCustomization::VerifyAndUpdateGeometrySurfacePropertiesTable()
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (AkSettings != nullptr)
		AkSettings->VerifyAndUpdateGeometrySurfacePropertiesTable();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
