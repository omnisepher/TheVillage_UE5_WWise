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

#include "WwiseLanguageId.generated.h"

USTRUCT(BlueprintType)
struct WWISERESOURCELOADER_API FWwiseLanguageId
{
	GENERATED_BODY()

	static const FWwiseLanguageId Sfx;

	FWwiseLanguageId()
	{}
	FWwiseLanguageId(int32 InLanguageId, const FName& InLanguageName) :
		LanguageId(InLanguageId),
		LanguageName(InLanguageName)
	{
		check(!LanguageName.IsNone());
	}

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	int32 LanguageId = 0;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Wwise")
	FName LanguageName;

	bool operator==(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId == Rhs.LanguageId;
	}

	bool operator!=(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId != Rhs.LanguageId;
	}

	bool operator>=(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId >= Rhs.LanguageId;
	}

	bool operator>(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId > Rhs.LanguageId;
	}

	bool operator<=(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId <= Rhs.LanguageId;
	}

	bool operator<(const FWwiseLanguageId& Rhs) const
	{
		return LanguageId < Rhs.LanguageId;
	}
};

inline uint32 GetTypeHash(const FWwiseLanguageId& Id)
{
	return GetTypeHash(Id.LanguageId);
}
