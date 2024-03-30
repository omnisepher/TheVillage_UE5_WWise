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

#include "Wwise/Metadata/WwiseMetadataBus.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Metadata/WwiseMetadataPluginGroup.h"

FWwiseMetadataBusReference::FWwiseMetadataBusReference(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader)
{
	Loader.LogParsed(TEXT("BusReference"), Id, Name);
}

FWwiseMetadataBus::FWwiseMetadataBus(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBusReference(Loader),
	PluginRefs(Loader.GetObjectPtr<FWwiseMetadataPluginReferenceGroup>(this, TEXT("PluginRefs"))),
	AuxBusRefs(Loader.GetArray<FWwiseMetadataBusReference>(this, TEXT("AuxBusRefs"))),
	MaxAttenuation(Loader.GetFloat(this, TEXT("MaxAttenuation"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("Bus"), Id, Name);
}
