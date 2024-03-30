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
#include "Engine/DataTable.h"

#include "WwiseExternalSourceCookieDefaultMedia.generated.h"

//Maps from an external source cookie to an entry in the FWwiseExternalSourceMediaInfo table
//
USTRUCT(BlueprintType)
struct WWISESIMPLEEXTERNALSOURCE_API FWwiseExternalSourceCookieDefaultMedia : public FTableRowBase
{
    GENERATED_BODY()

public:

    FWwiseExternalSourceCookieDefaultMedia(){}

    //Hash of the external source name, technically a uint32
    //Used as the search key external source in the default external source table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSource)
    int32 ExternalSourceCookie = 0;

    //Name of the external source name in Wwise
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSource)
    FString ExternalSourceName;

    //Id of the media in the ExternalMediaInfoTable
    //Used to lookup media in the ExternalMediaInfoTable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSource)
    int32 MediaInfoId = 0;

    // Not actually used, but helps keep track of what's pointing where. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSource)
    FString MediaName;
};