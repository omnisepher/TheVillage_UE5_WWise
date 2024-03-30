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

#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/WwiseStreamableFileHandler.h"
#include "Wwise/CookedData/WwiseLanguageCookedData.h"

struct FWwiseSharedLanguageId;
struct AkExternalSourceInfo;
struct FWwiseExternalSourceCookedData;
class FWwiseResourceCooker;

#if WITH_EDITORONLY_DATA
struct FWwiseSharedPlatformId;
#endif

class IWwiseExternalSourceManager : public IWwiseStreamableFileHandler
{
public:
	inline static IWwiseExternalSourceManager* Get()
	{
		if (auto* Module = IWwiseFileHandlerModule::GetModule())
		{
			return Module->GetExternalSourceManager();
		}
		return nullptr;
	}

	using FLoadExternalSourceCallback = TUniqueFunction<void(bool bSuccess)>;
	using FUnloadExternalSourceCallback = TUniqueFunction<void()>;

	virtual void LoadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath, 
		const FWwiseLanguageCookedData& InLanguage, FLoadExternalSourceCallback&& InCallback) = 0;
	virtual void UnloadExternalSource(const FWwiseExternalSourceCookedData& InExternalSourceCookedData, const FName& InRootPath,
		const FWwiseLanguageCookedData& InLanguage, FUnloadExternalSourceCallback&& InCallback) = 0;
	virtual void SetGranularity(AkUInt32 Uint32) = 0;

	virtual TArray<uint32> PrepareExternalSourceInfos(TArray<AkExternalSourceInfo>& OutInfo,
	                                                  const TArray<FWwiseExternalSourceCookedData>&& InCookedData) = 0;
	virtual void BindPlayingIdToExternalSources(const uint32 InPlayingId, const TArray<uint32>& InMediaIds) = 0;
	virtual void OnEndOfEvent(const uint32 InPlayingID) = 0;

	virtual void SetExternalSourceMediaById(const FName& ExternalSourceName, const int32 MediaId) = 0;
	virtual void SetExternalSourceMediaByName(const FName& ExternalSourceName, const FName& MediaName) = 0;
	virtual void SetExternalSourceMediaWithIds(const int32 ExternalSourceCookie, const int32 MediaId) = 0;


#if WITH_EDITORONLY_DATA
	virtual void Cook(FWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData, TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
		const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage) = 0;
#endif

	virtual FString GetStagingDirectory() const { return TEXT("ExternalSources"); }
};
