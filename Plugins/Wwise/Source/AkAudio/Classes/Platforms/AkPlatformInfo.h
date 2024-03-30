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

#include "Engine/GameEngine.h"
#include "AkAudioDevice.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseSharedPlatformId.h"
#endif

#include "AkPlatformInfo.generated.h"

UCLASS()
class AKAUDIO_API UAkPlatformInfo : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	static TMap<FString, FWwiseSharedPlatformId> UnrealTargetNameToSharedPlatformId;
	static TMap<FString, UAkPlatformInfo*> UnrealNameToPlatformInfo;

	virtual FString GetWwiseBankPlatformName(const TArray<FString>& AvailableWwisePlatforms) const
	{
		if (AvailableWwisePlatforms.Contains(WwisePlatform))
		{
			return WwisePlatform;
		}
		return {};
	}

	static FWwiseSharedPlatformId GetSharedPlatformInfo(const FString& PlatformName)
	{
		if (UnrealTargetNameToSharedPlatformId.Contains(PlatformName))
		{
			return UnrealTargetNameToSharedPlatformId[PlatformName];
		}

		const auto ProjectDatabase = FWwiseProjectDatabase::Get();
		if (UNLIKELY(!ProjectDatabase))
		{
			UE_LOG(LogAkAudio, Warning, TEXT("ProjectDatabase is not initialized"));
			return {};
		}
		
		const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);
		auto Platforms = DataStructure.GetPlatforms();

		if (const auto* CurrentPlatformInfo = GetAkPlatformInfo(PlatformName))
		{
			TArray<FString> AvailablePlatforms;
			for (auto WwisePlatform : Platforms)
			{
				AvailablePlatforms.Add(WwisePlatform.GetPlatformName().ToString());
			}
			const FString WwisePlatformName = CurrentPlatformInfo->GetWwiseBankPlatformName(AvailablePlatforms);
			for (auto WwisePlatform : Platforms)
			{
				if (WwisePlatform.GetPlatformName().ToString() == WwisePlatformName)
				{
					UnrealTargetNameToSharedPlatformId.Add(PlatformName, WwisePlatform);
					return UnrealTargetNameToSharedPlatformId[PlatformName];
				}
			}
			UE_LOG(LogAkAudio, Warning, TEXT("Could not find parsed platform that matches %s"), *CurrentPlatformInfo->WwisePlatform);
			return {};
		}

		UE_LOG(LogAkAudio, Warning, TEXT("Could not find platform info for %s"), *PlatformName)
		return {};
	}
#endif

	static UAkPlatformInfo* GetAkPlatformInfo(const FString& PlatformName)
	{
		UAkPlatformInfo* RetVal = nullptr;
#if WITH_EDITORONLY_DATA
		auto** FoundInfo = UnrealNameToPlatformInfo.Find(PlatformName);
		RetVal = FoundInfo ? *FoundInfo : nullptr;
#endif
		if (!RetVal)
		{
			const FString PlatformInfoClassName = FString::Format(TEXT("Ak{0}PlatformInfo"), { *PlatformName });
#if UE_5_1_OR_LATER
			auto* PlatformInfoClass = UClass::TryFindTypeSlow<UClass>(*PlatformInfoClassName);
#else
			auto* PlatformInfoClass = FindObject<UClass>(ANY_PACKAGE, *PlatformInfoClassName);
#endif
			if (PlatformInfoClass)
			{
				RetVal = PlatformInfoClass->GetDefaultObject<UAkPlatformInfo>();
			}
		}

		return RetVal;
	}

	FString WwisePlatform;
	FString Architecture;
	FString LibraryFileNameFormat;
	FString DebugFileNameFormat;
	bool bSupportsUPL = false;
	bool bUsesStaticLibraries = false;
	bool bForceReleaseConfig = false;
};
