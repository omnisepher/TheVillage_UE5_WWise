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

#include "Wwise/WwisePlatformId.h"

#include "WwiseSharedPlatformId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseSharedPlatformId
{
	GENERATED_BODY()

	TSharedRef<FWwisePlatformId, ESPMode::ThreadSafe> Platform;

	FWwiseSharedPlatformId() :
		Platform(new FWwisePlatformId)
	{}

	FWwiseSharedPlatformId(const FGuid& InPlatformGuid, const FName& InPlatformName) :
		Platform(new FWwisePlatformId(InPlatformGuid, InPlatformName))
	{}

#if WITH_EDITORONLY_DATA
	FWwiseSharedPlatformId(const FGuid& InPlatformGuid, const FName& InPlatformName, const FName& InRelativePath) :
		Platform(new FWwisePlatformId(InPlatformGuid, InPlatformName, InRelativePath))
	{}
#endif

	const FGuid& GetPlatformGuid() const
	{
		return Platform->PlatformGuid;
	}

	const FName& GetPlatformName() const
	{
		return Platform->PlatformName;
	}

	bool operator==(const FWwiseSharedPlatformId& Rhs) const
	{
		return Platform->PlatformGuid == Rhs.Platform->PlatformGuid;
	}

	bool operator!=(const FWwiseSharedPlatformId& Rhs) const
	{
		return Platform->PlatformGuid != Rhs.Platform->PlatformGuid;
	}
};

inline uint32 GetTypeHash(const FWwiseSharedPlatformId& Id)
{
	return GetTypeHash(Id.Platform->PlatformGuid);
}
