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

#if WITH_EDITOR

#include "Engine/EngineTypes.h"
#include "Input/Reply.h"

DECLARE_DELEGATE_RetVal(FReply, FOnButtonClickedMigration);

namespace AkUnrealEditorHelper
{
	AKAUDIO_API void ShowEventBasedPackagingMigrationDialog(FOnButtonClickedMigration in_OnClickedYes, FOnButtonClickedMigration in_OnClickedNo);
	AKAUDIO_API void SanitizePath(FString& Path, const FString& PreviousPath, const FText& DialogMessage);
	AKAUDIO_API bool SanitizeFolderPathAndMakeRelativeToContentDir(FString& Path, const FString& PreviousPath, const FText& DialogMessage);

	AKAUDIO_API bool SaveConfigFile(UObject* ConfigObject);
	AKAUDIO_API void DeleteOldSoundBanks();
	
	AKAUDIO_API FString GetLegacySoundBankDirectory();
	AKAUDIO_API FString GetContentDirectory();
	AKAUDIO_API void DeleteLegacySoundBanks();

	extern AKAUDIO_API const TCHAR* LocalizedFolderName;
}
#endif
