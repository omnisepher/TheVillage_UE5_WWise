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
#include "Wwise/CookedData/WwiseInitBankCookedData.h"
#include "Wwise/Loaded/WwiseLoadedInitBank.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/Info/WwiseObjectInfo.h"
#endif

#include "AkInitBank.generated.h"


UCLASS()
class AKAUDIO_API UAkInitBank : public UAkAudioType
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FWwiseInitBankCookedData InitBankCookedData;

#if WITH_EDITORONLY_DATA
	void PrepareCookedData();
#endif

	TArray<FWwiseLanguageCookedData> GetLanguages();

public:
	UAkInitBank():LoadedInitBank(nullptr){}

#if WITH_EDITORONLY_DATA
	void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
	virtual FWwiseObjectInfo* GetInfoMutable() override;

#endif

	virtual void UnloadData(bool bAsync = false) override;

	void LoadInitBank();
	void UnloadInitBank(bool bAsync);

protected:
	void Serialize(FArchive& Ar) override;

#if WITH_EDITORONLY_DATA
	virtual void MigrateWwiseObjectInfo() override;
#endif

	FWwiseLoadedInitBankPtrAtomic LoadedInitBank{nullptr};
};
