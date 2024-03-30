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

#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Misc/Guid.h"
#include "AssetRegistry/AssetData.h"

struct PropertyToChange;

namespace AkAssetMigration
{
	enum EBankTransferMode
	{
		NoTransfer,
		WAAPI,
		DefinitionFile,
	};

	struct FMigrationOperations
	{
		bool bDoDeprecatedAssetCleanup = false;
		bool bDoAssetMigration= false;
		bool bDoBankCleanup= false;
		bool bTransferAutoload = false;
		bool bDoProjectUpdate= false;
		bool bCancelled = false;
		bool bIgnoreBankTransferErrors = false;
		FString GeneratedSoundBankDirectory = "";
		FString DefinitionFilePath = "";
		EBankTransferMode BankTransferMethod  = EBankTransferMode::NoTransfer;
	};

	struct FMigrationContext
	{
		bool bBanksInProject = false;
		bool bDeprecatedAssetsInProject = false;
		bool bAssetsNeedMigration = false;
		bool bProjectSettingsNotUpToDate = false;
		int NumDeprecatedAssetsInProject = 0;
	};

	struct FLinkedAssetEntry
	{
		FString AssetName;
		FGuid WwiseGuid;
		FString AssetPath;
		UPackage* Package;
	};

	struct FBankEntry
	{
		FAssetData BankAssetData;
		TArray<FLinkedAssetEntry> LinkedEvents;
		TArray<FLinkedAssetEntry> LinkedAuxBusses;
	};

	struct FBankTransferError
	{
		FString ErrorMessage;
		bool bHasBankEntry;
		FBankEntry BankEntry;
	};

	enum EDefinitionFileCreationResult
	{
		Success,
		OpenFailure,
		WriteFailure
	};

	void PromptMigration(const FMigrationContext& MigrationOptions, FMigrationOperations& OutMigrationOperations);
	bool MigrateAudioBanks(const FMigrationOperations& TransferMode, const bool bCleanupAssets, const bool bWasUsingEBP, const bool bTransferAutoLoad, const FString DefinitionFilePath);
	bool MigrateWwiseAssets(const TArray<FAssetData> & WwiseAssets, bool bShouldSplitSwitchContainerMedia);
	void FindWwiseAssetsInProject(TArray<FAssetData>& OutFoundAssets);

	bool PromptFailedBankTransfer(const TArray<FBankTransferError>& ErrorMessages);
	FString FormatWaapiErrorMessage(const TArray<FBankTransferError>& ErrorMessages);

	void FindDeprecatedAssets(TArray<FAssetData>& OutDeprecatedAssets);
	bool DeleteDeprecatedAssets(const TArray<FAssetData>& InAssetsToDelete);

	void FillBanksToTransfer(TMap<FString, FBankEntry>& BanksToTransfer);
	TArray<FBankTransferError> TransferUserBanksWaapi(const TMap<FString, FBankEntry>& BanksToTransfer, TSet<FAssetData>& FailedBanks, const bool bIncludeMedia);
	bool CreateBankWaapi(const FString& BankName, const FBankEntry& BankEntry, FGuid& OutBankGuid, TArray<FBankTransferError>& ErrorMessages);
	bool SetBankIncludesWaapi(const FBankEntry& BankEntry, const FGuid& BankId, const bool bIncludeMedia, TArray<FBankTransferError>& ErrorMessages);
	bool SaveProjectWaapi(TArray<FBankTransferError>& ErrorMessages);
	EDefinitionFileCreationResult WriteBankDefinitionFile(const TMap<FString, FBankEntry>& InBanksToTransfer, const bool bIncludeMedia, const FString DefinitionFilePath);
	bool WriteBankDefinition(const FBankEntry& BankEntry, TUniquePtr<IFileHandle>& FileWriter,  const bool bIncludeMedia);

	bool MigrateProjectSettings(FString& ProjectContent, const bool bWasUsingEBP,  const bool bUseGeneratedSubFolders, const FString& GeneratedSoundBanksFolder);
	bool SetStandardSettings(FString& ProjectContent);
	
	struct PropertyToChange
	{
		FString Name;
		FString Value;
		FString Xml;

		PropertyToChange(FString n, FString v, FString x)
			: Name(n)
			, Value(v)
			, Xml(x)
		{}
	};

	bool InsertProperties(const TArray<PropertyToChange>& PropertiesToChange, FString& ProjectContent);
};