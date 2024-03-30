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

class WWISEPROJECTDATABASE_API FWwiseRefSwitchGroup : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::SwitchGroup;
	struct FGlobalIdsMap;

	WwiseRefIndexType SwitchGroupIndex;

	FWwiseRefSwitchGroup() {}
	FWwiseRefSwitchGroup(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InSwitchGroupIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		SwitchGroupIndex(InSwitchGroupIndex)
	{}
	const FWwiseMetadataSwitchGroup* GetSwitchGroup() const;
	bool IsControlledByGameParameter() const;

	uint32 SwitchGroupId() const;
	FGuid SwitchGroupGuid() const;
	FName SwitchGroupName() const;
	FName SwitchGroupObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefSwitchGroup& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& SwitchGroupIndex == Rhs.SwitchGroupIndex;
	}
	bool operator!=(const FWwiseRefSwitchGroup& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefSwitchGroup::FGlobalIdsMap
{
	WwiseSwitchGroupGlobalIdsMap GlobalIdsMap;
};
