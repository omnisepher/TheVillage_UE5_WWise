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

#include "AkUnrealEditorHelper.h"

#include "WwiseUnrealHelper.h"

#if WITH_EDITOR

#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "ISourceControlModule.h"
#include "SSettingsEditorCheckoutNotice.h"

#include "AkSettings.h"
#include "WwiseUnrealDefines.h"
#include "Wwise/Stats/AkAudio.h"

#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif

#define LOCTEXT_NAMESPACE "AkAudio"
namespace AkUnrealEditorHelper
{

	const TCHAR* LocalizedFolderName = TEXT("Localized");


	void SanitizePath(FString& Path, const FString& PreviousPath, const FText& DialogMessage)
	{
		WwiseUnrealHelper::TrimPath(Path);

		FText FailReason;
		if (!FPaths::ValidatePath(Path, &FailReason))
		{
			if (FApp::CanEverRender())
			{
				FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			}
			else
			{
				UE_LOG(LogAkAudio, Error, TEXT("%s"), *FailReason.ToString());
			}
			Path = PreviousPath;
			return;
		}

		const FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Path);
		if (!FPaths::DirectoryExists(AbsolutePath))
		{
			if (FApp::CanEverRender())
			{
				FMessageDialog::Open(EAppMsgType::Ok, DialogMessage);
			}
			else
			{
				UE_LOG(LogAkAudio, Error, TEXT("%s"), *DialogMessage.ToString());
			}
			Path = PreviousPath;
			return;
		}
	}

	bool SanitizeFolderPathAndMakeRelativeToContentDir(FString& Path, const FString& PreviousPath, const FText& DialogMessage)
	{
		WwiseUnrealHelper::TrimPath(Path);

		FString TempPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*Path);
		FText FailReason;
		if (!FPaths::ValidatePath(TempPath, &FailReason))
		{
			if (FApp::CanEverRender())
			{
				FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			}
			else
			{
				UE_LOG(LogAkAudio, Error, TEXT("%s"), *FailReason.ToString());
			}
			Path = PreviousPath;
			return false;
		}

		auto ContentDirectory = WwiseUnrealHelper::GetContentDirectory();
		if (!FPaths::FileExists(TempPath))
		{
			// Path might be a valid one (relative to game) entered manually. Check that.
			TempPath = FPaths::ConvertRelativePathToFull(ContentDirectory, Path);

			if (!FPaths::DirectoryExists(TempPath))
			{
				if (FApp::CanEverRender())
				{
					if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, DialogMessage))
					{
						Path = PreviousPath;
						return false;
					}
				}
				else
				{
					// Allow setting not yet existing paths when running in headless mode (e.g. migration)
					UE_LOG(LogAkAudio, Warning, TEXT("Path '%s' does not exist."), *Path);
				}
			}
		}

		// Make the path relative to the game dir
		FPaths::MakePathRelativeTo(TempPath, *ContentDirectory);
		Path = TempPath;

		if (Path != PreviousPath)
		{

			return true;
		}
		return false;
	}

	bool SaveConfigFile(UObject* ConfigObject)
	{
		const FString ConfigFilename = ConfigObject->GetDefaultConfigFilename();
		if(ISourceControlModule::Get().IsEnabled())
		{
			if (!SettingsHelpers::IsCheckedOut(ConfigFilename, true))
			{
				if (!SettingsHelpers::CheckOutOrAddFile(ConfigFilename, true))
				{
					return false;
				}
			}
		}

#if UE_5_0_OR_LATER
		return ConfigObject->TryUpdateDefaultConfigFile();
#else
		ConfigObject->UpdateDefaultConfigFile();
		return true;
#endif
	}

	FString GetLegacySoundBankDirectory()
	{
		if (const UAkSettings* AkSettings = GetDefault<UAkSettings>())
		{
			return FPaths::Combine(WwiseUnrealHelper::GetContentDirectory(), AkSettings->WwiseSoundDataFolder.Path);
		}
		else
		{
			return FPaths::Combine(WwiseUnrealHelper::GetContentDirectory(), UAkSettings::DefaultSoundDataFolder);
		}
	}

	FString GetContentDirectory()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	}

	void DeleteLegacySoundBanks()
	{
		const TArray<FString> ExtensionsToDelete = { "bnk", "wem", "json", "txt", "xml" };
		bool SuccessfulDelete = true;
		for (auto& Extension : ExtensionsToDelete)
		{
			TArray<FString> FoundFiles;
			FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(FoundFiles, *WwiseUnrealHelper::GetSoundBankDirectory(), *Extension);
			FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(FoundFiles, *GetLegacySoundBankDirectory(), *Extension);
			TSet<FString> FoundFilesSet(FoundFiles);
			for (auto& File : FoundFilesSet)
			{
				SuccessfulDelete |= FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*File);
			}
		}

		if (!SuccessfulDelete)
		{
			if (!FApp::CanEverRender())
			{
				UE_LOG(LogAkAudio, Warning, TEXT("Unable to delete legacy SoundBank files. Please ensure to manually delete them after migration is complete."));
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CannotDeleteOldBanks", "Unable to delete legacy SoundBank files. Please ensure to manually delete them after migration is complete."));
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE

#endif		// WITH_EDITOR