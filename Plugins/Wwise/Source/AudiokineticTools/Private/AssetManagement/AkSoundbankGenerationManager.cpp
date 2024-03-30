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

#include "AkSoundBankGenerationManager.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioDevice.h"
#include "AkAudioStyle.h"
#include "AkWaapiClient.h"
#include "AkWaapiUtils.h"
#include "IAudiokineticTools.h"

#include "Async/Async.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "AkAudioModule.h"
#include "Misc/ScopeExit.h"
#include "Internationalization/Text.h"
#include "Wwise/WwiseProjectDatabase.h"

#define LOCTEXT_NAMESPACE "AkAudio"

DECLARE_CYCLE_STAT(TEXT("AkSoundData - Waapi Call"), STAT_WaapiCall, STATGROUP_AkSoundBankGenerationSource);


AkSoundBankGenerationManager::AkSoundBankGenerationManager(const FInitParameters& InitParameters)
	: InitParameters(InitParameters)
{
	PlatformFile = &FPlatformFileManager::Get().GetPlatformFile();
}

AkSoundBankGenerationManager::~AkSoundBankGenerationManager()
{

}

void AkSoundBankGenerationManager::SetIsBuilding(bool bIsBuilding)
{
	bIsBuildingData = bIsBuilding;
}

void AkSoundBankGenerationManager::Init()
{

}

void AkSoundBankGenerationManager::DoGeneration()
{
	if (bIsBuildingData)
	{
		Notify(TEXT("SoundBankGenerationAborted"),
			TEXT("Wwise SoundBank generation aborted, there is already a generation task in progress!"),
			TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"),
			true);
		return;
	}

	SetIsBuilding(true);
	StartTime = FPlatformTime::Cycles64();

	if (InitParameters.GenerationMode != ESoundBankGenerationMode::Commandlet)
	{
		CreateNotificationItem();
	}

	bool bGenerationSuccess = false;
	switch (InitParameters.GenerationMode)
	{
#if AK_SUPPORT_WAAPI
	case ESoundBankGenerationMode::WAAPI:
		bGenerationSuccess = WAAPIGenerate();
		break;
#endif
	case ESoundBankGenerationMode::WwiseConsole:
	case ESoundBankGenerationMode::Commandlet:
	default:
		bGenerationSuccess = WwiseConsoleGenerate();
		break;

	}
	WrapUpGeneration(bGenerationSuccess, TEXT("WwiseConsole"));
}

void AkSoundBankGenerationManager::WrapUpGeneration(const bool bSuccess, const FString& BuilderName)
{
	SetIsBuilding(false);

	FString SuccessMessage;

	if(!bSuccess)
	{
		SuccessMessage = BuilderName + TEXT(" Sound Data Builder task failed");
		NotifyGenerationFailed();
	}
	else
	{
		SuccessMessage = BuilderName + TEXT(" Sound Data Builder task was successful");
		NotifyGenerationSucceeded();
	}

	auto EndTime = FPlatformTime::Cycles64();
	UE_LOG(LogAudiokineticTools, Display, TEXT("%s and took %f seconds."), *SuccessMessage,
		FPlatformTime::ToSeconds64(EndTime - StartTime));


	FWwiseProjectDatabase* ProjectDatabase = FWwiseProjectDatabase::Get();
	if(UNLIKELY(!ProjectDatabase))
	{
		return;
	}
	FAkAudioModule::AkAudioModuleInstance->UpdateWwiseResourceLoaderSettings();
	ProjectDatabase->UpdateDataStructure();
}

void AkSoundBankGenerationManager::CreateNotificationItem()
{
	if (InitParameters.GenerationMode != ESoundBankGenerationMode::Commandlet)
	{
		AsyncTask(ENamedThreads::Type::GameThread, [sharedThis = SharedThis(this)]
			{
				FNotificationInfo Info(LOCTEXT("GeneratingSoundBanks", "Generating Wwise SoundBanks..."));

				Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkBrowserTabIcon"));
				Info.bFireAndForget = false;
				Info.FadeOutDuration = 0.0f;
				Info.ExpireDuration = 0.0f;
	#if UE_4_26_OR_LATER
				Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { FGlobalTabmanager::Get()->TryInvokeTab(FName("OutputLog")); });
	#else
				Info.Hyperlink = FSimpleDelegate::CreateLambda([]() { FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog")); });
	#endif
				Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
				sharedThis->NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
			});
	}
}

void AkSoundBankGenerationManager::Notify(const FString& key, const FString& message, const FString& AudioCuePath, bool bSuccess)
{
	if (InitParameters.GenerationMode != ESoundBankGenerationMode::Commandlet)
	{
		AsyncTask(ENamedThreads::Type::GameThread, [sharedThis = SharedThis(this), key, message, AudioCuePath, bSuccess]
			{
				if (sharedThis->NotificationItem)
				{
					const FTextId TextId(TEXT(LOCTEXT_NAMESPACE), key);
					FText LocText = FText::ChangeKey(TextId.GetNamespace(), TextId.GetKey(), FText::FromString(message));
					sharedThis->NotificationItem->SetText(LocText);

					sharedThis->NotificationItem->SetCompletionState(
						bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
					sharedThis->NotificationItem->SetExpireDuration(3.5f);
					sharedThis->NotificationItem->SetFadeOutDuration(0.5f);
					sharedThis->NotificationItem->ExpireAndFadeout();
				}

				GEditor->PlayEditorSound(AudioCuePath);
			});
	}
}

void AkSoundBankGenerationManager::NotifyGenerationSucceeded()
{
	Notify(TEXT("SoundBankGenerationCompleted"),
		TEXT("Wwise SoundBank generation completed!"),
		TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"),
		true);
}

void AkSoundBankGenerationManager::NotifyGenerationFailed()
{
	Notify(TEXT("SoundBankGenerationFailed"),
		TEXT("Generating Wwise SoundBanks failed!"), 
		TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"),
		false);
}

void AkSoundBankGenerationManager::NotifyProfilingInProgress()
{
	Notify(TEXT("SoundBankGenerationProfiling"),
		TEXT("Cannot generate SoundBanks while Authoring is profiling."), 
		TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"),
		false);
}

void AkSoundBankGenerationManager::NotifyAuthoringUnavailable() 
{
	Notify(
		TEXT("SoundBankGenerationAuthoringLocked"), 
		TEXT("Cannot generate SoundBanks while Authoring is in its current state."),
		TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"),
		false);
}

bool AkSoundBankGenerationManager::WwiseConsoleGenerate()
{
	FString WwiseConsoleCommand = OverrideWwiseConsolePath.IsEmpty() ?  AkAudioBankGenerationHelper::GetWwiseConsoleApplicationPath() : OverrideWwiseConsolePath;

	FString WwiseConsoleArguments;
#if PLATFORM_MAC
	WwiseConsoleArguments = WwiseConsoleCommand + TEXT(" ");
	WwiseConsoleCommand = TEXT("/bin/sh");
#endif
	WwiseConsoleArguments += FString::Printf(TEXT("generate-soundbank \"%s\" --use-stable-guid "),
		*PlatformFile->ConvertToAbsolutePathForExternalAppForWrite(*WwiseUnrealHelper::GetWwiseProjectPath()));

	auto RootOutputPath = WwiseUnrealHelper::GetSoundBankDirectory();

	if (!RootOutputPath.IsEmpty())
	{
		// We can't pass a trailing / to WwiseConsole paths
		RootOutputPath.RemoveFromEnd(TEXT("/"));
		WwiseConsoleArguments += FString::Printf(TEXT(" --root-output-path \"%s\""), *RootOutputPath);
	}

	if (InitParameters.Platforms.Num() > 0)
	{
		WwiseConsoleArguments += FString::Printf(TEXT(" --platform"));
		for (auto& Platform : InitParameters.Platforms)
		{
			WwiseConsoleArguments += FString::Printf(TEXT(" \"%s\""), *Platform);
		}
	}

	if (InitParameters.SkipLanguages)
	{
		WwiseConsoleArguments += TEXT(" --skip-languages");
	}
	else
	{
		if (InitParameters.Languages.Num() > 0)
		{
			WwiseConsoleArguments += FString::Printf(TEXT(" --language"));
			for (auto& Language : InitParameters.Languages)
			{
				WwiseConsoleArguments += FString::Printf(TEXT(" \"%s\""), *Language);
			}
		}
	}

	return RunWwiseConsole(WwiseConsoleCommand, WwiseConsoleArguments);
}

bool AkSoundBankGenerationManager::RunWwiseConsole(const FString& WwiseConsoleCommand,const  FString& WwiseConsoleArguments)
{
	UE_LOG(LogAudiokineticTools, Display, TEXT("Running WwiseConsole command : %s %s"), *WwiseConsoleCommand, *WwiseConsoleArguments);
	bool bGenerationSuccess = true;

	// Create a pipe for the child process's STDOUT.
	int32 ReturnCode = 0;
	void* WritePipe = nullptr;
	void* ReadPipe = nullptr;
	FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

	ON_SCOPE_EXIT{
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
	};

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*WwiseConsoleCommand, *WwiseConsoleArguments, true, true, true, nullptr, 0, nullptr, WritePipe);
	if (ProcHandle.IsValid())
	{
		FString NewLine;
		FPlatformProcess::Sleep(0.1f);
		// Wait for it to finish and get return code
		while (FPlatformProcess::IsProcRunning(ProcHandle))
		{
			NewLine = FPlatformProcess::ReadPipe(ReadPipe);
			if (NewLine.Len() > 0)
			{
				UE_LOG(LogAudiokineticTools, Display, TEXT("%s"), *NewLine);
				NewLine.Empty();
			}
			FPlatformProcess::Sleep(0.25f);
		}

		NewLine = FPlatformProcess::ReadPipe(ReadPipe);
		if (NewLine.Len() > 0)
		{
			UE_LOG(LogAudiokineticTools, Display, TEXT("%s"), *NewLine);
		}

		FPlatformProcess::GetProcReturnCode(ProcHandle, &ReturnCode);

		switch (ReturnCode)
		{
		case 0:
			UE_LOG(LogAudiokineticTools, Display, TEXT("WwiseConsole successfully completed."));
			break;
		case 2:
			UE_LOG(LogAudiokineticTools, Warning, TEXT("WwiseConsole completed with warnings :\n %s %s"), *WwiseConsoleCommand, *WwiseConsoleArguments);
			break;
		default:
			UE_LOG(LogAudiokineticTools, Error, TEXT("WwiseConsole failed with error %d :\n %s %s"), ReturnCode, *WwiseConsoleCommand, *WwiseConsoleArguments);
			bGenerationSuccess = false;
			break;
		}
	}
	else
	{
		bGenerationSuccess = false;
		ReturnCode = -1;
		// Most chances are the path to the .exe or the project were not set properly in GEditorIni file.
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to run WwiseConsole:\n %s %s"), *WwiseConsoleCommand, *WwiseConsoleArguments);
	}

	return bGenerationSuccess;
}

#if AK_SUPPORT_WAAPI
bool AkSoundBankGenerationManager::WAAPIGenerate()
{
	if (!SubscribeToGenerationDone())
	{
		return false;
	}
	ON_SCOPE_EXIT
	{
		CleanupWaapiSubscriptions();
	};

	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
	TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> result;

	if (FAkWaapiClient::Get()->Call(ak::wwise::core::remote::getConnectionStatus, args, options, result, -1))
	{
		bool isConnected = false;
		if (result->TryGetBoolField(WwiseWaapiHelper::IS_CONNECTED, isConnected) && isConnected)
		{
			NotifyAuthoringUnavailable();
			return false;
		}
	}

	TArray<TSharedPtr<FJsonValue>> platformJsonArray;
	for (auto& platform : InitParameters.Platforms)
	{
		platformJsonArray.Add(MakeShared<FJsonValueString>(platform));
	}
	args->SetArrayField(WwiseWaapiHelper::PLATFORMS, platformJsonArray);

	if (InitParameters.Languages.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> LanguageJsonArray;
		for (auto& Language : InitParameters.Languages)
		{
			LanguageJsonArray.Add(MakeShared<FJsonValueString>(Language));
		}
		args->SetArrayField(WwiseWaapiHelper::LANGUAGES, LanguageJsonArray);
	}

	args->SetBoolField(WwiseWaapiHelper::SKIP_LANGUAGES, InitParameters.SkipLanguages);
	args->SetBoolField(WwiseWaapiHelper::WRITE_TO_DISK, true);

	// do we always want to rebuild init bank now?
	args->SetBoolField(WwiseWaapiHelper::REBUILD_INIT_BANK, true);

	bool WaapiCallSuccess = false;
	{
		SCOPE_CYCLE_COUNTER(STAT_WaapiCall);
		WaapiCallSuccess = FAkWaapiClient::Get()->Call(ak::wwise::core::soundbank::generate, args, options, result, -1);


		if (!WaapiCallSuccess)
		{
			auto Message = result->GetStringField(WwiseWaapiHelper::MESSSAGE);

			UE_LOG(LogAkAudio, Error, TEXT("WAAPI Sound Data generation failed: %s"), *Message);
		}
	}

	if (WaapiCallSuccess)
	{
		WaitForGenerationDoneEvent->Wait();
	}

	return WaapiGenerationSuccess;

}

bool AkSoundBankGenerationManager::SubscribeToGenerationDone()
{
	TSharedPtr<FJsonObject> Result;
	TSharedRef<FJsonObject> DoneOptions = MakeShared<FJsonObject>();
	auto SoundBankGenerationDoneCallback = WampEventCallback::CreateRaw(this, &AkSoundBankGenerationManager::OnSoundBankGenerationDone);
	FAkWaapiClient::Get()->Subscribe(ak::wwise::core::soundbank::generationDone, DoneOptions, SoundBankGenerationDoneCallback, GenerationDoneSubscriptionId, Result);
	WaitForGenerationDoneEvent = FGenericPlatformProcess::GetSynchEventFromPool();

	return GenerationDoneSubscriptionId != 0;

}

void AkSoundBankGenerationManager::CleanupWaapiSubscriptions()
{
	TSharedPtr<FJsonObject> result;
	FAkWaapiClient::Get()->Unsubscribe(GenerationDoneSubscriptionId, result);
	FGenericPlatformProcess::ReturnSynchEventToPool(WaitForGenerationDoneEvent);
}

void AkSoundBankGenerationManager::OnSoundBankGenerationDone(uint64_t id, TSharedPtr<FJsonObject> responseJson)
{
	const TArray<TSharedPtr<FJsonValue>>* logs = nullptr;
	if (responseJson->TryGetArrayField(TEXT("logs"), logs))
	{
		for (auto& entry : *logs)
		{
			const TSharedPtr<FJsonObject>* jsonEntry = nullptr;
			if (entry->TryGetObject(jsonEntry))
			{
				const auto severity = jsonEntry->Get()->GetStringField(TEXT("severity"));
				const auto message = jsonEntry->Get()->GetStringField(WwiseWaapiHelper::MESSSAGE);

				FString platform = "";

				const TSharedPtr<FJsonObject>* jsonPlatform = nullptr;
				if (jsonEntry->Get()->TryGetObjectField(TEXT("platform"), jsonPlatform))
				{
					jsonPlatform->Get()->TryGetStringField(WwiseWaapiHelper::NAME, platform);
				}

				if (severity == TEXT("Message"))
				{
					UE_LOG(LogAudiokineticTools, Display, TEXT("%s: %s"), *platform, *message);
				}
				else if (severity == TEXT("Warning"))
				{
					UE_LOG(LogAudiokineticTools, Warning, TEXT("%s: %s"), *platform, *message);
				}
				else if (severity == TEXT("Error") || severity == TEXT("Fatal Error"))
				{
					WaapiGenerationSuccess = false;
					UE_LOG(LogAudiokineticTools, Error, TEXT("%s: %s"), *platform, *message);
				}
			}
		}
	}
	else
	{
		FString outError;
		if (responseJson->TryGetStringField(TEXT("error"), outError))
		{
			WaapiGenerationSuccess = false;

			UE_LOG(LogAudiokineticTools, Error, TEXT("%s"), *outError);
		}
	}

	WaitForGenerationDoneEvent->Trigger();
}
#endif
#undef LOCTEXT_NAMESPACE
