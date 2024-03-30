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


#include "AkInclude.h"
#include "UObject/Object.h"
#include "Wwise/Info/WwiseObjectInfo.h"
#include "Wwise/WwiseResourceLoaderFuture.h"

#include "AkAudioType.generated.h"

class FWwiseProjectDatabase;
class FWwiseAnyRef;
UCLASS(Abstract)
class AKAUDIO_API UAkAudioType : public UObject
{
	GENERATED_BODY()

public:
	virtual ~UAkAudioType() override;

	///< When true, SoundBanks and medias associated with this asset will be loaded in the Wwise SoundEngine when Unreal loads this asset.
	UPROPERTY(EditAnywhere, Category = "AkAudioType|Behaviour")
	bool bAutoLoad = true;

// Deprecated ID properties used in migration
#if WITH_EDITORONLY_DATA
	UPROPERTY(meta=(Deprecated))
	FGuid ID_DEPRECATED;

	UPROPERTY(meta=(Deprecated))
	uint32 ShortID_DEPRECATED = 0;
#endif
	
	UPROPERTY(EditAnywhere, Category = "AkAudioType")
	TArray<UObject*> UserData;

public:
	void Serialize(FArchive& Ar) override;
	void PostLoad() override;
	void BeginDestroy() override;
	void FinishDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkAudioType")
	virtual void LoadData()   {}

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkAudioType")
	virtual void UnloadData(bool bAsync = false) {}

	void LogSerializationState(const FArchive& Ar);
	virtual AkUInt32 GetShortID() const;

	bool IsPostLoadThreadSafe() const override { return true; }
	bool IsDestructionThreadSafe() const override { return true; }

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkAudioType")
	int32 GetWwiseShortID() const { return GetShortID();}

	template<typename T>
	T* GetUserData()
	{
		for (auto Entry : UserData)
		{
			if (Entry && Entry->GetClass()->IsChildOf(T::StaticClass()))
			{
				return Entry;
			}
		}

		return nullptr;
	}
	
#if WITH_EDITORONLY_DATA
	virtual FWwiseObjectInfo* GetInfoMutable();
	virtual FWwiseObjectInfo GetInfo() const {return {};}
	virtual FGuid GetWwiseGuid() const { return GetInfo().WwiseGuid; }
	virtual FName GetWwiseName() const { return GetInfo().WwiseName; }
	virtual FName GetWwiseGroupName() { return {}; }
	virtual FName GetAssetDefaultName();
	virtual FName GetAssetDefaultPackagePath();
	virtual bool ObjectIsInSoundBanks() { return false; }
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;

	// Checks whether the metadata for this UAkAudioType matches what is in the Project Database
	virtual bool IsAssetOutOfDate(const FWwiseAnyRef& CurrentWwiseRef);
	virtual void FillInfo(const FWwiseAnyRef& CurrentWwiseRef);
	virtual void FillInfo() {}
	virtual void FillMetadata(FWwiseProjectDatabase* ProjectDatabase) {}
	virtual void CheckWwiseObjectInfo();
	virtual void MigrateWwiseObjectInfo();
	void WaitForResourceUnloaded();

	template <class InfoType>
	InfoType GetValidatedInfo(const InfoType& InInfo)
	{
		InfoType TempInfo(InInfo);
		ValidateShortID(TempInfo);
		return TempInfo;
	}
	virtual void ValidateShortID(FWwiseObjectInfo& WwiseInfo) const;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif

protected:
	FWwiseResourceUnloadFuture ResourceUnload;

	static bool CanLoadObjects();
	void LoadOnceSoundEngineReady();
};
