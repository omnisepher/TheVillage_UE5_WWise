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

#include "AkAudioModule.h"
#include "AkAudioDevice.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "WwiseUnrealDefines.h"

#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "WwiseInitBankLoader/WwiseInitBankLoader.h"

#include "Misc/ScopedSlowTask.h"

#include "UObject/UObjectIterator.h"


#include "Wwise/API/WwiseSoundEngineAPI.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseDataStructure.h"
#include "Wwise/WwiseResourceCooker.h"
#endif

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/FileManager.h"
#endif
#include "Async/Async.h"

IMPLEMENT_MODULE(FAkAudioModule, AkAudio)
#define LOCTEXT_NAMESPACE "AkAudio"

FAkAudioModule* FAkAudioModule::AkAudioModuleInstance = nullptr;
FSimpleMulticastDelegate FAkAudioModule::OnModuleInitialized;
FSimpleMulticastDelegate FAkAudioModule::OnWwiseAssetDataReloaded;

// WwiseUnrealHelper overrides

namespace WwiseUnrealHelper
{
	static FString GetWwisePluginDirectoryImpl()
	{
		return FAkPlatform::GetWwisePluginDirectory();
	}

	static FString GetWwiseProjectPathImpl()
	{
		FString projectPath;

		if (auto* settings = GetDefault<UAkSettings>())
		{
			projectPath = settings->WwiseProjectPath.FilePath;

			if (FPaths::IsRelative(projectPath))
			{
				projectPath = FPaths::ConvertRelativePathToFull(GetProjectDirectory(), projectPath);
			}

#if PLATFORM_WINDOWS
			projectPath.ReplaceInline(TEXT("/"), TEXT("\\"));
#endif
		}

		return projectPath;
	}

	static FString GetSoundBankDirectoryImpl()
	{
		const UAkSettingsPerUser* UserSettings = GetDefault<UAkSettingsPerUser>();
		FString SoundBankDirectory;
		if (UserSettings && !UserSettings->RootOutputPathOverride.Path.IsEmpty())
		{
			SoundBankDirectory = UserSettings->RootOutputPathOverride.Path;
			if(FPaths::IsRelative(UserSettings->RootOutputPathOverride.Path))
			{
				SoundBankDirectory = FPaths::Combine(GetContentDirectory(), UserSettings->RootOutputPathOverride.Path);
			}
		}
		else if (const UAkSettings* AkSettings = GetDefault<UAkSettings>())
		{
			if(AkSettings->RootOutputPath.Path.IsEmpty())
			{
				return {};
			}
			SoundBankDirectory = AkSettings->RootOutputPath.Path;
			if(FPaths::IsRelative(AkSettings->RootOutputPath.Path))
			{
				SoundBankDirectory = FPaths::Combine(GetContentDirectory(), AkSettings->RootOutputPath.Path);	
			}
		}
		else
		{
			UE_LOG(LogAkAudio, Warning, TEXT("WwiseUnrealHelper::GetSoundBankDirectory : Please set the Generated Soundbanks Folder in Wwise settings. Otherwise, sound will not function."));
			return {};
		}
		FPaths::CollapseRelativeDirectories(SoundBankDirectory);
		if(!SoundBankDirectory.EndsWith(TEXT("/")))
		{
			SoundBankDirectory.AppendChar('/');
		}

		return SoundBankDirectory;
	}

	static FString GetStagePathImpl()
	{
		const UAkSettings* Settings = GetDefault<UAkSettings>();
#if WITH_EDITORONLY_DATA
		if (Settings && !Settings->WwiseStagingDirectory.Path.IsEmpty())
		{
			return Settings->WwiseStagingDirectory.Path;
		}
		return TEXT("WwiseAudio");
#endif
		if (Settings && !Settings->WwiseStagingDirectory.Path.IsEmpty())
		{
			return FPaths::ProjectContentDir() / Settings->WwiseStagingDirectory.Path;
		}
		return FPaths::ProjectContentDir() / TEXT("WwiseAudio");
	}
}

void FAkAudioModule::StartupModule()
{
	IWwiseSoundEngineModule::ForceLoadModule();
	WwiseUnrealHelper::SetHelperFunctions(
		WwiseUnrealHelper::GetWwisePluginDirectoryImpl,
		WwiseUnrealHelper::GetWwiseProjectPathImpl,
		WwiseUnrealHelper::GetSoundBankDirectoryImpl,
		WwiseUnrealHelper::GetStagePathImpl);

#if WITH_EDITOR
	// It is not wanted to initialize the SoundEngine while running the GenerateSoundBanks commandlet.
	if (IsRunningCommandlet())
	{
		// We COULD use GetRunningCommandletClass(), but unfortunately it is set to nullptr in OnPostEngineInit.
		// We need to parse the command line.
		FString CmdLine(FCommandLine::Get());
		if (CmdLine.Contains(TEXT("run=GenerateSoundBanks")))
		{
			UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::StartupModule: Detected GenerateSoundBanks commandlet is running. AkAudioModule will not be initialized."));
			return;
		}

#if WITH_EDITORONLY_DATA
		if(!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
		{
			// Initialize the Rersource Cooker
			IWwiseResourceCookerModule::GetModule();
		}
#endif
	}
#endif

	if (AkAudioModuleInstance == this)
	{
		UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::StartupModule: AkAudioModuleInstance already exists."));
		return;
	}
	UE_CLOG(AkAudioModuleInstance, LogAkAudio, Warning, TEXT("FAkAudioModule::StartupModule: Updating AkAudioModuleInstance from (%p) to (%p)! Previous Module instance was improperly shut down!"), AkAudioModuleInstance, this);

	AkAudioModuleInstance = this;

	FScopedSlowTask SlowTask(0, LOCTEXT("InitWwisePlugin", "Initializing Wwise Plug-in AkAudioModule..."));

#if !UE_SERVER
	UpdateWwiseResourceLoaderSettings();
#endif
	
#if WITH_EDITORONLY_DATA
	if (auto* AkSettings = GetDefault<UAkSettings>())
	{
		if (AkSettings->AreSoundBanksGenerated())
		{
			ParseGeneratedSoundBankData();
			FWwiseInitBankLoader::Get()->UpdateInitBankInSettings();
		}
	}

	// Loading the File Handler Module, in case it loads a different module with UStructs, so it gets packaged (Ex.: Simple External Source Manager)
	IWwiseFileHandlerModule::GetModule();
#endif

	AkAudioDevice = new FAkAudioDevice;
	if (!AkAudioDevice)
	{
		UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::StartupModule: Couldn't create FAkAudioDevice. AkAudioModule will not be fully initialized."));
		bModuleInitialized = true;
		return;
	}

	if (!AkAudioDevice->Init())
	{
		UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::StartupModule: Couldn't initialize FAkAudioDevice. AkAudioModule will not be fully initialized."));
		bModuleInitialized = true;
		delete AkAudioDevice;
		AkAudioDevice = nullptr;
		return;
	}

	//Load init bank in Runtime
	UE_LOG(LogAkAudio, VeryVerbose, TEXT("FAkAudioModule::StartupModule: Loading Init Bank."));
	FWwiseInitBankLoader::Get()->LoadInitBank();

	OnTick = FTickerDelegate::CreateRaw(AkAudioDevice, &FAkAudioDevice::Update);
	TickDelegateHandle = FCoreTickerType::GetCoreTicker().AddTicker(OnTick);

	AkAudioDevice->LoadDelayedObjects();

	UE_LOG(LogAkAudio, VeryVerbose, TEXT("FAkAudioModule::StartupModule: Module Initialized."));
	OnModuleInitialized.Broadcast();
	bModuleInitialized = true;
}

void FAkAudioModule::ShutdownModule()
{
	UE_CLOG(AkAudioModuleInstance && AkAudioModuleInstance != this, LogAkAudio, Warning, TEXT("FAkAudioModule::ShutdownModule: Shutting down a different instance (%p) that was initially instantiated (%p)!"), this, AkAudioModuleInstance);

	FCoreTickerType::GetCoreTicker().RemoveTicker(TickDelegateHandle);

	if (AkAudioDevice)
	{
		AkAudioDevice->Teardown();
		delete AkAudioDevice;
		AkAudioDevice = nullptr;
	}

	if (IWwiseSoundEngineModule::IsAvailable())
	{
		WwiseUnrealHelper::SetHelperFunctions(nullptr, nullptr, nullptr, nullptr);
	}

	AkAudioModuleInstance = nullptr;
}

FAkAudioDevice* FAkAudioModule::GetAkAudioDevice() const
{
	return AkAudioDevice;
}

void FAkAudioModule::ReloadWwiseAssetData() const
{
	SCOPED_AKAUDIO_EVENT(TEXT("ReloadWwiseAssetData"));
	if (FAkAudioDevice::IsInitialized())
	{
		UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::ReloadWwiseAssetData : Reloading Wwise asset data."));
		if (AkAudioDevice)
		{
			AkAudioDevice->ClearSoundBanksAndMedia();
		}
		
		auto* InitBankLoader = FWwiseInitBankLoader::Get();
		if (LIKELY(InitBankLoader))
		{
			InitBankLoader->LoadInitBank();
		}
		else
		{
			UE_LOG(LogAkAudio, Error, TEXT("LoadInitBank: WwiseInitBankLoader is not initialized."));
		}

		for (TObjectIterator<UAkAudioType> AudioAssetIt; AudioAssetIt; ++AudioAssetIt)
		{
			AudioAssetIt->LoadData();
		}
		OnWwiseAssetDataReloaded.Broadcast();
	}
	else
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("FAkAudioModule::ReloadWwiseAssetData : Skipping asset data reload because the SoundEngine is not initialized."));
	}
}

void FAkAudioModule::UpdateWwiseResourceLoaderSettings()
{
	SCOPED_AKAUDIO_EVENT(TEXT("UpdateWwiseResourceLoaderSettings"));
	UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::UpdateWwiseResourceLoaderSettings : Updating Resource Loader settings."));

	auto* ResourceLoader = FWwiseResourceLoader::Get();
	if (!ResourceLoader)
	{
		UE_LOG(LogAkAudio, Error, TEXT("FAkAudioModule::UpdateWwiseResourceLoaderSettings : No Resource Loader!"));
		return;
	}
	auto* ResourceLoaderImpl = ResourceLoader->ResourceLoaderImpl.Get();
	if (!ResourceLoaderImpl)
	{
		UE_LOG(LogAkAudio, Error, TEXT("FAkAudioModule::UpdateWwiseResourceLoaderSettings : No Resource Loader Impl!"));
		return;
	}

	ResourceLoaderImpl->StagePath = WwiseUnrealHelper::GetStagePathImpl();

#if WITH_EDITORONLY_DATA
	ResourceLoaderImpl->GeneratedSoundBanksPath = FDirectoryPath{WwiseUnrealHelper::GetSoundBankDirectory()};
#endif
}

#if WITH_EDITORONLY_DATA
void FAkAudioModule::ParseGeneratedSoundBankData()
{
	SCOPED_AKAUDIO_EVENT(TEXT("ParseGeneratedSoundBankData"));
	if (auto* AkSettings = GetDefault<UAkSettings>())
	{
		if (!AkSettings->AreSoundBanksGenerated())
		{
			UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioModule::ParseGeneratedSoundBankData: SoundBanks are not yet generated, nothing to parse.\nCurrent Generated SoundBanks path is: %s"), *WwiseUnrealHelper::GetSoundBankDirectory());
			return;
		}
	}
	UE_LOG(LogAkAudio, Log, TEXT("FAkAudioModule::ParseGeneratedSoundBankData : Parsing Wwise project data."));
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioModule::ParseGeneratedSoundBankData : Could not get FWwiseProjectDatabase instance. Generated sound data will not be parsed."));
	}
	else
	{
		ProjectDatabase->UpdateDataStructure();
	}
}
#endif

#undef LOCTEXT_NAMESPACE