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

#include "AkSettingsPerUser.h"

#include "AkAudioDevice.h"
#include "Misc/Paths.h"
#include "WwiseUnrealDefines.h"

#if WITH_EDITOR
#include "AkUnrealEditorHelper.h"
#endif
//////////////////////////////////////////////////////////////////////////
// UAkSettingsPerUser

UAkSettingsPerUser::UAkSettingsPerUser(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	WwiseWindowsInstallationPath.Path = FPlatformMisc::GetEnvironmentVariable(TEXT("WWISEROOT"));
	VisualizeRoomsAndPortals = false;
	bShowReverbInfo = true;
#endif
}

#if WITH_EDITOR
void UAkSettingsPerUser::PreEditChange(FProperty* PropertyAboutToChange)
{
	PreviousWwiseWindowsInstallationPath = WwiseWindowsInstallationPath.Path;
	PreviousWwiseMacInstallationPath = WwiseMacInstallationPath.FilePath;
	PreviousGeneratedSoundBanksFolder = RootOutputPathOverride.Path;

	Super::PreEditChange(PropertyAboutToChange);
}

void UAkSettingsPerUser::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WwiseWindowsInstallationPath))
	{
		AkUnrealEditorHelper::SanitizePath(WwiseWindowsInstallationPath.Path, PreviousWwiseWindowsInstallationPath, FText::FromString("Please enter a valid Wwise Installation path"));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WwiseMacInstallationPath))
	{
		AkUnrealEditorHelper::SanitizePath(WwiseMacInstallationPath.FilePath, PreviousWwiseMacInstallationPath, FText::FromString("Please enter a valid Wwise Authoring Mac executable path"));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, bAutoConnectToWAAPI))
	{
		OnAutoConnectToWaapiChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, WaapiTranslatorTimeout))
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice)
		{
			AkAudioDevice->SetLocalOutput();
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, RootOutputPathOverride))
	{
		bool bPathChanged = AkUnrealEditorHelper::SanitizeFolderPathAndMakeRelativeToContentDir(
			RootOutputPathOverride.Path, PreviousGeneratedSoundBanksFolder, 
			FText::FromString("Please enter a valid directory path"));

		if (bPathChanged)
		{
			OnGeneratedSoundBanksPathChanged.Broadcast();
		}
		OnGeneratedSoundBanksPathChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, VisualizeRoomsAndPortals))
	{
		OnShowRoomsPortalsChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettingsPerUser, bShowReverbInfo))
	{
		OnShowReverbInfoChanged.Broadcast();
	}

	if(RootOutputPathOverride.Path.IsEmpty())
	{
		RootOutputPathOverride = GeneratedSoundBanksFolderOverride_DEPRECATED;	
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAkSettingsPerUser::ToggleVisualizeRoomsAndPortals()
{
	VisualizeRoomsAndPortals = !VisualizeRoomsAndPortals;
	OnShowRoomsPortalsChanged.Broadcast();
}

void UAkSettingsPerUser::ToggleShowReverbInfo()
{
	bShowReverbInfo = !bShowReverbInfo;
	OnShowReverbInfoChanged.Broadcast();
}
#endif
