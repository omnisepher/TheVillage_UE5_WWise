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

/*=============================================================================
	AudiokineticToolsModule.cpp
=============================================================================*/
#include "AudiokineticToolsModule.h"

#include "AudiokineticToolsPrivatePCH.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioDevice.h"
#include "AkAudioStyle.h"
#include "AkComponent.h"
#include "AkEventAssetBroker.h"
#include "AkGeometryComponent.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AkSurfaceReflectorSetComponent.h"
#include "AkReverbZone.h"
#include "WwiseUnrealDefines.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "AssetManagement/AkAssetMigrationManager.h"
#include "AssetManagement/AkGenerateSoundBanksTask.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ComponentAssetBroker.h"
#include "ContentBrowserModule.h"
#include "DetailsCustomization/AkGeometryComponentDetailsCustomization.h"
#include "DetailsCustomization/AkLateReverbComponentDetailsCustomization.h"
#include "DetailsCustomization/AkRoomComponentDetailsCustomization.h"
#include "DetailsCustomization/AkPortalComponentDetailsCustomization.h"
#include "DetailsCustomization/AkSurfaceReflectorSetDetailsCustomization.h"
#include "DetailsCustomization/AkSettingsDetailsCustomization.h"
#include "DetailsCustomization/AkReverbZoneDetailsCustomization.h"
#include "LevelEditor.h"
#include "Editor/UnrealEdEngine.h"
#include "Factories/ActorFactoryAkAmbientSound.h"
#include "Factories/AkAssetTypeActions.h"
#include "Framework/Application/SlateApplication.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "Interfaces/IProjectManager.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "ISequencerModule.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "IAudiokineticTools.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "MovieScene.h"
#include "Platforms/AkUEPlatform.h"
#include "ProjectDescriptor.h"
#include "PropertyEditorModule.h"
#include "Sequencer/MovieSceneAkAudioEventTrackEditor.h"
#include "Sequencer/MovieSceneAkAudioRTPCTrackEditor.h"
#include "Settings/ProjectPackagingSettings.h"
#include "AkUnrealEditorHelper.h"
#include "EditorBuildUtils.h"
#include "UnrealEdGlobals.h"
#include "UnrealEdMisc.h"
#include "Visualizer/AkAcousticPortalVisualizer.h"
#include "Visualizer/AkComponentVisualizer.h"
#include "Visualizer/AkSurfaceReflectorSetComponentVisualizer.h"
#include "WaapiPicker/WwiseTreeItem.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SSpacer.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "AssetManagement/GeneratedSoundBanksDirectoryWatcher.h"
#include "AssetManagement/WwiseProjectInfo.h"
#include "WwiseProject/AcousticTextureParamLookup.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseProjectDatabaseDelegates.h"
#include "AkAudioModule.h"
#include "AssetManagement/StaticPluginWriter.h"

#include "WwiseInitBankLoader/WwiseInitBankLoader.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FAudiokineticToolsModule* FAudiokineticToolsModule::AudiokineticToolsModuleInstance = nullptr;


TSharedRef<SDockTab> FAudiokineticToolsModule::CreateWwiseBrowserTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> BrowserTab =
	SNew(SDockTab)
		.Label(LOCTEXT("AkAudioWwiseBrowserTabTitle", "Wwise Browser"))
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SWwiseBrowser)
		];
	return BrowserTab;
}

void FAudiokineticToolsModule::RefreshWwiseProject()
{
	SoundBanksDirectoryWatcher.ConditionalRestartWatchers();
	if (auto* ProjectDatabase = FWwiseProjectDatabase::Get())
	{
		ProjectDatabase->UpdateDataStructure();
	}
}

void FAudiokineticToolsModule::OpenOnlineHelp()
{
	FPlatformProcess::LaunchFileInDefaultExternalApplication(TEXT("https://www.audiokinetic.com/library/?source=UE4&id=index.html"));
}

void FAudiokineticToolsModule::ToggleVisualizeRoomsAndPortals()
{
	UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();
	if (AkSettingsPerUser != nullptr)
	{
		AkSettingsPerUser->ToggleVisualizeRoomsAndPortals();
	}
}

bool FAudiokineticToolsModule::IsVisualizeRoomsAndPortalsEnabled()
{
	const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
	if (AkSettingsPerUser == nullptr)
		return false;
	return AkSettingsPerUser->VisualizeRoomsAndPortals;
}

ECheckBoxState FAudiokineticToolsModule::GetVisualizeRoomsAndPortalsCheckBoxState()
{
	return IsVisualizeRoomsAndPortalsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FAudiokineticToolsModule::ToggleShowReverbInfo()
{
	UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();
	if (AkSettingsPerUser != nullptr)
	{
		AkSettingsPerUser->ToggleShowReverbInfo();
	}
}

bool FAudiokineticToolsModule::IsReverbInfoEnabled()
{
	const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
	if (AkSettingsPerUser == nullptr)
		return false;
	return AkSettingsPerUser->bShowReverbInfo;
}

ECheckBoxState FAudiokineticToolsModule::GetReverbInfoCheckBoxState()
{
	return IsReverbInfoEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FAudiokineticToolsModule::CreateAkViewportCommands()
{
	// Extend the viewport menu and add the Audiokinetic commands
	{
		UToolMenu* ViewportMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelViewportToolBar.Options");
		FToolMenuSection& AkSection = ViewportMenu->AddSection("Audiokinetic", LOCTEXT("AkLabel", "Audiokinetic"), FToolMenuInsert("Audiokinetic", EToolMenuInsertType::Default));

		ToggleVizRoomsPortalsAction.ExecuteAction.BindStatic(&FAudiokineticToolsModule::ToggleVisualizeRoomsAndPortals);
		ToggleVizRoomsPortalsAction.GetActionCheckState.BindStatic(&FAudiokineticToolsModule::GetVisualizeRoomsAndPortalsCheckBoxState);

		AkSection.AddMenuEntry(
			NAME_None,
			LOCTEXT("ToggleVizRoomsAndPortals_Label", "Visualize Rooms And Portals"),
			LOCTEXT("ToggleVizRoomsAndPortals_Tip", "Toggles the visualization of rooms and portals in the viewport. This requires 'realtime' to be enabled in the viewport."),
			FSlateIcon(),
			ToggleVizRoomsPortalsAction,
			EUserInterfaceActionType::ToggleButton
		);

		ToggleReverbInfoAction.ExecuteAction.BindStatic(&FAudiokineticToolsModule::ToggleShowReverbInfo);
		ToggleReverbInfoAction.GetActionCheckState.BindStatic(&FAudiokineticToolsModule::GetReverbInfoCheckBoxState);

		AkSection.AddMenuEntry(
			NAME_None,
			LOCTEXT("ToggleReverbInfo_Label", "Show Reverb Info"),
			LOCTEXT("ToggleReverbInfo_Tip", "When enabled, information about AkReverbComponents will be displayed in viewports, above the component's UPrimitiveComponent parent. This requires 'realtime' to be enabled in the viewport."),
			FSlateIcon(),
			ToggleReverbInfoAction,
			EUserInterfaceActionType::ToggleButton
		);
	}
}

void FAudiokineticToolsModule::RegisterWwiseMenus()
{
	// Extend the build menu to handle Audiokinetic-specific entries
#if UE_5_0_OR_LATER
	{
		UToolMenu* BuildMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
		FToolMenuSection& WwiseBuildSection = BuildMenu->AddSection("AkBuild", LOCTEXT("AkBuildLabel", "Audiokinetic"), FToolMenuInsert("LevelEditorGeometry", EToolMenuInsertType::Default));

		FUIAction GenerateSoundDataUIAction;
		GenerateSoundDataUIAction.ExecuteAction.BindStatic(&AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow, false);
		WwiseBuildSection.AddMenuEntry(
			NAME_None,
			LOCTEXT("AkAudioBank_GenerateSoundBanks", "Generate SoundBanks..."),
			LOCTEXT("AkAudioBank_GenerateSoundBanksTooltip", "Generates Wwise SoundBanks."),
			FSlateIcon(),
			GenerateSoundDataUIAction
		);

		FUIAction RefreshProjectUIAction;
		RefreshProjectUIAction.ExecuteAction.BindRaw(this, &FAudiokineticToolsModule::RefreshWwiseProject);
		WwiseBuildSection.AddMenuEntry(
			NAME_None,
			LOCTEXT("RefreshWwiseProject", "Refresh Project Database"),
			LOCTEXT("RefreshWwiseProjectTooltip", "Reparse the the Wwise Project in GeneratedSoundBanks and reload Wwise assets."),
			FSlateIcon(),
			RefreshProjectUIAction
		);
	}
#else
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelViewportToolbarBuildMenuExtenderAk = FLevelEditorModule::FLevelEditorMenuExtender::CreateLambda([this](const TSharedRef<FUICommandList> CommandList)
	{
		TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
		Extender->AddMenuExtension("LevelEditorGeometry", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.BeginSection("Audiokinetic", LOCTEXT("Audiokinetic", "Audiokinetic"));
			{
				FUIAction GenerateSoundDataUIAction;
				GenerateSoundDataUIAction.ExecuteAction.BindStatic(&AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow, false);
				MenuBuilder.AddMenuEntry(
					LOCTEXT("AkAudioBank_GenerateSoundBanks", "Generate SoundBanks..."),
					LOCTEXT("AkAudioBank_GenerateSoundBanksTooltip", "Generates Wwise SoundBanks."),
					FSlateIcon(),
					GenerateSoundDataUIAction
				);

				FUIAction RefreshProjectUIAction;
				RefreshProjectUIAction.ExecuteAction.BindRaw(this, &FAudiokineticToolsModule::RefreshWwiseProject);
				MenuBuilder.AddMenuEntry(
					LOCTEXT("AkAudioBank_RefreshProject", "Refresh Project"),
					LOCTEXT("AkAudioBank_RefreshProjectTooltip", "Refresh the Wwise Project"),
					FSlateIcon(),
					RefreshProjectUIAction
				);
			}
			MenuBuilder.EndSection();

		}));

		return Extender.ToSharedRef();
	});
	LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Add(LevelViewportToolbarBuildMenuExtenderAk);
	LevelViewportToolbarBuildMenuExtenderAkHandle = LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().Last().GetHandle();
#endif

	// Extend the Help menu to display a link to our documentation
	{
		UToolMenu* HelpMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Help");
		FToolMenuSection& WwiseHelpSection = HelpMenu->AddSection("AkHelp", LOCTEXT("AkHelpLabel", "Audiokinetic"), FToolMenuInsert("HelpBrowse", EToolMenuInsertType::Default));

		WwiseHelpSection.AddEntry(FToolMenuEntry::InitMenuEntry(
			NAME_None,
			LOCTEXT("AkWwiseHelpEntry", "Wwise Help"),
			LOCTEXT("AkWwiseHelpEntryToolTip", "Shows the online Wwise documentation."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FAudiokineticToolsModule::OpenOnlineHelp))
		));
	}
}

void FAudiokineticToolsModule::UpdateUnrealCultureToWwiseCultureMap(const WwiseProjectInfo& wwiseProjectInfo)
{

	if (!wwiseProjectInfo.IsProjectInfoParsed())
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("AudiokineticToolsModule::UpdateUnrealCultureToWwiseCultureMap: Wwise project not parsed. Unreal culture to Wwise culture map will not be updated."));
		return;
	}
	static constexpr auto InvariantCultureLCID = 0x007F;

	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (!AkSettings)
	{
		return;
	}

	TMap<FString, FString> wwiseToUnrealMap;
	for (auto& entry : WwiseLanguageToUnrealCultureList)
	{
		wwiseToUnrealMap.Add(entry.WwiseLanguage, entry.UnrealCulture);
	}

	TMap<FString, int> languageCountMap;

	for (auto& language : wwiseProjectInfo.GetSupportedLanguages())
	{
		if (auto* foundUnrealCulture = wwiseToUnrealMap.Find(language.Name))
		{
			auto culturePtr = FInternationalization::Get().GetCulture(*foundUnrealCulture);

			if (culturePtr && culturePtr->GetLCID() != InvariantCultureLCID)
			{
				int& langCount = languageCountMap.FindOrAdd(culturePtr->GetTwoLetterISOLanguageName());
				++langCount;
			}
		}
	}

	TSet<FString> foundCultures;

	bool modified = false;
	for (auto& language : wwiseProjectInfo.GetSupportedLanguages())
	{
		if (auto* foundUnrealCulture = wwiseToUnrealMap.Find(language.Name))
		{
			auto culturePtr = FInternationalization::Get().GetCulture(*foundUnrealCulture);

			if (culturePtr && culturePtr->GetLCID() != InvariantCultureLCID)
			{
				int* langCount = languageCountMap.Find(culturePtr->GetTwoLetterISOLanguageName());

				if (langCount && *langCount > 1)
				{
					auto newKey = *foundUnrealCulture;
					if (!AkSettings->UnrealCultureToWwiseCulture.Contains(newKey))
					{
						AkSettings->UnrealCultureToWwiseCulture.Add(newKey, language.Name);
						modified = true;
					}

					foundCultures.Add(newKey);
				}
				else
				{
					auto newKey = culturePtr->GetTwoLetterISOLanguageName();
					if (!AkSettings->UnrealCultureToWwiseCulture.Contains(newKey))
					{
						AkSettings->UnrealCultureToWwiseCulture.Add(newKey, language.Name);
						modified = true;
					}

					foundCultures.Add(newKey);
				}
			}
		}
		else
		{
			for (auto& entry : AkSettings->UnrealCultureToWwiseCulture)
			{
				if (entry.Value == language.Name)
				{
					foundCultures.Add(entry.Key);
					break;
				}
			}
		}
	}

	TSet<FString> keysToRemove;
	for (auto& entry : AkSettings->UnrealCultureToWwiseCulture)
	{
		if (!foundCultures.Contains(entry.Key))
		{
			keysToRemove.Add(entry.Key);
		}
	}

	for (auto& keyToRemove : keysToRemove)
	{
		AkSettings->UnrealCultureToWwiseCulture.Remove(keyToRemove);
		modified = true;
	}

	if (modified)
	{
		AkSettings->SaveConfig();
	}
}

void FAudiokineticToolsModule::VerifyGeneratedSoundBanksPath(UAkSettings* AkSettings, UAkSettingsPerUser* AkSettingsPerUser)
{
	if (!AkSettings->GeneratedSoundBanksPathExists())
	{
		if (!AkSettingsPerUser->SuppressGeneratedSoundBanksPathWarnings && FApp::CanEverRender())
		{
			if (EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SettingsNotSet", "GeneratedSoundBanks folder does not seem to be set. Would you like to open the settings window to set it?")))
			{
				FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("Project"), FName("Wwise"), FName("Integration"));
			}
		}
		else
		{
			UE_LOG(LogAudiokineticTools, Log, TEXT("GeneratedSoundBanks folder not found. The Wwise Browser will not be usable."));
		}
	}
	else
	{
		// First-time plugin migration: Project might be relative to Engine path. Fix-up the path to make it relative to the game.
		const auto ProjectDir = FPaths::ProjectContentDir();
		FString FullGameDir = FPaths::ConvertRelativePathToFull(ProjectDir);
		FString TempPath = FPaths::ConvertRelativePathToFull(FullGameDir, AkSettings->RootOutputPath.Path);
		if (!FPaths::DirectoryExists(TempPath))
		{
			if (!AkSettingsPerUser->SuppressGeneratedSoundBanksPathWarnings && FApp::CanEverRender())
			{
				TSharedPtr<SWindow> Dialog = SNew(SWindow)
					.Title(LOCTEXT("ResetWwisePath", "Reset GeneratedSoundBanks Folder Path"))
					.SupportsMaximize(false)
					.SupportsMinimize(false)
					.FocusWhenFirstShown(true)
					.SizingRule(ESizingRule::Autosized);

				TSharedRef<SWidget> DialogContent = SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(0.25f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AkUpdateWwisePath", "The Wwise Unreal Engine Integration plug-in's update process requires the Wwise GeneratedSoundBanks Folder to be set in the Project Settings dialog. Would you like to open the Project Settings?"))							.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.FillHeight(0.75f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SCheckBox)
						.Padding(FMargin(6.0, 2.0))
						.OnCheckStateChanged_Lambda([&](ECheckBoxState DontAskState) {
							AkSettingsPerUser->SuppressGeneratedSoundBanksPathWarnings = (DontAskState == ECheckBoxState::Checked);
						})
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AkDontShowAgain", "Don't show this again"))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SSpacer)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 3.0f, 0.0f, 3.0f)
						[
							SNew(SButton)
							.Text(LOCTEXT("Yes", "Yes"))
							.OnClicked_Lambda([&]() -> FReply {
								FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("Project"), FName("Plugins"), FName("Wwise"));
								Dialog->RequestDestroyWindow();
								AkSettingsPerUser->SaveConfig();
								return FReply::Handled();
							})
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 3.0f, 0.0f, 3.0f)
						[
							SNew(SButton)
							.Text(LOCTEXT("No", "No"))
							.OnClicked_Lambda([&]() -> FReply {
								Dialog->RequestDestroyWindow();
								AkSettingsPerUser->SaveConfig();
								return FReply::Handled();
							})
						]
					]
				;

				Dialog->SetContent(DialogContent);
				FSlateApplication::Get().AddModalWindow(Dialog.ToSharedRef(), nullptr);
			}
			else
			{
				UE_LOG(LogAudiokineticTools, Log, TEXT("GeneratedSoundBanks folder not found. The Wwise Browser will not be usable."));
			}
		}
		else
		{
			FPaths::MakePathRelativeTo(TempPath, *ProjectDir);
			if (AkSettings->RootOutputPath.Path != TempPath)
			{
				AkSettings->WwiseProjectPath.FilePath = TempPath;
				AkUnrealEditorHelper::SaveConfigFile(AkSettings);
			}
		}
	}
}

void FAudiokineticToolsModule::OnAssetRegistryFilesLoaded()
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();
	auto* CurrentProject = IProjectManager::Get().GetCurrentProject();
	bool doModifyProject = true;

	WwiseProjectInfo wwiseProjectInfo;
	wwiseProjectInfo.Parse();
	UpdateUnrealCultureToWwiseCultureMap(wwiseProjectInfo);

	if (GUnrealEd != NULL)
	{
		GUnrealEd->RegisterComponentVisualizer(UAkComponent::StaticClass()->GetFName(), MakeShareable(new FAkComponentVisualizer));
		GUnrealEd->RegisterComponentVisualizer(UAkSurfaceReflectorSetComponent::StaticClass()->GetFName(), MakeShareable(new FAkSurfaceReflectorSetComponentVisualizer));
		GUnrealEd->RegisterComponentVisualizer(UAkPortalComponent::StaticClass()->GetFName(), MakeShareable(new UAkPortalComponentVisualizer));
	}

	AkSettings->InitGeometrySurfacePropertiesTable();
	AkSettings->InitReverbAssignmentTable();
	AkSettings->EnsurePluginContentIsInAlwaysCook();

	AkAcousticTextureParamLookup AcousticTextureParamLookup;
	AcousticTextureParamLookup.UpdateParamsMap();

	if (!IsRunningCommandlet() )
	{
		if (FApp::CanEverRender())
		{
			AssetMigrationManager.CreateMigrationMenuOption();
			AssetMigrationManager.EditorTryMigration();
		}
		SoundBanksDirectoryWatcher.Initialize();
		SoundBanksDirectoryWatcher.OnSoundBanksGenerated.AddStatic(&FAudiokineticToolsModule::ParseGeneratedSoundBankData);
	}

	// If we're on the project loader screen, we don't want to display the dialog.
	// In that case, CurrentProject is nullptr.
	if (CurrentProject && AkSettings && AkSettingsPerUser)
	{
		VerifyGeneratedSoundBanksPath(AkSettings, AkSettingsPerUser);

		if (doModifyProject)
		{
			AssetMigrationManager.SetStandardProjectSettings();
		}
	}
}

void FAudiokineticToolsModule::StartupModule()
{
	AudiokineticToolsModuleInstance = this;
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		auto AudiokineticAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Audiokinetic")), LOCTEXT("AudiokineticAssetCategory", "Audiokinetic"));

		AkAssetTypeActionsArray =
		{
			MakeShared<FAssetTypeActions_AkAudioEvent>(AudiokineticAssetCategoryBit),
			MakeShared<FAssetTypeActions_AkAcousticTexture>(AudiokineticAssetCategoryBit),
			MakeShared<FAssetTypeActions_AkAuxBus>(AudiokineticAssetCategoryBit),
			MakeShared<FAssetTypeActions_AkRtpc>(AudiokineticAssetCategoryBit),
			MakeShared<FAssetTypeActions_AkTrigger>(AudiokineticAssetCategoryBit),
		};

		for (auto& AkAssetTypeActions : AkAssetTypeActionsArray)
			AssetTools.RegisterAssetTypeActions(AkAssetTypeActions.ToSharedRef());
	}

	if (FModuleManager::Get().IsModuleLoaded("LevelEditor") && !IsRunningCommandlet() && FApp::CanEverRender())
	{
		RegisterWwiseMenus();
		CreateAkViewportCommands();
	}

	RegisterSettings();

	AkEventBroker = MakeShared<FAkEventAssetBroker>();
	FComponentAssetBrokerage::RegisterBroker(AkEventBroker, UAkComponent::StaticClass(), true, true);

	auto& TabSpawnerEntry = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SWwiseBrowser::WwiseBrowserTabName, FOnSpawnTab::CreateRaw(this, &FAudiokineticToolsModule::CreateWwiseBrowserTab))
		.SetDisplayName(NSLOCTEXT("FAudiokineticToolsModule", "BrowserTabTitle", "Wwise Browser"))
		.SetTooltipText(NSLOCTEXT("FAudiokineticToolsModule", "BrowserTooltipText", "Open the Wwise Browser tab."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.AkBrowserTabIcon"));

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	OnAssetRegistryFilesLoadedHandle = AssetRegistryModule.Get().OnFilesLoaded().AddRaw(this, &FAudiokineticToolsModule::OnAssetRegistryFilesLoaded);

	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>(TEXT("Sequencer"));
	RTPCTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMovieSceneAkAudioRTPCTrackEditor::CreateTrackEditor));
	EventTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FMovieSceneAkAudioEventTrackEditor::CreateTrackEditor));

	// Since we are initialized in the PostEngineInit phase, our Ambient Sound actor factory is not registered. We need to register it ourselves.
	if (GEditor)
	{
		if (auto NewFactory = NewObject<UActorFactoryAkAmbientSound>())
		{
			GEditor->ActorFactories.Add(NewFactory);
		}
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UAkSurfaceReflectorSetComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkSurfaceReflectorSetDetailsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAkLateReverbComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkLateReverbComponentDetailsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAkRoomComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkRoomComponentDetailsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAkPortalComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkPortalComponentDetailsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAkGeometryComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkGeometryComponentDetailsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAkSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkSettingsDetailsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(AAkReverbZone::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAkReverbZoneDetailsCustomization::MakeInstance));

	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}

	if (FAkAudioModule::AkAudioModuleInstance && FAkAudioModule::AkAudioModuleInstance->bModuleInitialized)
	{
		OnAkAudioInit();	
	}
	else
	{
		FAkAudioModule::OnModuleInitialized.AddRaw(this, &FAudiokineticToolsModule::OnAkAudioInit);
	}


	//Project Database initial parse occurs before AudiokineticTools' Initialization. Call it here manually.
	SetStaticPluginsInformation();
	StaticPluginHandle = FWwiseProjectDatabaseDelegates::Get()->GetOnDatabaseUpdateCompletedDelegate().AddLambda(
		[this]()
		{
			SetStaticPluginsInformation();
		}
	);
	
	FEditorDelegates::BeginPIE.AddRaw(this, &FAudiokineticToolsModule::BeginPIE);
}

void FAudiokineticToolsModule::OnAkAudioInit()
{
	FAkAudioStyle::Initialize();

	if (UAkSettings* Settings = GetMutableDefault<UAkSettings>())
	{
		Settings->OnGeneratedSoundBanksPathChanged.AddRaw(this, &FAudiokineticToolsModule::OnSoundBanksFolderChanged);
		OnDatabaseUpdateTextureHandle = FWwiseProjectDatabaseDelegates::Get()->GetOnDatabaseUpdateCompletedDelegate().AddRaw(this, &FAudiokineticToolsModule::RefreshAndUpdateTextureParams);
	}
	if (UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>())
	{
		UserSettings->OnGeneratedSoundBanksPathChanged.AddRaw(this, &FAudiokineticToolsModule::OnSoundBanksFolderChanged);
	}
	OnDatabaseUpdateCompleteHandle = FWwiseProjectDatabaseDelegates::Get()->GetOnDatabaseUpdateCompletedDelegate().AddRaw(this, &FAudiokineticToolsModule::AssetReloadPrompt);
	
#if AK_SUPPORT_WAAPI
	if (!IsRunningCommandlet())
	{
		FAkWaapiClient::Initialize();
		if (UAkSettings* AkSettings = GetMutableDefault<UAkSettings>())
		{
			AkSettings->InitWaapiSync();
		}
	}
#endif
}

void FAudiokineticToolsModule::OnSoundBanksFolderChanged()
{
	FAkAudioModule::AkAudioModuleInstance->UpdateWwiseResourceLoaderSettings();
	ParseGeneratedSoundBankData();
}

void FAudiokineticToolsModule::BeginPIE(const bool bIsSimulating)
{

	UAkSettings* Settings = GetMutableDefault<UAkSettings>();
	if(Settings && !Settings->GeneratedSoundBanksPathExists() && FAkAudioModule::AkAudioModuleInstance)
	{
		DisplayGeneratedSoundBanksWarning();
	}

	UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>();
	if(UserSettings && !UserSettings->RootOutputPathOverride.Path.IsEmpty())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Using Root Output Path Override: %s"), *WwiseUnrealHelper::GetSoundBankDirectory());
	}
}

void FAudiokineticToolsModule::SetStaticPluginsInformation()
{
	for(auto& Platform : AkUnrealPlatformHelper::GetAllSupportedWwisePlatforms())
	{
		StaticPluginWriter::OutputPluginInformation(*Platform);
	}
}

void FAudiokineticToolsModule::DisplayGeneratedSoundBanksWarning()
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	GeneratedSoundBanksWarning.HideGeneratedSoundBanksNotification();
	GeneratedSoundBanksWarning.DisplayGeneratedSoundBanksWarning();
}

void FAudiokineticToolsModule::AssetReloadPrompt()
{
	const UAkSettingsPerUser* UserSettings = GetDefault<UAkSettingsPerUser>();
	if (UserSettings->AskForWwiseAssetReload && FApp::CanEverRender())
	{
		OpenAssetReloadPopup();
	}
	else
	{
		FAkAudioModule::AkAudioModuleInstance->ReloadWwiseAssetData();
	}
}

void FAudiokineticToolsModule::OpenAssetReloadPopup()
{
	ReloadPopup.HideRefreshNotification();
	ReloadPopup.NotifyProjectRefresh();
}


void FAudiokineticToolsModule::ParseGeneratedSoundBankData()
{
	FAkAudioModule::ParseGeneratedSoundBankData();
}

void FAudiokineticToolsModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		auto& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		for (auto AkAssetTypeActions : AkAssetTypeActionsArray)
			if (AkAssetTypeActions.IsValid())
				AssetTools.UnregisterAssetTypeActions(AkAssetTypeActions.ToSharedRef());
	}

	AkAssetTypeActionsArray.Empty();

	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		auto& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetAllLevelEditorToolbarBuildMenuExtenders().RemoveAll([this](const FLevelEditorModule::FLevelEditorMenuExtender& Extender)
		{
			return Extender.GetHandle() == LevelViewportToolbarBuildMenuExtenderAkHandle;
		});

		if (MainMenuExtender.IsValid())
		{
			LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MainMenuExtender);
		}
	}
	LevelViewportToolbarBuildMenuExtenderAkHandle.Reset();
	StaticPluginHandle.Reset();
	UnregisterSettings();

	if (GUnrealEd != NULL)
	{
		GUnrealEd->UnregisterComponentVisualizer(UAkComponent::StaticClass()->GetFName());
	}

	FGlobalTabmanager::Get()->UnregisterTabSpawner(SWwiseBrowser::WwiseBrowserTabName);

	if (FModuleManager::Get().IsModuleLoaded(TEXT("Sequencer")))
	{
		auto& SequencerModule = FModuleManager::GetModuleChecked<ISequencerModule>(TEXT("Sequencer"));
		SequencerModule.UnRegisterTrackEditor(RTPCTrackEditorHandle);
		SequencerModule.UnRegisterTrackEditor(EventTrackEditorHandle);
	}

	// Only found way to close the tab in the case of a hot-reload. We need a pointer to the DockTab, and the only way of getting it seems to be InvokeTab.
	if (IsValid(GUnrealEd))
	{
#if UE_4_26_OR_LATER
		auto WwiseBrowserTab = FGlobalTabmanager::Get()->TryInvokeTab(SWwiseBrowser::WwiseBrowserTabName);
		if (WwiseBrowserTab.IsValid())
		{
			WwiseBrowserTab->RequestCloseTab();
		}
#else
		FGlobalTabmanager::Get()->InvokeTab(SWwiseBrowser::WwiseBrowserTabName)->RequestCloseTab();
#endif
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SWwiseBrowser::WwiseBrowserTabName);

	if (UObjectInitialized())
	{
		FComponentAssetBrokerage::UnregisterBroker(AkEventBroker);
	}

	if (UObjectInitialized() && !IsEngineExitRequested())
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		if (PropertyModule)
		{
			PropertyModule->UnregisterCustomClassLayout(UAkSurfaceReflectorSetComponent::StaticClass()->GetFName());
			PropertyModule->UnregisterCustomClassLayout(UAkLateReverbComponent::StaticClass()->GetFName());
			PropertyModule->UnregisterCustomClassLayout(UAkRoomComponent::StaticClass()->GetFName());
		}
	}

	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}

	if (OnDatabaseUpdateTextureHandle.IsValid())
	{
		FWwiseProjectDatabaseDelegates::Get()->GetOnDatabaseUpdateCompletedDelegate().Remove(OnDatabaseUpdateTextureHandle);
		OnDatabaseUpdateTextureHandle.Reset();
	}

#if WITH_EDITOR
	FAkAudioStyle::Shutdown();
#if AK_SUPPORT_WAAPI
	FAkWaapiClient::DeleteInstance();
#endif
#endif

	SoundBanksDirectoryWatcher.Uninitialize(true);
	AudiokineticToolsModuleInstance = nullptr;
}

void FAudiokineticToolsModule::RefreshAndUpdateTextureParams()
{
	AkAcousticTextureParamLookup AcousticTextureParamLookup;
	AcousticTextureParamLookup.UpdateParamsMap();

	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (AkSettings)
	{
		AkSettings->RefreshAcousticTextureParams();
	}
}

EEditorBuildResult FAudiokineticToolsModule::BuildAkEventData(UWorld* world, FName name)
{
	if (!AkAssetDatabase::Get().CheckIfLoadingAssets())
	{
		AkGenerateSoundBanksTask::ExecuteForEditorPlatform();
		return EEditorBuildResult::InProgress;
	}
	else
	{
		return EEditorBuildResult::Skipped;
	}
}

TMap<FString, SettingsRegistrationStruct>& FAudiokineticToolsModule::GetWwisePlatformNameToSettingsRegistrationMap()
{
	static TMap<FString, SettingsRegistrationStruct> WwisePlatformNameToWwiseSettingsRegistrationMap;
	if (WwisePlatformNameToWwiseSettingsRegistrationMap.Num() == 0)
	{
		auto RegisterIntegrationSettings = SettingsRegistrationStruct(UAkSettings::StaticClass(),
			"Integration",
			LOCTEXT("WwiseIntegrationSettingsName", "Integration Settings"),
			LOCTEXT("WwiseIntegrationSettingsDescription", "Configure the Wwise Integration"));

		auto RegisterPerUserSettings = SettingsRegistrationStruct(UAkSettingsPerUser::StaticClass(),
			"User Settings",
			LOCTEXT("WwiseRuntimePerUserSettingsName", "User Settings"),
			LOCTEXT("WwiseRuntimePerUserSettingsDescription", "Configure the Wwise Integration per user"));

		WwisePlatformNameToWwiseSettingsRegistrationMap.Add(FString("Integration"), RegisterIntegrationSettings);
		WwisePlatformNameToWwiseSettingsRegistrationMap.Add(FString("User"), RegisterPerUserSettings);

		for (const auto& AvailablePlatform : AkUnrealPlatformHelper::GetAllSupportedUnrealPlatforms())
		{
			FString SettingsClassName = FString::Format(TEXT("/Script/AkAudio.Ak{0}InitializationSettings"), { *AvailablePlatform });
#if UE_5_1_OR_LATER
			auto* SettingsClass = UClass::TryFindTypeSlow<UClass>(*SettingsClassName);
#else
			auto* SettingsClass = FindObject<UClass>(ANY_PACKAGE, *SettingsClassName);
#endif
			if (SettingsClass)
			{
				FString CategoryNameKey = FString::Format(TEXT("Wwise{0}SettingsName"), { *AvailablePlatform });
				FString DescriptionNameKey = FString::Format(TEXT("Wwise{0}SettingsDescription"), { *AvailablePlatform });
				FString DescriptionText = FString::Format(TEXT("Configure the Wwise {0} Initialization Settings"), { *AvailablePlatform });
				FText PlatformNameText = FText::FromString(*AvailablePlatform);
				FString AdditionalDescriptionText = TEXT("");
				if (AkUnrealPlatformHelper::IsEditorPlatform(AvailablePlatform))
				{
					AdditionalDescriptionText = TEXT("\nYou must restart the Unreal Editor for changes to be applied to the Wwise Sound Engine running in the Editor");
				}
				FText PlatformDescriptionText = FText::Format(LOCTEXT("WwiseSettingsDescription", "Configure the Wwise {0} Initialization Settings{1}"), PlatformNameText, FText::FromString(*AdditionalDescriptionText));
				auto RegisterPlatform = SettingsRegistrationStruct(SettingsClass, FName(*AvailablePlatform),
					PlatformNameText,
					PlatformDescriptionText);
				WwisePlatformNameToWwiseSettingsRegistrationMap.Add(*AvailablePlatform, RegisterPlatform);
			}
		}
	}
	return WwisePlatformNameToWwiseSettingsRegistrationMap;
}

void FAudiokineticToolsModule::RegisterSettings()
{
	if (auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		auto UpdatePlatformSettings = [SettingsModule, this]
		{
			auto SettingsRegistrationMap = GetWwisePlatformNameToSettingsRegistrationMap();

			TSet<FString> SettingsThatShouldBeRegistered = { FString("Integration"), FString("User") };

			for (const auto& AvailablePlatform : AkUnrealPlatformHelper::GetAllSupportedUnrealPlatformsForProject())
			{
				if (SettingsRegistrationMap.Contains(AvailablePlatform))
				{
					SettingsThatShouldBeRegistered.Add(AvailablePlatform);
				}
			}

			auto SettingsToBeUnregistered = RegisteredSettingsNames.Difference(SettingsThatShouldBeRegistered);
			for (const auto& SettingsName : SettingsToBeUnregistered)
			{
				SettingsRegistrationMap[SettingsName].Unregister(SettingsModule);
				RegisteredSettingsNames.Remove(SettingsName);
			}

			auto SettingsToBeRegistered = SettingsThatShouldBeRegistered.Difference(RegisteredSettingsNames);
			for (const auto& SettingsName : SettingsToBeRegistered)
			{
				if (RegisteredSettingsNames.Contains(SettingsName))
					continue;

				SettingsRegistrationMap[SettingsName].Register(SettingsModule);
				RegisteredSettingsNames.Add(SettingsName);
			}
		};

		UpdatePlatformSettings();

		IProjectManager& ProjectManager = IProjectManager::Get();
		ProjectManager.OnTargetPlatformsForCurrentProjectChanged().AddLambda(UpdatePlatformSettings);
	}
}

void FAudiokineticToolsModule::UnregisterSettings()
{
	if (auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		auto SettingsRegistrationMap = GetWwisePlatformNameToSettingsRegistrationMap();
		for (const auto& SettingsName : RegisteredSettingsNames)
		{
			SettingsRegistrationMap[SettingsName].Unregister(SettingsModule);
		}
		RegisteredSettingsNames.Empty();
	}
}

#undef LOCTEXT_NAMESPACE
