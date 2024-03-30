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
	AkAudioBankGenerationHelpers.cpp: Wwise Helpers to generate banks from the editor and when cooking.
------------------------------------------------------------------------------------*/

#include "AkAudioBankGenerationHelpers.h"

#include "AkAudioDevice.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "WwiseUnrealDefines.h"
#include "IAudiokineticTools.h"
#include "AssetManagement/AkAssetDatabase.h"

#include "ObjectTools.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "Interfaces/IMainFrameModule.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "AssetManagement/WwiseProjectInfo.h"
#include "UI/SGenerateSoundBanks.h"

#define LOCTEXT_NAMESPACE "AkAudio"

namespace AkAudioBankGenerationHelper
{
	FString GetWwiseConsoleApplicationPath()
	{
		const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
		FString ApplicationToRun;
		ApplicationToRun.Empty();

		if (AkSettingsPerUser)
		{
#if PLATFORM_WINDOWS
			ApplicationToRun = AkSettingsPerUser->WwiseWindowsInstallationPath.Path;
#else
			ApplicationToRun = AkSettingsPerUser->WwiseMacInstallationPath.FilePath;
#endif
			if (FPaths::IsRelative(ApplicationToRun))
			{
				ApplicationToRun = FPaths::ConvertRelativePathToFull(WwiseUnrealHelper::GetProjectDirectory(), ApplicationToRun);
			}
			if (!(ApplicationToRun.EndsWith(TEXT("/")) || ApplicationToRun.EndsWith(TEXT("\\"))))
			{
				ApplicationToRun += TEXT("/");
			}

#if PLATFORM_WINDOWS
			if (FPaths::FileExists(ApplicationToRun + TEXT("Authoring/x64/Release/bin/WwiseConsole.exe")))
			{
				ApplicationToRun += TEXT("Authoring/x64/Release/bin/WwiseConsole.exe");
			}
			else
			{
				ApplicationToRun += TEXT("Authoring/Win32/Release/bin/WwiseConsole.exe");
			}
			ApplicationToRun.ReplaceInline(TEXT("/"), TEXT("\\"));
#elif PLATFORM_MAC
			ApplicationToRun += TEXT("Contents/Tools/WwiseConsole.sh");
			ApplicationToRun = TEXT("\"") + ApplicationToRun + TEXT("\"");
#endif
		}

		return ApplicationToRun;
	}

	void CreateGenerateSoundDataWindow(bool ProjectSave)
	{
		if (!FApp::CanEverRender())
		{
			return;
		}

		if (AkAssetDatabase::Get().CheckIfLoadingAssets())
		{
			return;
		}

		TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
			.Title(LOCTEXT("AkAudioGenerateSoundData", "Generate SoundBanks"))
			.ClientSize(FVector2D(600.f, 332.f))
			.SupportsMaximize(false).SupportsMinimize(false)
			.SizingRule(ESizingRule::FixedSize)
			.FocusWhenFirstShown(true);

		TSharedRef<SGenerateSoundBanks> WindowContent = SNew(SGenerateSoundBanks);
		if (!WindowContent->ShouldDisplayWindow())
		{
			return;
		}

		// Add our SGenerateSoundBanks to the window
		WidgetWindow->SetContent(WindowContent);

		// Set focus to our SGenerateSoundBanks widget, so our keyboard keys work right away
		WidgetWindow->SetWidgetToFocusOnActivate(WindowContent);

		// This creates a windows that blocks the rest of the UI. You can only interact with the "Generate SoundBanks" window.
		// If you choose to use this, comment the rest of the function.
		//GEditor->EditorAddModalWindow(WidgetWindow);

		// This creates a window that still allows you to interact with the rest of the editor. If there is an attempt to delete
		// a UAkAudioBank (from the content browser) while this window is opened, the editor will generate a (cryptic) error message
		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		if (ParentWindow.IsValid())
		{
			// Parent the window to the main frame 
			FSlateApplication::Get().AddModalWindow(WidgetWindow, ParentWindow.ToSharedRef());
		}
		else
		{
			// Spawn new window
			FSlateApplication::Get().AddWindow(WidgetWindow);
		}
	}
}

#undef LOCTEXT_NAMESPACE
