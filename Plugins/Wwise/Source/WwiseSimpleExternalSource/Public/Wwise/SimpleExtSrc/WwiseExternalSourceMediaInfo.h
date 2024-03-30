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

#include "WwiseExternalSourceMediaInfo.generated.h"

//Contains the necessary info package and load an external source media
//There should be one entry for each external source media in the project 
USTRUCT(BlueprintType)
struct WWISESIMPLEEXTERNALSOURCE_API FWwiseExternalSourceMediaInfo : public FTableRowBase
{
    GENERATED_BODY()

    FWwiseExternalSourceMediaInfo(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
    int32 ExternalSourceMediaInfoId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
    FName MediaName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
    int32 CodecID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	bool bIsStreamed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	bool bUseDeviceMemory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	int32 MemoryAlignment = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ExternalSourceMedia)
	int32 PrefetchSize = 0;

};