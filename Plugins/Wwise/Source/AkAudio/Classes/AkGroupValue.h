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

#include "AkAudioType.h"
#include "Wwise/CookedData/WwiseGroupValueCookedData.h"
#include "Wwise/Loaded/WwiseLoadedGroupValue.h"
#include "Wwise/Info/WwiseGroupValueInfo.h"
#include "AkGroupValue.generated.h"

UCLASS(Abstract)
class AKAUDIO_API UAkGroupValue : public UAkAudioType
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, VisibleAnywhere, Category = "AkGroupValue")
	FWwiseGroupValueCookedData GroupValueCookedData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AkGroupValue")
	FWwiseGroupValueInfo GroupValueInfo;
#endif

	UPROPERTY(meta =(Deprecated,  DeprecationMessage="Use Group ID from Load Data. Used for migration from older versions."))
	uint32 GroupShortID_DEPRECATED = 0;
	
public:
	virtual void LoadData()   override {LoadGroupValue();}
	virtual void UnloadData(bool bAsync = false) override {UnloadGroupValue(bAsync);}
	virtual AkUInt32 GetShortID() const override {return GroupValueCookedData.Id;}
	AkUInt32 GetGroupID() const {return GroupValueCookedData.GroupId;}

#if WITH_EDITORONLY_DATA
	virtual FWwiseObjectInfo* GetInfoMutable() override {return &GroupValueInfo;}
	virtual FWwiseObjectInfo GetInfo() const override {return GroupValueInfo;}
	virtual void MigrateWwiseObjectInfo() override;
	virtual void ValidateShortID(FWwiseObjectInfo& WwiseInfo) const override;
	virtual bool SplitAssetName(FString& OutGroupName, FString& OutValueName) const;
#endif

protected :
	virtual void LoadGroupValue(){};
	void UnloadGroupValue(bool bAsync);
	
	FWwiseLoadedGroupValuePtrAtomic LoadedGroupValue{nullptr};
};
