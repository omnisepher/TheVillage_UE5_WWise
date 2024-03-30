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

#include "WwiseLanguageCookedData.generated.h"

UENUM(BlueprintType)
enum class EWwiseLanguageRequirement : uint8
{
	IsDefault,
	IsOptional,
	SFX
};

USTRUCT(BlueprintType)
struct WWISEFILEHANDLER_API FWwiseLanguageCookedData
{
	GENERATED_BODY()

	static const FWwiseLanguageCookedData Sfx;

	/**
	 * @brief Short ID for the Language
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 LanguageId = 0;

	/**
	 * @brief Language name as set in Wwise
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName LanguageName;

	/**
	 * @brief Is this language the default in Wwise
	*/
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	EWwiseLanguageRequirement LanguageRequirement = EWwiseLanguageRequirement::SFX;

	FWwiseLanguageCookedData();
	FWwiseLanguageCookedData(int32 LangId, const FName& LangName, EWwiseLanguageRequirement LangRequirement);

	void Serialize(FArchive& Ar);

	FName GetLanguageName() const { return LanguageName; }
	int32 GetLanguageId() const { return LanguageId; }
};

inline uint32 GetTypeHash(const FWwiseLanguageCookedData& InCookedData)
{
	return HashCombine(GetTypeHash(InCookedData.LanguageId), GetTypeHash(InCookedData.LanguageName));
}
inline bool operator==(const FWwiseLanguageCookedData& InLhs, const FWwiseLanguageCookedData& InRhs)
{
	return InLhs.LanguageId == InRhs.LanguageId && InLhs.LanguageName == InRhs.LanguageName;
}
inline bool operator!=(const FWwiseLanguageCookedData& InLhs, const FWwiseLanguageCookedData& InRhs)
{
	return (InLhs.LanguageId != InRhs.LanguageId) || (InLhs.LanguageName != InRhs.LanguageName);
}

