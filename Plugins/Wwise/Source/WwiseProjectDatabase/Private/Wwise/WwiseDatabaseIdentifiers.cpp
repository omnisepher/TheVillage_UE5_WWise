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

#include "Wwise/WwiseDatabaseIdentifiers.h"

uint32 GetTypeHash(const FWwiseDatabaseMediaIdKey& MediaId)
{
	return HashCombine(
		GetTypeHash(MediaId.MediaId),
		GetTypeHash(MediaId.SoundBankId));
}

uint32 GetTypeHash(const FWwiseDatabaseLocalizableIdKey& LocalizableId)
{
	return HashCombine(HashCombine(
		GetTypeHash(LocalizableId.Id),
		GetTypeHash(LocalizableId.SoundBankId)),
		GetTypeHash(LocalizableId.LanguageId));
}

uint32 GetTypeHash(const FWwiseDatabaseGroupValueKey& GroupId)
{
	return HashCombine(
		GetTypeHash(GroupId.GroupId),
		GetTypeHash(GroupId.Id));
}

uint32 GetTypeHash(const FWwiseDatabaseLocalizableGroupValueKey& LocalizableGroupValue)
{
	return HashCombine(
		GetTypeHash(LocalizableGroupValue.GroupValue),
		GetTypeHash(LocalizableGroupValue.LanguageId));
}

uint32 GetTypeHash(const FWwiseDatabaseLocalizableGuidKey& LocalizableGuid)
{
	return HashCombine(
		GetTypeHash(LocalizableGuid.Guid),
		GetTypeHash(LocalizableGuid.LanguageId));
}
uint32 GetTypeHash(const FWwiseDatabaseLocalizableNameKey& LocalizableName)
{
	return HashCombine(
		GetTypeHash(LocalizableName.Name),
		GetTypeHash(LocalizableName.LanguageId));
}
