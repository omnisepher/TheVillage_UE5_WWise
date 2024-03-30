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

#include "Wwise/Ref/WwiseRefProjectInfo.h"

class WWISEPROJECTDATABASE_API FWwiseRefLanguage : public FWwiseRefProjectInfo
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Language;

	WwiseRefIndexType LanguageIndex;

	FWwiseRefLanguage() :
		LanguageIndex(INDEX_NONE)
	{}
	FWwiseRefLanguage(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InLanguageIndex) :
		FWwiseRefProjectInfo(InRootMediaRef, InJsonFilePath),
		LanguageIndex(InLanguageIndex)
	{}
	const FWwiseMetadataLanguage* GetLanguage() const;

	uint32 LanguageId() const;
	FGuid LanguageGuid() const;
	FName LanguageName() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefLanguage& Rhs) const
	{
		return FWwiseRefProjectInfo::operator==(Rhs)
			&& LanguageIndex == Rhs.LanguageIndex;
	}
	bool operator!=(const FWwiseRefLanguage& Rhs) const { return !operator==(Rhs); }
};
