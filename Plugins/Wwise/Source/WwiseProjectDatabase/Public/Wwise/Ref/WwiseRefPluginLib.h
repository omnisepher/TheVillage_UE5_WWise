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

#include "Wwise/Ref/WwiseRefPluginInfo.h"

class WWISEPROJECTDATABASE_API FWwiseRefPluginLib : public FWwiseRefPluginInfo
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::PluginLib;
	struct FGlobalIdsMap;

	WwiseRefIndexType PluginLibIndex;

	FWwiseRefPluginLib() :
		PluginLibIndex(INDEX_NONE)
	{}
	FWwiseRefPluginLib(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		WwiseRefIndexType InPluginIndex) :
		FWwiseRefPluginInfo(InRootMediaRef, InJsonFilePath),
		PluginLibIndex(InPluginIndex)
	{}
	const FWwiseMetadataPluginLib* GetPluginLib() const;

	uint32 PluginLibId() const;
	FName PluginLibName() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
	bool operator==(const FWwiseRefPluginLib& Rhs) const
	{
		return FWwiseRefPluginInfo::operator==(Rhs)
			&& PluginLibIndex == Rhs.PluginLibIndex;
	}
	bool operator!=(const FWwiseRefPluginLib& Rhs) const { return !operator==(Rhs); }
};

struct WWISEPROJECTDATABASE_API FWwiseRefPluginLib::FGlobalIdsMap
{
	WwisePluginLibGlobalIdsMap GlobalIdsMap;
};
