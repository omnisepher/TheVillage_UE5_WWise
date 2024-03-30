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

#include "AkAssetMigrationManager.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "AkUnrealEditorHelper.h"
#include "AkAssetMigrationHelper.h"
#include "AkAudioBank.h"
#include "AkAudioEvent.h"
#include "AkAudioModule.h"
#include "AkInitBank.h"
#include "AkWaapiClient.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "Framework/Notifications/NotificationManager.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "IAudiokineticTools.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Settings/ProjectPackagingSettings.h"
#include "WwiseUnrealHelper.h"
#include "ToolMenus.h"
#include "FileHelpers.h"

#define LOCTEXT_NAMESPACE "AkAudio"

bool AkAssetMigrationManager::IsProjectMigrated()
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (!AkSettings)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("AkAssetMigrationManager::IsProjectMigrated: Could not get AkSettings. Cannot determine whether the project is migrated."));
		return false;
	}
	return AkSettings->bAssetsMigrated && AkSettings->bProjectMigrated && AkSettings->bSoundBanksTransfered;

}

bool AkAssetMigrationManager::IsMigrationRequired(AkAssetMigration::FMigrationContext& MigrationOptions )
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (!AkSettings)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("AkAssetMigrationManager::IsMigrationRequired: Could not getAkSettings. Cannot determine whether the project should be migrated."));
		return false;
	}
	auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	auto& AssetRegistry = AssetRegistryModule.Get();
	bool bUpdateAkSettings = false;
	if (!AkSettings->bAssetsMigrated)
	{
		TArray<FAssetData> WwiseAssetsInProject;
		AkAssetMigration::FindWwiseAssetsInProject(WwiseAssetsInProject);

		if (WwiseAssetsInProject.Num() == 0)
		{
			MigrationOptions.bAssetsNeedMigration = false;
			AkSettings->bAssetsMigrated = true;
			bUpdateAkSettings = true;
		}
		else if (WwiseAssetsInProject.Num() == 1)
		{
			//Init bank asset is automatically created by the integration
			if (WwiseAssetsInProject[0].GetClass() == UAkInitBank::StaticClass())
			{
				AkSettings->bAssetsMigrated = true;
				MigrationOptions.bAssetsNeedMigration = false;
				bUpdateAkSettings= true;
			}
			else
			{
				MigrationOptions.bAssetsNeedMigration = true;
			}
		}
		else
		{
			MigrationOptions.bAssetsNeedMigration = true;
		}
	}
	else
	{
		MigrationOptions.bAssetsNeedMigration = false;
	}

	//Check if project migration is needed
	if (!AkSettings->bProjectMigrated)
	{
		if (AkSettings->UseEventBasedPackaging || AkSettings->SplitMediaPerFolder || IsSoundDataPathInDirectoriesToAlwaysStage(AkUnrealEditorHelper::GetLegacySoundBankDirectory()) )
		{
			// project migration needed
			MigrationOptions.bProjectSettingsNotUpToDate = true;
		}
		else
		{
			MigrationOptions.bProjectSettingsNotUpToDate = false;
			AkSettings->bProjectMigrated = true;
			bUpdateAkSettings = true;
		}
	}
	else
	{
		MigrationOptions.bProjectSettingsNotUpToDate = false;
	}

	TArray<FAssetData> Banks;
#if UE_5_1_OR_LATER
	AssetRegistry.GetAssetsByClass(UAkAudioBank::StaticClass()->GetClassPathName(), Banks);
#else
	AssetRegistry.GetAssetsByClass(UAkAudioBank::StaticClass()->GetFName(), Banks);
#endif
	if (Banks.Num() > 0)
	{
		MigrationOptions.bBanksInProject = true;
	}
	else
	{
		MigrationOptions.bBanksInProject = false;
		if (!AkSettings->bSoundBanksTransfered)
		{
			AkSettings->bSoundBanksTransfered = true;
			bUpdateAkSettings = true;
		}
	}

	TArray<FAssetData> DeprecatedAssetsInProject;
	AkAssetMigration::FindDeprecatedAssets(DeprecatedAssetsInProject);
	MigrationOptions.NumDeprecatedAssetsInProject = DeprecatedAssetsInProject.Num();
	if (DeprecatedAssetsInProject.Num() > 0)
	{
		MigrationOptions.bDeprecatedAssetsInProject = true;
	}
	else
	{
		MigrationOptions.bDeprecatedAssetsInProject = false;
	}

	if (bUpdateAkSettings)
	{
		if(!AkUnrealEditorHelper::SaveConfigFile(AkSettings))
		{
			const FString Message = TEXT("Unable to checkout settings file for Wwise integration settings (DefaultGame.ini). Please revert all changes done, and restart the migration process.");
			if (FApp::CanEverRender())
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
			}
			else
			{
				UE_LOG(LogAudiokineticTools, Error, TEXT("AkAssetMigrationManager: %s"), *Message);
			}
		}
	}

	if ((MigrationOptions.bBanksInProject || !AkSettings->bSoundBanksTransfered) || MigrationOptions.bDeprecatedAssetsInProject  || MigrationOptions.bAssetsNeedMigration || MigrationOptions.bProjectSettingsNotUpToDate)
	{
		return true;
	}
	return false;
}

void AkAssetMigrationManager::EditorTryMigration()
{
	//This should only be done with the editor open
	if (!FApp::CanEverRender())
	{
		return;
	}

	AkAssetMigration::FMigrationContext MigrationOptions;
	 if (!IsMigrationRequired(MigrationOptions))
	 {
	 	RemoveMigrationMenuOption();
	 	return;
	 }

	AkAssetMigration::FMigrationOperations MigrationOperations;
	AkAssetMigration::PromptMigration(MigrationOptions, MigrationOperations);
	if (MigrationOperations.bCancelled)
	{
		return;
	}
	PerformMigration(MigrationOperations);
}

AkAssetMigrationManager::MigrationResult AkAssetMigrationManager::PerformMigration(AkAssetMigration::FMigrationOperations MigrationOperations)
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	bool bShouldUpdateConfig = false;
	const bool bWasUsingEBP = AkSettings->UseEventBasedPackaging;
	MigrationResult Result;
	// Do bank migration first as users may back out at this point and modifying Wwise settings can lock the Wwise project for WAAPI
	bool bContinue= true;
	if (MigrationOperations.BankTransferMethod != AkAssetMigration::EBankTransferMode::NoTransfer || MigrationOperations.bDoBankCleanup || MigrationOperations.bTransferAutoload)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("AkAssetMigrationManager: Migrating SoundBanks."));
		const bool bSuccess = AkAssetMigration::MigrateAudioBanks(MigrationOperations, MigrationOperations.bDoBankCleanup, bWasUsingEBP, MigrationOperations.bTransferAutoload, MigrationOperations.DefinitionFilePath);
		Result.bBankTransferSucceeded = bSuccess; 
		if (bSuccess)
		{
			AkSettings->bSoundBanksTransfered = true;
			bShouldUpdateConfig = true;
		}
		else
		{
			bContinue = false;
			if (MigrationOperations.bDoAssetMigration)
			{
				Result.bAssetMigrationSucceeded = false;
			}
			if (MigrationOperations.bDoDeprecatedAssetCleanup)
			{
				Result.bAssetCleanupSucceeded = false;
			}
		}
	}

	if (MigrationOperations.bDoAssetMigration && bContinue)
	{

		UE_LOG(LogAudiokineticTools, Display, TEXT("AkAssetMigrationManager: Migrating Wwise assets."));
		TArray<FAssetData> WwiseAssetsInProject;
		AkAssetMigration::FindWwiseAssetsInProject(WwiseAssetsInProject);
		if (AkAssetMigration::MigrateWwiseAssets(WwiseAssetsInProject, AkSettings->SplitSwitchContainerMedia))
		{
			AkSettings->bAssetsMigrated = true;
			bShouldUpdateConfig = true;
		}
		else
		{
			Result.bAssetMigrationSucceeded = false;
		}
	}

	if (MigrationOperations.bDoDeprecatedAssetCleanup && bContinue)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("AkAssetMigrationManager: Deleting deprecated assets."));
		TArray<FAssetData> DeprecatedAssetsInProject;
		AkAssetMigration::FindDeprecatedAssets(DeprecatedAssetsInProject);
		Result.bAssetCleanupSucceeded = AkAssetMigration::DeleteDeprecatedAssets(DeprecatedAssetsInProject);
		DeprecatedAssetsInProject.Empty();
	}

	
	if (MigrationOperations.bDoProjectUpdate && bContinue)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("AkAssetMigrationManager: Updating project settings."));
		if (!bWasUsingEBP)
		{
			AkSettings->RemoveSoundDataFromAlwaysStageAsUFS(AkSettings->WwiseSoundDataFolder.Path);
			AkUnrealEditorHelper::DeleteLegacySoundBanks();
		}
		else
		{
			AkSettings->RemoveSoundDataFromAlwaysCook(FString::Printf(TEXT("/Game/%s"), *AkSettings->WwiseSoundDataFolder.Path));
		}
		Result.bProjectMigrationSucceeded = MigrateProjectSettings(bWasUsingEBP, AkSettings->SplitMediaPerFolder, MigrationOperations.GeneratedSoundBankDirectory);
		AkSettings->UpdateGeneratedSoundBanksPath(MigrationOperations.GeneratedSoundBankDirectory);
		AkSettings->SplitMediaPerFolder = false;
		AkSettings->UseEventBasedPackaging = false;
		AkSettings->bProjectMigrated = true;

		bShouldUpdateConfig = true;
	}

	if (bShouldUpdateConfig)
	{
		if(!AkUnrealEditorHelper::SaveConfigFile(AkSettings))
		{
			const FString Message = TEXT("Unable to checkout settings file for Wwise integration settings (DefaultGame.ini). Please revert all changes done, and restart the migration process.");
			if (FApp::CanEverRender())
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
			}
			else
			{
				UE_LOG(LogAudiokineticTools, Error, TEXT("AkAssetMigrationManager: %s"), *Message);
			}
		}
	}

	//Any of these operations may have dirtied some assets, asset migration should handle saving on its own, but might as well be safe.
	if ( MigrationOperations.bDoAssetMigration || MigrationOperations.bDoBankCleanup || MigrationOperations.bTransferAutoload || MigrationOperations.bDoDeprecatedAssetCleanup)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("AkAssetMigrationManager: Saving all dirty assets in the project."));

		if (!UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true))
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("AkAssetMigrationManager: Failed to save all dirty packages. Please inspect the log for more details."));
		}
	}

	if (!Result.bAssetCleanupSucceeded || !Result.bAssetMigrationSucceeded || (!Result.bBankTransferSucceeded && !MigrationOperations.bIgnoreBankTransferErrors) || !Result.bProjectMigrationSucceeded)
	{
		Result.bSuccess = false;
	}

	if (!FApp::CanEverRender())
	{
		return Result;
	}

	FString ResultString = Result.bSuccess ? "Success" : "Failure";
	FNotificationInfo Info(FText::Format(LOCTEXT("AkAssetManagementManagerResult", "Migration completed - {0}"), FText::FromString(ResultString)));
	Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkBrowserTabIcon"));
	Info.bFireAndForget = true;
	Info.FadeOutDuration = 0.6f;
	Info.ExpireDuration = 4.6f;
	FSlateNotificationManager::Get().AddNotification(Info);
	if (Result.bSuccess)
	{
		GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
	}
	else
	{
		GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	}

	return Result;
}


bool AkAssetMigrationManager::IsSoundDataPathInDirectoriesToAlwaysStage(const FString& SoundDataPath)
{
	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
	for (int32 i = PackagingSettings->DirectoriesToAlwaysStageAsUFS.Num() - 1; i >= 0; --i)
	{
		if (PackagingSettings->DirectoriesToAlwaysStageAsUFS[i].Path == SoundDataPath)
		{
			return true;
		}
	}
	for (int32 i = PackagingSettings->DirectoriesToAlwaysCook.Num() - 1; i >= 0; --i)
	{
		if (PackagingSettings->DirectoriesToAlwaysCook[i].Path == SoundDataPath)
		{
			return true;
		}
	}
	return false;
}


void AkAssetMigrationManager::CreateMigrationMenuOption()
{
	// Extend the build menu to handle Audiokinetic-specific entries
#if UE_5_0_OR_LATER
	{
		UToolMenu* BuildMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
		FToolMenuSection& WwiseBuildSection = BuildMenu->AddSection(MigrationMenuSectionName, LOCTEXT("AkBuildLabel", "Audiokinetic Migration"), FToolMenuInsert("LevelEditorGeometry", EToolMenuInsertType::Default));
		FUIAction MigrationUIAction;
		MigrationUIAction.ExecuteAction.BindRaw(this, &AkAssetMigrationManager::EditorTryMigration);
		WwiseBuildSection.AddMenuEntry(
			NAME_None,
			LOCTEXT("AKAudioBank_PostMigration", "Finish Project Migration"),
			LOCTEXT("AkAudioBank_PostMigrationTooltip", "Transfer Bank hierarchy to Wwise, clean up bank files, delete Wwise media assets, clean up Wwise assets"),
			FSlateIcon(),
			MigrationUIAction
		);
	}
#else
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelViewportToolbarBuildMenuExtenderAkMigration = FLevelEditorModule::FLevelEditorMenuExtender::CreateLambda([this](const TSharedRef<FUICommandList> CommandList)
		{
			TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
			Extender->AddMenuExtension("LevelEditorGeometry", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.BeginSection("Audiokinetic Migration", LOCTEXT("AudiokineticMigration", "Audiokinetic Migration"));
					{
						FUIAction MigrationAction;
						MigrationAction.ExecuteAction.BindRaw(this, &AkAssetMigrationManager::EditorTryMigration);
						MenuBuilder.AddMenuEntry(
							LOCTEXT("AKAudioBank_PostMigration", "Finish Project Migration"),
							LOCTEXT("AkAudioBank_PostMigrationTooltip", "Transfer Bank hierarchy to Wwise, clean up bank files, delete Wwise media assets, clean up Wwise assets"),
							FSlateIcon(),
							MigrationAction
						);
					}
					MenuBuilder.EndSection();
				}));

			return Extender.ToSharedRef();
		});

	LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Add(LevelViewportToolbarBuildMenuExtenderAkMigration);
	LevelViewportToolbarBuildMenuExtenderAkMigrationHandle = LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Last().GetHandle();
#endif
}

void AkAssetMigrationManager::RemoveMigrationMenuOption()
{
#if UE_5_0_OR_LATER
	UToolMenu* BuildMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
	BuildMenu->RemoveSection(MigrationMenuSectionName);
#else
	if (LevelViewportToolbarBuildMenuExtenderAkMigrationHandle.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			auto& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().RemoveAll([=](const FLevelEditorModule::FLevelEditorMenuExtender& Extender)
				{
					return Extender.GetHandle() == LevelViewportToolbarBuildMenuExtenderAkMigrationHandle;
				});
		}
		LevelViewportToolbarBuildMenuExtenderAkMigrationHandle.Reset();
	}
#endif
}

void AkAssetMigrationManager::ClearSoundBanksForMigration()
{
	auto soundBankDirectory = AkUnrealEditorHelper::GetLegacySoundBankDirectory();

	TArray<FString> foundFiles;

	auto& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	platformFile.FindFilesRecursively(foundFiles, *soundBankDirectory, TEXT(".bnk"));
	platformFile.FindFilesRecursively(foundFiles, *soundBankDirectory, TEXT(".json"));

	if (foundFiles.Num() > 0)
	{
		platformFile.DeleteDirectoryRecursively(*AkUnrealEditorHelper::GetLegacySoundBankDirectory());
	}
}

bool AkAssetMigrationManager::MigrateProjectSettings(const bool bWasUsingEBP, const bool bUseGeneratedSubFolders, const FString& GeneratedSoundBanksFolder)
{
	const auto ProjectPath = WwiseUnrealHelper::GetWwiseProjectPath();
	FString ProjectContent;
	bool bSuccess = FFileHelper::LoadFileToString(ProjectContent, *ProjectPath);
	if (bSuccess)
	{
		bool bModified = AkAssetMigration::MigrateProjectSettings(ProjectContent, bWasUsingEBP, bUseGeneratedSubFolders, GeneratedSoundBanksFolder);
		bModified |= AkAssetMigration::SetStandardSettings(ProjectContent);

		if (bModified)
		{
			bSuccess = FFileHelper::SaveStringToFile(ProjectContent, *ProjectPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		}
	}
	return bSuccess;
}


bool AkAssetMigrationManager::SetStandardProjectSettings()
{
	const auto ProjectPath = WwiseUnrealHelper::GetWwiseProjectPath();
	FString ProjectContent;
	bool bSuccess = FFileHelper::LoadFileToString(ProjectContent, *ProjectPath);
	if (bSuccess)
	{
		const bool bModified = AkAssetMigration::SetStandardSettings(ProjectContent);
		if (bModified)
		{
			bSuccess = FFileHelper::SaveStringToFile(ProjectContent, *ProjectPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		}
	}
	return bSuccess;
}

// This whole hack is because Unreal XML classes doesn't
// handle <!CDATA[]> which the Wwise project file use.
// Doing it the dirty way instead.
bool AkAssetMigrationManager::SetGeneratedSoundBanksPath(const FString& ProjectContent, const FString& GeneratedSoundBanksFolder)
{
	const auto SoundBankPathPosition = ProjectContent.Find(TEXT("SoundBankHeaderFilePath"));
	if (SoundBankPathPosition != INDEX_NONE)
	{
		const FString ValueDelimiter = TEXT("Value=\"");
		const auto SoundBankPathValueStartPosition = ProjectContent.Find(*ValueDelimiter, ESearchCase::IgnoreCase, ESearchDir::FromStart, SoundBankPathPosition) + ValueDelimiter.Len();
		if (SoundBankPathValueStartPosition != INDEX_NONE)
		{
			const auto SoundBankPathValueEndPosition = ProjectContent.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, SoundBankPathValueStartPosition);

			if (SoundBankPathValueEndPosition != INDEX_NONE)
			{
				auto GeneratedPath = ProjectContent.Mid(SoundBankPathValueStartPosition, SoundBankPathValueEndPosition - SoundBankPathValueStartPosition);
				if(FPaths::IsRelative(GeneratedPath))
				{
					auto WwiseProjectDirectory = FPaths::GetPath(WwiseUnrealHelper::GetWwiseProjectPath());
					GeneratedPath = FPaths::Combine(WwiseProjectDirectory, GeneratedPath);
				}
				FPaths::MakePathRelativeTo(GeneratedPath, *FPaths::ProjectContentDir());
				UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
				if (AkSettings)
				{
					AkSettings->UpdateGeneratedSoundBanksPath(GeneratedPath);
					return true;
				}
			}
		}
	}

	return false;
}


#undef LOCTEXT_NAMESPACE
