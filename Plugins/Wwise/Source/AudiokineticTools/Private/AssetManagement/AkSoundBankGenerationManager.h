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
#include "Containers/UnrealString.h"
#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "HAL/ThreadSafeBool.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "AssetManagement/WwiseProjectInfo.h"
#include "WwiseUnrealHelper.h"

DECLARE_STATS_GROUP(TEXT("AkSoundBankGeneration"), STATGROUP_AkSoundBankGenerationSource, STATCAT_Wwise);


class AkSoundBankGenerationManager : public TSharedFromThis<AkSoundBankGenerationManager, ESPMode::ThreadSafe>
{

public:
	enum ESoundBankGenerationMode
	{
		WwiseConsole = 0,
		Commandlet,
		WAAPI
	};

	struct FInitParameters
	{
		TArray<FString> Platforms;
		TArray<FString> Languages;
		bool SkipLanguages = false;
		ESoundBankGenerationMode GenerationMode = WwiseConsole;
	};


	AkSoundBankGenerationManager(const FInitParameters& InitParameters);
	~AkSoundBankGenerationManager();

	void Init();
	void DoGeneration();

	void SetOverrideWwiseConsolePath(const FString& value) { OverrideWwiseConsolePath = value; }


private:
	void CreateNotificationItem();
	void Notify(const FString& key, const FString& message, const FString& AudioCuePath, bool bSuccess);
	void NotifyGenerationFailed();
	void NotifyGenerationSucceeded();
	void NotifyProfilingInProgress();
	void NotifyAuthoringUnavailable();

	void WrapUpGeneration(const bool bSuccess, const FString& BuilderName);
	void SetIsBuilding(bool bIsBuilding);

	TSharedPtr<SNotificationItem> NotificationItem;
	uint64 StartTime;
	FInitParameters InitParameters;
	IPlatformFile* PlatformFile = nullptr;

	//WwiseConsole generation 
	bool WwiseConsoleGenerate();
	bool RunWwiseConsole(const FString& WwiseConsoleCommand, const FString& WwiseConsoleArguments);
	FString OverrideWwiseConsolePath;

	//WAAPI generation 
#if AK_SUPPORT_WAAPI
	bool WAAPIGenerate();
	bool SubscribeToGenerationDone();
	void CleanupWaapiSubscriptions();
	void OnSoundBankGenerationDone(uint64_t Id, TSharedPtr<FJsonObject> ResponseJson);
#endif

	uint64 GenerationDoneSubscriptionId = 0;
	FDelegateHandle ConnectionLostHandle;
	FEvent* WaitForGenerationDoneEvent = nullptr;
	FThreadSafeBool WaapiGenerationSuccess = true;
	FThreadSafeBool bIsBuildingData = false;

};