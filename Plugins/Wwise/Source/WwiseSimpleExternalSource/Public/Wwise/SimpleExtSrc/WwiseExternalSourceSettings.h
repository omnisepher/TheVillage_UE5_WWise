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

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "AkUnrealEditorHelper.h"
#include "Net/RepLayout.h"
#include "Wwise/Stats/SimpleExtSrc.h"

#include "WwiseExternalSourceSettings.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnTablesChanged);

UCLASS(config = Game)
class WWISESIMPLEEXTERNALSOURCE_API UWwiseExternalSourceSettings : public UObject
{
	GENERATED_BODY()

public:
	//Table of all information required to properly load all external source media in the project
	//All files in this table are packaged in the built project
	UPROPERTY(config, EditAnywhere, Category = ExternalSources, meta = (AllowedClasses = "/Script/Engine.DataTable"))
	FSoftObjectPath MediaInfoTable;

	//Optional table that defines a default media entry in the MediaInfoTable to load when an External Source is loaded
	UPROPERTY(config, EditAnywhere, Category = ExternalSources, meta = (AllowedClasses = "/Script/Engine.DataTable"))
	FSoftObjectPath ExternalSourceDefaultMedia;

	//Staging location for External Source Media when cooking the project
	//This is the location from which to load external source media in the built project
	UPROPERTY(config, EditAnywhere, Category = ExternalSources, meta =(RelativeToGameContentDir))
	FDirectoryPath ExternalSourceStagingDirectory;

	FOnTablesChanged OnTablesChanged;

	static FString GetExternalSourceStagingDirectory()
	{
		if (const UWwiseExternalSourceSettings* ExtSettings = GetDefault<UWwiseExternalSourceSettings>())
		{
			return ExtSettings->ExternalSourceStagingDirectory.Path;
		}
		UE_LOG(LogWwiseSimpleExtSrc, Error, 
			TEXT("UWwiseExternalSourceSettings::GetExternalSourceStagingDirectory : Could not get staging directory from external source settings"));
		return {};
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override
	{
		Super::PostEditChangeProperty( PropertyChangedEvent );
		AkUnrealEditorHelper::SaveConfigFile(this);
		const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

		if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UWwiseExternalSourceSettings, MediaInfoTable) )
		{
			OnTablesChanged.Broadcast();
		}
		else if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UWwiseExternalSourceSettings, ExternalSourceDefaultMedia))
		{
			OnTablesChanged.Broadcast();
		}
	}
#endif

};
