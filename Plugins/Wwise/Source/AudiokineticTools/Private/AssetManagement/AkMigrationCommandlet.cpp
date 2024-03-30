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

#include "AssetManagement/AkMigrationCommandlet.h"

#include "AkAssetMigrationManager.h"
#include "AkAudioBankGenerationHelpers.h"
#include "AkWaapiClient.h"
#include "AkSoundBankGenerationManager.h"
#include "IAudiokineticTools.h"

#define LOCTEXT_NAMESPACE "AkAudio"

static constexpr auto MigrationHelpSwitch = TEXT("help");
static constexpr auto PerformWaapiBankTransfer = TEXT("transfer-banks-waapi");
static constexpr auto WriteBankDefinitionFileSwitch = TEXT("generate-bank-definition");
static constexpr auto DoBankCleanupSwitch = TEXT("delete-banks");
static constexpr auto TransferAutoLoadSwitch = TEXT("transfer-bank-autoload");
static constexpr auto IgnoreBankErrorsSwitch = TEXT("ignore-bank-errors");

static constexpr auto DoAssetMigrationSwitch = TEXT("migrate-assets");
static constexpr auto DoProjectUpdateSwitch = TEXT("update-settings");
static constexpr auto GeneratedSoundBanksPathSwitch = TEXT("generated-soundbanks-folder");

static constexpr auto DoDeprecatedAssetCleanupSwitch = TEXT("delete-deprecated-assets");

UAkMigrationCommandlet::UAkMigrationCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;

	HelpDescription = TEXT("Commandlet for migrating Wwise SoundBanks.");

	HelpParamNames.Add(MigrationHelpSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Print this help message. This will quit the commandlet immediately."));

	HelpParamNames.Add(PerformWaapiBankTransfer);
	HelpParamDescriptions.Add(TEXT("(Optional) Transfer SoundBanks in Unreal project to Wwise using WAAPI. This parameter can't be used with 'generate-bank-definition'."));

	HelpParamNames.Add(WriteBankDefinitionFileSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Create a SoundBank definition file at the specified path. Use an absolute file path (C:/...).\nThe file will then have to be imported in Wwise manually to create the SoundBanks. This parameter can't be used with 'waapi-bank-transfer'."));
	
	HelpParamNames.Add(TransferAutoLoadSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Transfer AkAudioBank's AutoLoad property to assets that were grouped in it."));

	HelpParamNames.Add(DoBankCleanupSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Delete all SoundBank assets in the Unreal project (after performing SoundBank Transfer)."));

	HelpParamNames.Add(IgnoreBankErrorsSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Ignore any errors that occurred during bank transfer through WAAPI or when writing to the SoundBank Definition file and continue migration."));

	HelpParamNames.Add(DoAssetMigrationSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Migrate the Wwise assets in the project."
		"- Will dirty and save all Wwise assets.\n"
		"- The Split Switch Container Media setting will be applied to events individually.\n"
		"- WARNING : After performing this operation, it will no longer be possible to transfer the contents of AkAudioBanks to Wwise as the references will have been cleared.\n"));

	HelpParamNames.Add(DoProjectUpdateSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Update the Wwise and Unreal Project settings."
		"If you were using EBP : \n"
		"  - The \"Enable Auto Defined SoundBanks\" setting will be enabled in the Wwise Project \n"
		"  - The \"Split Media Per Folder\" setting will be migrated to the \"Create Sub-Folders for Generated Files\" setting the Wwise Project \n"
		"  - The Wwise Sound Data Folder will be removed from DirectoriesToAlwaysCook \n"
		"\n"
		"If you are using the Legacy Workflow: \n"
		"  - The Wwise Sound Data Folder will be removed from DirectoriesToAlwaysStageAsUFS\n"
		"  - Generated .bnk and .wem files in {0} will be deleted\n"
		"In all cases: \n"
		"  - The \"Root Output Path\" SoundBank setting in Wwise will be imported to the \"Generated SoundBanks Folder\" integration setting in Unreal Engine.\n"));

	
	HelpParamNames.Add(GeneratedSoundBanksPathSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) The path to the GeneratedSoundBanks folder used with the 'update-settings' switch. Use an absolute file path.\nThis sets the Generated SoundBank Folder setting in the Unreal project."));

	HelpParamNames.Add(DoDeprecatedAssetCleanupSwitch);
	HelpParamDescriptions.Add(TEXT("(Optional) Delete all deprecated Wwise assets that are still in the project"));

	HelpUsage = TEXT("<Editor.exe> <path_to_uproject> -run=AkMigration [ [-waapi-bank-transfer] OR [-generate-bank-definition=<Path/To/Save/FileName>.tsv] ] [-delete-banks] [-migrate-assets] [-update-settings] [-delete-deprecated-assets]");
}

void UAkMigrationCommandlet::PrintHelp() const
{
	UE_LOG(LogAudiokineticTools, Display, TEXT("%s"), *HelpDescription);
	UE_LOG(LogAudiokineticTools, Display, TEXT("Usage: %s"), *HelpUsage);
	UE_LOG(LogAudiokineticTools, Display, TEXT("Parameters:"));
	for (int32 i = 0; i < HelpParamNames.Num(); ++i)
	{
		UE_LOG(LogAudiokineticTools, Display, TEXT("\t- %s:\n %s"), *HelpParamNames[i], *HelpParamDescriptions[i]);
	}
	UE_LOG(LogAudiokineticTools, Display, TEXT("For more information, see %s"), *HelpWebLink);
	//TODO vdcormier: Add link to the online documentation when it exists
}

int32 UAkMigrationCommandlet::Main(const FString& Params)
{
	int32 ReturnCode = 0;
	FString DefinitionPath = "";
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> ParamVals;

	ParseCommandLine(*Params, Tokens, Switches, ParamVals);

	AkAssetMigration::FMigrationContext MigrationOptions;
	AkAssetMigration::FMigrationOperations MigrationOperations;
	AkAssetMigrationManager AssetMigrationManager;
	UE_LOG(LogAudiokineticTools, Display, TEXT("UAkMigrationCommandlet: Beginning project migration"));

	if (Switches.Contains(MigrationHelpSwitch))
	{
		PrintHelp();
		return 0;
	}
	if(!AssetMigrationManager.IsMigrationRequired(MigrationOptions))
	{
		UE_LOG(LogAudiokineticTools, Warning, TEXT("UAkMigrationCommandlet: Project has been already migrated."));
	}

	//Bank transfer options
	const bool bPerformWaapiBankTransfer = Switches.Contains(PerformWaapiBankTransfer);
	const bool bWriteBankDefinitionFile = Switches.Contains(WriteBankDefinitionFileSwitch) || ParamVals.Contains(WriteBankDefinitionFileSwitch);
	const FString* BankDefinitionValue = ParamVals.Find(WriteBankDefinitionFileSwitch);
	const bool bDoBankCleanup = Switches.Contains(DoBankCleanupSwitch);
	const bool bTransferAutoload = Switches.Contains(TransferAutoLoadSwitch);
	const bool bIgnoreBankErrors = Switches.Contains(IgnoreBankErrorsSwitch);

	//Asset migration options
	const bool bDoAssetMigration = Switches.Contains(DoAssetMigrationSwitch);
	const bool bDoDeprecatedAssetCleanup = Switches.Contains(DoDeprecatedAssetCleanupSwitch);

	//Project settings options
	const bool bUpdateProjectSettings = Switches.Contains(DoProjectUpdateSwitch);
	const FString* GeneratedSoundBanksPathValue = ParamVals.Find(GeneratedSoundBanksPathSwitch);

	if(bPerformWaapiBankTransfer && bWriteBankDefinitionFile)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: Can't use both 'waapi-bank-transfer' and 'generate-bank-definition' at the same time."));
		return 1;
	}

	if (bPerformWaapiBankTransfer)
	{
#if AK_SUPPORT_WAAPI
		FAkWaapiClient::Initialize();
		if (UAkSettings* AkSettings = GetMutableDefault<UAkSettings>())
		{
			AkSettings->InitWaapiSync();
		}
		FAkWaapiClient* WaapiClient = FAkWaapiClient::Get();

		if (!WaapiClient)
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("'transfer-banks-waapi' switch is set, but could not get WAAPI Client, cancelling migration."));
			return 1;
		}
		else if (!WaapiClient->IsConnected())
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("'transfer-banks-waapi' switch is set, but could not connect WAAPI Client, cancelling migration."));
			return 1;
		}
#else
		UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: 'waapi-bank-transfer' switch is enabled, but the Unreal project does not support WAAPI. Ensure your current platform supports AkAutobahn."));
		return 1;
#endif
		MigrationOperations.BankTransferMethod = AkAssetMigration::WAAPI;
	}

	if (bWriteBankDefinitionFile)
	{
		MigrationOperations.BankTransferMethod = AkAssetMigration::DefinitionFile;
		if (!BankDefinitionValue)
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: 'generate-bank-definition' file name not provided. Usage: '-generate-bank-definition=<Path/To/Save/FileName>.tsv'."));
			return 1;
		}
		FString Path = *BankDefinitionValue;
		FString Extension = FPaths::GetExtension(Path);
		if (Extension != "tsv")
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: Wrong file extension. The Definition File should be a '.tsv' file. The provided extension was '.%s'."), *Extension);
			return 1;
		}
		MigrationOperations.DefinitionFilePath = Path;
	}

	MigrationOperations.bDoBankCleanup = bDoBankCleanup;
	MigrationOperations.bTransferAutoload = bTransferAutoload;
	MigrationOperations.bDoAssetMigration = bDoAssetMigration;
	MigrationOperations.bDoDeprecatedAssetCleanup = bDoDeprecatedAssetCleanup;
	MigrationOperations.bIgnoreBankTransferErrors = bIgnoreBankErrors;

	MigrationOperations.bDoProjectUpdate = bUpdateProjectSettings;
	if (GeneratedSoundBanksPathValue)
	{
		MigrationOperations.GeneratedSoundBankDirectory = *GeneratedSoundBanksPathValue;
		if (!bUpdateProjectSettings)
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("UAkMigrationCommandlet: 'generated-soundbanks-folder' switch is set, but not 'update-settings'. The Generated SoundBanks Path setting will not be updated."));
		}
	}

	AkAssetMigrationManager::MigrationResult Result =  AssetMigrationManager.PerformMigration(MigrationOperations);

	if (!Result.bSuccess)
	{
		ReturnCode = 1;
	}

	if (!Result.bProjectMigrationSucceeded)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: Failed to migrate all project settings. Inspect the log for more details."));
	}

	if (!Result.bBankTransferSucceeded)
	{
		if (MigrationOperations.BankTransferMethod == AkAssetMigration::WAAPI)
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: Failed to transfer banks using WAAPI. Inspect the log for more details."));
		}
		else if (MigrationOperations.BankTransferMethod == AkAssetMigration::WAAPI)
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: Failed to create SoundBank definition file. Inspect the log for more details."));
		}
	}
	
	if (!Result.bAssetMigrationSucceeded)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: Failed to update all Wwise assets. Inspect the log for more details."));
	}

	if (!Result.bAssetCleanupSucceeded)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("UAkMigrationCommandlet: Failed to delete all deprecated assets. Inspect the log for more details."));
	}
	
	return ReturnCode;
}

#undef LOCTEXT_NAMESPACE
