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

#include "Wwise/WwiseReconcile.h"
#include "Wwise/Stats/Reconcile.h"
#include "AssetRegistry/AssetData.h"
#include "Misc/App.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "AkAudio"

bool IWwiseReconcile::ReconcileAssets(EWwiseReconcileOperationFlags OperationFlags)
{
	FScopedSlowTask ReconcileTask(GetNumberOfAssets(), LOCTEXT("ReconcileTask", "Reconciling Wwise Assets"));
	if(FApp::CanEverRender())
	{
		ReconcileTask.MakeDialog(true);		
	}
	auto ProjectDatabase = FWwiseProjectDatabase::Get();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not load project database"));
		return false;
	}
	//Locking the Project Database to prevent changes during reconcile.
	FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);

	if(GuidToWwiseRef.Num() == 0)
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not find SoundBanks information. Make sure your SoundBanks are generated."));
		return false;
	}
	
	bool bSucceeded = true;
	int NumberOfOperationsCompleted = 0;

	if (AssetsToDelete.Num() != 0)
	{
		int NumberOfAssetsDeleted = DeleteAssets(ReconcileTask);
		NumberOfOperationsCompleted += NumberOfAssetsDeleted;
		if (NumberOfAssetsDeleted > 0 && !ReconcileTask.ShouldCancel())
		{
			UE_LOG(LogWwiseReconcile, Display, TEXT("Deleted %i assets out of %i."), NumberOfAssetsDeleted, AssetsToDelete.Num());
		}
		else if(!ReconcileTask.ShouldCancel())
		{
			UE_LOG(LogWwiseReconcile, Warning, TEXT("Failed to delete outdated AkAudioType assets"));
			bSucceeded = false;
		}
	}

	if (AssetsToUpdate.Num() != 0)
	{
		int NumberOfAssetsUpdated = UpdateExistingAssets(ReconcileTask).Num();
		NumberOfOperationsCompleted += NumberOfAssetsUpdated;
		if(NumberOfAssetsUpdated > 0 && !ReconcileTask.ShouldCancel())
		{
			UE_LOG(LogWwiseReconcile, Verbose, TEXT("Updated %i assets out of %i."), NumberOfAssetsUpdated, AssetsToUpdate.Num());
		}
		else if(!ReconcileTask.ShouldCancel())
		{
			UE_LOG(LogWwiseReconcile, Warning, TEXT("Failed to consolidate existing AkAudioType assets"));
			bSucceeded = false;
		}
	}

	if (AssetsToRename.Num() != 0)
	{
		if(RenameExistingAssets(ReconcileTask) &&!ReconcileTask.ShouldCancel())
		{
			NumberOfOperationsCompleted += AssetsToRename.Num();
			UE_LOG(LogWwiseReconcile, Verbose, TEXT("Updated %i assets out of %i."), AssetsToRename.Num(), AssetsToRename.Num());
		}
		else if(!ReconcileTask.ShouldCancel())
		{
			UE_LOG(LogWwiseReconcile, Warning, TEXT("Failed to rename existing AkAudioType assets"));
			bSucceeded = false;
		}
	}

	if (AssetsToCreate.Num() != 0)
	{
		int NumberOfAssetsCreated = CreateAssets(ReconcileTask).Num();
		NumberOfOperationsCompleted += NumberOfAssetsCreated;
		if (NumberOfAssetsCreated > 0 && !ReconcileTask.ShouldCancel())
		{
			UE_LOG(LogWwiseReconcile, Display, TEXT("Created %i assets out of %i."), NumberOfAssetsCreated, AssetsToCreate.Num());
		}
		else if(!ReconcileTask.ShouldCancel())
		{
			UE_LOG(LogWwiseReconcile, Warning, TEXT("No New AkAudioType assets created"));
			bSucceeded = false;
		}
	}

	if(ReconcileTask.ShouldCancel())
	{
		UE_LOG(LogWwiseReconcile, Log, TEXT("Reconcile Operation was cancelled by user."));
	}
	else
	{
		UE_LOG(LogWwiseReconcile, Verbose, TEXT("Successfully did %i operations out of %i."), NumberOfOperationsCompleted, GetNumberOfAssets());
	}

	OnWwiseAssetsReconciled.Broadcast();
	return bSucceeded;
}
#undef LOCTEXT