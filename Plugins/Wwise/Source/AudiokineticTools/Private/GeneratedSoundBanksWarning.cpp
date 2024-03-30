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
	SGeneratedSoundBanks.cpp
------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------
 includes.
------------------------------------------------------------------------------------*/
#include "GeneratedSoundBanksWarning.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "WwiseUnrealDefines.h"
#include "DesktopPlatformModule.h"
#include "IAudiokineticTools.h"
#include "IDesktopPlatform.h"
#include "WwiseUnrealHelper.h"
#include "Async/Async.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FGeneratedSoundBanksWarning::FGeneratedSoundBanksWarning()
{
}

void FGeneratedSoundBanksWarning::DisplayGeneratedSoundBanksWarning()
{
	if (!FApp::CanEverRender())
	{
		FString SoundBankDirectory = WwiseUnrealHelper::GetSoundBankDirectory();
		UE_LOG(LogAudiokineticTools, Warning, TEXT("Couldn't find GeneratedSoundBanks info at path: \n%s"), *SoundBankDirectory);
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		FString SoundBankDirectory = WwiseUnrealHelper::GetSoundBankDirectory();
		FText InfoString = FText::FormatOrdered(LOCTEXT("GeneratedSoundBanksWarning", "Couldn't find GeneratedSoundBanks info at path: \n{0}\nSet the GeneratedSoundBanks folder?"), FText::FromString(SoundBankDirectory));
		FSimpleDelegate SetGeneratedSoundBanksPathDelegate = FSimpleDelegate();
		SetGeneratedSoundBanksPathDelegate.BindRaw(this, &FGeneratedSoundBanksWarning::OpenSettingsMenu);

		FSimpleDelegate HideDelegate = FSimpleDelegate();
		HideDelegate.BindRaw(this, &FGeneratedSoundBanksWarning::HideGeneratedSoundBanksNotification);
		FNotificationButtonInfo SetGeneratedSoundBanksPathButton = FNotificationButtonInfo(LOCTEXT("WwiseOpenGeneratedSoundbanks", "Set GeneratedSoundBanks Path"), FText(), SetGeneratedSoundBanksPathDelegate);
		FNotificationButtonInfo DismissButton = FNotificationButtonInfo(LOCTEXT("WwiseDismiss", "Dismiss"), FText(), HideDelegate);
		FNotificationInfo Info(InfoString);
		Info.WidthOverride = 400;
		Info.ButtonDetails.Add(SetGeneratedSoundBanksPathButton);
		Info.ButtonDetails.Add(DismissButton);
		Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkBrowserTabIcon"));
		Info.bUseSuccessFailIcons = false;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 10.0f;

		GeneratedSoundBanksWarning = FSlateNotificationManager::Get().AddNotification(Info);

		if (GeneratedSoundBanksWarning.IsValid())
		{
			GeneratedSoundBanksWarning->SetCompletionState(SNotificationItem::CS_Pending);
		}
	});
}

void FGeneratedSoundBanksWarning::HideGeneratedSoundBanksNotification()
{
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		if (GeneratedSoundBanksWarning.IsValid())
		{
			GeneratedSoundBanksWarning->Fadeout();
		}
	});
}

void FGeneratedSoundBanksWarning::OpenSettingsMenu()
{
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const FString Title = LOCTEXT("GeneratedSoundBanksFolderSelector", "Select the folder containing the Generated SoundBanks info").ToString();
	FString Output;
	bool bSuccess = FDesktopPlatformModule::Get()->OpenDirectoryDialog(ParentWindowHandle, Title, WwiseUnrealHelper::GetContentDirectory(), Output);
	if(bSuccess)
	{
		UAkSettings* Settings = GetMutableDefault<UAkSettings>();
		if(Settings->UpdateGeneratedSoundBanksPath(Output))
		{
			Settings->SaveConfig();
		}
	}
}
#undef LOCTEXT_NAMESPACE