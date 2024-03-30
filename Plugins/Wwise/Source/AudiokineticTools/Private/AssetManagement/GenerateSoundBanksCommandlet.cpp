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

#include "AssetManagement/GenerateSoundBanksCommandlet.h"

#if WITH_EDITOR
#include "AkAudioBankGenerationHelpers.h"
#include "AkSoundBankGenerationManager.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "Containers/Ticker.h"
#include "Framework/Application/SlateApplication.h"
#include "IAudiokineticTools.h"

#include "ISourceControlModule.h"
#include "SourceControlHelpers.h"
#include "AssetManagement/WwiseProjectInfo.h"

#define LOCTEXT_NAMESPACE "AkAudio"
#endif

static constexpr auto HelpSwitch = TEXT("help");
static constexpr auto LanguagesSwitch = TEXT("languages");
static constexpr auto PlatformsSwitch = TEXT("platforms");
static constexpr auto WwiseConsolePathSwitch = TEXT("wwiseConsolePath");

UGenerateSoundBanksCommandlet::UGenerateSoundBanksCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;

	HelpDescription = TEXT("Commandlet allowing to generate Wwise SoundBanks.");

	HelpParamNames.Add(PlatformsSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Comma separated list of platforms for which SoundBanks will be generated, as specified in the Wwise project. If not specified, SoundBanks will be generated for all platforms."));

	HelpParamNames.Add(LanguagesSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Comma separated list of languages for which SoundBanks will be generated, as specified in the Wwise project. If not specified, SoundBanks will be generated for all platforms."));

	HelpParamNames.Add(WwiseConsolePathSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Full path to the Wwise command-line application to use to generate the SoundBanks. If not specified, the path found in the Wwise settings will be used."));

	HelpParamNames.Add(HelpSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Print this help message. This will quit the commandlet immediately."));

	HelpUsage = TEXT("<Editor.exe> <path_to_uproject> -run=GenerateSoundBanks [-platforms=listOfPlatforms] [-languages=listOfLanguages] [-rebuild]");
	HelpWebLink = TEXT("https://www.audiokinetic.com/library/edge/?source=UE4&id=using_features_generatecommandlet.html");
}

void UGenerateSoundBanksCommandlet::PrintHelp() const
{
	UE_LOG(LogAudiokineticTools, Display, TEXT("%s"), *HelpDescription);
	UE_LOG(LogAudiokineticTools, Display, TEXT("Usage: %s"), *HelpUsage);
	UE_LOG(LogAudiokineticTools, Display, TEXT("Parameters:"));
	for (int32 i = 0; i < HelpParamNames.Num(); ++i)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("\t- %s: %s"), *HelpParamNames[i], *HelpParamDescriptions[i]);
	}
	UE_LOG(LogAudiokineticTools, Display, TEXT("For more information, see %s"), *HelpWebLink);
}

int32 UGenerateSoundBanksCommandlet::Main(const FString& Params)
{
	int32 ReturnCode = 0;
#if WITH_EDITOR
	AkSoundBankGenerationManager::FInitParameters InitParameters;

	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> ParamVals;

	ParseCommandLine(*Params, Tokens, Switches, ParamVals);

	if (Switches.Contains(HelpSwitch))
	{
		PrintHelp();
		return 0;
	}

	WwiseProjectInfo wwiseProjectInfo;
	wwiseProjectInfo.Parse();
	const FString* PlatformValue = ParamVals.Find(PlatformsSwitch);
	if (PlatformValue)
	{
		TArray<FString> PlatformNames;
		PlatformValue->ParseIntoArray(PlatformNames, TEXT(","));
		InitParameters.Platforms.Append(PlatformNames);
	}

	const FString* LanguageValue = ParamVals.Find(LanguagesSwitch);
	if (LanguageValue)
	{
		TArray<FString> LanguageNames;
		LanguageValue->ParseIntoArray(LanguageNames, TEXT(","));
		InitParameters.Languages.Append(LanguageNames);
	}

	InitParameters.GenerationMode = AkSoundBankGenerationManager::ESoundBankGenerationMode::Commandlet;

	auto AkSettings = GetMutableDefault<UAkSettings>();
	auto AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();

	auto SoundBankGenerationManager = MakeShared<AkSoundBankGenerationManager, ESPMode::ThreadSafe>(InitParameters);

	if (const FString* overrideWwiseConsolePath = ParamVals.Find(WwiseConsolePathSwitch))
	{
		SoundBankGenerationManager->SetOverrideWwiseConsolePath(*overrideWwiseConsolePath);
	}

	SoundBankGenerationManager->Init();

	SoundBankGenerationManager->DoGeneration();
#endif

	return ReturnCode;
}

#undef LOCTEXT_NAMESPACE
