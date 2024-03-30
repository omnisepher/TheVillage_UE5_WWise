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

#include "AkAssetMigrationHelper.h"

#include "Application/SlateWindowHelper.h"
#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Styling/SlateTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SDirectoryPicker.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Layout/SExpandableArea.h"

class SEditableTextBox;

/**
 * A File path box (that actually lets you create a new file).
 */
class SDefinitionFilePicker : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_OneParam(FOnFileChanged, const FString& /*Directory*/);

	SLATE_BEGIN_ARGS(SDefinitionFilePicker)
		 : _IsEnabled(true) {}
		SLATE_ARGUMENT(FString, FilePath)
		SLATE_ARGUMENT(FText, Message)
		SLATE_ATTRIBUTE(bool, IsEnabled)
		/** Called when a path has been picked or modified. */
		SLATE_EVENT(FOnFileChanged, OnFileChanged)
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs);
	FString GetFilePath() const;

	/**
	* Declares a delegate that is executed when a file was picked in the SFilePicker widget.
	*
	* The first parameter will contain the path to the picked file.
	*/
	DECLARE_DELEGATE_OneParam(FOnFilePicked, const FString& /*PickedPath*/);
	
private:
	void OnFileTextChanged(const FText& InFilePath);
	void OnFileTextCommitted(const FText& InText, ETextCommit::Type InCommitType);
	FText GetFilePathText() const;
	bool OpenDefinitionFilePicker(FString& OutDirectory, const FString& DefaultPath);
	FReply BrowseForFile();

private:
	FString FilePath;
	FText Message;

	/** Holds a delegate that is executed when a file was picked. */
	FOnFileChanged OnFileChanged;
	TSharedPtr<SEditableTextBox> EditableTextBox;
};


class SBankTransferWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBankTransferWidget) {}
	SLATE_END_ARGS()

	TSharedPtr<SCheckBox> SoundBankTransferCheckBox;
	TSharedPtr<SCheckBox> TransferAutoLoadCheckBox;
	TSharedPtr<SCheckBox> DeleteSoundBanksCheckBox;
	TSharedPtr<SDefinitionFilePicker> SoundBankDefinitionFilePathPicker;

	AkAssetMigration::EBankTransferMode BankTransferMethod = AkAssetMigration::EBankTransferMode::NoTransfer;
	FString SoundBankDefinitionFilePath = TEXT("");

	void Construct(const FArguments& InArgs);
	void SetDefinitionFilePath(const FString& PickedPath);
	void SetTransferMethod(AkAssetMigration::EBankTransferMode TransferMethod);
	void OnCheckedTransferBanks(ECheckBoxState NewState);
	bool GetDefinitionFilePath(FString& OutFilePath) const ;
	bool CheckWaapiConnection() const;

	EVisibility GetDefinitionFilePathVisibility() const;
	EVisibility GetTransferMethodVisibility() const;
	TSharedRef<SWidget> OnGetTransferMethodMenu();
	FText GetTransferMethodText() const;
	FLinearColor GetDropDownColour() const;
	FSlateColor GetDropDownBorderColour() const;

private :
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;
};


class SDeprecatedAssetCleanupWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDeprecatedAssetCleanupWidget) {}
		SLATE_ARGUMENT(int, NumDeprecatedAssets)
	SLATE_END_ARGS()
	TSharedPtr<SCheckBox> DeleteAssetsCheckBox;
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;

};


class SAssetMigrationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssetMigrationWidget) {}

	SLATE_END_ARGS()
	TSharedPtr<SCheckBox> MigrateAssetsCheckBox;
	void Construct(const FArguments& InArgs);

private :
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;
};


class SProjectMigrationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProjectMigrationWidget) {}
	SLATE_END_ARGS()

	TSharedPtr<SCheckBox> AutoMigrateCheckbox;
	TSharedPtr<SDirectoryPicker> GeneratedSoundBanksFolderPickerWidget;

	void Construct(const FArguments& InArgs);

	EVisibility GetPathVisibility() const;

private :
	TSharedPtr<SExpandableArea> ExpandableSection;
	TSharedPtr<SExpandableArea> ExpandableDetails;
};

class SMigrationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMigrationWidget) {}
		SLATE_ARGUMENT(TSharedPtr<SWindow>, Dialog)
		SLATE_ARGUMENT(bool, ShowBankTransfer)
		SLATE_ARGUMENT(bool, ShowDeprecatedAssetCleanup)
		SLATE_ARGUMENT(bool, ShowAssetMigration)
		SLATE_ARGUMENT(bool, ShowProjectMigration)
		SLATE_ARGUMENT(int, NumDeprecatedAssets)

	SLATE_END_ARGS()
			
	TSharedPtr<SWindow> Dialog;
	TSharedPtr<SBankTransferWidget> BankTransferWidget;
	TSharedPtr<SDeprecatedAssetCleanupWidget> DeprecatedAssetCleanupWidget;
	TSharedPtr<SAssetMigrationWidget> AssetMigrationWidget;
	TSharedPtr<SProjectMigrationWidget> ProjectMigrationWidget;

	void Construct(const FArguments& InArgs);
	FReply OnContinueClicked();
	FReply OnCancelClicked();
	EVisibility GetBankTransferWidgetVisibility() const;
	EVisibility GetMediaCleanupWidgetVisibility() const;
	EVisibility GetAssetMigrationWidgetVisibility() const;
	EVisibility GetProjectMigrationWidgetVisibility() const;

	bool CanClickContinue() const;
	FText GetContinueToolTip() const;
	bool bCancel = false;

private:
	bool bShowBankTransfer;
	bool bShowMediaCleanup;
	bool bShowAssetMigration;
	bool bShowProjectMigration;
};

class SBankMigrationFailureWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBankMigrationFailureWidget) {}
		SLATE_ARGUMENT(TSharedPtr<SWindow>, Dialog)
		SLATE_ARGUMENT(FText, ErrorMessage);
	SLATE_END_ARGS()

	TSharedPtr<SWindow> Dialog;
	void Construct(const FArguments& InArgs);

	FReply OnCancelClicked();
	FReply OnIgnoreClicked();

	bool bCancel = false;
};

