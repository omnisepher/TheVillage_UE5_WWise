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

#include "WwiseUEFeatures.h"
#include "WwiseItemType.h"
#include "AssetRegistry/AssetData.h"

class FWwiseAnyRef;

namespace AkUnrealAssetDataHelper
{
	WWISERECONCILE_API bool IsSameType(const FAssetData& AssetData, EWwiseItemType::Type ItemType);

	WWISERECONCILE_API FName GetUClassName(EWwiseItemType::Type ItemType);

	// Gets the AssetClass prior to UE 5.1, otherwise the AssetClassPath
	WWISERECONCILE_API FName GetAssetClassName(const FAssetData& AssetData);

	WWISERECONCILE_API bool IsAssetAkAudioType(const FAssetData& AssetData);

	WWISERECONCILE_API bool IsAssetTransient(const FAssetData& AssetData);

	// Sets the AssetClass prior to UE 5.1, otherwise the AssetClassPath
	WWISERECONCILE_API void SetAssetClassName(FAssetData& AssetData, UClass* Class);

	WWISERECONCILE_API FString GetAssetDefaultPackagePath(const FAssetData& AssetData);

	WWISERECONCILE_API FString GetAssetDefaultPackagePath(const FWwiseAnyRef* WwiseRef);

	WWISERECONCILE_API FName GetAssetDefaultName(const FAssetData& AssetData);

	WWISERECONCILE_API FName GetAssetDefaultName(const FWwiseAnyRef* WwiseRef);
	
	template <typename T>
	bool AssetOfType(const FAssetData& AssetData)
	{
#if UE_5_1_OR_LATER
	return AssetData.AssetClassPath == T::StaticClass()->GetClassPathName();
#else
	return AssetData.AssetClass == T::StaticClass()->GetFName();
#endif
	}

}
