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

#include "WwiseInitBankLoader/WwiseInitBankLoader.h"

#if WITH_EDITORONLY_DATA
#include "Platforms/AkPlatformInfo.h"
#include "Wwise/WwiseProjectDatabase.h"
#endif

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#endif

#include "AkAudioDevice.h"
#include "AkSettings.h"
#include "AkUnrealEditorHelper.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "WwiseInitBankLoader"

FWwiseInitBankLoader* FWwiseInitBankLoader::Get()
{
	static FWwiseInitBankLoader* Instance = new FWwiseInitBankLoader;
	return Instance;
}

FWwiseInitBankLoader::FWwiseInitBankLoader()
{
}

#if WITH_EDITORONLY_DATA
void FWwiseInitBankLoader::UpdateInitBankInSettings()
{
	UE_LOG(LogAkAudio, VeryVerbose, TEXT("FWwiseInitBankLoader::UpdateInitBankInSettings: Updating InitBank in Settings."));

	if (!GetInitBankAsset())
	{
		auto* Settings = GetMutableDefault<UAkSettings>();
		if (UNLIKELY(!Settings))
		{
			UE_LOG(LogAkAudio, Error, TEXT("UpdateInitBankInSettings: Could not retrieve AkSettings."));
			return;
		}

		const FSoftObjectPath Path = Settings->DefaultAssetCreationPath / TEXT("InitBank");
		if (const auto LoadedUObject = Path.TryLoad())
		{
			Settings->InitBank = Cast<UAkInitBank>(LoadedUObject);
			UE_CLOG(Settings->InitBank.IsValid(), LogAkAudio, Display, TEXT("UpdateInitBankInSettings: Using existing InitBank at %s"), *Settings->DefaultAssetCreationPath);
		}
		
		if (!Settings->InitBank.IsValid())
		{
			FScopedSlowTask SlowTask(0, LOCTEXT("WwiseInitBankCreating", "Creating InitBank asset..."));
			UE_LOG(LogAkAudio, Display, TEXT("UpdateInitBankInSettings: Creating required InitBank at %s"), *Settings->DefaultAssetCreationPath);

			auto& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			Settings->InitBank = Cast<UAkInitBank>(AssetToolsModule.CreateAsset(TEXT("InitBank"), Settings->DefaultAssetCreationPath, UAkInitBank::StaticClass(), nullptr));
		}

		const ELoadingPhase::Type CurrentPhase = IPluginManager::Get().GetLastCompletedLoadingPhase();
		if (CurrentPhase == ELoadingPhase::None || CurrentPhase < ELoadingPhase::PostEngineInit)
		{
			PostInitDelegate = FCoreDelegates::OnPostEngineInit.AddRaw(this, &FWwiseInitBankLoader::OnPostInitSavePackage);
		}
		else
		{
			OnPostInitSavePackage();
		}

	}
}
#endif


void FWwiseInitBankLoader::LoadInitBank() const
{
	auto* InitBankAsset = GetInitBankAsset();
	if (UNLIKELY(!InitBankAsset))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("LoadInitBank: InitBankAsset not initialized"));
		return;
	}

	UE_LOG(LogAkAudio, Verbose, TEXT("LoadInitBank: Loading Init Bank asset"));
	InitBankAsset->AddToRoot();
	InitBankAsset->LoadInitBank();
}

void FWwiseInitBankLoader::UnloadInitBank() const
{
	auto* InitBankAsset = GetInitBankAsset();
	if (UNLIKELY(!InitBankAsset))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UnloadInitBank: InitBankAsset not initialized"));
		return;
	}

	UE_LOG(LogAkAudio, Verbose, TEXT("UnloadInitBank: Unloading init bank asset"));
	InitBankAsset->RemoveFromRoot();
	InitBankAsset->UnloadInitBank(true);
}

UAkInitBank* FWwiseInitBankLoader::GetInitBankAsset() const
{
	const auto* Settings = GetDefault<UAkSettings>();
	if (UNLIKELY(!Settings))
	{
		UE_LOG(LogAkAudio, Error, TEXT("GetInitBankAsset: Could not retrieve AkSettings."));
		return nullptr;
	}
	if (Settings->InitBank.IsNull())
	{
		UE_LOG(LogAkAudio, Log, TEXT("FWwiseInitBankLoader::GetInitBankAsset: InitBank asset is not set in Settings."));
		return nullptr;
	}
	
	return Settings->InitBank.LoadSynchronous();
}

#if WITH_EDITORONLY_DATA
void FWwiseInitBankLoader::OnPostInitSavePackage() const
{
	auto* Settings = GetMutableDefault<UAkSettings>();
	if (UNLIKELY(!Settings))
	{
		UE_LOG(LogAkAudio, Error, TEXT("OnPostInitSavePackage: Could not retrieve AkSettings."));
		return;
	}
	if (UNLIKELY(!Settings->InitBank.IsValid()))
	{
		UE_LOG(LogAkAudio, Error, TEXT("OnPostInitSavePackage: Trying to save invalid Init Bank in %s"), *Settings->DefaultAssetCreationPath);
	}
	UE_LOG(LogAkAudio, Display, TEXT("OnPostInitSavePackage: Saving new Init Bank in %s..."), *Settings->DefaultAssetCreationPath);
	UEditorLoadingAndSavingUtils::SavePackages({ Settings->InitBank->GetPackage() }, true);
	AkUnrealEditorHelper::SaveConfigFile(Settings);
}
#endif

#undef LOCTEXT_NAMESPACE
