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

#pragma once

#include "CoreMinimal.h"
#include "Input/DragAndDrop.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "AkAudioStyle.h"

class AKAUDIO_API FWwiseUmgDragDropOp : public FDragDropOperation
{
public:

	DRAG_DROP_OPERATOR_TYPE(FWwiseUmgDragDropOp, FDragDropOperation)

	static TSharedRef<FWwiseUmgDragDropOp> New(const TArray<TSharedPtr<FWwiseTreeItem>>& in_WwiseAssets)
	{
		TSharedRef<FWwiseUmgDragDropOp> Operation = MakeShareable(new FWwiseUmgDragDropOp);
		
		Operation->MouseCursor = EMouseCursor::GrabHandClosed;
		if (in_WwiseAssets.Num() && in_WwiseAssets[0].IsValid())
		{
			Operation->WwiseAssets = in_WwiseAssets;
			Operation->Icon = FAkAudioStyle::GetBrush(in_WwiseAssets[0]->ItemType);
		}
		Operation->Construct();

		return Operation;
	}
	
	const TArray< TSharedPtr<FWwiseTreeItem> >& GetWiseItems() const
	{
		return WwiseAssets;
	}

public:
	FText GetDecoratorText() const
	{
		FString Text = ((WwiseAssets.Num() == 1) && WwiseAssets[0].IsValid()) ? WwiseAssets[0]->DisplayName : TEXT("");

		if (WwiseAssets.Num() > 1 )
		{
			Text = FString::Printf(TEXT("Can't handle more than one item"));
		}
		return FText::FromString(Text);
	}

	const FSlateBrush* GetIcon() const
	{
		return Icon;
	}

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return 
			SNew(SBorder)
			.BorderImage(FAkAudioStyle::GetBrush("AudiokineticTools.AssetDragDropTooltipBackground"))
			.Content()
			[
				SNew(SHorizontalBox)

				// Left slot is wwise item icon.
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(this, &FWwiseUmgDragDropOp::GetIcon)
				]

				// Right slot for the item name.
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(3,0,3,0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock) 
						.Text(this, &FWwiseUmgDragDropOp::GetDecoratorText)
					]
				]
			];
	}

private:
	/** Data for the asset this item represents */
	TArray<TSharedPtr<FWwiseTreeItem>> WwiseAssets;

	const FSlateBrush* Icon;
};
