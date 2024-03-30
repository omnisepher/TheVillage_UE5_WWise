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

#include "WwiseBrowser/WwiseBrowserHelpers.h"

#include "WwiseUEFeatures.h"
#include "AkAcousticTexture.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkRtpc.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkEffectShareSet.h"
#include "AkInitBank.h"
#include "WwiseUnrealHelper.h"
#include "AkAssetFactories.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "IAudiokineticTools.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Async/Async.h"
#include "Engine/ObjectReferencer.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "PackageTools.h"

#define LOCTEXT_NAMESPACE "AkAudio"


EWwiseItemType::Type WwiseBrowserHelpers::GetTypeFromClass(UClass* Class)
{
	if (Class == UAkAudioEvent::StaticClass())
	{
		return EWwiseItemType::Event;
	}
	if (Class == UAkAcousticTexture::StaticClass())
	{
		return EWwiseItemType::AcousticTexture;
	}
	if (Class == UAkRtpc::StaticClass())
	{
		return EWwiseItemType::GameParameter;
	}
	if (Class == UAkStateValue::StaticClass())
	{
		return EWwiseItemType::State;
	}
	if (Class == UAkSwitchValue::StaticClass())
	{
		return EWwiseItemType::Switch;
	}
	if (Class == UAkTrigger::StaticClass())
	{
		return EWwiseItemType::Trigger;
	}
	if (Class == UAkEffectShareSet::StaticClass())
	{
		return EWwiseItemType::EffectShareSet;
	}
	if (Class == UAkAuxBus::StaticClass())
	{
		return EWwiseItemType::AuxBus;
	}
	if(Class == UAkInitBank::StaticClass())
	{
		return EWwiseItemType::InitBank;
	}
	return EWwiseItemType::None;
}

void WwiseBrowserHelpers::FindOrCreateAssetsRecursive(const FWwiseTreeItemPtr& WwiseTreeItem,
	TArray<WwiseBrowserAssetPayload>& InOutBrowserAssetPayloads, TSet<FGuid>& InOutKnownGuids,
	const EAssetCreationMode AssetCreationMode, const FString& PackagePath, const FString& CurrentRelativePath)
{
	FString Name;
	UClass* WwiseAssetClass = nullptr;
	FString CurrentRelativePackagePath = PackagePath / CurrentRelativePath;

	if (WwiseTreeItem->ItemId.IsValid() && InOutKnownGuids.Contains(WwiseTreeItem->ItemId))
	{
		return;
	}

	if (WwiseTreeItem->ItemType == EWwiseItemType::Event)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkAudioEvent::StaticClass();
	}
	if (WwiseTreeItem->ItemType == EWwiseItemType::AcousticTexture)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkAcousticTexture::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::AuxBus)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkAuxBus::StaticClass();
		for (FWwiseTreeItemPtr Child : WwiseTreeItem->GetChildren())
		{
			FString NewRelativePath = CurrentRelativePath / WwiseTreeItem->DisplayName;
			FindOrCreateAssetsRecursive(Child, InOutBrowserAssetPayloads, InOutKnownGuids, AssetCreationMode, PackagePath, NewRelativePath);
		}
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::GameParameter)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkRtpc::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::State)
	{
		Name = WwiseTreeItem->GetSwitchAssetName();
		WwiseAssetClass = UAkStateValue::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::Switch)
	{
		Name = WwiseTreeItem->GetSwitchAssetName();
		WwiseAssetClass = UAkSwitchValue::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::Trigger)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkTrigger::StaticClass();
	}
	else if (WwiseTreeItem->ItemType == EWwiseItemType::EffectShareSet)
	{
		Name = WwiseTreeItem->DisplayName;
		WwiseAssetClass = UAkEffectShareSet::StaticClass();
	}
	else if (WwiseTreeItem->IsFolder())
	{
		//Add object to prevent Drag and Drop in the world.
		WwiseAssetClass = UAkDragDropBlocker::StaticClass();
		Name = WwiseTreeItem->DisplayName;
		for (FWwiseTreeItemPtr Child : WwiseTreeItem->GetChildren())
		{
			FString NewRelativePath = CurrentRelativePath / Name;
			FindOrCreateAssetsRecursive(Child, InOutBrowserAssetPayloads, InOutKnownGuids, AssetCreationMode, PackagePath, NewRelativePath);
		}
	}

	if (WwiseAssetClass)
	{
		TArray<FAssetData> SearchResults;
		WwiseBrowserAssetPayload Payload;
		AkAssetDatabase::Get().FindAssetsByGuidAndClass(WwiseTreeItem->ItemId, WwiseAssetClass, Payload.ExistingAssets);
		if (Payload.ExistingAssets.Num() == 0)
		{
			Payload.CreatedAsset = CreateBrowserAsset(Name, WwiseTreeItem, WwiseAssetClass, AssetCreationMode, PackagePath / CurrentRelativePath);
		}

		Payload.Name = UPackageTools::SanitizePackageName(Name);
		Payload.RelativePackagePath = UPackageTools::SanitizePackageName(CurrentRelativePath);
		Payload.WwiseObjectGuid = WwiseTreeItem->ItemId;
		InOutBrowserAssetPayloads.Add(Payload);
		InOutKnownGuids.Add(WwiseTreeItem->ItemId);
	}
}


FAssetData WwiseBrowserHelpers::CreateBrowserAsset(const FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass, const EAssetCreationMode AssetCreationMode, const FString& PackagePath)
{
	//We shouldn't call NewObject outside of the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [AssetName, WwiseTreeItem, AssetClass, AssetCreationMode, PackagePath]
		{
			CreateBrowserAssetTask(AssetName, WwiseTreeItem, AssetClass, AssetCreationMode, PackagePath);
		});

		// Spoof the FAssetData for the asset that will be created asynchronously
		const FString SanitizedName = UPackageTools::SanitizePackageName(AssetName);
		//Folder asset are always created in the transient package
		if (AssetCreationMode == EAssetCreationMode::Transient || WwiseTreeItem->IsFolder())
		{
			FString AssetPackage = UPackageTools::SanitizePackageName(GetTransientPackage()->GetPathName() / AssetClass->GetName());
#if UE_5_1_OR_LATER
			return FAssetData(FName(AssetPackage), FName(GetTransientPackage()->GetPathName()), FName(*SanitizedName), AssetClass->GetClassPathName());
#else
			return FAssetData(FName(AssetPackage), FName(GetTransientPackage()->GetPathName()), FName(*SanitizedName), AssetClass->GetFName());
#endif
		}
		else if (AssetCreationMode == EAssetCreationMode::InPackage)
		{
			FString AssetPackage = UPackageTools::SanitizePackageName(PackagePath / AssetName);
#if UE_5_1_OR_LATER
			return FAssetData(FName(AssetPackage), FName(PackagePath), FName(*SanitizedName), AssetClass->GetClassPathName());
#else
			return FAssetData(FName(AssetPackage), FName(PackagePath), FName(*SanitizedName), AssetClass->GetFName());
#endif
		}
	}
	return CreateBrowserAssetTask(AssetName, WwiseTreeItem, AssetClass, AssetCreationMode, PackagePath);
}


FAssetData WwiseBrowserHelpers::CreateBrowserAssetTask(const FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass, const EAssetCreationMode AssetCreationMode, const FString& PackagePath)
{
	if (!ensureMsgf(IsInGameThread(), TEXT("WwiseBrowserHelpers::CreateBrowserAsset : Not in the Game thread. Assets will not be created.")))
	{
		return {};
	}
	//Folder asset are always created in the transient package
	if (AssetCreationMode == EAssetCreationMode::Transient || WwiseTreeItem->IsFolder())
	{
		return CreateTransientAsset(AssetName, WwiseTreeItem, AssetClass);
	}
	else //if (AssetCreationMode == EAssetCreationMode::InPackage)
	{
		return CreateAssetInPackage(AssetName, WwiseTreeItem, PackagePath, AssetClass);
	}
}

FAssetData WwiseBrowserHelpers::CreateTransientAsset(const FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass)
{
	//Create a sub-package in transient to avoid asset name collisions for different wwise object type
	const FString PackageName = UPackageTools::SanitizePackageName(GetTransientPackage()->GetPathName() / AssetClass->GetName());
	UPackage* Pkg = CreatePackage(*PackageName);

	UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Browser: Creating new temporary %s asset for Drag operation in '%s' in '%s'."), *AssetClass->GetName(), *AssetName, *PackageName);
	return CreateAsset(AssetName, WwiseTreeItem, AssetClass, Pkg);
}

FAssetData WwiseBrowserHelpers::CreateAssetInPackage(const FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, const FString& PackagePath, UClass* AssetClass)
{
	const FString PackageName = UPackageTools::SanitizePackageName(PackagePath / AssetName);
	UPackage* Pkg = CreatePackage(*PackageName);

	UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Browser: Creating new %s asset '%s' in '%s'."), *AssetClass->GetName(), *AssetName, *PackageName);
	return CreateAsset(AssetName, WwiseTreeItem, AssetClass, Pkg);
}

FAssetData WwiseBrowserHelpers::CreateAsset(const FString& AssetName, const FWwiseTreeItemPtr& WwiseTreeItem, UClass* AssetClass, UPackage* Pkg)
{
	// Verify the asset class
	if (!ensureMsgf(AssetClass, TEXT("The new asset '%s' wasn't created due to a problem finding the appropriate class for the new asset.")))
	{
		return nullptr;
	}

	const auto Factory = GetAssetFactory(WwiseTreeItem);
	const FString SanitizedName = UPackageTools::SanitizePackageName(AssetName);
	UObject* NewObj = nullptr;
	EObjectFlags Flags = RF_Public | RF_Transactional | RF_Standalone;
	if (Factory)
	{
		NewObj = Factory->FactoryCreateNew(AssetClass, Pkg, FName(*SanitizedName), Flags, nullptr, GWarn);
	}
	else if (AssetClass)
	{
		NewObj = NewObject<UObject>(Pkg, AssetClass, FName(*SanitizedName), Flags);
	}
	return FAssetData(NewObj);
}

void WwiseBrowserHelpers::SaveSelectedAssets(TArray<WwiseBrowserAssetPayload> Assets, const FString& RootPackagePath, const EAssetCreationMode AssetCreationMode, const EAssetDuplicationMode AssetDuplicationMode)
{
	//It is probably dangerous to manipulate UObjects (rename/delete/duplicate) outside of the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [Assets, RootPackagePath, AssetCreationMode, AssetDuplicationMode]
		{
			SaveSelectedAssetsTask(Assets, RootPackagePath, AssetCreationMode, AssetDuplicationMode);
		});

		return;
	}
	SaveSelectedAssetsTask(Assets, RootPackagePath, AssetCreationMode, AssetDuplicationMode);
}

void WwiseBrowserHelpers::SaveSelectedAssetsTask(TArray<WwiseBrowserAssetPayload> Assets, const FString& RootPackagePath, const EAssetCreationMode AssetCreationMode, const EAssetDuplicationMode AssetDuplicationMode)
{
	if (!ensureMsgf(IsInGameThread(), TEXT("WwiseBrowserHelpers::SaveSelectedAssets : Not in the Game thread. Assets will not be saved or moved.")))
	{
		return;
	}
	auto AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetsToRename;
	TArray<FAssetData> AssetsToDelete;
	TArray<FAssetData> RenamedTransientAssets;
	TArray<UPackage*> PackagesToSave;

	for (WwiseBrowserAssetPayload& AssetResult : Assets)
	{
		const bool bPreExisting = AssetResult.ExistingAssets.Num() > 0;
		FString Path = AssetResult.RelativePackagePath;
		FString PackagePath = RootPackagePath;
		if (!Path.IsEmpty())
		{
			PackagePath = UPackageTools::SanitizePackageName(PackagePath / Path);
		}
		FString NewAssetPath = PackagePath / AssetResult.Name;

		if (bPreExisting)
		{
			if (AssetDuplicationMode == EAssetDuplicationMode::NoDuplication)
			{
				continue;
			}
			//Make sure none of the existing assets would be overwritten by the new asset we want to duplicate
			bool bMatch = false;
			for (auto ExistingAsset : AssetResult.ExistingAssets)
			{
				if (ExistingAsset.PackageName.ToString() == NewAssetPath)
				{
					bMatch = true;
					break;
				}
			}
			if (bMatch)
			{
				//Asset already exists, nothing to do
				continue;
			}
			
			UE_LOG(LogAudiokineticTools, Log, TEXT("Wwise Browser: Duplicating existing asset '%s' into '%s'."), *AssetResult.ExistingAssets[0].GetFullName(), *PackagePath);
			auto NewAsset = AssetToolsModule->Get().DuplicateAsset(AssetResult.Name, PackagePath, AssetResult.ExistingAssets[0].GetAsset());
			if (NewAsset)
			{
				PackagesToSave.Add(NewAsset->GetPackage());
			}
		}
		else
		{
			UObject* NewAsset = AssetResult.CreatedAsset.GetAsset();
			if (IsValid(NewAsset))
			{
				if (NewAsset->IsA<UAkDragDropBlocker>())
				{
					AssetsToDelete.Add(AssetResult.CreatedAsset);
					continue;
				}
			}
			if (AssetCreationMode == EAssetCreationMode::Transient)
			{
				//Drag/Drop assets are created in transient package, and we need to move them to the drop location (or the default folder)
				FAssetRenameData NewAssetRenameData(NewAsset, PackagePath, AssetResult.Name);
				AssetsToRename.Add(NewAssetRenameData);
				RenamedTransientAssets.Add(AssetResult.CreatedAsset);
				UE_LOG(LogAudiokineticTools, Verbose, TEXT("Wwise Browser: Temporary asset '%s' will be moved to '%s'."), *NewAsset->GetFullName(), *PackagePath);
			}
			else if (AssetCreationMode == EAssetCreationMode::InPackage)
			{
				UE_LOG(LogAudiokineticTools, Verbose, TEXT("Wwise Browser: Saving new asset '%s'."), *NewAsset->GetFullName());
				//Assets were created in the destination package but we still need to save them
				PackagesToSave.Add(NewAsset->GetPackage());
			}
		}
	}
	if (AssetsToRename.Num() > 0)
	{
		bool bRenameSuccess = AssetToolsModule->Get().RenameAssets(AssetsToRename);

		//We really don't want to leave assets hanging around in the transient package
		if (!bRenameSuccess && AssetCreationMode == EAssetCreationMode::Transient)
		{
			TArray<UObject*> ObjectsToDelete;
			for (auto Asset : RenamedTransientAssets)
			{
				if (auto TransientObject = Asset.GetAsset())
				{
					ObjectsToDelete.Add(TransientObject);
					UE_LOG(LogAudiokineticTools, Warning, TEXT("Wwise Browser: Failed to rename temporary asset '%s' created in Drag/Drop, it will be deleted."), *Asset.GetFullName());
				}
			}
			ObjectTools::ForceDeleteObjects(ObjectsToDelete);
		}
	}
	if (AssetsToDelete.Num() > 0)
	{
		UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Browser: Deleting '%d' temporary packages."), PackagesToSave.Num());
		ObjectTools::DeleteAssets(AssetsToDelete, false);
	}
	if (PackagesToSave.Num() > 0)
	{
		UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Wwise Browser: Saving '%d' new packages."), PackagesToSave.Num());
		UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);
	}
}

UAkAssetFactory* WwiseBrowserHelpers::GetAssetFactory(const FWwiseTreeItemPtr& WwiseTreeItem)
{
	UFactory* Factory = nullptr;
	switch (WwiseTreeItem->ItemType)
	{
	case EWwiseItemType::Event:
		Factory = UAkAudioEventFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::AcousticTexture:
		Factory = UAkAcousticTextureFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::AuxBus:
		Factory = UAkAuxBusFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::GameParameter:
		Factory = UAkRtpcFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::Switch:
		Factory = UAkSwitchValueFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::State:
		Factory = UAkStateValueFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::Trigger:
		Factory = UAkTriggerFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	case EWwiseItemType::EffectShareSet:
		Factory = UAkEffectShareSetFactory::StaticClass()->GetDefaultObject<UFactory>();
		break;
	default:
		return nullptr;
	}
	if (Factory)
	{
		if (auto AkAssetFactory = Cast<UAkAssetFactory>(Factory))
		{
			AkAssetFactory->AssetID = WwiseTreeItem->ItemId;
			AkAssetFactory->WwiseObjectName = WwiseTreeItem->DisplayName;
			return AkAssetFactory;
		}
	}
	return nullptr;
}

bool WwiseBrowserHelpers::CanCreateAsset(const FWwiseTreeItemPtr& Item)
{
	return !Item->IsOfType({ EWwiseItemType::Sound });
}

FLinearColor WwiseBrowserHelpers::GetTextColor(bool bUpToDate)
{
	FColor Color;
	return bUpToDate ? FLinearColor::Gray : FLinearColor(1.f, 0.33f, 0);
}

#undef LOCTEXT_NAMESPACE
