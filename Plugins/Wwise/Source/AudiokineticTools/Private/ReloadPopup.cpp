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

#include "ReloadPopup.h"

#include "AkAudioStyle.h"
#include "AkAudioModule.h"
#include "Async/Async.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "AkAudio"

TSharedPtr<SNotificationItem> FReloadPopup::RefreshNotificationItem;

FReloadPopup::FReloadPopup()
{
	
}

void FReloadPopup::NotifyProjectRefresh()
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		FText InfoString = LOCTEXT("ReloadPopupRefreshData", "Wwise project database was updated.\nReload Wwise Asset Data?");
		FSimpleDelegate RefreshDelegate = FSimpleDelegate();
		RefreshDelegate.BindRaw(this, &FReloadPopup::Reload);

		FSimpleDelegate HideDelegate = FSimpleDelegate();
		HideDelegate.BindRaw(this, &FReloadPopup::HideRefreshNotification);
		FNotificationButtonInfo RefreshButton = FNotificationButtonInfo(LOCTEXT("WwiseDataRefresh", "Refresh"), FText(), RefreshDelegate);
		FNotificationButtonInfo HideButton = FNotificationButtonInfo(LOCTEXT("WwiseDataNotNow", "Not Now"), FText(), HideDelegate);
		FNotificationInfo Info(InfoString);
		Info.ButtonDetails.Add(RefreshButton);
		Info.ButtonDetails.Add(HideButton);
		Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkBrowserTabIcon"));
		Info.bUseSuccessFailIcons = false;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 10.0f;

		RefreshNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

		if (RefreshNotificationItem.IsValid())
		{
			RefreshNotificationItem->SetCompletionState(SNotificationItem::CS_Pending);
		}
	});
}

void FReloadPopup::Reload()
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		FAkAudioModule::AkAudioModuleInstance->ReloadWwiseAssetData();
		if (RefreshNotificationItem)
		{
			RefreshNotificationItem->Fadeout();
		}
	});
}

void FReloadPopup::HideRefreshNotification()
{
	if (!FApp::CanEverRender())
	{
		return;
	}
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		if (RefreshNotificationItem)
		{
			RefreshNotificationItem->Fadeout();
		}
	});
}
#undef LOCTEXT_NAMESPACE