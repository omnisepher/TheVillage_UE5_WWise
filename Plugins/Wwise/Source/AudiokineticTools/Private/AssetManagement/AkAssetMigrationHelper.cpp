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

#include "AkAssetMigrationHelper.h"

#include "AkAssetDatabase.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkDeprecated.h"
#include "AkAudioBank.h"
#include "AkWaapiClient.h"
#include "AkWaapiUtils.h"
#include "AkMigrationWidgets.h"
#include "AkCustomVersion.h"
#include "AkUnrealEditorHelper.h"
#include "WwiseUnrealHelper.h"
#include "WwiseDefines.h"
#include "IAudiokineticTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "WwiseUnrealDefines.h"

#include "GenericPlatform/GenericPlatformFile.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "ObjectTools.h"
#include "FileHelpers.h"
#include "Misc/FileHelper.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Interfaces/IMainFrameModule.h"
#include "Mathematics/APConversion.h"
#include "Misc/App.h"

#define LOCTEXT_NAMESPACE "AkAudio"

namespace AkAssetMigration
{
	void PromptMigration(const FMigrationContext& MigrationOptions, FMigrationOperations& OutMigrationOperations)
	{
		if (!FApp::CanEverRender())
		{
			OutMigrationOperations.bCancelled =true;
			return;
		}

		TSharedPtr<SWindow> Dialog = SNew(SWindow)
			.Title(LOCTEXT("BankMigrationDialog", "Wwise Integration Migration"))
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.FocusWhenFirstShown(true)
			.HasCloseButton(false)
			.SizingRule(ESizingRule::Autosized);

		TSharedPtr<SMigrationWidget> MigrationWidget;

		Dialog->SetContent(
			SAssignNew(MigrationWidget, SMigrationWidget)
			.Dialog(Dialog)
			.ShowBankTransfer(MigrationOptions.bBanksInProject)
			.ShowDeprecatedAssetCleanup(MigrationOptions.bDeprecatedAssetsInProject)
			.ShowAssetMigration(MigrationOptions.bAssetsNeedMigration)
			.ShowProjectMigration(MigrationOptions.bProjectSettingsNotUpToDate)
			.NumDeprecatedAssets(MigrationOptions.NumDeprecatedAssetsInProject)
		);

		FSlateApplication::Get().AddModalWindow(Dialog.ToSharedRef(), nullptr);
		
		if (MigrationOptions.bBanksInProject)
		{
			OutMigrationOperations.BankTransferMethod = MigrationWidget->BankTransferWidget->BankTransferMethod;
			OutMigrationOperations.bDoBankCleanup = MigrationWidget->BankTransferWidget->DeleteSoundBanksCheckBox->IsChecked();
			OutMigrationOperations.bTransferAutoload = MigrationWidget->BankTransferWidget->TransferAutoLoadCheckBox->IsChecked();
			OutMigrationOperations.DefinitionFilePath = MigrationWidget->BankTransferWidget->SoundBankDefinitionFilePath;
		}
		if (MigrationOptions.bDeprecatedAssetsInProject)
		{
			OutMigrationOperations.bDoDeprecatedAssetCleanup = MigrationWidget->DeprecatedAssetCleanupWidget->DeleteAssetsCheckBox->IsChecked();
		}
		if (MigrationOptions.bAssetsNeedMigration)
		{
			OutMigrationOperations.bDoAssetMigration = MigrationWidget->AssetMigrationWidget->MigrateAssetsCheckBox->IsChecked();
		}
		if (MigrationOptions.bProjectSettingsNotUpToDate)
		{
			OutMigrationOperations.bDoProjectUpdate = MigrationWidget->ProjectMigrationWidget->AutoMigrateCheckbox->IsChecked();
			OutMigrationOperations.GeneratedSoundBankDirectory = MigrationWidget->ProjectMigrationWidget->GeneratedSoundBanksFolderPickerWidget->GetDirectory();
		}
		OutMigrationOperations.bCancelled = MigrationWidget->bCancel;
	}

	bool PromptFailedBankTransfer(const FString& ErrorMessage)
	{
		if (!FApp::CanEverRender())
		{
			return false;
		}
		TSharedPtr<SWindow> Dialog = SNew(SWindow)
			.Title(LOCTEXT("BankTransfer", "Bank Transfer Failure"))
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.FocusWhenFirstShown(true)
			.HasCloseButton(false)
			.SizingRule(ESizingRule::Autosized);

		TSharedPtr<SBankMigrationFailureWidget> FailedBankTransferWidget;

		Dialog->SetContent(
			SAssignNew(FailedBankTransferWidget, SBankMigrationFailureWidget)
			.Dialog(Dialog)
			.ErrorMessage(FText::Format( LOCTEXT("BankTransferErrorMessage", "{0}"), FText::FromString(ErrorMessage)))
		);

		FSlateApplication::Get().AddModalWindow(Dialog.ToSharedRef(), nullptr);
		return !FailedBankTransferWidget->bCancel;
	}

		
	FString FormatWaapiErrorMessage(const TArray<FBankTransferError>& ErrorMessages)
	{
		FString CombinedErrors;
		for (auto BankErrors : ErrorMessages)
		{
			if (BankErrors.bHasBankEntry)
			{
				CombinedErrors += "SoundBank: " + BankErrors.BankEntry.BankAssetData.AssetName.ToString() + "\n";
			}
			CombinedErrors += "Error: " + BankErrors.ErrorMessage + "\n";
			if (BankErrors.bHasBankEntry)
			{
				CombinedErrors += "Wwise Assets in SoundBank : \n";

				for (auto LinkedAsset : BankErrors.BankEntry.LinkedEvents)
				{
					CombinedErrors += FString::Format(TEXT("GUID: {0} - Name: {1}\n"), { LinkedAsset.WwiseGuid.ToString(), LinkedAsset.AssetName });
				}
				for (auto LinkedAsset : BankErrors.BankEntry.LinkedAuxBusses)
				{
					CombinedErrors += FString::Format(TEXT("GUID: {0} - Name: {1}\n"), { LinkedAsset.WwiseGuid.ToString(), LinkedAsset.AssetName });
				}
			}
		}
		return CombinedErrors;
	}

	void FindDeprecatedAssets(TArray<FAssetData>& OutDeprecatedAssets)
	{
		OutDeprecatedAssets.Empty();
		FARFilter Filter;
#if UE_5_1_OR_LATER
		Filter.ClassPaths.Add(UAkMediaAsset::StaticClass()->GetClassPathName());
		Filter.ClassPaths.Add(UAkLocalizedMediaAsset::StaticClass()->GetClassPathName());
		Filter.ClassPaths.Add(UAkExternalMediaAsset::StaticClass()->GetClassPathName());
		Filter.ClassPaths.Add(UAkFolder::StaticClass()->GetClassPathName());
		Filter.ClassPaths.Add(UAkAssetPlatformData::StaticClass()->GetClassPathName());
#else
		Filter.ClassNames.Add(UAkMediaAsset::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkLocalizedMediaAsset::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkExternalMediaAsset::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkFolder::StaticClass()->GetFName());
		Filter.ClassNames.Add(UAkAssetPlatformData::StaticClass()->GetFName());
#endif
		auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().GetAssets(Filter, OutDeprecatedAssets);
	}

	bool DeleteAssets(const TArray<FAssetData>& InAssetsToDelete)
	{
		/*
		 *Deleting an Asset that is referenced somewhere needs a confirmation which is impossible using a commandlet.
		 *Calling ForceDelete on the UObjects of the Assets will ignore the confirmation and delete the assets.
		*/
		if(!FApp::CanEverRender())
		{
			TArray<UObject*> Objects;
			for(auto Asset : InAssetsToDelete)
			{
				Objects.Add(Asset.GetAsset());
			}
			int DeletedObjects = ObjectTools::ForceDeleteObjects(Objects, false);
			return DeletedObjects == Objects.Num();
		}
		else
		{
			int DeletedObjects = ObjectTools::DeleteAssets(InAssetsToDelete, true);
			return DeletedObjects == InAssetsToDelete.Num();
		}
	}

	bool DeleteDeprecatedAssets(const TArray<FAssetData>& InAssetsToDelete)
	{
		bool bSuccess = true;
		bSuccess &= DeleteAssets(InAssetsToDelete);
		
		const FString MediaFolderPath = FPaths::Combine(AkUnrealEditorHelper::GetLegacySoundBankDirectory(), WwiseUnrealHelper::MediaFolderName);
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		PlatformFile.DeleteDirectoryRecursively(*MediaFolderPath);

		const FString LocalizedFolderPath = FPaths::Combine(AkUnrealEditorHelper::GetLegacySoundBankDirectory(), AkUnrealEditorHelper::LocalizedFolderName);
		PlatformFile.DeleteDirectoryRecursively(*LocalizedFolderPath);

		return bSuccess;
	}

	void FindWwiseAssetsInProject(TArray<FAssetData>& OutWwiseAssets)
	{
		FARFilter Filter;
		Filter.bRecursiveClasses = true;
#if UE_5_1_OR_LATER
		Filter.ClassPaths.Add(UAkAudioType::StaticClass()->GetClassPathName());
		//We want to delete these asset types during cleanup so no need to dirty them
		Filter.RecursiveClassPathsExclusionSet.Add(UAkAudioBank::StaticClass()->GetClassPathName());
		Filter.RecursiveClassPathsExclusionSet.Add(UAkFolder::StaticClass()->GetClassPathName());
#else
		Filter.ClassNames.Add(UAkAudioType::StaticClass()->GetFName());
		Filter.RecursiveClassesExclusionSet.Add(UAkAudioBank::StaticClass()->GetFName());
		Filter.RecursiveClassesExclusionSet.Add(UAkFolder::StaticClass()->GetFName());
#endif
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().GetAssets(Filter, OutWwiseAssets);
	}

	bool MigrateWwiseAssets(const TArray<FAssetData> & WwiseAssets, bool bShouldSplitSwitchContainerMedia)
	{
		TArray<UPackage*> WwisePackagesToSave;
		EWwiseEventSwitchContainerLoading SwitchContainerShouldLoad = bShouldSplitSwitchContainerMedia
			? EWwiseEventSwitchContainerLoading::LoadOnReference
			: EWwiseEventSwitchContainerLoading::AlwaysLoad;

		for (auto& Asset : WwiseAssets)
		{
			UAkAudioType* AssetPtr = Cast<UAkAudioType>(Asset.GetAsset());

			if (AssetPtr)
			{
				AssetPtr->MigrateWwiseObjectInfo();
				if (AssetPtr->GetClass() == UAkAudioEvent::StaticClass())
				{
					UAkAudioEvent* Event = Cast<UAkAudioEvent>(AssetPtr);
					Event->EventInfo.SwitchContainerLoading = SwitchContainerShouldLoad;
				}

				if (!AssetPtr->MarkPackageDirty())
				{					
					UE_LOG(LogAudiokineticTools, Warning, TEXT("Could not dirty asset %s during migration, will still try to save it."), *AssetPtr->GetFullName());
				}
				WwisePackagesToSave.Add(AssetPtr->GetPackage());
			}
			else
			{
				UE_LOG(LogAudiokineticTools, Error, TEXT("Could not get asset '%s' in order to migrate it."), *Asset.AssetName.ToString());
			}
		}

		if (WwisePackagesToSave.Num() > 0)
		{
			if (!UEditorLoadingAndSavingUtils::SavePackages(WwisePackagesToSave, false))
			{
				UE_LOG(LogAudiokineticTools, Warning, TEXT("MigrateWwiseAssets: Could not save assets during asset migration. Please save them manually, or run the migration operation again."));
				return false;
			}
		}

		return true;
	}

	bool MigrateAudioBanks(const FMigrationOperations& MigrationOperations, const bool bCleanupAssets, const bool bWasUsingEBP, const bool bTransferAutoLoad, const FString DefinitionFile)
	{
		TSet<FAssetData> FailedBanks;
		TMap<FString, FBankEntry> BanksToTransfer;
		FillBanksToTransfer(BanksToTransfer);
		const bool bIncludeMedia = !bWasUsingEBP;
		bool bContinueMigration = true;
		bool bTransferSuccess = true;

		FString ErrorMessage;
		if (MigrationOperations.BankTransferMethod == EBankTransferMode::WAAPI)
		{
			TArray<FBankTransferError> ErrorMessages = TransferUserBanksWaapi( BanksToTransfer, FailedBanks, bIncludeMedia);
			ErrorMessage = FormatWaapiErrorMessage(ErrorMessages);
			bTransferSuccess = FailedBanks.Num() == 0;
		}
		else if (MigrationOperations.BankTransferMethod == EBankTransferMode::DefinitionFile)
		{
			auto Result = WriteBankDefinitionFile( BanksToTransfer, bIncludeMedia, DefinitionFile);
			bTransferSuccess = Result == Success;

			if (!bTransferSuccess)
			{
				switch (Result)
				{
					case OpenFailure:
						ErrorMessage = FString::Format(TEXT("Failed to open bank definition file for write '{0}'."), {DefinitionFile});
						break;
					case WriteFailure:
						ErrorMessage = FString::Format(TEXT("Failed to write to bank definition file '{0}'."), {DefinitionFile});
						break;
					default:
						break;
				}

			}
		}

		if (!bTransferSuccess)
		{
			if (!FApp::CanEverRender())
			{
				bContinueMigration = MigrationOperations.bIgnoreBankTransferErrors;
			}
			else //Migrating in Unreal Editor
			{
				bContinueMigration = PromptFailedBankTransfer(ErrorMessage);
			}
		}

		if (!bContinueMigration)
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("MigrateAudioBanks : Errors encountered while transferring SoundBanks: \n%s"), *ErrorMessage);
			UE_LOG(LogAudiokineticTools, Error, TEXT("MigrateAudioBanks : Migration aborted due to SoundBank transfer errors."));
			return false;
		}

		if (!ErrorMessage.IsEmpty())
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("MigrateAudioBanks : Errors encountered while transferring SoundBanks: \n%s"), *ErrorMessage);
			UE_LOG(LogAudiokineticTools, Display, TEXT("MigrateAudioBanks : Migration will continue and ignore SoundBank transfer errors."));
		}

		if (bTransferAutoLoad)
		{
			UE_LOG(LogAudiokineticTools, Display, TEXT("MigrateAudioBanks: Transferring AutoLoad setting from AkAudioBank assets to Event and Aux Bus assets."));

			for (auto& Bank : BanksToTransfer)
			{
				UAkAudioBank* BankAsset = Cast<UAkAudioBank>(Bank.Value.BankAssetData.GetAsset());
				if (!BankAsset)
				{
#if UE_5_1_OR_LATER
					UE_LOG(LogAudiokineticTools, Warning, TEXT("MigrateAudioBanks: Could not load UAkAudioBank Asset '%s'. AutoLoad property will not be transferred. "),
						*Bank.Value.BankAssetData.GetObjectPathString());
#else
					UE_LOG(LogAudiokineticTools, Warning, TEXT("MigrateAudioBanks: Could not load UAkAudioBank Asset '%s'. AutoLoad property will not be transferred. "),
						*Bank.Value.BankAssetData.ObjectPath.ToString());
#endif
					continue;
				}
				if (!BankAsset->AutoLoad_DEPRECATED)
				{
					TArray<FLinkedAssetEntry>& LinkedAuxBusses = Bank.Value.LinkedAuxBusses;
					TArray<FLinkedAssetEntry>& LinkedEvents = Bank.Value.LinkedEvents;
					for (auto& LinkedAuxBus : LinkedAuxBusses)
					{
						bool MigrateSuccess = false;
						auto FoundAssetData = AkAssetDatabase::Get().FindAssetByObjectPath(LinkedAuxBus.AssetPath);
						if (FoundAssetData.IsValid())
						{
							if (auto* BusAsset = Cast<UAkAuxBus>(FoundAssetData.GetAsset()))
							{
								BusAsset->bAutoLoad = false;
								MigrateSuccess = BusAsset->MarkPackageDirty();
							}
						}

						if(!MigrateSuccess)
						{
							UE_LOG(LogAudiokineticTools, Warning, TEXT("MigrateAudioBanks: Could not successfully disable AutoLoad on AuxBus asset %s during migration. It will be automatically loaded."), *LinkedAuxBus.AssetPath);
						}
					}
					for (auto& LinkedEvent : LinkedEvents)
					{
						bool MigrateSuccess = false;
						auto FoundAssetData = AkAssetDatabase::Get().FindAssetByObjectPath(LinkedEvent.AssetPath);
						if (FoundAssetData.IsValid())
						{
							if (auto* EventAsset = Cast<UAkAudioEvent>(FoundAssetData.GetAsset()))
							{
								EventAsset->bAutoLoad = false;
								MigrateSuccess = EventAsset->MarkPackageDirty();
							}
						}

						if (!MigrateSuccess)
						{
							UE_LOG(LogAudiokineticTools, Warning, TEXT("MigrateAudioBanks: Could not successfully disable AutoLoad on Event asset %s during migration. It will be automatically loaded."), *LinkedEvent.AssetPath);
						}
					}
				}
			}
		}
		if (bCleanupAssets)
		{
			UE_LOG(LogAudiokineticTools, Display, TEXT("MigrateAudioBanks: Deleting deprecated AkAudioBank assets."));
			TSet<FAssetData> ProjectBanks;
			for (auto Bank : BanksToTransfer)
			{
				ProjectBanks.Add(Bank.Value.BankAssetData);
			}
			TArray<FAssetData> BanksToDelete = ProjectBanks.Array();
			DeleteAssets(BanksToDelete);
		}
		return true;
	}

	void FillBanksToTransfer(TMap<FString, FBankEntry>& BanksToTransfer)
	{
		auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		auto& AssetRegistry = AssetRegistryModule.Get();

		TArray<FAssetData> Banks;
#if UE_5_1_OR_LATER
		AssetRegistry.GetAssetsByClass(UAkAudioBank::StaticClass()->GetClassPathName(), Banks);
#else
		AssetRegistry.GetAssetsByClass(UAkAudioBank::StaticClass()->GetFName(), Banks);
#endif
		for (FAssetData& BankData : Banks)
		{
			FString BankName = BankData.AssetName.ToString();
			BanksToTransfer.Add(BankName, { BankData, {}, {} });
		}

		TArray< FAssetData> Events;
#if UE_5_1_OR_LATER
		AssetRegistry.GetAssetsByClass(UAkAudioEvent::StaticClass()->GetClassPathName(), Events);
#else
		AssetRegistry.GetAssetsByClass(UAkAudioEvent::StaticClass()->GetFName(), Events);
#endif
		for (FAssetData EventData : Events)
		{
			if (UAkAudioEvent* Event = Cast<UAkAudioEvent>(EventData.GetAsset()))
			{
				if (Event->RequiredBank_DEPRECATED != nullptr)
				{
					FString BankName = Event->RequiredBank_DEPRECATED->GetName();
					FLinkedAssetEntry EventEntry = { Event->GetName(), Event->EventInfo.WwiseGuid, Event->GetPathName() };

					if (BanksToTransfer.Contains(BankName))
					{
						BanksToTransfer[BankName].LinkedEvents.Add(EventEntry);
					}
					else
					{
						FAssetData BankAssetData = FAssetData(Event->RequiredBank_DEPRECATED);
						BanksToTransfer.Add(BankName, { BankAssetData, {EventEntry}, {} });
					}
				}
			}
		}

		TArray< FAssetData> AuxBusses;
#if UE_5_1_OR_LATER
		AssetRegistry.GetAssetsByClass(UAkAuxBus::StaticClass()->GetClassPathName(), AuxBusses);
#else
		AssetRegistry.GetAssetsByClass(UAkAuxBus::StaticClass()->GetFName(), AuxBusses);
#endif
		for (FAssetData AuxBusData : AuxBusses)
		{
			if (UAkAuxBus* AuxBus = Cast<UAkAuxBus>(AuxBusData.GetAsset()))
			{
				if (AuxBus->RequiredBank_DEPRECATED != nullptr)
				{
					FString BankName = AuxBus->RequiredBank_DEPRECATED->GetName();
					FLinkedAssetEntry AuxBusEntry = { AuxBus->GetName(), AuxBus->AuxBusInfo.WwiseGuid, AuxBus->GetPathName() };
					if (BanksToTransfer.Contains(BankName))
					{
						BanksToTransfer[BankName].LinkedAuxBusses.Add(AuxBusEntry);
					}
					else
					{
						FAssetData BankAssetData = FAssetData(AuxBus->RequiredBank_DEPRECATED);
						BanksToTransfer.Add(BankName, { BankAssetData, {}, {AuxBusEntry} });
					}
				}
			}
		}
		UE_LOG(LogAudiokineticTools, Display, TEXT("MigrateAudioBanks: Found %d SoundBank assets in project."), BanksToTransfer.Num());
	}

	TArray<FBankTransferError> TransferUserBanksWaapi(const TMap<FString, FBankEntry>& InBanksToTransfer, TSet<FAssetData>& OutFailedBanks, const bool bIncludeMedia)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("MigrateAudioBanks: Transfering SoundBanks with WAAPI."));
		TArray<FBankTransferError> ErrorMessages;
		for (TPair<FString, FBankEntry > BankEntry : InBanksToTransfer)
		{
			FGuid BankID;
			if (!CreateBankWaapi(BankEntry.Key, BankEntry.Value, BankID, ErrorMessages))
			{
				OutFailedBanks.Add(BankEntry.Value.BankAssetData);
				continue;
			}
			if (!SetBankIncludesWaapi(BankEntry.Value, BankID, bIncludeMedia, ErrorMessages))
			{
				OutFailedBanks.Add(BankEntry.Value.BankAssetData);
			}
		}
		SaveProjectWaapi(ErrorMessages);
		return ErrorMessages;
	}

	EDefinitionFileCreationResult WriteBankDefinitionFile(const TMap<FString, FBankEntry>& InBanksToTransfer, const bool bIncludeMedia, const FString DefinitionFilePath)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("MigrateAudioBanks: Writing SoundBanks Definition File to '%s'."), *DefinitionFilePath);
		// open file to start writing
		IPlatformFile* PlatformFile = &FPlatformFileManager::Get().GetPlatformFile();;
		FString FileLocation =  IFileManager::Get().ConvertToRelativePath(*DefinitionFilePath);
		TUniquePtr<IFileHandle> FileWriter = TUniquePtr<IFileHandle>(PlatformFile->OpenWrite(*FileLocation));
		if (!FileWriter.IsValid())
		{
			return OpenFailure;
		}

		bool bWriteSuccess = true;
		for (TPair<FString, FBankEntry > BankEntry : InBanksToTransfer)
		{
			bWriteSuccess &= WriteBankDefinition(BankEntry.Value, FileWriter, bIncludeMedia);
		}

		if (!bWriteSuccess)
		{
			return WriteFailure;
		}

		FileWriter->Flush();
		return Success;
	}

	bool WriteBankDefinition(const FBankEntry& BankEntry, TUniquePtr<IFileHandle>& FileWriter, const bool bIncludeMedia)
	{
		bool bWriteSuccess = true;
		FString MediaString = bIncludeMedia? "\tMedia": "";
		for (FLinkedAssetEntry Event : BankEntry.LinkedEvents)
		{
			auto Line = BankEntry.BankAssetData.AssetName.ToString() + TEXT("\t\"") + Event.AssetName + TEXT("\"") + TEXT("\tEvent") + MediaString + TEXT("\tStructure") + LINE_TERMINATOR;
			FTCHARToUTF8 Utf8Formatted(*Line);
			bWriteSuccess &= FileWriter->Write(reinterpret_cast<const uint8*>(Utf8Formatted.Get()), Utf8Formatted.Length());
		}
		for (FLinkedAssetEntry Bus : BankEntry.LinkedAuxBusses)
		{
			auto Line = BankEntry.BankAssetData.AssetName.ToString() + TEXT("\t-AuxBus\t\"") + Bus.AssetName + TEXT("\"") + MediaString + TEXT("\tStructure") + LINE_TERMINATOR;
			FTCHARToUTF8 Utf8Formatted(*Line);
			bWriteSuccess &= FileWriter->Write(reinterpret_cast<const uint8*>(Utf8Formatted.Get()), Utf8Formatted.Length());
		}
		return bWriteSuccess;
	}

	bool CreateBankWaapi(const FString& BankName, const FBankEntry& BankEntry, FGuid& OutBankGuid, TArray<FBankTransferError>& ErrorMessages)
	{
#if AK_SUPPORT_WAAPI
		FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
		if (!WaapiClient)
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to create SoundBank for <%s>. \nCould not get WAAPI Client."), *BankEntry.BankAssetData.PackageName.ToString());
			ErrorMessages.Add({"Could not get WAAPI Client", true, BankEntry});
			return false;
		}
		else if (!WaapiClient->IsConnected())
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to create SoundBank for <%s>. \nWAAPI Client not connected."), *BankEntry.BankAssetData.PackageName.ToString());
			ErrorMessages.Add({"WAAPI Client not connected", true, BankEntry});
			return false;
		}

		TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
		Args->SetStringField(WwiseWaapiHelper::PARENT, TEXT("\\SoundBanks\\Default Work Unit"));
		Args->SetStringField(WwiseWaapiHelper::ON_NAME_CONFLICT, WwiseWaapiHelper::RENAME);
		Args->SetStringField(WwiseWaapiHelper::TYPE,  WwiseWaapiHelper::SOUNDBANK_TYPE);
		Args->SetStringField(WwiseWaapiHelper::NAME, BankName);

		TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> Result;
		FString IdString;
		if (!WaapiClient->Call(ak::wwise::core::object::create, Args, Options, Result))
		{
			FString ErrorMessage;
			if (Result.IsValid())
			{
				Result->TryGetStringField(TEXT("message"), ErrorMessage);
			}
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to create SoundBank for <%s>.\nMessage : <%s>."), *BankEntry.BankAssetData.PackageName.ToString(), *ErrorMessage);
			ErrorMessages.Add({ErrorMessage, true, BankEntry});
			return false;
		}

		if (Result.IsValid() && !Result->TryGetStringField(WwiseWaapiHelper::ID, IdString))
		{
			FString ErrorMessage;
			if (Result.IsValid())
			{
				Result->TryGetStringField(TEXT("message"), ErrorMessage);
			}			
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to create SoundBank for <%s>.\nMessage : <%s>."), *BankEntry.BankAssetData.PackageName.ToString(), *ErrorMessage);
			ErrorMessages.Add({ErrorMessage, true, BankEntry});
			return false; // error parsing Json
		}
		FGuid::ParseExact(IdString, EGuidFormats::DigitsWithHyphensInBraces, OutBankGuid);
		return true;
#else
		UE_LOG(LogAudiokineticTools, Error, TEXT("SetBankIncludesWaapi: WAAPI not supported"));
		ErrorMessages.Add({TEXT("WAAPI not supported"), false, {}});
		return false;
#endif
	}

	bool SetBankIncludesWaapi(const FBankEntry& BankEntry, const FGuid& BankId, const bool bIncludeMedia, TArray<FBankTransferError>& ErrorMessages)
	{
#if AK_SUPPORT_WAAPI
		FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
		if (!WaapiClient)
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("WAAPI command to add Wwise objects to SoundBank '%s' failed. \nCould not get WAAPI Client."), *BankEntry.BankAssetData.PackageName.ToString());
			ErrorMessages.Add({"Could not get WAAPI Client", true, BankEntry});
			return false;
		}
		else if (!WaapiClient->IsConnected())
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("WAAPI command to add Wwise objects to SoundBank '%s' failed. \nWAAPI Client not connected."), *BankEntry.BankAssetData.PackageName.ToString());
			ErrorMessages.Add({"WAAPI Client not connected", true, BankEntry});
			return false;
		}

		TSet<FString> IncludeIds;
		for (FLinkedAssetEntry Event : BankEntry.LinkedEvents)
		{
			if (Event.WwiseGuid.IsValid())
			{
				IncludeIds.Add(Event.WwiseGuid.ToString(EGuidFormats::DigitsWithHyphensInBraces));
			}
			else
			{
				IncludeIds.Add(TEXT("Event:") + Event.AssetName);
			}
		}

		for (FLinkedAssetEntry AuxBus : BankEntry.LinkedAuxBusses)
		{
			if (AuxBus.WwiseGuid.IsValid())
			{
				IncludeIds.Add(AuxBus.WwiseGuid.ToString(EGuidFormats::DigitsWithHyphensInBraces));
			}
			else
			{
				IncludeIds.Add(TEXT("AuxBus:") + AuxBus.AssetName);
			}
		}
		if (IncludeIds.Num() < 0)
		{
			return true;
		}

		TArray<TSharedPtr<FJsonValue>> Filters;
		Filters.Add(MakeShared< FJsonValueString>(TEXT("events")));
		Filters.Add(MakeShared< FJsonValueString>(TEXT("structures")));
		if (bIncludeMedia)
		{
			Filters.Add(MakeShared< FJsonValueString>(TEXT("media")));
		}

		TArray<TSharedPtr<FJsonValue>> IncludeIdJson;
		for (const FString IncludedId : IncludeIds)
		{
			TSharedPtr<FJsonObject> IncludedObject = MakeShared< FJsonObject>();
			IncludedObject->SetStringField(WwiseWaapiHelper::OBJECT, IncludedId);
			IncludedObject->SetArrayField(WwiseWaapiHelper::FILTER, Filters);
			IncludeIdJson.Add(MakeShared< FJsonValueObject>(IncludedObject));
		}

		TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
		Args->SetStringField(WwiseWaapiHelper::SOUNDBANK_FIELD, BankId.ToString(EGuidFormats::DigitsWithHyphensInBraces));
		Args->SetStringField(WwiseWaapiHelper::OPERATION, TEXT("add"));
		Args->SetArrayField(WwiseWaapiHelper::INCLUSIONS, IncludeIdJson);

		TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> Result;
		if (!WaapiClient->Call(ak::wwise::core::soundbank::setInclusions, Args, Options, Result))
		{
			FString ErrorMessage;
			if (Result.IsValid())
			{
				Result->TryGetStringField(TEXT("message"), ErrorMessage);
			}
			UE_LOG(LogAudiokineticTools, Warning, TEXT("WAAPI command to add Wwise objects to SoundBank '%s' failed. \nError Message : <%s>."), *BankEntry.BankAssetData.PackageName.ToString(), *ErrorMessage);
			ErrorMessages.Add({ErrorMessage, true, BankEntry});
			return false;
		}
		return true;

#else
		UE_LOG(LogAudiokineticTools, Error, TEXT("SetBankIncludesWaapi: WAAPI not supported"));
		ErrorMessages.Add({TEXT("WAAPI not supported"), false, {}});
		return false;
#endif
	}
	
	bool SaveProjectWaapi(TArray<FBankTransferError>& ErrorMessages)
	{
#if AK_SUPPORT_WAAPI
		FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();
		if (!WaapiClient)
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to save Wwise project.\n Could not get Waapi Client."));
			ErrorMessages.Add({TEXT("Failed to save Wwise project over WAAPI. Please save the project manually."), false, {}});
			return false;
		}
		else if (!WaapiClient->IsConnected())
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to save Wwise project.\n Waapi Client not connected."));
			ErrorMessages.Add({TEXT("Failed to save Wwise project over WAAPI. Please save the project manually."), false, {}});
			return false;
		}

		TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>(); 
		TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> Result;
		FString IdString;
		if (!WaapiClient->Call(ak::wwise::core::project::save, Args, Options, Result))
		{
			FString ErrorMessage;
			if (Result.IsValid())
			{
				Result->TryGetStringField(TEXT("message"), ErrorMessage);
			}
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Failed to save Wwise project.\nMessage : <%s>."), *ErrorMessage);
			ErrorMessages.Add({TEXT("Failed to save Wwise project over WAAPI. Please save the project manually."), false, {}});
			ErrorMessages.Add({ErrorMessage, false, {}});
			return false;
		}
		return true;
#else
		UE_LOG(LogAudiokineticTools, Error, TEXT("SetBankIncludesWaapi: WAAPI not supported"));
		ErrorMessages.Add({TEXT("WAAPI not supported"), false, {}});
		return false;
#endif
	}

	bool MigrateProjectSettings(FString& ProjectContent, const bool bWasUsingEBP, const bool bUseGeneratedSubFolders, const FString& GeneratedSoundBanksFolder )
	{
		//migrate split media per id
		TArray<PropertyToChange> PropertiesToAdd;
		if (bWasUsingEBP)
		{
			PropertiesToAdd.Add({ TEXT("AutoSoundBankEnabled"), TEXT("True"), TEXT("<Property Name=\"AutoSoundBankEnabled\" Type=\"bool\" Value=\"True\"/>") });
		}

		if (bUseGeneratedSubFolders)
		{
			PropertiesToAdd.Add({ TEXT("MediaAutoBankSubFolders"), TEXT("True"), TEXT("<Property Name=\"MediaAutoBankSubFolders\" Type=\"bool\" Value=\"True\"/>") });
		}

		static const TArray<FString> LogCentralItemsToRemove = 
		{
			TEXT("<IgnoreItem MessageId=\"MediaDuplicated\"/>"),
			TEXT("<IgnoreItem MessageId=\"MediaNotFound\"/>")
		};

		bool bModified = false;
		if (PropertiesToAdd.Num() >0)
		{
			bModified = InsertProperties(PropertiesToAdd, ProjectContent);
		}
		for (const FString& LogItemToRemove : LogCentralItemsToRemove)
		{
			if (ProjectContent.Contains(LogItemToRemove))
			{
				ProjectContent.ReplaceInline(*LogItemToRemove, TEXT(""));
				bModified = true;
			}
		}
		return bModified;
	}
		
	bool SetStandardSettings(FString& ProjectContent)
	{
		static const TArray<PropertyToChange> PropertiesToAdd = {
			{ TEXT("GenerateMultipleBanks"), TEXT("True"), TEXT("<Property Name=\"GenerateMultipleBanks\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("GenerateSoundBankJSON"), TEXT("True"), TEXT("<Property Name=\"GenerateSoundBankJSON\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGenerateEstimatedDuration"), TEXT("True"), TEXT("<Property Name=\"SoundBankGenerateEstimatedDuration\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGenerateMaxAttenuationInfo"), TEXT("True"), TEXT("<Property Name=\"SoundBankGenerateMaxAttenuationInfo\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGeneratePrintGUID"), TEXT("True"), TEXT("<Property Name=\"SoundBankGeneratePrintGUID\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("SoundBankGeneratePrintPath"), TEXT("True"), TEXT("<Property Name=\"SoundBankGeneratePrintPath\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("CopyLooseStreamedMedia"), TEXT("True"), TEXT("<Property Name=\"CopyLooseStreamedMedia\" Type=\"bool\" Value=\"True\"/>") },
			{ TEXT("RemoveUnusedGeneratedFiles"), TEXT("True"), TEXT("<Property Name=\"RemoveUnusedGeneratedFiles\" Type=\"bool\" Value=\"True\"/>") },
		};

		return InsertProperties(PropertiesToAdd, ProjectContent);
	}

	bool InsertProperties(const TArray<PropertyToChange>& PropertiesToChange, FString& ProjectContent)
	{
		static const auto PropertyListStart = TEXT("<PropertyList>");
		static const FString EndTag = TEXT(">");
		static const TCHAR EmptyElementEndChar = '/';
		static const FString ValueTag = TEXT("<Value>");
		static const FString EndValueTag = TEXT("</Value>");

		static const FString ValueAttribute = TEXT("Value=\"");
		static const FString EndValueAttribute = TEXT("\"");

		bool bModified = false;

		int32 PropertyListPosition = ProjectContent.Find(PropertyListStart);
		if (PropertyListPosition != -1)
		{
			int32 InsertPosition = PropertyListPosition + FCString::Strlen(PropertyListStart);

			for (PropertyToChange ItemToAdd : PropertiesToChange)
			{
				auto idx = ProjectContent.Find(ItemToAdd.Name);
				if (idx == -1)
				{
					ProjectContent.InsertAt(InsertPosition, FString::Printf(TEXT("\n\t\t\t\t%s"), *ItemToAdd.Xml));
					bModified = true;
				}
				else
				{
					FString ValueText;
					FString EndValueText;
					int32 EndTagIdx = ProjectContent.Find(EndTag, ESearchCase::IgnoreCase, ESearchDir::FromStart, idx);
					if (ProjectContent[EndTagIdx - 1] == EmptyElementEndChar)
					{
						// The property is an empty element, the value will be in an attribute
						ValueText = ValueAttribute;
						EndValueText = EndValueAttribute;
					}
					else
					{
						// We are in a ValueList
						ValueText = ValueTag;
						EndValueText = EndValueTag;
					}

					int32 ValueIdx = ProjectContent.Find(ValueText, ESearchCase::IgnoreCase, ESearchDir::FromStart, idx);
					int32 EndValueIdx = ProjectContent.Find(EndValueText, ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueIdx);
					if (ValueIdx != -1 && ValueIdx > idx && ValueIdx < EndValueIdx)
					{
						ValueIdx += ValueText.Len();
						auto ValueEndIdx = ProjectContent.Find(EndValueText, ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueIdx);
						if (ValueEndIdx != -1)
						{
							FString value = ProjectContent.Mid(ValueIdx, ValueEndIdx - ValueIdx);
							if (value != ItemToAdd.Value)
							{
								ProjectContent.RemoveAt(ValueIdx, ValueEndIdx - ValueIdx, false);
								ProjectContent.InsertAt(ValueIdx, ItemToAdd.Value);
								bModified = true;
							}
						}
					}
					else
					{
						UE_LOG(LogAudiokineticTools, Warning, TEXT("Could not change value for %s in Wwise project. Some features might not work properly."), *ItemToAdd.Name);
					}
				}
			}
		}

		return bModified;
	}
}

#undef LOCTEXT_NAMESPACE
