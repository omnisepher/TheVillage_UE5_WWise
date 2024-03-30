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

#include "Wwise/WwiseResourceCookerModule.h"
#include "Wwise/WwiseResourceCooker.h"

class WWISERESOURCECOOKER_API FWwiseResourceCookerModule : public IWwiseResourceCookerModule
{
public:
	FWwiseResourceCooker* GetResourceCooker() override;
	FWwiseResourceCooker* InstantiateResourceCooker() override;

	FWwiseResourceCooker* CreateCookerForPlatform(
		const ITargetPlatform* TargetPlatform,
		const FWwiseSharedPlatformId& InPlatform,
		EWwiseExportDebugNameRule InExportDebugNameRule = EWwiseExportDebugNameRule::Release) override;
	void DestroyCookerForPlatform(const ITargetPlatform* TargetPlatform) override;
	FWwiseResourceCooker* GetCookerForPlatform(const ITargetPlatform* TargetPlatform) override;

	void DestroyAllCookerPlatforms() override;
	
	void ShutdownModule() override;

protected:
	FRWLock Lock;
	TUniquePtr<FWwiseResourceCooker> ResourceCooker;

	TMap<const ITargetPlatform*, TUniquePtr<FWwiseResourceCooker>> CookingPlatforms;

	void OnModifyCook(TConstArrayView<const ITargetPlatform*> InTargetPlatforms, TArray<FName>& InOutPackagesToCook, TArray<FName>& InOutPackagesToNeverCook) override;
};
