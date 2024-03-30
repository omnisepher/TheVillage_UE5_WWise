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

#include "Wwise/Metadata/WwiseMetadataMedia.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataMediaReference::FWwiseMetadataMediaReference(FWwiseMetadataLoader& Loader) :
	Id(Loader.GetUint32(this, TEXT("Id")))
{
	Loader.LogParsed(TEXT("MediaReference"), Id);
}

FWwiseMetadataMediaAttributes::FWwiseMetadataMediaAttributes(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataMediaReference(Loader),
	Language(Loader.GetString(this, TEXT("Language"))),
	bStreaming(Loader.GetBool(this, TEXT("Streaming"))),
	Location(LocationFromString(Loader.GetString(this, TEXT("Location")))),
	bUsingReferenceLanguage(Loader.GetBool(this, TEXT("UsingReferenceLanguage"), EWwiseRequiredMetadata::Optional)),
	Align(Loader.GetUint32(this, TEXT("Align"), EWwiseRequiredMetadata::Optional)),
	bDeviceMemory(Loader.GetBool(this, TEXT("DeviceMemory"), EWwiseRequiredMetadata::Optional))
{
	Loader.LogParsed(TEXT("MediaAttributes"), Id);
}

EWwiseMetadataMediaLocation FWwiseMetadataMediaAttributes::LocationFromString(const FName& LocationString)
{
	if (LocationString == "Memory")
	{
		return EWwiseMetadataMediaLocation::Memory;
	}
	else if (LocationString == "Loose")
	{
		return EWwiseMetadataMediaLocation::Loose;
	}
	else if (LocationString == "OtherBank")
	{
		return EWwiseMetadataMediaLocation::OtherBank;
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("FWwiseMetadataMediaAttributes: Unknown Location: %s"), *LocationString.ToString());
		return EWwiseMetadataMediaLocation::Unknown;
	}
}

FWwiseMetadataMedia::FWwiseMetadataMedia(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataMediaAttributes(Loader),
	ShortName(Loader.GetString(this, TEXT("ShortName"))),
	Path(Loader.GetString(this, TEXT("Path"), EWwiseRequiredMetadata::Optional)),
	CachePath(Loader.GetString(this, TEXT("CachePath"), EWwiseRequiredMetadata::Optional)),
	PrefetchSize(Loader.GetUint32(this, TEXT("PrefetchSize"), EWwiseRequiredMetadata::Optional))
{
	if (UNLIKELY(Path.IsNone() && Location == EWwiseMetadataMediaLocation::Loose))
	{
		Loader.Fail(TEXT("!Path+Location=Loose"));
	}
	else if (UNLIKELY(Path.IsNone() && Location == EWwiseMetadataMediaLocation::Memory && bStreaming))
	{
		Loader.Fail(TEXT("!Path+Streaming"));
	}
	else if (UNLIKELY(!Path.IsNone() && Location == EWwiseMetadataMediaLocation::Memory && !bStreaming))
	{
		Loader.Fail(TEXT("Path+Memory"));
	}
	Loader.LogParsed(TEXT("Media"), Id);
}
