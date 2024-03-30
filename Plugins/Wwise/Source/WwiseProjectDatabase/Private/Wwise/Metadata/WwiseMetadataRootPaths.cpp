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

#include "Wwise/Metadata/WwiseMetadataRootPaths.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"

FWwiseMetadataRootPaths::FWwiseMetadataRootPaths()
{
	UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Using default FWwiseMetadataRootPaths"));
}

FWwiseMetadataRootPaths::FWwiseMetadataRootPaths(FWwiseMetadataLoader& Loader) :
	ProjectRoot(Loader.GetString(this, TEXT("ProjectRoot"))),
	SourceFilesRoot(Loader.GetString(this, TEXT("SourceFilesRoot"))),
	SoundBanksRoot(Loader.GetString(this, TEXT("SoundBanksRoot"))),
	ExternalSourcesInputFile(Loader.GetString(this, TEXT("ExternalSourcesInputFile"))),
	ExternalSourcesOutputRoot(Loader.GetString(this, TEXT("ExternalSourcesOutputRoot")))
{
	Loader.LogParsed(TEXT("RootPaths"));
}
