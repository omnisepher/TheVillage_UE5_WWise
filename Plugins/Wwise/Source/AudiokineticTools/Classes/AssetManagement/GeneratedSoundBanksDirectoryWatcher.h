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
#include "IDirectoryWatcher.h"
#include "Widgets/Notifications/SNotificationList.h"

DECLARE_MULTICAST_DELEGATE(OnSoundBankGenerationDoneDelegate);

class AUDIOKINETICTOOLS_API GeneratedSoundBanksDirectoryWatcher
{
public:
	bool DoesWwiseProjectExist();
	void CheckIfCachePathChanged();
	void Initialize();
	void Uninitialize(const bool bIsModuleShutdown = false);
	void StartWatchers();

	//Watch the SoundBankInfoCache.dat file. Changes to this file indicate that sound data generation is done
	bool StartCacheWatcher(const FString& CachePath);

	//Watch for changes to GeneratedSoundBanks folder, triggering a countdown timer to parse once no more changes are detected after a certain delay
	void StartSoundBanksWatcher(const FString& GeneratedSoundBanksFolder);

	void StopWatchers();
	void StopSoundBanksWatcher();
	void StopCacheWatcher();
	void RestartWatchers();
	void ConditionalRestartWatchers();
	bool ShouldRestartWatchers();

	OnSoundBankGenerationDoneDelegate OnSoundBanksGenerated;

private:
	FString CachePath;
	FString SoundBankDirectory;
	void OnCacheChanged(const TArray<FFileChangeData>& ChangedFiles);
	void OnGeneratedSoundBanksChanged(const TArray<FFileChangeData>& ChangedFiles);

	void TimerTick(float DeltaSeconds);
	void EndParseTimer();
	void OnSoundBankGenerationDone() const;
	void NotifyFilesChanged();
	void HideNotification();
	void UpdateNotificationOnGenerationComplete() const;
	void UpdateNotification() const;

	FDelegateHandle GeneratedSoundBanksHandle;
	FDelegateHandle CacheChangedHandle;
	FDelegateHandle PostEditorTickHandle;
	FDelegateHandle ProjectParsedHandle;

	FDelegateHandle SettingsChangedHandle;
	FDelegateHandle UserSettingsChangedHandle;

	bool bParseTimerRunning = false;
	float ParseTimer= 0;
	const float ParseDelaySeconds = 10.0f;

	TSharedPtr<SNotificationItem> NotificationItem;

	bool bCacheFolderExists = false;
	bool bGeneratedSoundBanksFolderExists = false;

};

