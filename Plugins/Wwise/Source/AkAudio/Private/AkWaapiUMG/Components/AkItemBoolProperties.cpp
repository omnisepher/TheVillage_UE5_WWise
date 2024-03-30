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

/*------------------------------------------------------------------------------------
includes.
------------------------------------------------------------------------------------*/

#include "AkWaapiUMG/Components/AkItemBoolProperties.h"
#include "AkAudioDevice.h"
#include "Widgets/Input/SButton.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "AkWaapiUMG/Components/WwiseBoolPropertyDragDropOp.h"

/*------------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------------*/
#define LOCTEXT_NAMESPACE "AkWaapiUMG"

/*------------------------------------------------------------------------------------
UAkItemBoolPropertiesConv
------------------------------------------------------------------------------------*/

UAkItemBoolPropertiesConv::UAkItemBoolPropertiesConv(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Property initialization
}

FString UAkItemBoolPropertiesConv::Conv_FAkBoolPropertyToControlToString(const FAkBoolPropertyToControl& INAkBoolPropertyToControl)
{
	return INAkBoolPropertyToControl.ItemProperty;
}

FText UAkItemBoolPropertiesConv::Conv_FAkBoolPropertyToControlToText(const FAkBoolPropertyToControl& INAkBoolPropertyToControl)
{
	return FText::FromString(*INAkBoolPropertyToControl.ItemProperty);
}
/*------------------------------------------------------------------------------------
Implementation
------------------------------------------------------------------------------------*/
/////////////////////////////////////////////////////
// UAkItemBoolProperties

UAkItemBoolProperties::UAkItemBoolProperties(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

TSharedRef<SWidget> UAkItemBoolProperties::RebuildWidget()
{
    bool autoFocus = GetWorld()->WorldType != EWorldType::Game &&
                     GetWorld()->WorldType != EWorldType::PIE &&
                     GetWorld()->WorldType != EWorldType::GamePreview;
	return
	SNew(SBorder)
	.BorderImage(FAkAudioStyle::GetBrush("AudiokineticTools.GroupBorder"))
	.Padding(FMargin(0.0f, 3.0f))
	.BorderBackgroundColor(FLinearColor(.6, .6, .6, 1.0f))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(PropertyTextBlock, STextBlock)
				.IsEnabled(true)
				.ToolTipText(LOCTEXT("editable_Tooltip", "Property Name"))
				.MinDesiredWidth(300.f)
				.Text(FText::FromString(TEXT("Choose a Boolean property")))
			]
		]
		+ SVerticalBox::Slot()
		[
			SAssignNew(WwiseProperties, SAkItemBoolProperties)
			.FocusSearchBoxWhenOpened(autoFocus)
			.SelectionMode(ESelectionMode::Single)
			.OnSelectionChanged(FOnSelectionChanged::CreateUObject(this, &UAkItemBoolProperties::PropertySelectionChanged))
			.OnDragDetected(FOnDragDetected::CreateUObject(this, &UAkItemBoolProperties::HandleOnDragDetected))
		]
	];
}

void UAkItemBoolProperties::PropertySelectionChanged(TSharedPtr< FString > PropertySelected, ESelectInfo::Type SelectInfo)
{
	if (PropertySelected.IsValid())
	{
		const auto& PropertySelectedString = *PropertySelected.Get();
		PropertyTextBlock->SetText(FText::FromString(PropertySelectedString));

		if (OnSelectionChanged.IsBound())
		{
			OnSelectionChanged.Broadcast(PropertySelectedString);
		}
	}
}

FReply UAkItemBoolProperties::HandleOnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && WwiseProperties.IsValid())
	{
		const TArray<TSharedPtr<FString>>& PropertySelected = WwiseProperties->GetSelectedProperties();
		if (PropertySelected.Num() == 1)
		{		
			if(OnPropertyDragged.IsBound())
			{
				OnPropertyDragged.Broadcast(*PropertySelected[0].Get());
			}
			return FReply::Handled().BeginDragDrop(FWwiseBoolPropertyDragDropOp::New(PropertySelected));
		}
	}
	return FReply::Unhandled();
}

FString UAkItemBoolProperties::GetSelectedProperty() const
{
	const TArray<TSharedPtr<FString>> selectedList = WwiseProperties->GetSelectedProperties();
	return selectedList.Num()? *selectedList[0] : TEXT("");
}

FString UAkItemBoolProperties::GetSearchText() const
{
	return WwiseProperties->GetSearchText();
}

void UAkItemBoolProperties::SetSearchText(const FString& newText)
{
	WwiseProperties->SetSearchText(newText);
}

void UAkItemBoolProperties::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	PropertyTextBlock.Reset();
	WwiseProperties.Reset();
}

#if WITH_EDITOR
const FText UAkItemBoolProperties::GetPaletteCategory()
{
	return LOCTEXT("Wwise", "Wwise");
}

#endif
//
/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
