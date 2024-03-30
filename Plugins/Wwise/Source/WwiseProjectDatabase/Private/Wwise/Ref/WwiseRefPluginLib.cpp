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

#include "Wwise/Ref/WwiseRefPluginLib.h"
#include "Wwise/Metadata/WwiseMetadataPluginInfo.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataPluginLib.h"

const TCHAR* const FWwiseRefPluginLib::NAME = TEXT("PluginLib");

const FWwiseMetadataPluginLib* FWwiseRefPluginLib::GetPluginLib() const
{
	const auto* PluginInfo = GetPluginInfo();
	if (UNLIKELY(!PluginInfo))
	{
		return nullptr;
	}
	const auto& PluginLibs = PluginInfo->PluginLibs;
	if (PluginLibs.IsValidIndex(PluginLibIndex))
	{
		return &PluginLibs[PluginLibIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get PluginLib index #%zu"), PluginLibIndex);
		return nullptr;
	}
}

uint32 FWwiseRefPluginLib::PluginLibId() const
{
	const auto* PluginLib = GetPluginLib();
	if (UNLIKELY(!PluginLib))
	{
		return 0;
	}
	return PluginLib->LibId;
}

FName FWwiseRefPluginLib::PluginLibName() const
{
	const auto* PluginLib = GetPluginLib();
	if (UNLIKELY(!PluginLib))
	{
		return {};
	}
	return PluginLib->LibName;
}

uint32 FWwiseRefPluginLib::Hash() const
{
	auto Result = FWwiseRefPluginInfo::Hash();
	Result = HashCombine(Result, GetTypeHash(PluginLibIndex));
	return Result;
}
