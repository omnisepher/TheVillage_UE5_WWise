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
#include "WwiseUnrealObjectHelper.generated.h"

USTRUCT(BlueprintType, Meta = (Category = "Wwise|Types", DisplayName = "AkUint64"))
struct FAkUInt64Wrapper
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Value", DisplayName = "UInt64 Value")
	uint64 UInt64Value = 0;
};

USTRUCT(BlueprintType, Meta = (Category = "Wwise|Types", DisplayName = "AkUInt32"))
struct FAkUInt32Wrapper
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Value", DisplayName = "UInt32 Value")
	uint32 UInt32Value = 0;
};

USTRUCT(BlueprintType, Meta = (Category = "Wwise|Types", DisplayName = "AkOutputDeviceID"))
struct FAkOutputDeviceID : public FAkUInt64Wrapper
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType, Meta = (Category = "Wwise|Types", DisplayName = "AkUniqueID"))
struct FAkUniqueID : public FAkUInt32Wrapper
{
	GENERATED_BODY()
};
