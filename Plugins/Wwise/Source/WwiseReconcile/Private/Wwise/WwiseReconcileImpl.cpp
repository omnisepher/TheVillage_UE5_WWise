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

#include "Wwise/WwiseReconcileImpl.h"
#include "AkUnrealAssetDataHelper.h"
#include "Wwise/Stats/Reconcile.h"
#include "AkAcousticTexture.h"
#include "AkAudioEvent.h"
#include "AkAudioType.h"
#include "AkAuxBus.h"
#include "AkEffectShareSet.h"
#include "AkInitBank.h"
#include "AkRtpc.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkUnrealAssetDataHelper.h"
#include "WwiseUnrealHelper.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "Misc/App.h"
#include "Misc/EngineBuildSettings.h"
#include "Misc/ScopedSlowTask.h"

void FWwiseReconcileImpl::GetAllWwiseRefs()
{
	GuidToWwiseRef.Empty();
	auto ProjectDatabase = FWwiseProjectDatabase::Get();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not load project database"));
		return;
	}
	FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);

	// Check to make sure there are no issues getting data for the CurrentPlatform
	if (DataStructure.GetSoundBanks().Num() == 0)
	{
		FName PlatformName = DataStructure.GetCurrentPlatform().GetPlatformName();
		UE_LOG(LogWwiseReconcile, Error, TEXT("No data loaded from Wwise project database for the curent platform %s"), *PlatformName.ToString());
		return;
	}
	if (DataStructure.GetCurrentPlatformData()->Guids.Num() == 0)
	{
		FName PlatformName = DataStructure.GetCurrentPlatform().GetPlatformName();
		UE_LOG(LogWwiseReconcile, Error, TEXT("No data loaded from Wwise project database for the curent platform %s"), *PlatformName.ToString());
		return;
	}

	for (const auto& WwiseRef : DataStructure.GetCurrentPlatformData()->Guids)
	{
		if (WwiseRef.Value.GetType() != EWwiseRefType::SoundBank)
		{
			GuidToWwiseRef.Add(WwiseRef.Key.Guid, { &WwiseRef.Value });
		}
	}
}

bool FWwiseReconcileImpl::IsAssetOutOfDate(const FAssetData& AssetData, const FWwiseAnyRef& WwiseRef)
{
	auto StringGuid = AssetData.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseGuid));
	auto StringShortId = AssetData.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseShortId));
	auto WwiseName = AssetData.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseName));
	uint32 ShortId = 0;
	if(StringShortId.IsSet())
	{
		ShortId = uint32(FCString::Atoi(*StringShortId.GetValue()));
	}
	if (StringGuid.IsSet())
	{
		const auto AssetGuid = WwiseRef.GetGuid();
		const auto AssetId = WwiseRef.GetId(); 
		return AssetGuid != FGuid(StringGuid.AsString()) ||
			AssetId != ShortId ||
			WwiseName != WwiseRef.GetName().ToString();
	}
	return false;
}

UClass* FWwiseReconcileImpl::GetUClassFromWwiseRefType(EWwiseRefType RefType)
{
	switch (RefType)
	{
	case EWwiseRefType::Event:
		return UAkAudioEvent::StaticClass();
	case EWwiseRefType::AuxBus:
		return UAkAuxBus::StaticClass();
	case EWwiseRefType::AcousticTexture:
		return UAkAcousticTexture::StaticClass();
	case EWwiseRefType::State:
		return UAkStateValue::StaticClass();
	case EWwiseRefType::Switch:
		return UAkSwitchValue::StaticClass();
	case EWwiseRefType::GameParameter:
		return UAkRtpc::StaticClass();
	case EWwiseRefType::Trigger:
		return UAkTrigger::StaticClass();
	case EWwiseRefType::PluginShareSet:
		return UAkEffectShareSet::StaticClass();
	case EWwiseRefType::None:
		return nullptr;
	default:
		return nullptr;
	}
}

void FWwiseReconcileImpl::GetAllAssets(TArray<FWwiseReconcileItem>& ReconcileItems)
{
	GetAllWwiseRefs();
	auto AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	if (!AssetToolsModule)
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not load the AssetTools Module"));
		return;
	}

	auto AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	if (!AssetRegistryModule)
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not load the AssetRegistry Module"));
		return;
	}

	TArray<FAssetData> Assets;

#if UE_5_1_OR_LATER
	AssetRegistryModule->Get().GetAssetsByClass(UAkAudioType::StaticClass()->GetClassPathName(), Assets, true);
#else
	AssetRegistryModule->Get().GetAssetsByClass(UAkAudioType::StaticClass()->GetFName(), Assets, true);
#endif

	ReconcileItems.Empty();
	for (const FAssetData& AssetData : Assets)
	{
		// Exclude the Init bank
		if (AkUnrealAssetDataHelper::AssetOfType<UAkInitBank>(AssetData))
		{
			continue;
		}
		auto Value = AssetData.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseGuid));
		FWwiseReconcileItem Item;
		Item.Asset = AssetData;
		if (Value.IsSet())
		{
			Item.ItemId = FGuid(Value.GetValue());
			if (auto WwiseRef = GuidToWwiseRef.Find(Item.ItemId))
			{
				WwiseRef->bAssetExists = true;
				Item.WwiseAnyRef = { *WwiseRef };
			}
		}
		ReconcileItems.Add(Item);
	}
	for (auto WwiseRef : GuidToWwiseRef)
	{
		if (!WwiseRef.Value.bAssetExists)
		{
			FWwiseReconcileItem Item;
			Item.WwiseAnyRef = { WwiseRef.Value };
			Item.ItemId = WwiseRef.Key;
			ReconcileItems.Add(Item);
		}
	}
}

void FWwiseReconcileImpl::GetAssetChanges(TArray<FWwiseReconcileItem>& ReconcileItems,
	EWwiseReconcileOperationFlags OperationFlags)
{
	AssetsToCreate.Empty();
	AssetsToDelete.Empty();
	AssetsToRename.Empty();
	AssetsToUpdate.Empty();

	for (int i = 0; i < ReconcileItems.Num();)
	{
		auto& ReconcileItem = ReconcileItems[i];
		if(!ReconcileItem.WwiseAnyRef.WwiseAnyRef || !ReconcileItem.WwiseAnyRef.WwiseAnyRef->IsValid())
		{
			if (EnumHasAnyFlags(OperationFlags, EWwiseReconcileOperationFlags::Delete))
			{
				if (ReconcileItem.Asset.IsValid())
				{
					ReconcileItem.OperationRequired |= EWwiseReconcileOperationFlags::Delete;
					AssetsToDelete.Add(ReconcileItem.Asset);
				}
				else
				{
					ReconcileItems.RemoveAt(i);
					continue;
				}
			}
			i++;
			continue;
		}
		const FWwiseAnyRef* WwiseRefValue = ReconcileItem.WwiseAnyRef.WwiseAnyRef;
		EWwiseRefType RefType = WwiseRefValue->GetType();
		UClass* RefClass = GetUClassFromWwiseRefType(RefType);
		FAssetData Asset = ReconcileItem.Asset;

		if (!RefClass)
		{
			i++;
			continue;
		}

		if (!Asset.IsValid())
		{
			if (EnumHasAnyFlags(OperationFlags, EWwiseReconcileOperationFlags::Create))
			{
				ReconcileItem.OperationRequired |= EWwiseReconcileOperationFlags::Create;
				AssetsToCreate.Add(ReconcileItem.WwiseAnyRef);
			}
		}

		else if (EnumHasAnyFlags(OperationFlags, EWwiseReconcileOperationFlags::UpdateExisting))
		{
			if (RefClass && Asset.IsValid())
			{
				FName AssetName = AkUnrealAssetDataHelper::GetAssetDefaultName(WwiseRefValue);

				if(IsAssetOutOfDate(Asset, *WwiseRefValue))
				{
					ReconcileItem.OperationRequired |= EWwiseReconcileOperationFlags::UpdateExisting;
					AssetsToUpdate.Add(ReconcileItem.Asset);
				}

				if (Asset.AssetName != AssetName)
				{
					ReconcileItem.OperationRequired |= EWwiseReconcileOperationFlags::RenameExisting;
					AssetsToRename.Add(ReconcileItem.Asset);
				}
			}
		}
		if(ReconcileItem.OperationRequired == EWwiseReconcileOperationFlags::None)
		{
			ReconcileItems.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}
}

TArray<FAssetData> FWwiseReconcileImpl::UpdateExistingAssets(FScopedSlowTask& SlowTask)
{
	check(IsInGameThread());

	TArray<UPackage*> PackagesToSave;
	TArray<FAssetData> UpdatedAssets;

	for (const auto& AssetData : AssetsToUpdate)
	{
		if(SlowTask.ShouldCancel())
		{
			return UpdatedAssets;
		}
		auto AkAudioAsset = Cast<UAkAudioType>(AssetData.GetAsset());
		if (!LIKELY(AkAudioAsset))
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Failed to update Wwise asset %s."), *AssetData.AssetName.ToString());
			continue;
		}
		UE_LOG(LogWwiseReconcile, Verbose, TEXT("Updating Wwise asset %s."), *AssetData.AssetName.ToString());
		AkAudioAsset->FillInfo();
		FAssetData NewAssetData = FAssetData(AkAudioAsset);
		PackagesToSave.Add(NewAssetData.GetPackage());
		UpdatedAssets.Add(NewAssetData);
		AkAudioAsset->MarkPackageDirty();
		SlowTask.EnterProgressFrame();
	}

	if (!UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false))
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Failed to save updated Wwise assets."));
		return {};
	}

	return UpdatedAssets;
}

void FWwiseReconcileImpl::ConvertWwiseItemTypeToReconcileItem(const TArray<TSharedPtr<FWwiseTreeItem>>& InWwiseItems,
	TArray<FWwiseReconcileItem>& OutReconcileItems, EWwiseReconcileOperationFlags OperationFlags, bool bFirstLevel)
{
	if(bFirstLevel)
	{
		GetAllWwiseRefs();
	}
	for(const auto& WwiseTreeItem : InWwiseItems)
	{
		if(WwiseTreeItem->IsFolder())
		{
			ConvertWwiseItemTypeToReconcileItem(WwiseTreeItem->GetChildren(), OutReconcileItems, OperationFlags, false);
			if(!WwiseTreeItem->ShouldDisplayInfo())
			{
				continue;
			}
		}
		auto WwiseRef = GuidToWwiseRef.Find(WwiseTreeItem->ItemId);
		if(WwiseTreeItem->UEAssetExists())
		{
			for(auto Asset : WwiseTreeItem->Assets)
			{
				FWwiseReconcileItem ReconcileItem;
				ReconcileItem.ItemId = WwiseTreeItem->ItemId;
				ReconcileItem.Asset = Asset;
				if (WwiseRef)
				{
					ReconcileItem.WwiseAnyRef = *WwiseRef;
				}
				OutReconcileItems.AddUnique(ReconcileItem);
			}
		}
		else
		{
			FWwiseReconcileItem ReconcileItem;
			ReconcileItem.ItemId = WwiseTreeItem->ItemId;
			if(WwiseRef)
			{
				ReconcileItem.WwiseAnyRef = *WwiseRef;
			}
			OutReconcileItems.AddUnique(ReconcileItem);
		}
	}
	if (bFirstLevel)
	{
		GetAssetChanges(OutReconcileItems, OperationFlags);
	}
}

bool FWwiseReconcileImpl::RenameExistingAssets(FScopedSlowTask& SlowTask)
{
#if !UE_5_0_OR_LATER
	if(IsRunningCommandlet())
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Renaming through the commandlet is only supported with Unreal Engine 5. Use the Wwise Browser to rename assets instead."));
		SlowTask.EnterProgressFrame((float)AssetsToRename.Num() / GetNumberOfAssets());
		return false;
	}
#endif
	
	TArray<FAssetRenameData> AssetsToRenameData;

	for (const auto& AssetData : AssetsToRename)
	{
		if(SlowTask.ShouldCancel())
		{
			return false;
		}
		auto AkAudioAsset = Cast<UAkAudioType>(AssetData.GetAsset());
		if (!LIKELY(AkAudioAsset))
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Failed to rename Wwise asset %s."), *AssetData.AssetName.ToString());
			continue;
		}
		FName NewAssetName = AkAudioAsset->GetAssetDefaultName();
		FString NewAssetPath = AssetData.PackagePath.ToString() / NewAssetName.ToString() + "." + NewAssetName.ToString();

		int MaxPath = 0;
		//Windows Max Path Length can be 32767 while the max path length for cooking remains 260.
#if PLATFORM_WINDOWS
		MaxPath = MAX_PATH;
#else
		MaxPath = FPlatformMisc::GetMaxPathLength();
#endif
		if (NewAssetPath.Len() > MaxPath || NewAssetPath.Len() >= NAME_SIZE)
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Could not rename asset '%s' to '%s', path exceeds Platform Max Path Length (%i). Please import this asset manually."), *AssetData.AssetName.ToString(), *NewAssetName.ToString(), MaxPath);
			continue;
		}

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
#if UE_5_1_OR_LATER
		FAssetData Asset = AssetRegistryModule.GetRegistry().GetAssetByObjectPath(NewAssetPath);
#else
		FAssetData Asset = AssetRegistryModule.GetRegistry().GetAssetByObjectPath(FName(NewAssetPath));
#endif
		if(!Asset.IsValid())
		{
			UE_LOG(LogWwiseReconcile, Verbose, TEXT("Renaming Wwise asset %s to %s."), *AssetData.AssetName.ToString(), *NewAssetName.ToString());
			// FIXME If the package has been changed, the asset remains in the old folder
			FAssetRenameData AssetRenameData(AkAudioAsset, AssetData.PackagePath.ToString(), NewAssetName.ToString());
			AssetsToRenameData.Add(AssetRenameData);
		}
		else
		{
			UE_LOG(LogWwiseReconcile, Warning, TEXT("Asset %s already exists at %s."), *NewAssetName.ToString(), *AssetData.PackagePath.ToString());
		}

	}
	auto AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	if (!LIKELY(AssetToolsModule))
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not load the AssetTools Module"));
		return false;
	}
	if (AssetsToRenameData.Num() > 0 && !AssetToolsModule->Get().RenameAssets(AssetsToRenameData))
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Failed to rename updated Wwise assets."));
		return false;
	}
	SlowTask.EnterProgressFrame((float)AssetsToRename.Num() / GetNumberOfAssets());

	return true;
}

int FWwiseReconcileImpl::GetNumberOfAssets()
{
	return AssetsToDelete.Num() + AssetsToCreate.Num() + AssetsToRename.Num() + AssetsToUpdate.Num();
}

int32 FWwiseReconcileImpl::DeleteAssets(FScopedSlowTask& SlowTask)
{
	check(IsInGameThread());

	if (AssetsToDelete.Num() == 0)
	{
		return 0;
	}

	TArray<UObject*> ObjectsToDelete;
	for (const auto& AssetData : AssetsToDelete)
	{
		bool bReferenced = false;
		bool bReferencedByUndo = false;

		check(AssetData.IsValid())

		auto Asset = AssetData.GetAsset();
		check(IsValid(Asset))

		if (UNLIKELY(!AssetData.IsValid() || !IsValid(Asset)))
		{
			continue;
		}

		ObjectTools::GatherObjectReferencersForDeletion(Asset, bReferenced, bReferencedByUndo);
		if (SlowTask.ShouldCancel())
		{
			return 0;
		}
		if (!bReferenced && !bReferencedByUndo)
		{
			ObjectsToDelete.Add(AssetData.GetAsset());		
		}
		else
		{
			UE_LOG(LogWwiseReconcile, Warning, TEXT("Asset %s is referenced and will not be deleted. Please delete this asset manually."), *AssetData.AssetName.ToString());
		}
	}

	int32 NumDeletedObjects = ObjectTools::ForceDeleteObjects(ObjectsToDelete, false);
	SlowTask.EnterProgressFrame((float)ObjectsToDelete.Num() / (float)GetNumberOfAssets());
	if (NumDeletedObjects != ObjectsToDelete.Num())
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not delete assets. Verify that none of the assets are still being referenced."))
	}

	return NumDeletedObjects;
}

TArray<FAssetData> FWwiseReconcileImpl::CreateAssets(FScopedSlowTask& SlowTask)
{
	check(IsInGameThread());

	TArray<UPackage*> PackagesToSave;
	TArray<FAssetData> NewAssets;

	for (const auto& Asset : AssetsToCreate)
	{
		if(SlowTask.ShouldCancel())
		{
			return NewAssets;
		}
		const FWwiseAnyRef WwiseRef = *Asset.WwiseAnyRef;

		FName AssetName = AkUnrealAssetDataHelper::GetAssetDefaultName(&WwiseRef);
		FString AssetPackagePath = GetAssetPackagePath(WwiseRef);

		UClass* NewAssetClass = GetUClassFromWwiseRefType(WwiseRef.GetType());

		if (!NewAssetClass)
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Could not determine which type of asset to create for '%s' in '%s'."), *AssetName.ToString(), *AssetPackagePath);
			continue;
		}
		
		int PackageLength = AssetViewUtils::GetPackageLengthForCooking(AssetPackagePath / AssetName.ToString(), FEngineBuildSettings::IsInternalBuild());
		int MaxPath = AssetViewUtils::GetMaxCookPathLen();
		if (PackageLength > MaxPath || PackageLength >= NAME_SIZE)
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Could not create asset '%s' at location '%s', path exceeds Platform Max Path Length (%i). Please import this asset manually."), *AssetName.ToString(), *AssetPackagePath, MaxPath);
			continue;
		}

		FString NewAssetPath = AssetPackagePath / AssetName.ToString() + "." + AssetName.ToString();
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
#if UE_5_1_OR_LATER
		FAssetData AssetFound = AssetRegistryModule.GetRegistry().GetAssetByObjectPath(NewAssetPath);
#else
		FAssetData AssetFound = AssetRegistryModule.GetRegistry().GetAssetByObjectPath(FName(NewAssetPath));
#endif
		if(AssetFound.IsValid())
		{
			UE_LOG(LogWwiseReconcile, Warning, TEXT("Asset %s already exists at %s."), *AssetName.ToString(), *AssetPackagePath);
			continue;
		}

		UE_LOG(LogWwiseReconcile, Verbose, TEXT("Creating new asset '%s' in '%s'."), *AssetName.ToString(), *AssetPackagePath);

		auto AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		if (!LIKELY(AssetToolsModule))
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Could not load the AssetRegistry Module"));
			return {};
		}

		UAkAudioType* NewAkAudioObject = Cast<UAkAudioType>(
			AssetToolsModule->Get().CreateAsset(AssetName.ToString(), AssetPackagePath, NewAssetClass, nullptr));

		if (!NewAkAudioObject)
		{
			UE_LOG(LogWwiseReconcile, Error, TEXT("Could not save asset %s"), *AssetName.ToString());
			continue;
		}

		NewAkAudioObject->FillInfo(WwiseRef);
		NewAkAudioObject->PostLoad();

		NewAssets.Add(FAssetData(NewAkAudioObject));

		UE_LOG(LogWwiseReconcile, Verbose, TEXT("Created asset %s"), *AssetName.ToString());

		PackagesToSave.Add(NewAkAudioObject->GetPackage());
		SlowTask.EnterProgressFrame();
	}

	if (!UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false))
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Could not save packages"));
		return {};
	}

	return NewAssets;
}

FString FWwiseReconcileImpl::GetAssetPackagePath(const FWwiseAnyRef& WwiseRef)
{
	return AkUnrealAssetDataHelper::GetAssetDefaultPackagePath(&WwiseRef);;
}
