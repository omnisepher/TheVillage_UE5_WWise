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

#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCookerModule.h"

struct FWwiseSoundBankCookedData;
struct FWwiseGameParameterCookedData;
struct FWwiseTriggerCookedData;
struct FWwiseAcousticTextureCookedData;
class FWwiseCookingCache;

class WWISERESOURCECOOKER_API FWwiseResourceCooker
{
public:
	using WriteAdditionalFileFunction = TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)>;

	static FWwiseResourceCooker* GetDefault()
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->GetResourceCooker();
		}
		return nullptr;
	}
	static FWwiseResourceCooker* Instantiate()
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->InstantiateResourceCooker();
		}
		return nullptr;
	}
	static FWwiseResourceCooker* CreateForPlatform(
		const ITargetPlatform* TargetPlatform,
		const FWwiseSharedPlatformId& InPlatform,
		EWwiseExportDebugNameRule InExportDebugNameRule = EWwiseExportDebugNameRule::Release)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->CreateCookerForPlatform(TargetPlatform, InPlatform, InExportDebugNameRule);
		}
		return nullptr;
	}

	static void DestroyForPlatform(const ITargetPlatform* TargetPlatform)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			Module->DestroyCookerForPlatform(TargetPlatform);
		}
	}

	static FWwiseResourceCooker* GetForPlatform(const ITargetPlatform* TargetPlatform)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->GetCookerForPlatform(TargetPlatform);
		}
		return nullptr;
	}

	static FWwiseResourceCooker* GetForArchive(const FArchive& InArchive)
	{
		if (auto* Module = IWwiseResourceCookerModule::GetModule())
		{
			return Module->GetCookerForArchive(InArchive);
		}
		return nullptr;
	}

	FWwiseResourceCooker() {}
	virtual ~FWwiseResourceCooker() {}

	void CookAuxBus(const FWwiseObjectInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookEvent(const FWwiseEventInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookExternalSource(uint32 InCookie, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookInitBank(const FWwiseObjectInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookMedia(const FWwiseObjectInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookShareSet(const FWwiseObjectInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookSoundBank(const FWwiseObjectInfo& InInfo, WriteAdditionalFileFunction WriteAdditionalFile);

	bool PrepareCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseObjectInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseObjectInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo);
	bool PrepareCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie);
	bool PrepareCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseObjectInfo& InInfo);
	bool PrepareCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo, EWwiseGroupType InGroupType);
	bool PrepareCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo = FWwiseObjectInfo::DefaultInitBank);
	bool PrepareCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseObjectInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedShareSetCookedData& OutCookedData, const FWwiseObjectInfo& InInfo);
	bool PrepareCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo);
	bool PrepareCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseObjectInfo& InInfo);

	virtual FWwiseProjectDatabase* GetProjectDatabase() { return nullptr; }
	virtual const FWwiseProjectDatabase* GetProjectDatabase() const { return nullptr; }

	virtual void PrepareResourceCookerForPlatform(FWwiseProjectDatabase*&& InProjectDatabaseOverride, EWwiseExportDebugNameRule InExportDebugNameRule) {}

	virtual void SetSandboxRootPath(const TCHAR* InPackageFilename);
	FString GetSandboxRootPath() const {return SandboxRootPath;}

	FWwiseResourceLoader* GetResourceLoader();
	const FWwiseResourceLoader* GetResourceLoader() const;

	FWwiseSharedLanguageId GetCurrentLanguage() const;
	FWwiseSharedPlatformId GetCurrentPlatform() const;

	// Low-level operations

	virtual FWwiseCookingCache* GetCookingCache() { return nullptr; }

	void CookLocalizedAuxBusToSandbox(const FWwiseLocalizedAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookLocalizedSoundBankToSandbox(const FWwiseLocalizedSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookLocalizedEventToSandbox(const FWwiseLocalizedEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);
	void CookLocalizedShareSetToSandbox(const FWwiseLocalizedShareSetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile);

	virtual void CookAuxBusToSandbox(const FWwiseAuxBusCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookEventToSandbox(const FWwiseEventCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookExternalSourceToSandbox(const FWwiseExternalSourceCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookInitBankToSandbox(const FWwiseInitBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookMediaToSandbox(const FWwiseMediaCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookShareSetToSandbox(const FWwiseShareSetCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}
	virtual void CookSoundBankToSandbox(const FWwiseSoundBankCookedData& InCookedData, WriteAdditionalFileFunction WriteAdditionalFile) {}

	virtual void CookFileToSandbox(const FString& InInputPathName, const FName& InOutputPathName, WriteAdditionalFileFunction WriteAdditionalFile, bool bInStageRelativeToContent = false) {}

	virtual bool GetAcousticTextureCookedData(FWwiseAcousticTextureCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const { return false; }
	virtual bool GetAuxBusCookedData(FWwiseLocalizedAuxBusCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const { return false; }
	virtual bool GetEventCookedData(FWwiseLocalizedEventCookedData& OutCookedData, const FWwiseEventInfo& InInfo) const { return false; }
	virtual bool GetExternalSourceCookedData(FWwiseExternalSourceCookedData& OutCookedData, uint32 InCookie) const { return false; }
	virtual bool GetGameParameterCookedData(FWwiseGameParameterCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const { return false; }
	virtual bool GetInitBankCookedData(FWwiseInitBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo = FWwiseObjectInfo::DefaultInitBank) const { return false; }
	virtual bool GetMediaCookedData(FWwiseMediaCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const { return false; }
	virtual bool GetShareSetCookedData(FWwiseLocalizedShareSetCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const { return false; }
	virtual bool GetSoundBankCookedData(FWwiseLocalizedSoundBankCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const { return false; }
	virtual bool GetStateCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const { return false; }
	virtual bool GetSwitchCookedData(FWwiseGroupValueCookedData& OutCookedData, const FWwiseGroupValueInfo& InInfo) const { return false; }
	virtual bool GetTriggerCookedData(FWwiseTriggerCookedData& OutCookedData, const FWwiseObjectInfo& InInfo) const { return false; }

protected:
	FString SandboxRootPath;
};