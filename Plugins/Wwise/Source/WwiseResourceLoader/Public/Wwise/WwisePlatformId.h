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

#include "WwisePlatformId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwisePlatformId
{
	GENERATED_BODY()

		FWwisePlatformId() :
		PlatformGuid(),
		PlatformName()
#if WITH_EDITORONLY_DATA
		, PathRelativeToGeneratedSoundBanks()
#endif
	{}

	FWwisePlatformId(const FGuid& InPlatformGuid, const FName& InPlatformName) :
		PlatformGuid(InPlatformGuid),
		PlatformName(InPlatformName)
	{}

#if WITH_EDITORONLY_DATA
	FWwisePlatformId(const FGuid& InPlatformGuid, const FName& InPlatformName, const FName& InGeneratedSoundBanksPath) :
		PlatformGuid(InPlatformGuid),
		PlatformName(InPlatformName),
		PathRelativeToGeneratedSoundBanks(InGeneratedSoundBanksPath)
	{}
#endif

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
		FGuid PlatformGuid;

	/**
	 * @brief Optional debug name. Can be empty in release, contain the name, or the full path of the asset.
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
		FName PlatformName;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Wwise")
		FName PathRelativeToGeneratedSoundBanks;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Wwise")
		FName ExternalSourceRootPath;
#endif

	bool operator==(const FWwisePlatformId& Rhs) const
	{
		return PlatformGuid == Rhs.PlatformGuid;
	}

	bool operator!=(const FWwisePlatformId& Rhs) const
	{
		return PlatformGuid != Rhs.PlatformGuid;
	}
};
inline uint32 GetTypeHash(const FWwisePlatformId& Id)
{
	return GetTypeHash(Id.PlatformGuid);
}
