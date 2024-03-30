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

#include "Wwise/Metadata/WwiseMetadataSettings.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/WwiseProjectDatabaseModule.h"

FWwiseMetadataSettings::FWwiseMetadataSettings() :
	bAutoSoundBankDefinition(false),
	bCopyLooseStreamedMediaFiles(false),
	bSubFoldersForGeneratedFiles(false),
	bRemoveUnusedGeneratedFiles(false),
	bSourceControlGeneratedFiles(false),
	bGenerateHeaderFile(false),
	bGenerateContentTxtFile(false),
	bGenerateMetadataXML(false),
	bGenerateMetadataJSON(false),
	bGenerateAllBanksMetadata(false),
	bGeneratePerBankMetadata(false),
	bUseSoundBankNames(false),
	bAllowExceedingMaxSize(false),
	bMaxAttenuationInfo(false),
	bEstimatedDurationInfo(false),
	bPrintObjectGuid(false),
	bPrintObjectPath(false)
{
	UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Using default Settings"));
}

FWwiseMetadataSettings::FWwiseMetadataSettings(FWwiseMetadataLoader& Loader) :
	bAutoSoundBankDefinition(Loader.GetBool(this, TEXT("AutoSoundBankDefinition"))),
	bCopyLooseStreamedMediaFiles(Loader.GetBool(this, TEXT("CopyLooseStreamedMediaFiles"))),
	bSubFoldersForGeneratedFiles(Loader.GetBool(this, TEXT("SubFoldersForGeneratedFiles"))),
	bRemoveUnusedGeneratedFiles(Loader.GetBool(this, TEXT("RemoveUnusedGeneratedFiles"))),
	bSourceControlGeneratedFiles(Loader.GetBool(this, TEXT("SourceControlGeneratedFiles"))),
	bGenerateHeaderFile(Loader.GetBool(this, TEXT("GenerateHeaderFile"))),
	bGenerateContentTxtFile(Loader.GetBool(this, TEXT("GenerateContentTxtFile"))),
	bGenerateMetadataXML(Loader.GetBool(this, TEXT("GenerateMetadataXML"))),
	bGenerateMetadataJSON(Loader.GetBool(this, TEXT("GenerateMetadataJSON"))),
	bGenerateAllBanksMetadata(Loader.GetBool(this, TEXT("GenerateAllBanksMetadata"))),
	bGeneratePerBankMetadata(Loader.GetBool(this, TEXT("GeneratePerBankMetadata"))),
	bUseSoundBankNames(Loader.GetBool(this, TEXT("UseSoundBankNames"))),
	bAllowExceedingMaxSize(Loader.GetBool(this, TEXT("AllowExceedingMaxSize"))),
	bMaxAttenuationInfo(Loader.GetBool(this, TEXT("MaxAttenuationInfo"))),
	bEstimatedDurationInfo(Loader.GetBool(this, TEXT("EstimatedDurationInfo"))),
	bPrintObjectGuid(Loader.GetBool(this, TEXT("PrintObjectGuid"))),
	bPrintObjectPath(Loader.GetBool(this, TEXT("PrintObjectPath")))
{
	Loader.LogParsed(TEXT("Settings"));
}
