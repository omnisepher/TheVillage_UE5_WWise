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

#include "Wwise/Metadata/WwiseMetadataLoadable.h"

enum class EWwiseMetadataPluginLibType : uint32
{
	Source,
	Effect,
	AudioDevice,
	Metadata,
	Unknown = (uint32)-1
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginLibAttributes : public FWwiseMetadataLoadable
{
	FName LibName;
	uint32 LibId;
	EWwiseMetadataPluginLibType Type;
	FName DLL;
	FName StaticLib;

	FWwiseMetadataPluginLibAttributes(FWwiseMetadataLoader& Loader);

private:
	static EWwiseMetadataPluginLibType TypeFromString(const FName& TypeString);
};

struct WWISEPROJECTDATABASE_API FWwiseMetadataPluginLib : public FWwiseMetadataPluginLibAttributes
{
	FWwiseMetadataPluginLib(FWwiseMetadataLoader& Loader);
};
