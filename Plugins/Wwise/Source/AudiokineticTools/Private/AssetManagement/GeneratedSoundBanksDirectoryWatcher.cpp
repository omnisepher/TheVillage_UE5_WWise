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

#include "AssetManagement/GeneratedSoundBanksDirectoryWatcher.h"

#include "AkAudioModule.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "WwiseUnrealHelper.h"
#include "IAudiokineticTools.h"
#include "DirectoryWatcherModule.h"
#include "Async/Async.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseProjectDatabaseDelegates.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/Metadata/WwiseMetadataProjectInfo.h"

#define LOCTEXT_NAMESPACE "AkAudio"


bool GeneratedSoundBanksDirectoryWatcher::DoesWwiseProjectExist()
{
	return FPaths::FileExists(WwiseUnrealHelper::GetWwiseProjectPath());
}

void GeneratedSoundBanksDirectoryWatcher::CheckIfCachePathChanged()
{
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (UNLIKELY(!ProjectDatabase) || !IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}

	const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
	const FWwiseRefPlatform Platform = DataStructure.GetPlatform(ProjectDatabase->GetCurrentPlatform());
	if (auto* ProjectInfo = Platform.ProjectInfo.GetProjectInfo())
	{
		const FString SourceCachePath = WwiseUnrealHelper::GetSoundBankDirectory() / ProjectInfo->CacheRoot.ToString();
		if (SourceCachePath != CachePath || !CacheChangedHandle.IsValid())
		{
			UE_LOG(LogAudiokineticTools, Verbose, TEXT("GeneratedSoundBanksDirectoryWatcher::CheckIfCachePathChanged: Cache path changed, restarting cache watcher."));

			StopCacheWatcher();
			StartCacheWatcher(SourceCachePath);
		}
	}
}

void GeneratedSoundBanksDirectoryWatcher::Initialize()
{
	ProjectParsedHandle = FWwiseProjectDatabaseDelegates::Get()->GetOnDatabaseUpdateCompletedDelegate().AddRaw(this, &GeneratedSoundBanksDirectoryWatcher::CheckIfCachePathChanged);
	if (UAkSettings* AkSettings = GetMutableDefault<UAkSettings>())
	{
		// When GeneratedSoundBanks folder changes we need to reset the watcher
		if (SettingsChangedHandle.IsValid())
		{
			AkSettings->OnGeneratedSoundBanksPathChanged.Remove(SettingsChangedHandle);
			SettingsChangedHandle.Reset();
		}

		SettingsChangedHandle = AkSettings->OnGeneratedSoundBanksPathChanged.AddRaw(this, &GeneratedSoundBanksDirectoryWatcher::RestartWatchers);
	}

	if (UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>())
	{
		// When GeneratedSoundBanks Override folder changes we need to reset the watcher
		if (UserSettingsChangedHandle.IsValid())
		{
			UserSettings->OnGeneratedSoundBanksPathChanged.Remove(UserSettingsChangedHandle);
			UserSettingsChangedHandle.Reset();
		}

		UserSettingsChangedHandle = UserSettings->OnGeneratedSoundBanksPathChanged.AddRaw(this, &GeneratedSoundBanksDirectoryWatcher::RestartWatchers);
	}

	StartWatchers();
}

void GeneratedSoundBanksDirectoryWatcher::StartWatchers()
{
	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}

	//Start GeneratedSoundBanksWatcher to watch files which can be updated by source control (or direct manipulation)
	StartSoundBanksWatcher(WwiseUnrealHelper::GetSoundBankDirectory());

	// If there is a wwise project, we also watch the cache root file which notifies us when bank generation is done
	if (DoesWwiseProjectExist())
	{
		auto* ProjectDatabase = FWwiseProjectDatabase::Get();
		if (UNLIKELY(!ProjectDatabase))
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("GeneratedSoundBanksDirectoryWatcher::StartWatchers: Could not get WwiseProjectDatabase. Wwise Cache watcher will not be initialized"));
			return;
		}
		const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
		const FWwiseRefPlatform Platform = DataStructure.GetPlatform(ProjectDatabase->GetCurrentPlatform());
		if (Platform.IsValid())
		{
			if (auto* ProjectInfo = Platform.ProjectInfo.GetProjectInfo())
			{
				const FString SourceCachePath = WwiseUnrealHelper::GetSoundBankDirectory() / ProjectInfo->CacheRoot.ToString();
				StartCacheWatcher(SourceCachePath);
			}
		}
		else
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("GeneratedSoundBanksDirectoryWatcher::StartWatchers: Could not get Project Info for current platform from WwiseProjectDatabase. Wwise Cache watcher will not be initialized"));
		}
	}
}

bool GeneratedSoundBanksDirectoryWatcher::StartCacheWatcher(const FString& InCachePath)
{
	if (CacheChangedHandle.IsValid())
	{
		StopCacheWatcher();
	}

	if (!FPaths::DirectoryExists(InCachePath))
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("GeneratedSoundBanksDirectoryWatcher::StartCacheWatcher: Cache directory to watch does not exist %s"), *InCachePath);
		bCacheFolderExists = false;
		return false;
	}

	bCacheFolderExists = true;
	CachePath = InCachePath;
	UE_LOG(LogAudiokineticTools, Verbose, TEXT("GeneratedSoundBanksDirectoryWatcher::StartCacheWatcher: Starting cache watcher - %s."), *CachePath);

	auto& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	return DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(
		CachePath
		, IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &GeneratedSoundBanksDirectoryWatcher::OnCacheChanged)
		, CacheChangedHandle
		, IDirectoryWatcher::WatchOptions::IgnoreChangesInSubtree
	);
}

void GeneratedSoundBanksDirectoryWatcher::StartSoundBanksWatcher(const FString& GeneratedSoundBanksFolder)
{
	if (GeneratedSoundBanksHandle.IsValid())
	{
		StopSoundBanksWatcher();
	}

	if (!FPaths::DirectoryExists(GeneratedSoundBanksFolder))
	{
		UE_LOG(LogAudiokineticTools, Warning, TEXT("GeneratedSoundBanksDirectoryWatcher::StartSoundBanksWatcher: Generated Soundbanks Folder '%s' to watch not found.\nMake sure the Generated SoundBanks Folder setting is correct and ensure that SoundBanks are generated. Press the 'Refresh' button in the Wwise Browser to restart the watcher."), *GeneratedSoundBanksFolder);
		bGeneratedSoundBanksFolderExists = false;
		return;
	}

	bGeneratedSoundBanksFolderExists = true;
	UE_LOG(LogAudiokineticTools, Verbose, TEXT("GeneratedSoundBanksDirectoryWatcher::StartSoundBanksWatcher: Starting Generated Soundbanks watcher - %s."), *GeneratedSoundBanksFolder);
	SoundBankDirectory = GeneratedSoundBanksFolder;
	auto& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(
		GeneratedSoundBanksFolder
		, IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &GeneratedSoundBanksDirectoryWatcher::OnGeneratedSoundBanksChanged)
		, GeneratedSoundBanksHandle
		, IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges
	);
}

void GeneratedSoundBanksDirectoryWatcher::OnCacheChanged(const TArray<FFileChangeData>& ChangedFiles)
{
	for (FFileChangeData FileData : ChangedFiles)
	{
		const FString FileName = FPaths::GetBaseFilename(FileData.Filename);
		if (FileName == TEXT("SoundBankInfoCache"))
		{
			if (bParseTimerRunning)
			{
				UpdateNotificationOnGenerationComplete();
				EndParseTimer();
			}
			UE_LOG(LogAudiokineticTools, Verbose, TEXT("GeneratedSoundBanksDirectoryWatcher: SoundBankInfoCache updated."));
			OnSoundBankGenerationDone();
			break;
		}
	}
	UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("GeneratedSoundBanksDirectoryWatcher: Modifications to files in Wwise project cache detected."));
}

void GeneratedSoundBanksDirectoryWatcher::OnGeneratedSoundBanksChanged(const TArray<FFileChangeData>& ChangedFiles)
{
	ParseTimer = ParseDelaySeconds;
	UE_LOG(LogAudiokineticTools, Verbose, TEXT("GeneratedSoundBanksDirectoryWatcher: %d files changed in the Generated Soundbanks folder."), ChangedFiles.Num());
	if (!PostEditorTickHandle.IsValid() && !bParseTimerRunning)
	{
		bParseTimerRunning = true;
		PostEditorTickHandle = GEngine->OnPostEditorTick().AddRaw(this, &GeneratedSoundBanksDirectoryWatcher::TimerTick);
		NotifyFilesChanged();
	}
}

bool GeneratedSoundBanksDirectoryWatcher::ShouldRestartWatchers()
{
	const bool bCacheWatcherNeedsRestart = DoesWwiseProjectExist() && (!bCacheFolderExists || !CacheChangedHandle.IsValid());
	const bool bGeneratedSoundBanksWatcherNeedsRestart = !bGeneratedSoundBanksFolderExists || !GeneratedSoundBanksHandle.IsValid();
	return  bCacheWatcherNeedsRestart || bGeneratedSoundBanksWatcherNeedsRestart;
}

void GeneratedSoundBanksDirectoryWatcher::TimerTick(float DeltaSeconds)
{
	if (ParseTimer < 0 && bParseTimerRunning)
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("GeneratedSoundBanksDirectoryWatcher: No files have changed in the last %d seconds."), ParseDelaySeconds);
		OnSoundBankGenerationDone();
		EndParseTimer();
	}
	else if (!bParseTimerRunning)
	{
		EndParseTimer();
	}
	else if (ParseTimer > 0)
	{
		ParseTimer -= DeltaSeconds;
	}
	UpdateNotification();

}

void GeneratedSoundBanksDirectoryWatcher::EndParseTimer()
{
	bParseTimerRunning = false;
	ParseTimer = 0;
	GEngine->OnPostEditorTick().Remove(PostEditorTickHandle);
	PostEditorTickHandle.Reset();
	HideNotification();
}

void GeneratedSoundBanksDirectoryWatcher::OnSoundBankGenerationDone() const
{
	UE_LOG(LogAudiokineticTools, Verbose, TEXT("GeneratedSoundBanksDirectoryWatcher: Soundbank generation done."));
	OnSoundBanksGenerated.Broadcast();
}

void GeneratedSoundBanksDirectoryWatcher::NotifyFilesChanged()
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{

		FText InfoString = LOCTEXT("GeneratedSoundbanksWatcherInfoString", "Changes in generated soundbanks detected. \nProject data will be re-parsed in {parseSeconds} seconds.");
		FFormatNamedArguments NamedArguments;
		NamedArguments.Add(TEXT("parseSeconds"), static_cast<int>(this->ParseTimer));
		InfoString = FText::Format(InfoString, NamedArguments);
		FNotificationInfo Info(InfoString);

		Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkBrowserTabIcon"));
		Info.bFireAndForget = false;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 0.0f;
#if UE_4_26_OR_LATER
		Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { FGlobalTabmanager::Get()->TryInvokeTab(FName("OutputLog")); });
#else
		Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog")); });
#endif
		Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
		this->NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	});
}

void GeneratedSoundBanksDirectoryWatcher::HideNotification()
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		if (this->NotificationItem)
		{
			this->NotificationItem->Fadeout();
		}
	});
}

void GeneratedSoundBanksDirectoryWatcher::UpdateNotificationOnGenerationComplete() const
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		if (this->NotificationItem)
		{
			FText InfoString = LOCTEXT("GeneratedSoundbanksWatcherInfoString", "Detected that sound data generation is finished.");
			this->NotificationItem->SetText(InfoString);
			this->NotificationItem->SetFadeOutDuration(2.0f);
		}
	});
}

void GeneratedSoundBanksDirectoryWatcher::UpdateNotification() const
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		if (this->NotificationItem)
		{
			FText InfoString = LOCTEXT("GeneratedSoundbanksWatcherInfoString", "Changes in generated soundbanks detected. \nProject data will be re-parsed in {parseSeconds} seconds.");
			FFormatNamedArguments NamedArguments;
			NamedArguments.Add(TEXT("parseSeconds"), static_cast<int>(this->ParseTimer));
			InfoString = FText::Format(InfoString, NamedArguments);
			this->NotificationItem->SetText(InfoString);
		}
	});
}

void GeneratedSoundBanksDirectoryWatcher::StopWatchers()
{
	StopCacheWatcher();
	StopSoundBanksWatcher();
}

void GeneratedSoundBanksDirectoryWatcher::StopSoundBanksWatcher()
{
	if (GeneratedSoundBanksHandle.IsValid())
	{
		auto& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		DirectoryWatcherModule.Get()->UnregisterDirectoryChangedCallback_Handle(SoundBankDirectory, GeneratedSoundBanksHandle);
		GeneratedSoundBanksHandle.Reset();
	}
}

void GeneratedSoundBanksDirectoryWatcher::StopCacheWatcher()
{
	if (CacheChangedHandle.IsValid())
	{
		auto& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		DirectoryWatcherModule.Get()->UnregisterDirectoryChangedCallback_Handle(CachePath, CacheChangedHandle);
		CacheChangedHandle.Reset();
	}
}

void GeneratedSoundBanksDirectoryWatcher::RestartWatchers()
{
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		StopWatchers();
		StartWatchers();
	});
}

void GeneratedSoundBanksDirectoryWatcher::ConditionalRestartWatchers()
{
	if(ShouldRestartWatchers())
	{
		RestartWatchers();
	}
}

void GeneratedSoundBanksDirectoryWatcher::Uninitialize(const bool bIsModuleShutdown)
{
	StopWatchers();

	if (ProjectParsedHandle.IsValid())
	{
		FWwiseProjectDatabaseDelegates::Get()->GetOnDatabaseUpdateCompletedDelegate().Remove(ProjectParsedHandle);
		ProjectParsedHandle.Reset();
	}

	//Can't access settings while module is being shutdown
	if (!bIsModuleShutdown)
	{
		if (SettingsChangedHandle.IsValid())
		{
			if (UAkSettings* AkSettings = GetMutableDefault<UAkSettings>())
			{
				AkSettings->OnGeneratedSoundBanksPathChanged.Remove(SettingsChangedHandle);
			}
			SettingsChangedHandle.Reset();
		}
		if (UserSettingsChangedHandle.IsValid())
		{
			if (UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>())
			{
				UserSettings->OnGeneratedSoundBanksPathChanged.Remove(UserSettingsChangedHandle);
			}
			UserSettingsChangedHandle.Reset();
		}
	}
}

#undef LOCTEXT_NAMESPACE
