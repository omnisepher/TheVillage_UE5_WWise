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

#include "Wwise/WwiseReconcile.h"

class WWISERECONCILE_API FWwiseReconcileImpl : public IWwiseReconcile
{
protected:
	virtual bool IsAssetOutOfDate(const FAssetData& AssetData, const FWwiseAnyRef& WwiseRef) override;
	virtual void GetAllWwiseRefs() override;
	
public:
	FWwiseReconcileImpl():IWwiseReconcile(){};
	virtual ~FWwiseReconcileImpl(){};
	virtual FString GetAssetPackagePath(const FWwiseAnyRef& WwiseRef) override;
	virtual void GetAllAssets(TArray<FWwiseReconcileItem>& ReconcileItems) override;
	virtual TArray<FAssetData> CreateAssets(FScopedSlowTask& SlowTask) override;
	virtual TArray<FAssetData> UpdateExistingAssets(FScopedSlowTask& SlowTask) override;
	virtual void ConvertWwiseItemTypeToReconcileItem(const TArray<TSharedPtr<FWwiseTreeItem>>& InWwiseItems, TArray<FWwiseReconcileItem>& OutReconcileItems, EWwiseReconcileOperationFlags OperationFlags = EWwiseReconcileOperationFlags::All, bool bFirstLevel = true) override;
	virtual bool RenameExistingAssets(FScopedSlowTask& SlowTask) override;
	virtual int GetNumberOfAssets() override;
	virtual int32 DeleteAssets(FScopedSlowTask& SlowTask) override;
	virtual UClass* GetUClassFromWwiseRefType(EWwiseRefType RefType) override;
	virtual void GetAssetChanges(TArray<FWwiseReconcileItem>& ReconcileItems, EWwiseReconcileOperationFlags OperationFlags = EWwiseReconcileOperationFlags::All) override;
};
