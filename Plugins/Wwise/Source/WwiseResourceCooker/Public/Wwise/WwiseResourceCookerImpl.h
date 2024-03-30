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

#include "Wwise/WwiseResourceCooker.h"

class WWISERESOURCECOOKER_API FWwiseResourceCookerImpl : public FWwiseResourceCooker
{
public:
	FWwiseResourceCookerImpl();
	~FWwiseResourceCookerImpl() override;

	EWwiseExportDebugNameRule ExportDebugNameRule;

	FWwiseProjectDatabase* GetProjectDatabase() override;
	const FWwiseProjectDatabase* GetProjectDatabase() const override;

	void PrepareResourceCookerForPlatform(FWwiseProjectDatabase*&& InProjectDatabaseOverride, EWwiseExportDebugNameRule InExportDebugNameRule) override;

protected:
	TUniquePtr<FWwiseCookingCache> CookingCache;
	TUniquePtr<FWwiseProjectDatabase> ProjectDatabaseOverride;

	FWwiseCookingCache* GetCookingCache() override { return CookingCache.Get(); }

	void CookAuxBusToSandbox(const FWwiseAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookEventToSandbox(const FWwiseEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookExternalSourceToSandbox(const FWwiseExternalSourceCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookInitBankToSandbox(const FWwiseInitBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookMediaToSandbox(const FWwiseMediaCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookShareSetToSandbox(const FWwiseShareSetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;
	void CookSoundBankToSandbox(const FWwiseSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) override;

	void CookFileToSandbox(const FString& InInputPathName, const FName& InOutputPathName, WriteAdditionalFileFunction WriteAdditionalFile, bool bInStageRelativeToContent = false) override;

	bool GetAcousticTextureCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const override;
	bool GetAuxBusCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const override;
	bool GetEventCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo) const override;
	bool GetExternalSourceCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie) const override;
	bool GetGameParameterCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const override;
	bool GetInitBankCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo = FWwiseObjectInfo::DefaultInitBank) const override;
	bool GetMediaCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const override;
	bool GetShareSetCookedData(FWwiseLocalizedShareSetCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const override;
	bool GetSoundBankCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const override;
	bool GetStateCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const override;
	bool GetSwitchCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const override;
	bool GetTriggerCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const override;

	virtual bool FillSoundBankBaseInfo(FWwiseSoundBankCookedData& OutSoundBankCookedData,
		const FWwiseMetadataPlatformInfo& InPlatformInfo,
		const FWwiseMetadataSoundBank& InSoundBank) const;
	virtual bool FillMediaBaseInfo(FWwiseMediaCookedData& OutMediaCookedData,
		const FWwiseMetadataPlatformInfo& InPlatformInfo,
		const FWwiseMetadataSoundBank& InSoundBank,
		const FWwiseMetadataMediaReference& InMediaReference) const;
	virtual bool FillMediaBaseInfo(FWwiseMediaCookedData& OutMediaCookedData,
		const FWwiseMetadataPlatformInfo& InPlatformInfo,
		const FWwiseMetadataSoundBank& InSoundBank,
		const FWwiseMetadataMedia& InMedia) const;
	virtual bool FillExternalSourceBaseInfo(FWwiseExternalSourceCookedData& OutExternalSourceCookedData,
		const FWwiseMetadataExternalSource& InExternalSource) const;

	virtual bool AddRequirementsForMedia(TSet<FWwiseSoundBankCookedData>& OutSoundBankSet, TSet<FWwiseMediaCookedData>& OutMediaSet,
		const FWwiseRefMedia& InMediaRef, const FWwiseSharedLanguageId& InLanguage,
		const FWwisePlatformDataStructure& InPlatformData) const;
	virtual bool AddRequirementsForExternalSource(TSet<FWwiseExternalSourceCookedData>& OutExternalSourceSet,
		const FWwiseRefExternalSource& InExternalSourceRef) const;
};