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
/*=============================================================================
	AudiokineticToolsModule.h
=============================================================================*/
#include "AudiokineticToolsPrivatePCH.h"
#include "AkAcousticPortal.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkComponent.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AssetManagement/AkAssetMigrationManager.h"
#include "AssetManagement/AkGenerateSoundBanksTask.h"
#include "AssetManagement/GeneratedSoundBanksDirectoryWatcher.h"
#include "AssetManagement/WwiseProjectInfo.h"
#include "AssetToolsModule.h"
#include "Factories/AkAssetTypeActions.h"
#include "IAudiokineticTools.h"

#include "WwiseBrowser/SWwiseBrowser.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "GeneratedSoundBanksWarning.h"
#include "ReloadPopup.h"

#include "ComponentAssetBroker.h"
#include "ToolMenu.h"
#include "LevelEditor.h"
#include "ISequencerModule.h"
#include "ISettingsModule.h"
#include "Internationalization/Culture.h"
#include "Modules/ModuleManager.h"
#include "Sequencer/MovieSceneAkAudioRTPCTrackEditor.h"
#include "EditorBuildUtils.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SHyperlink.h"

#define LOCTEXT_NAMESPACE "AkAudio"
DEFINE_LOG_CATEGORY(LogAudiokineticTools);

namespace
{
	struct WwiseLanguageToUnrealCulture
	{
		const TCHAR* WwiseLanguage;
		const TCHAR* UnrealCulture;
	};

	// This list come from the fixed list of languages that were used before Wwise 2017.1
	const WwiseLanguageToUnrealCulture WwiseLanguageToUnrealCultureList[] = {
		{TEXT("Arabic"), TEXT("ar")},
		{TEXT("Bulgarian"), TEXT("bg")},
		{TEXT("Chinese(HK)"), TEXT("zh-HK")},
		{TEXT("Chinese(Malaysia)"), TEXT("zh")},
		{TEXT("Chinese(PRC)"), TEXT("zh-CN")},
		{TEXT("Chinese(Taiwan)"), TEXT("zh-TW")},
		{TEXT("Czech"), TEXT("cs")},
		{TEXT("Danish"), TEXT("da")},
		{TEXT("English(Australia)"), TEXT("en-AU")},
		{TEXT("English(Canada)"), TEXT("en-CA")},
		{TEXT("English(US)"), TEXT("en-US")},
		{TEXT("English(UK)"), TEXT("en-GB")},
		{TEXT("Finnish"), TEXT("fi")},
		{TEXT("French(Canada)"), TEXT("fr-CA")},
		{TEXT("French(France)"), TEXT("fr-FR")},
		{TEXT("German"), TEXT("de")},
		{TEXT("Greek"), TEXT("el")},
		{TEXT("Hebrew"), TEXT("he")},
		{TEXT("Hungarian"), TEXT("hu")},
		{TEXT("Indonesian"), TEXT("id")},
		{TEXT("Italian"), TEXT("it")},
		{TEXT("Japanese"), TEXT("ja")},
		{TEXT("Korean"), TEXT("ko")},
		{TEXT("Norwegian "), TEXT("no")},
		{TEXT("Polish"), TEXT("pl")},
		{TEXT("Portuguese(Brazil)"), TEXT("pt-BR")},
		{TEXT("Portuguese(Portugal)"), TEXT("pt-PT")},
		{TEXT("Romanian"), TEXT("ro")},
		{TEXT("Russian"), TEXT("ru")},
		{TEXT("Slovenian"), TEXT("sl")},
		{TEXT("Spanish(Mexico)"), TEXT("es-MX")},
		{TEXT("Spanish(Spain)"), TEXT("es-ES")},
		{TEXT("Swedish"), TEXT("sv")},
		{TEXT("Thai"), TEXT("th")},
		{TEXT("Turkish"), TEXT("tr")},
		{TEXT("Ukrainian"), TEXT("uk")},
		{TEXT("Vietnamese"), TEXT("vi")},
	};
}

struct SettingsRegistrationStruct
{
	SettingsRegistrationStruct(UClass* SettingsClass, const FName& SectionName, const FText& DisplayName, const FText& Description)
		: SettingsClass(SettingsClass), SectionName(SectionName), DisplayName(DisplayName), Description(Description)
	{}

	void Register(ISettingsModule* SettingsModule) const
	{
		SettingsModule->RegisterSettings("Project", "Wwise", SectionName, DisplayName, Description, SettingsObject());
	}

	void Unregister(ISettingsModule* SettingsModule) const
	{
		SettingsModule->UnregisterSettings("Project", "Wwise", SectionName);
	}

private:
	UClass* SettingsClass;
	const FName SectionName;
	const FText DisplayName;
	const FText Description;

	UObject* SettingsObject() const { return SettingsClass->GetDefaultObject(); }
};

class FAudiokineticToolsModule : public IAudiokineticTools
{
public:
	/**
	 * Creates a new WwiseBrowser tab.
	 *
	 * @param SpawnTabArgs The arguments for the tab to spawn.
	 * @return The spawned tab.
	 */
	TSharedRef<SDockTab> CreateWwiseBrowserTab(const FSpawnTabArgs& SpawnTabArgs);
	void RefreshWwiseProject() override;
	void OpenOnlineHelp();
	static void ToggleVisualizeRoomsAndPortals();
	static bool IsVisualizeRoomsAndPortalsEnabled();
	static ECheckBoxState GetVisualizeRoomsAndPortalsCheckBoxState();
	static void ToggleShowReverbInfo();
	static bool IsReverbInfoEnabled();
	static ECheckBoxState GetReverbInfoCheckBoxState();

	void CreateAkViewportCommands();
	void RegisterWwiseMenus();
	void UpdateUnrealCultureToWwiseCultureMap(const WwiseProjectInfo& wwiseProjectInfo);
	void VerifyGeneratedSoundBanksPath(UAkSettings* AkSettings, UAkSettingsPerUser* AkSettingsPerUser);
	void OnAssetRegistryFilesLoaded();

	virtual void StartupModule() override;
	virtual void OnAkAudioInit();
	void OnSoundBanksFolderChanged();
	void BeginPIE(bool bIsSimulating);

	void SetStaticPluginsInformation();
	void DisplayGeneratedSoundBanksWarning();
	void AssetReloadPrompt();
	void OpenAssetReloadPopup();
	static void ParseGeneratedSoundBankData();
	virtual void ShutdownModule() override;
	static EEditorBuildResult BuildAkEventData(UWorld* world, FName name);

	static FAudiokineticToolsModule* AudiokineticToolsModuleInstance;

	void RefreshAndUpdateTextureParams();

private:
	static TMap<FString, SettingsRegistrationStruct>& GetWwisePlatformNameToSettingsRegistrationMap();
	TSet<FString> RegisteredSettingsNames;
	void RegisterSettings();
	void UnregisterSettings();

	/** Ak-specific viewport actions */
	FUIAction ToggleVizRoomsPortalsAction;
	FUIAction ToggleReverbInfoAction;

	TArray<TSharedPtr<FAssetTypeActions_Base>> AkAssetTypeActionsArray;
	TSharedPtr<FExtender> MainMenuExtender;
	FLevelEditorModule::FLevelEditorMenuExtender LevelViewportToolbarBuildMenuExtenderAk;
	FDelegateHandle LevelViewportToolbarBuildMenuExtenderAkHandle;
	FDelegateHandle OnAssetRegistryFilesLoadedHandle;
	FDelegateHandle RTPCTrackEditorHandle;
	FDelegateHandle EventTrackEditorHandle;
	FDelegateHandle StaticPluginHandle;

	/** Allow to create an AkComponent when Drag & Drop of an AkEvent */
	TSharedPtr<IComponentAssetBroker> AkEventBroker;

	FDoEditorBuildDelegate buildDelegate;
	AkAssetMigrationManager AssetMigrationManager;
	GeneratedSoundBanksDirectoryWatcher SoundBanksDirectoryWatcher;
	FDelegateHandle OnDatabaseUpdateCompleteHandle;
	FDelegateHandle OnDatabaseUpdateTextureHandle;

	FReloadPopup ReloadPopup = FReloadPopup();
	FGeneratedSoundBanksWarning GeneratedSoundBanksWarning = FGeneratedSoundBanksWarning();
};

IMPLEMENT_MODULE(FAudiokineticToolsModule, AudiokineticTools);

#undef LOCTEXT_NAMESPACE
