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

#if WITH_EDITOR
#include "AkWaapiBlueprints/AkWaapiFieldNamesCustomization.h"
#include "AkAudioDevice.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "DetailWidgetRow.h"
#include "AkWaapiBlueprints/AkWaapiFieldNames.h"
#include "Components/SlateWrapperTypes.h"
#include "AkAudioStyle.h"

#if UE_5_0_OR_LATER
#include "Framework/Docking/TabManager.h"
#endif

#define LOCTEXT_NAMESPACE "AkWaapiFieldNamesCustomization"

TSharedRef<IPropertyTypeCustomization> FAkWaapiFieldNamesCustomization::MakeInstance()
{
	return MakeShareable(new FAkWaapiFieldNamesCustomization());
}

void FAkWaapiFieldNamesCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FieldNameHandle = StructPropertyHandle->GetChildHandle("FieldName");

	if (FieldNameHandle.IsValid())
	{
		TSharedPtr<SWidget> PickerWidget = nullptr;

		PickerWidget = SAssignNew(PickerButton, SButton)
			.ButtonStyle(FAkAudioStyle::Get(), "AudiokineticTools.HoverHintOnly")
			.ToolTipText(LOCTEXT("WwiseFieldNameToolTipText", "Choose A Field Name"))
			.OnClicked(FOnClicked::CreateSP(this, &FAkWaapiFieldNamesCustomization::OnPickContent, FieldNameHandle.ToSharedRef()))
			.ContentPadding(2.0f)
			.ForegroundColor(FSlateColor::UseForeground())
			.IsFocusable(false)
			[
				SNew(SImage)
				.Image(FAkAudioStyle::GetBrush("AudiokineticTools.Button_EllipsisIcon"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			];

			HeaderRow.ValueContent()
			.MinDesiredWidth(125.0f)
			.MaxDesiredWidth(600.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					FieldNameHandle->CreatePropertyValueWidget()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
				.VAlign(VAlign_Center)
				[
					PickerWidget.ToSharedRef()
				]
			]
			.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FAkWaapiFieldNamesCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

FReply FAkWaapiFieldNamesCustomization::OnPickContent(TSharedRef<IPropertyHandle> PropertyHandle)
{
	Window = SNew(SWindow)
		.Title(LOCTEXT("FieldNamePickerWindowTitle", "Choose A Field Name"))
		.SizingRule(ESizingRule::UserSized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(FVector2D(350, 400));

	Window->SetContent(
		SNew(SBorder)
		[
			SNew(SAkWaapiFieldNames)
			.FocusSearchBoxWhenOpened(true)
			.SelectionMode(ESelectionMode::Single)
			.OnSelectionChanged(this, &FAkWaapiFieldNamesCustomization::FieldNameSelectionChanged)
		]
	);

	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	FSlateApplication::Get().AddWindowAsNativeChild(Window.ToSharedRef(), RootWindow.ToSharedRef());
	return FReply::Handled();
}

void FAkWaapiFieldNamesCustomization::FieldNameSelectionChanged(TSharedPtr< FString > in_FieldName, ESelectInfo::Type SelectInfo)
{
	if (in_FieldName.IsValid())
	{
		FieldNameHandle->SetValue(*in_FieldName.Get());
		Window->RequestDestroyWindow();
	}
}

#undef LOCTEXT_NAMESPACE

#endif//WITH_EDITOR
