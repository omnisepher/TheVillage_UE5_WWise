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
#include "WwiseExternalSourceSettings.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "UObject/StrongObjectPtr.h"
#include "Wwise/WwiseExternalSourceManagerImpl.h"

struct FWwiseExternalSourceMediaInfo;

class WWISESIMPLEEXTERNALSOURCE_API FWwiseSimpleExtSrcManager :  public FWwiseExternalSourceManagerImpl
{
public:
	FWwiseSimpleExtSrcManager();
	~FWwiseSimpleExtSrcManager();

	virtual void LoadMediaTables();
	virtual void ReloadExternalSources();

	virtual FString GetStagingDirectory() const override;

	virtual void SetExternalSourceMediaById(const FName& ExternalSourceName, const int32 MediaId) override;
	virtual void SetExternalSourceMediaByName(const FName& ExternalSourceName, const FName& MediaName) override;
	virtual void SetExternalSourceMediaWithIds(const int32 ExternalSourceCookie, const int32 MediaId) override;

	#if WITH_EDITORONLY_DATA
	virtual void Cook(FWwiseResourceCooker& InResourceCooker, const FWwiseExternalSourceCookedData& InCookedData, 
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile,
		const FWwiseSharedPlatformId& InPlatform, const FWwiseSharedLanguageId& InLanguage) override;
	#endif

protected:
	virtual void LoadExternalSourceMedia(const uint32 InExternalSourceCookie, const FName& InExternalSourceName, const FName& InRootPath, FLoadExternalSourceCallback&& InCallback) override;
	virtual void UnloadExternalSourceMedia(const uint32 InExternalSourceCookie, const FName& InExternalSourceName, const FName& InRootPath, FUnloadExternalSourceCallback&& InCallback) override;

	virtual void OnTablesChanged();
	virtual void OnMediaInfoTableChanged();
	virtual void OnDefaultExternalSourceTableChanged();
	virtual void FillExternalSourceToMediaMap(const UDataTable& InMappingTable);
	virtual void FillMediaNameToIdMap(const UDataTable& InMappingTable);

	virtual void SetExternalSourceMedia(const uint32 ExternalSourceCookie, const uint32 MediaInfoId, const FName& ExternalSourceName = FName());
	virtual FWwiseFileStateSharedPtr CreateOp(const FWwiseExternalSourceMediaInfo& ExternalSourceMediaInfo, const FName& InRootPath);

	TStrongObjectPtr<UDataTable> MediaInfoTable;
	TStrongObjectPtr<UDataTable> ExternalSourceDefaultMedia;
	FStreamableManager StreamableManager;
	TMap<uint32, uint32> CookieToMediaId;
	TMap<FName, uint32> MediaNameToId;

	//We cook all media in one shot, so we use this to track whether this cooking has been performed yet
	bool bCooked = false;
#if WITH_EDITOR
	FDelegateHandle ExtSettingsTableChangedDelegate;
	FDelegateHandle MediaInfoTableChangedDelegate;
	FDelegateHandle ExternalSourceDefaultMediaTableChangedDelegate;
#endif
};
