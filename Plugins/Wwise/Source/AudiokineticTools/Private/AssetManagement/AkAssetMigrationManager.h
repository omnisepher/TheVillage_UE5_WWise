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

#include "AkAssetMigrationHelper.h"
#include "AkSettings.h"
#include "WwiseUnrealDefines.h"
#include "LevelEditor.h"

class AkAssetMigrationManager
{
public:
	struct MigrationResult
	{
		bool bSuccess = true;
		bool bBankTransferSucceeded = true;
		bool bProjectMigrationSucceeded = true;
		bool bAssetMigrationSucceeded = true;
		bool bAssetCleanupSucceeded = true;

		TArray<FString> ErrorReport;
	};
	
	void Init();
	void Uninit();

	void EditorTryMigration();
	MigrationResult PerformMigration(AkAssetMigration::FMigrationOperations MigrationOperations);
	bool IsProjectMigrated();
	bool IsMigrationRequired(AkAssetMigration::FMigrationContext& MigrationOptions);
	void CreateMigrationMenuOption();
	void RemoveMigrationMenuOption();

	static void ClearSoundBanksForMigration();
	static bool MigrateProjectSettings(const bool bWasUsingEBP, const bool bUseGeneratedSubFolders, const FString& GeneratedSoundBanksFolder);
	static bool SetStandardProjectSettings();
	static bool SetGeneratedSoundBanksPath(const FString& ProjectContent, const FString& GeneratedSoundBanksFolder);
	static bool IsSoundDataPathInDirectoriesToAlwaysStage(const FString& SoundDataPath);

private:
#if !UE_5_0_OR_LATER
	FLevelEditorModule::FLevelEditorMenuExtender LevelViewportToolbarBuildMenuExtenderAkMigration;
	FDelegateHandle LevelViewportToolbarBuildMenuExtenderAkMigrationHandle;
#endif 
	FName MigrationMenuSectionName = "AkMigration";
};