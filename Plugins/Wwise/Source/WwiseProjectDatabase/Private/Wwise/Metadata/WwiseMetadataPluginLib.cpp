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

#include "Wwise/Metadata/WwiseMetadataPluginLib.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataPluginLibAttributes::FWwiseMetadataPluginLibAttributes(FWwiseMetadataLoader& Loader) :
	LibName(Loader.GetString(this, TEXT("LibName"))),
	LibId(Loader.GetUint32(this, TEXT("LibId"))),
	Type(TypeFromString(Loader.GetString(this, TEXT("Type")))),
	DLL(Loader.GetString(this, TEXT("DLL"), EWwiseRequiredMetadata::Optional)),
	StaticLib(Loader.GetString(this, TEXT("StaticLib"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("PluginLibAttributes"), LibId, LibName);
}

EWwiseMetadataPluginLibType FWwiseMetadataPluginLibAttributes::TypeFromString(const FName& TypeString)
{
	if (TypeString == "Effect")
	{
		return EWwiseMetadataPluginLibType::Effect;
	}
	else if (TypeString == "Source")
	{
		return EWwiseMetadataPluginLibType::Source;
	}
	else if (TypeString == "AudioDevice")
	{
		return EWwiseMetadataPluginLibType::AudioDevice;
	}
	else if (TypeString == "Metadata")
	{
		return EWwiseMetadataPluginLibType::Metadata;
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataPluginLibAttributes: Unknown Type: %s"), *TypeString.ToString());
		return EWwiseMetadataPluginLibType::Unknown;
	}
}

FWwiseMetadataPluginLib::FWwiseMetadataPluginLib(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataPluginLibAttributes(Loader)
{
	Loader.LogParsed(TEXT("PluginLib"), LibId, LibName);
}
