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

#include "Wwise/Ref/WwiseRefSoundBank.h"

class WWISEPROJECTDATABASE_API FWwiseRefGameParameter : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::GameParameter;
	struct FGlobalIdsMap;

	WwiseRefIndexType GameParameterIndex;

	FWwiseRefGameParameter() {}
	FWwiseRefGameParameter(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InGameParameterIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		GameParameterIndex(InGameParameterIndex)
	{}
	const FWwiseMetadataGameParameter* GetGameParameter() const;

	uint32 GameParameterId() const;
	FGuid GameParameterGuid() const;
	FName GameParameterName() const;
	FName GameParameterObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefGameParameter& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& GameParameterIndex == Rhs.GameParameterIndex;
	}
	bool operator!=(const FWwiseRefGameParameter& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefGameParameter::FGlobalIdsMap
{
	WwiseGameParameterGlobalIdsMap GlobalIdsMap;
};
