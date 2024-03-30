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

#include "Wwise/WwiseLanguageId.h"
#include "Wwise/CookedData/WwiseLanguageCookedData.h"
#include "WwiseSharedLanguageId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseSharedLanguageId
{
	GENERATED_BODY()

	static const FWwiseSharedLanguageId Sfx;

	TSharedPtr<FWwiseLanguageId> Language;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	EWwiseLanguageRequirement LanguageRequirement = EWwiseLanguageRequirement::SFX;

	FWwiseSharedLanguageId();
	FWwiseSharedLanguageId(int32 InLanguageId, const FName& InLanguageName, EWwiseLanguageRequirement InLanguageRequirement);
	FWwiseSharedLanguageId(const FWwiseLanguageId& InLanguageId, EWwiseLanguageRequirement InLanguageRequirement);
	~FWwiseSharedLanguageId();

	int32 GetLanguageId() const
	{
		return Language->LanguageId;
	}

	const FName& GetLanguageName() const
	{
		return Language->LanguageName;
	}

	bool operator==(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language == *Rhs.Language);
	}

	bool operator!=(const FWwiseSharedLanguageId& Rhs) const
	{
		return (Language.IsValid() != Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language != *Rhs.Language);
	}

	bool operator>=(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language >= *Rhs.Language);
	}

	bool operator>(const FWwiseSharedLanguageId& Rhs) const
	{
		return (Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language > *Rhs.Language);
	}

	bool operator<=(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && !Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language <= *Rhs.Language);
	}

	bool operator<(const FWwiseSharedLanguageId& Rhs) const
	{
		return (!Language.IsValid() && Rhs.Language.IsValid())
			|| (Language.IsValid() && Rhs.Language.IsValid() && *Language < *Rhs.Language);
	}
};

inline uint32 GetTypeHash(const FWwiseSharedLanguageId& Id)
{
	return GetTypeHash(Id.Language->LanguageId);
}
