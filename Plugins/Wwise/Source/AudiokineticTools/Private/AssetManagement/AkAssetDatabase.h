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

#include "HAL/CriticalSection.h"
#include "Misc/Guid.h"
#include "AssetRegistry/AssetData.h"

class FAssetRegistryModule;
class FAssetToolsModule;
struct FAssetRenameData;

class AUDIOKINETICTOOLS_API AkAssetDatabase
{
public:
	static AkAssetDatabase& Get();

	bool FindAllAssets(TArray<FAssetData>& OutData);
	bool FindAssets(const FGuid& AkGuid, TArray<FAssetData>& OutData);
	bool FindAssets(const FString& AkAssetName, TArray<FAssetData>& OutData);
	FAssetData FindAssetByObjectPath(const FSoftObjectPath& AssetPath);
	bool FindFirstAsset(const FGuid& AkGuid, FAssetData& OutAsset);
	bool FindFirstAsset(const FString& AkAssetName, FAssetData& OutAsset);
	bool FindAssetsByGuidAndClass(const FGuid& AkGuid, const UClass* StaticClass, TArray<FAssetData>& OutWwiseAssets);

	bool RenameAsset(const FGuid& Id, const FString& AssetName, const FString& RelativePath);

	void DeleteAsset(const FGuid& Id);
	void DeleteAssets(const TSet<FGuid>& AssetsId);

	void FixUpRedirectors(const FString& AssetPackagePath);

	bool CheckIfLoadingAssets();

	mutable FCriticalSection InitBankLock;

private:
	AkAssetDatabase();

	bool IsAkAudioType(const FAssetData& AssetData);

private:
	FAssetRegistryModule* AssetRegistryModule = nullptr;
	FAssetToolsModule* AssetToolsModule = nullptr;
};
