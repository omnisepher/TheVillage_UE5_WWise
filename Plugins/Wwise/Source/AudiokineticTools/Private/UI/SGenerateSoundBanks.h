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
	SGenerateSoundBanks.h
------------------------------------------------------------------------------------*/
#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "AssetManagement/WwiseProjectInfo.h"

class SCheckBox;

class SGenerateSoundBanks : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SGenerateSoundBanks )
	{}
	SLATE_END_ARGS( )

	SGenerateSoundBanks(void);

	AUDIOKINETICTOOLS_API void Construct(const FArguments& InArgs);
	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyboardEvent ) override;

	/** override the base method to allow for keyboard focus */
	virtual bool SupportsKeyboardFocus() const
	{
		return true;
	}

	bool ShouldDisplayWindow() { return PlatformNames.Num() != 0; }

private:
	void PopulateList();

private:
	FReply OnGenerateButtonClicked();
	TSharedRef<ITableRow> MakePlatformListItemWidget(TSharedPtr<FString> Platform, const TSharedRef<STableViewBase>& OwnerTable);

private:
	TSharedPtr<SListView<TSharedPtr<FString>>> PlatformList;
	TSharedPtr<SListView<TSharedPtr<FString>>> LanguageList;
	TSharedPtr<SCheckBox> SkipLanguagesCheckBox;

	TArray<TSharedPtr<FString>> PlatformNames;
	TArray<TSharedPtr<FString>> LanguagesNames;

	WwiseProjectInfo wwiseProjectInfo;
};