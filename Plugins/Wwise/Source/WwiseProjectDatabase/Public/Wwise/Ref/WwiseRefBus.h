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

class WWISEPROJECTDATABASE_API FWwiseRefBus : public FWwiseRefSoundBank
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Bus;
	struct FGlobalIdsMap;

	WwiseRefIndexType BusIndex;

	FWwiseRefBus() {}
	FWwiseRefBus(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InSoundBankIndex, uint32 InLanguageId,
		WwiseRefIndexType InBusIndex) :
		FWwiseRefSoundBank(InRootMediaRef, InJsonFilePath, InSoundBankIndex, InLanguageId),
		BusIndex(InBusIndex)
	{}
	const FWwiseMetadataBus* GetBus() const;

	uint32 BusId() const;
	FGuid BusGuid() const;
	FName BusName() const;
	FName BusObjectPath() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefBus& Rhs) const
	{
		return FWwiseRefSoundBank::operator ==(Rhs)
			&& BusIndex == Rhs.BusIndex;
	}
	bool operator!=(const FWwiseRefBus& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefBus::FGlobalIdsMap
{
	WwiseBusGlobalIdsMap GlobalIdsMap;
};
