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

#include "Wwise/Loaded/WwiseLoadedExternalSource.h"
#include "Wwise/Stats/ResourceLoader.h"

#include <inttypes.h>


FWwiseLoadedExternalSourceInfo::FWwiseLoadedExternalSourceInfo(const FWwiseExternalSourceCookedData& InExternalSource):
	ExternalSourceCookedData(InExternalSource)
{}

bool FWwiseLoadedExternalSourceInfo::FLoadedData::IsLoaded() const
{
	return bLoaded;
}

FString FWwiseLoadedExternalSourceInfo::GetDebugString() const
{
	return ExternalSourceCookedData.GetDebugString();
}

FWwiseLoadedExternalSourceInfo::FWwiseLoadedExternalSourceInfo(const FWwiseLoadedExternalSourceInfo& InOriginal):
	ExternalSourceCookedData(InOriginal.ExternalSourceCookedData)
{
}
