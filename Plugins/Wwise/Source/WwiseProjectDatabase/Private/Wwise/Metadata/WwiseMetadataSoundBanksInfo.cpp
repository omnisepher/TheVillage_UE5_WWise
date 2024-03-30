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

#include "Wwise/Metadata/WwiseMetadataSoundBanksInfo.h"
#include "Wwise/Metadata/WwiseMetadataRootPaths.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"

FWwiseMetadataSoundBanksInfoAttributes::FWwiseMetadataSoundBanksInfoAttributes(FWwiseMetadataLoader& Loader):
	Platform(Loader.GetString(this, TEXT("Platform"))),
	BasePlatform(Loader.GetString(this, TEXT("BasePlatform"))),
	SchemaVersion(Loader.GetUint32(this, TEXT("SchemaVersion"))),
	SoundBankVersion(Loader.GetUint32(this, TEXT("SoundBankVersion")))
{
	Loader.LogParsed(TEXT("SoundBanksInfoAttributes"));
}

FWwiseMetadataSoundBanksInfo::FWwiseMetadataSoundBanksInfo(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataSoundBanksInfoAttributes(Loader),
	RootPaths(Loader.GetObjectPtr<FWwiseMetadataRootPaths>(this, TEXT("RootPaths"))),
	DialogueEvents(Loader.GetArray<FWwiseMetadataDialogueEvent>(this, TEXT("DialogueEvents"))),
	SoundBanks(Loader.GetArray<FWwiseMetadataSoundBank>(this, TEXT("SoundBanks"))),
	FileHash(Loader.GetGuid(this, TEXT("FileHash")))
{
	Loader.LogParsed(TEXT("SoundBanksInfo"));
}

FWwiseMetadataSoundBanksInfo::~FWwiseMetadataSoundBanksInfo()
{
	if (RootPaths)
	{
		delete RootPaths;
		RootPaths = nullptr;
	}
}
