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

#include "Platforms/AkUEPlatform.h"
#include "AkAudioDevice.h"
#include "Interfaces/IProjectManager.h"
#include "ProjectDescriptor.h"

#if WITH_EDITOR
#include "PlatformInfo.h"
#include "Platforms/AkPlatformInfo.h"
TMap<FString, UAkPlatformInfo*> UAkPlatformInfo::UnrealNameToPlatformInfo;

TMap<FString, FWwiseSharedPlatformId> UAkPlatformInfo::UnrealTargetNameToSharedPlatformId;

TSet<FString> AkUnrealPlatformHelper::GetAllSupportedUnrealPlatforms()
{
	TSet<FString> SupportedPlatforms;
#if UE_5_0_OR_LATER
	for (const PlatformInfo::FTargetPlatformInfo* TargetPlatformInfo : PlatformInfo::GetPlatformInfoArray())
	{
		auto Info = *TargetPlatformInfo;
		FName PlatformInfoName = Info.Name;
		FString VanillaName = Info.VanillaInfo->Name.ToString();
#else
	for (const PlatformInfo::FPlatformInfo& Info : PlatformInfo::GetPlatformInfoArray())
	{
		FName PlatformInfoName = Info.PlatformInfoName;
		FString VanillaName = Info.VanillaPlatformName.ToString();
#endif
		bool bIsGame = Info.PlatformType == EBuildTargetType::Game;
		if (Info.IsVanilla() && bIsGame && (PlatformInfoName != TEXT("AllDesktop")))
		{
			VanillaName.RemoveFromEnd(TEXT("NoEditor"));
			SupportedPlatforms.Add(VanillaName);
		}
	}

	return SupportedPlatforms;

}

TSet<FString> AkUnrealPlatformHelper::GetAllSupportedUnrealPlatformsForProject()
{
	TSet<FString> SupportedPlatforms = GetAllSupportedUnrealPlatforms();
	IProjectManager& ProjectManager = IProjectManager::Get();
	auto* CurrentProject = ProjectManager.GetCurrentProject();
	if (CurrentProject && CurrentProject->TargetPlatforms.Num() > 0)
	{
		auto& TargetPlatforms = CurrentProject->TargetPlatforms;
		TSet<FString> AvailablePlatforms;
		for (const auto& TargetPlatform : TargetPlatforms)
		{
			FString PlatformName = TargetPlatform.ToString();
			PlatformName.RemoveFromEnd(TEXT("NoEditor"));
			AvailablePlatforms.Add(PlatformName);
		}

		auto Intersection = SupportedPlatforms.Intersect(AvailablePlatforms);
		if (Intersection.Num() > 0)
		{
			SupportedPlatforms = Intersection;
		}
	}

	return SupportedPlatforms;
}

TArray<TSharedPtr<FString>> AkUnrealPlatformHelper::GetAllSupportedWwisePlatforms(bool ProjectScope /* = false */)
{
	TArray<TSharedPtr<FString>> WwisePlatforms;

	TSet<FString> TemporaryWwisePlatformNames;
	auto UnrealPlatforms = ProjectScope ? GetAllSupportedUnrealPlatformsForProject() : GetAllSupportedUnrealPlatforms();
	for (const auto& AvailablePlatform : UnrealPlatforms)
	{
		FString SettingsClassName = FString::Format(TEXT("Ak{0}InitializationSettings"), { *AvailablePlatform });
#if UE_5_1_OR_LATER
		SettingsClassName = "/Script/AkAudio." + SettingsClassName;
		if (UClass::TryFindTypeSlow<UClass>(*SettingsClassName))
#else
		if (FindObject<UClass>(ANY_PACKAGE, *SettingsClassName))
#endif
		{
			TemporaryWwisePlatformNames.Add(AvailablePlatform);
		}
	}

	TemporaryWwisePlatformNames.Sort([](const FString& L, const FString& R) { return L.Compare(R) < 0; });

	for (const auto& WwisePlatformName : TemporaryWwisePlatformNames)
	{
		WwisePlatforms.Add(TSharedPtr<FString>(new FString(WwisePlatformName)));
	}

	return WwisePlatforms;
}

bool AkUnrealPlatformHelper::IsEditorPlatform(FString Platform)
{
	return Platform == "Windows" || Platform == "Mac" || Platform == "Linux";
}
#endif // WITH_EDITOR
