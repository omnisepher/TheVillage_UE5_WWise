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

#include "AkSettings.h"

#include "AkAcousticTexture.h"
#include "AkAuxBus.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAudioModule.h"
#include "AkSettingsPerUser.h"
#include "WwiseUnrealDefines.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "StringMatchAlgos/Array2D.h"
#include "StringMatchAlgos/StringMatching.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"

#if WITH_EDITOR
#include "AkAudioStyle.h"
#include "AssetToolsModule.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Platforms/AkUEPlatform.h"
#include "Settings/ProjectPackagingSettings.h"
#include "ISettingsEditorModule.h"
#include "ISourceControlModule.h"
#include "SourceControlHelpers.h"
#include "AkUnrealEditorHelper.h"

#if AK_SUPPORT_WAAPI
#include "AkWaapiClient.h"
#include "AkWaapiUtils.h"
#include "Async/Async.h"

bool WAAPIGetTextureParams(FGuid textureID, FAkAcousticTextureParams& params)
{
	auto waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr)
	{
		/* Construct the relevant WAAPI json fields. */
		TArray<TSharedPtr<FJsonValue>> fromID;
		fromID.Add(MakeShareable(new FJsonValueString(textureID.ToString(EGuidFormats::DigitsWithHyphensInBraces))));

		TSharedRef<FJsonObject> getArgsJson = FAkWaapiClient::CreateWAAPIGetArgumentJson(FAkWaapiClient::WAAPIGetFromOption::ID, fromID);

		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
		TArray<TSharedPtr<FJsonValue>> StructJsonArray;
		StructJsonArray.Add(MakeShareable(new FJsonValueString("id")));
		TArray<FString> absorptionStrings{ "@AbsorptionLow", "@AbsorptionMidLow", "@AbsorptionMidHigh", "@AbsorptionHigh" };
		for (int i = 0; i < absorptionStrings.Num(); ++i)
			StructJsonArray.Add(MakeShareable(new FJsonValueString(absorptionStrings[i])));

		options->SetArrayField(FAkWaapiClient::WAAPIStrings::RETURN, StructJsonArray);

		TSharedPtr<FJsonObject> outJsonResult;
		if (waapiClient->Call(ak::wwise::core::object::get, getArgsJson, options, outJsonResult, 500, false))
		{
			/* Get absorption values from WAAPI return json. */
			TArray<TSharedPtr<FJsonValue>> returnJson = outJsonResult->GetArrayField(FAkWaapiClient::WAAPIStrings::RETURN);
			if (returnJson.Num() > 0)
			{
				auto jsonObj = returnJson[0]->AsObject();
				if (jsonObj != nullptr)
				{
					TSharedPtr<FJsonObject> absorptionObject = nullptr;
					for (int i = 0; i < absorptionStrings.Num(); ++i)
					{
						params.AbsorptionValues[i] = (float)(jsonObj->GetNumberField(absorptionStrings[i])) / 100.0f;
					}
					return true;
				}
			}
		}
	}
	return false;
}

bool WAAPIGetObjectColorIndex(FGuid textureID, int& index)
{
	auto waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr)
	{
		/* Construct the relevant WAAPI json fields. */
		TArray<TSharedPtr<FJsonValue>> fromID;
		fromID.Add(MakeShareable(new FJsonValueString(textureID.ToString(EGuidFormats::DigitsWithHyphensInBraces))));
		TSharedRef<FJsonObject> getArgsJson = FAkWaapiClient::CreateWAAPIGetArgumentJson(FAkWaapiClient::WAAPIGetFromOption::ID, fromID);

		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
		TArray<TSharedPtr<FJsonValue>> StructJsonArray;
		StructJsonArray.Add(MakeShareable(new FJsonValueString("id")));
		StructJsonArray.Add(MakeShareable(new FJsonValueString("@Color")));

		options->SetArrayField(FAkWaapiClient::WAAPIStrings::RETURN, StructJsonArray);

		TSharedPtr<FJsonObject> outJsonResult;
		if (waapiClient->Call(ak::wwise::core::object::get, getArgsJson, options, outJsonResult, 500, false))
		{
			/* Get absorption values from WAAPI return json. */
			TArray<TSharedPtr<FJsonValue>> returnJson = outJsonResult->GetArrayField(FAkWaapiClient::WAAPIStrings::RETURN);
			if (returnJson.Num() > 0)
			{
				auto jsonObj = returnJson[0]->AsObject();
				if (jsonObj != nullptr)
				{
					index = (int)(jsonObj->GetNumberField("@Color"));
					return true;
				}
			}
		}
	}
	return false;
}

bool WAAPIGetObjectOverrideColor(FGuid textureID)
{
	auto waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr)
	{
		/* Construct the relevant WAAPI json fields. */
		TArray<TSharedPtr<FJsonValue>> fromID;
		fromID.Add(MakeShareable(new FJsonValueString(textureID.ToString(EGuidFormats::DigitsWithHyphensInBraces))));
		TSharedRef<FJsonObject> getArgsJson = FAkWaapiClient::CreateWAAPIGetArgumentJson(FAkWaapiClient::WAAPIGetFromOption::ID, fromID);

		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
		TArray<TSharedPtr<FJsonValue>> StructJsonArray;
		StructJsonArray.Add(MakeShareable(new FJsonValueString("id")));
		StructJsonArray.Add(MakeShareable(new FJsonValueString("@OverrideColor")));

		options->SetArrayField(FAkWaapiClient::WAAPIStrings::RETURN, StructJsonArray);

		TSharedPtr<FJsonObject> outJsonResult;
		if (waapiClient->Call(ak::wwise::core::object::get, getArgsJson, options, outJsonResult, 500, false))
		{
			/* Get absorption values from WAAPI return json. */
			TArray<TSharedPtr<FJsonValue>> returnJson = outJsonResult->GetArrayField(FAkWaapiClient::WAAPIStrings::RETURN);
			if (returnJson.Num() > 0)
			{
				auto jsonObj = returnJson[0]->AsObject();
				if (jsonObj != nullptr)
				{
					return jsonObj->GetBoolField("@OverrideColor");
				}
			}
		}
	}
	return false;
}
#endif // AK_SUPPORT_WAAPI
#endif // WITH_EDITOR

#define LOCTEXT_NAMESPACE "AkSettings"

//////////////////////////////////////////////////////////////////////////
// UAkSettings

namespace AkSettings_Helper
{
#if WITH_EDITOR
	void MigrateMultiCoreRendering(bool EnableMultiCoreRendering, const FString& PlatformName)
	{
		FString SettingsClassName = FString::Format(TEXT("Ak{0}InitializationSettings"), { *PlatformName });
#if UE_5_1_OR_LATER
		SettingsClassName = "/Script/AkAudio." + SettingsClassName;
		auto* SettingsClass = UClass::TryFindTypeSlow<UClass>(*SettingsClassName);
#else
		auto* SettingsClass = FindObject<UClass>(ANY_PACKAGE, *SettingsClassName);
#endif
		if (!SettingsClass)
		{
			return;
		}

		auto* MigrationFunction = SettingsClass->FindFunctionByName(TEXT("MigrateMultiCoreRendering"));
		auto* Settings = SettingsClass->GetDefaultObject();
		if (!MigrationFunction || !Settings)
		{
			return;
		}

		Settings->ProcessEvent(MigrationFunction, &EnableMultiCoreRendering);

		AkUnrealEditorHelper::SaveConfigFile(Settings);
	}
#endif

	void MatchAcousticTextureNamesToPhysMaterialNames(
		const TArray<FAssetData>& PhysicalMaterials,
		const TArray<FAssetData>& AcousticTextures,
		TArray<int32>& assignments)
	{
		uint32 NumPhysMat = (uint32)PhysicalMaterials.Num();
		uint32 NumAcousticTex = (uint32)AcousticTextures.Num();

		// Create a scores matrix
		Array2D<float> scores(NumPhysMat, NumAcousticTex, 0);

		for (uint32 i = 0; i < NumPhysMat; ++i)
		{
			TArray<bool> perfectObjectMatches;
			perfectObjectMatches.Init(false, NumAcousticTex);

			if (PhysicalMaterials[i].GetAsset())
			{
				FString physMaterialName = PhysicalMaterials[i].GetAsset()->GetName();

				if (physMaterialName.Len() == 0)
					continue;

				for (uint32 j = 0; j < NumAcousticTex; ++j)
				{
					// Skip objects for which we already found a perfect match
					if (perfectObjectMatches[j] == true)
						continue;

					if (AcousticTextures[j].GetAsset())
					{
						FString acousticTextureName = AcousticTextures[j].GetAsset()->GetName();

						if (acousticTextureName.Len() == 0)
							continue;

						// Calculate longest common substring length
						float lcs = LCS::GetLCSScore(
							physMaterialName.ToLower(),
							acousticTextureName.ToLower());

						scores(i, j) = lcs;

						if (FMath::IsNearlyEqual(lcs, 1.f))
						{
							assignments[i] = j;
							perfectObjectMatches[j] = true;
							break;
						}
					}
				}
			}
		}

		for (uint32 i = 0; i < NumPhysMat; ++i)
		{
			if (assignments[i] == -1)
			{
				float bestScore = 0.f;
				int32 matchedIdx = -1;
				for (uint32 j = 0; j < NumAcousticTex; ++j)
				{
					if (scores(i, j) > bestScore)
					{
						bestScore = scores(i, j);
						matchedIdx = j;
					}
				}
				if (bestScore >= 0.2f)
					assignments[i] = matchedIdx;
			}
		}
	}
}

FString UAkSettings::DefaultSoundDataFolder = TEXT("WwiseAudio");

UAkSettings::UAkSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WwiseSoundDataFolder.Path = DefaultSoundDataFolder;

	GlobalDecayAbsorption = 0.5f;

#if WITH_EDITOR
	AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	//register to asset modification delegates
	auto& AssetRegistry = AssetRegistryModule->Get();
	AssetRegistry.OnAssetAdded().AddUObject(this, &UAkSettings::OnAssetAdded);
	AssetRegistry.OnAssetRemoved().AddUObject(this, &UAkSettings::OnAssetRemoved);
#endif // WITH_EDITOR
}

UAkSettings::~UAkSettings()
{
#if WITH_EDITOR
#if AK_SUPPORT_WAAPI
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr)
	{
		if (WaapiProjectLoadedHandle.IsValid())
		{
			waapiClient->OnProjectLoaded.Remove(WaapiProjectLoadedHandle);
			WaapiProjectLoadedHandle.Reset();
		}
		if (WaapiConnectionLostHandle.IsValid())
		{
			waapiClient->OnConnectionLost.Remove(WaapiConnectionLostHandle);
			WaapiConnectionLostHandle.Reset();
		}
		ClearWaapiTextureCallbacks();
	}
#endif
#endif
}

ECollisionChannel UAkSettings::ConvertFitToGeomCollisionChannel(EAkCollisionChannel CollisionChannel)
{
	if (CollisionChannel != EAkCollisionChannel::EAKCC_UseIntegrationSettingsDefault)
		return (ECollisionChannel)CollisionChannel;

	const UAkSettings* AkSettings = GetDefault<UAkSettings>();

	if (AkSettings)
		return AkSettings->DefaultFitToGeometryCollisionChannel;

	return ECollisionChannel::ECC_WorldStatic;
}

ECollisionChannel UAkSettings::ConvertOcclusionCollisionChannel(EAkCollisionChannel CollisionChannel)
{
	if (CollisionChannel != EAkCollisionChannel::EAKCC_UseIntegrationSettingsDefault)
		return (ECollisionChannel)CollisionChannel;

	const UAkSettings* AkSettings = GetDefault<UAkSettings>();

	if (AkSettings)
		return AkSettings->DefaultOcclusionCollisionChannel;

	return ECollisionChannel::ECC_WorldStatic;
}

void UAkSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();

	if (AkSettingsPerUser)
	{
		bool didChanges = false;

		if (!WwiseWindowsInstallationPath_DEPRECATED.Path.IsEmpty())
		{
			AkSettingsPerUser->WwiseWindowsInstallationPath = WwiseWindowsInstallationPath_DEPRECATED;
			WwiseWindowsInstallationPath_DEPRECATED.Path.Reset();
			didChanges = true;
		}

		if (!WwiseMacInstallationPath_DEPRECATED.FilePath.IsEmpty())
		{
			AkSettingsPerUser->WwiseMacInstallationPath = WwiseMacInstallationPath_DEPRECATED;
			WwiseMacInstallationPath_DEPRECATED.FilePath.Reset();
			didChanges = true;
		}

		if (bAutoConnectToWAAPI_DEPRECATED)
		{
			AkSettingsPerUser->bAutoConnectToWAAPI = true;
			bAutoConnectToWAAPI_DEPRECATED = false;
			didChanges = true;
		}

		if (didChanges)
		{
			AkUnrealEditorHelper::SaveConfigFile(this);
			AkSettingsPerUser->SaveConfig();
		}
	}

	if (!MigratedEnableMultiCoreRendering)
	{
		MigratedEnableMultiCoreRendering = true;

		for (const auto& PlatformName : AkUnrealPlatformHelper::GetAllSupportedWwisePlatforms())
		{
			AkSettings_Helper::MigrateMultiCoreRendering(bEnableMultiCoreRendering_DEPRECATED, *PlatformName);
		}
	}
	if(RootOutputPath.Path.IsEmpty())
	{
		RootOutputPath = GeneratedSoundBanksFolder_DEPRECATED;
		AkUnrealEditorHelper::SaveConfigFile(this);
	}

#endif // WITH_EDITOR
}

#if WITH_EDITOR
void UAkSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	PreviousWwiseProjectPath = WwiseProjectPath.FilePath;
	PreviousWwiseGeneratedSoundBankFolder = RootOutputPath.Path;

	Super::PreEditChange(PropertyAboutToChange);
}

bool UAkSettings::UpdateGeneratedSoundBanksPath(FString Path)
{
	PreviousWwiseGeneratedSoundBankFolder = RootOutputPath.Path;
	RootOutputPath.Path = Path;
	return UpdateGeneratedSoundBanksPath();
}

bool UAkSettings::GeneratedSoundBanksPathExists() const
{
	return FPaths::DirectoryExists(WwiseUnrealHelper::GetSoundBankDirectory());
}

bool UAkSettings::AreSoundBanksGenerated() const
{
	return FPaths::FileExists(FPaths::Combine(WwiseUnrealHelper::GetSoundBankDirectory(), TEXT("ProjectInfo.json")));
}

void UAkSettings::RefreshAcousticTextureParams() const
{
	for (auto const& texture : AcousticTextureParamsMap)
	{
		OnTextureParamsChanged.Broadcast(texture.Key);
	}
}

bool UAkSettings::UpdateGeneratedSoundBanksPath()
{
	bool bPathChanged = AkUnrealEditorHelper::SanitizeFolderPathAndMakeRelativeToContentDir(
		RootOutputPath.Path, PreviousWwiseGeneratedSoundBankFolder, 
		FText::FromString("Please enter a valid directory path"));
			
	if (bPathChanged)
	{
		OnGeneratedSoundBanksPathChanged.Broadcast();
	}
	else
	{
		UE_LOG(LogAkAudio, Log, TEXT("AkSettings: The given GeneratedSoundBanks folder was the same as the previous one."));
	}
	return bPathChanged;
}

void UAkSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	ISettingsEditorModule& SettingsEditorModule = FModuleManager::GetModuleChecked<ISettingsEditorModule>("SettingsEditor");

	if ( PropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, MaxSimultaneousReverbVolumes))
	{
		MaxSimultaneousReverbVolumes = FMath::Clamp<uint8>( MaxSimultaneousReverbVolumes, 0, AK_MAX_AUX_PER_OBJ );
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if( AkAudioDevice )
		{
			AkAudioDevice->SetMaxAuxBus(MaxSimultaneousReverbVolumes);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, AudioRouting))
	{
		OnAudioRoutingUpdate();
	}
	
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, WwiseProjectPath))
	{
		SanitizeProjectPath(WwiseProjectPath.FilePath, PreviousWwiseProjectPath, FText::FromString("Please enter a valid Wwise project"));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, GeometrySurfacePropertiesTable))
	{
		InitGeometrySurfacePropertiesTable();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, ReverbAssignmentTable))
	{
		InitReverbAssignmentTable();
		if (ReverbAssignmentTable.IsValid() && !ReverbAssignmentTableChangedHandle.IsValid())
		{
			ReverbAssignmentTableChangedHandle = ReverbAssignmentTable->OnDataTableChanged().AddUObject(this, &UAkSettings::OnReverbAssignmentTableChanged);
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, GlobalDecayAbsorption))
	{
		OnGlobalDecayAbsorptionChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, DefaultReverbAuxBus))
	{
		OnReverbAssignmentChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, HFDampingName)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, DecayEstimateName)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, TimeToFirstReflectionName)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, HFDampingRTPC)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, DecayEstimateRTPC)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, TimeToFirstReflectionRTPC))
	{
		OnReverbRTPCChanged.Broadcast();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, WwiseStagingDirectory))
	{
		FAkAudioModule::AkAudioModuleInstance->UpdateWwiseResourceLoaderSettings();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAkSettings, RootOutputPath))
	{
		UpdateGeneratedSoundBanksPath();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAkSettings::UpdateGeometrySurfacePropertiesTable(
	const TArray<FAssetData>& PhysicalMaterialAssets,
	const TArray<FAssetData>& AcousticTextureAssets)
{
	auto GeometryTable = GeometrySurfacePropertiesTable.LoadSynchronous();
	if (GeometryTable == nullptr)
	{
		return;
	}

	TArray<int32> assignments;
	assignments.Init(-1, PhysicalMaterialAssets.Num());
	AkSettings_Helper::MatchAcousticTextureNamesToPhysMaterialNames(PhysicalMaterialAssets, AcousticTextureAssets, assignments);

	for (int i = 0; i < PhysicalMaterialAssets.Num(); i++)
	{
		auto physicalMaterial = Cast<UPhysicalMaterial>(PhysicalMaterialAssets[i].GetAsset());
		FWwiseGeometrySurfacePropertiesRow GeometrySurfaceProperties;

		FName Key = FName(physicalMaterial->GetPathName());
		auto GeometrySurfacePropertiesFound = GeometryTable->FindRow<FWwiseGeometrySurfacePropertiesRow>(Key, TEXT("Find Physical Material"), false);

		if (!GeometrySurfacePropertiesFound)
		{
			if (assignments[i] != -1)
			{
				int32 acousticTextureIdx = assignments[i];
				GeometrySurfaceProperties.AcousticTexture = Cast<UAkAcousticTexture>(AcousticTextureAssets[acousticTextureIdx].GetAsset());
				GeometryTable->AddRow(Key, GeometrySurfaceProperties);
			}
			else
			{
				GeometryTable->AddRow(Key, GeometrySurfaceProperties);
			}
		}
		else
		{
			if (assignments[i] != -1)
			{
				if (GeometrySurfacePropertiesFound->AcousticTexture == nullptr)
				{
					int32 acousticTextureIdx = assignments[i];
					GeometrySurfaceProperties.AcousticTexture = Cast<UAkAcousticTexture>(AcousticTextureAssets[acousticTextureIdx].GetAsset());
					GeometryTable->AddRow(Key, GeometrySurfaceProperties);
				}
			}
		}
	}
}

void UAkSettings::InitGeometrySurfacePropertiesTable()
{
	auto GeometryTable = GeometrySurfacePropertiesTable.LoadSynchronous();

	if (GeometryTable != nullptr)
	{
		if (GeometryTable->RowStruct->GetStructCPPName() != "FWwiseGeometrySurfacePropertiesRow")
		{
			UE_LOG(LogAkAudio, Log, TEXT("GeometrySurfacePropertiesTable cannot be assigned to %s. It must be assigned to a Data Table with FWwiseGeometrySurfacePropertiesRow type rows."), *GeometryTable->GetPathName());

			GeometryTable = nullptr;
			GeometrySurfacePropertiesTable = nullptr;
		}
	}

	if (GeometryTable == nullptr)
	{
		// find a valid GeometrySurfacePropertiesTable
		TArray<FAssetData> TableAssets;
#if UE_5_1_OR_LATER
		AssetRegistryModule->Get().GetAssetsByClass(UDataTable::StaticClass()->GetClassPathName(), TableAssets);
#else
		AssetRegistryModule->Get().GetAssetsByClass(UDataTable::StaticClass()->GetFName(), TableAssets);
#endif
		for (const auto& TableAsset : TableAssets)
		{
			auto Table = Cast<UDataTable>(TableAsset.GetAsset());
			// verify it has the correct structure
			if (Table && Table->RowStruct && Table->RowStruct->GetStructCPPName() == "FWwiseGeometrySurfacePropertiesRow")
			{
				UE_LOG(LogAkAudio, Log, TEXT("No GeometrySurfacePropertiesTable is assigned in the Integration Settings. Assigning %s."), *Table->GetPathName());
				GeometryTable = Table;
				GeometrySurfacePropertiesTable = TSoftObjectPtr<UDataTable>(Table);
				AkUnrealEditorHelper::SaveConfigFile(this);
				break;
			}
		}
	}

	if (GeometryTable == nullptr)
	{
		// create a new asset
		auto& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		auto NewTable = Cast<UDataTable>(AssetToolsModule.CreateAsset(TEXT("DefaultGeometrySurfacePropertiesTable"), DefaultAssetCreationPath, UDataTable::StaticClass(), nullptr));
		NewTable->RowStruct = FWwiseGeometrySurfacePropertiesRow::StaticStruct();
		GeometryTable = NewTable;
		GeometrySurfacePropertiesTable = TSoftObjectPtr<UDataTable>(NewTable);
		AkUnrealEditorHelper::SaveConfigFile(this);
	}

	if (GeometryTable == nullptr)
	{
		UE_LOG(LogAkAudio, Log, TEXT("No GeometrySurfacePropertiesTable is assigned in the Integration Settings. Couldn't find a corresponding asset or create a new one."));
		return;
	}

	// Migrate the old AkGeometryMap if it exists
	if (AkGeometryMap.Num() != 0)
	{
		UE_LOG(LogAkAudio, Log, TEXT("AkGeometryMap is deprecated. Its contents will be moved to the asset assigned to the GeometrySurfacePropertiesTable Integration Setting."));
		for (const auto& MapElement : AkGeometryMap)
		{
			auto PhysicalMaterial = MapElement.Key.LoadSynchronous();
			auto GeometrySurfacePropeties = MapElement.Value;
			
			GeometryTable->AddRow(FName(PhysicalMaterial->GetPathName()), FWwiseGeometrySurfacePropertiesRow(GeometrySurfacePropeties.AcousticTexture, GeometrySurfacePropeties.OcclusionValue));
		}
		VerifyAndUpdateGeometrySurfacePropertiesTable();
		AkGeometryMap.Empty();
		AkUnrealEditorHelper::SaveConfigFile(this);
	} 

	FillGeometrySurfacePropertiesTable();

	bGeometrySurfacePropertiesTableInitialized = true;
}

void UAkSettings::FillGeometrySurfacePropertiesTable()
{
	// Fill the table with existing physical materials and acoustic textures
	TArray<FAssetData> PhysicalMaterialAssets, AcousticTextureAssets;
#if UE_5_1_OR_LATER
	AssetRegistryModule->Get().GetAssetsByClass(UPhysicalMaterial::StaticClass()->GetClassPathName(), PhysicalMaterialAssets);
	AssetRegistryModule->Get().GetAssetsByClass(UAkAcousticTexture::StaticClass()->GetClassPathName(), AcousticTextureAssets);
#else
	AssetRegistryModule->Get().GetAssetsByClass(UPhysicalMaterial::StaticClass()->GetFName(), PhysicalMaterialAssets);
	AssetRegistryModule->Get().GetAssetsByClass(UAkAcousticTexture::StaticClass()->GetFName(), AcousticTextureAssets);
#endif
	UpdateGeometrySurfacePropertiesTable(PhysicalMaterialAssets, AcousticTextureAssets);
}

void UAkSettings::VerifyAndUpdateGeometrySurfacePropertiesTable()
{
	// do not allow rows with invalid physical materials
	TSet<FName> ToRemove;

	auto GeometryTable = GeometrySurfacePropertiesTable.LoadSynchronous();
	if (GeometryTable == nullptr)
	{
		return;
	}

	GeometryTable->ForeachRow<FWwiseGeometrySurfacePropertiesRow>("Verify GeometrySurfacePropertiesTable contents",
		[this, &ToRemove](const FName& Key, const FWwiseGeometrySurfacePropertiesRow& Value)
		{
#if UE_5_1_OR_LATER
			auto PhysicalMaterialAsset = AssetRegistryModule->Get().GetAssetByObjectPath(FSoftObjectPath(Key.ToString()));
#else
			auto PhysicalMaterialAsset = AssetRegistryModule->Get().GetAssetByObjectPath(Key);
#endif
			if (PhysicalMaterialAsset == nullptr)
			{
				ToRemove.Add(Key);
			}
		}
	);

	for (const auto& Key : ToRemove)
	{
		UE_LOG(LogAkAudio, Log, TEXT("GeometrySurfacePropertiesTable: Invalid row '%s' will be removed."), *Key.ToString());
		GeometryTable->RemoveRow(Key);
	}

	FillGeometrySurfacePropertiesTable();
}

void UAkSettings::SetAcousticTextureParams(const FGuid& textureID, const FAkAcousticTextureParams& params)
{
	if (AcousticTextureParamsMap.Contains(textureID))
		AcousticTextureParamsMap[textureID] = params;
	else
		AcousticTextureParamsMap.Add(textureID, params);

#if AK_SUPPORT_WAAPI
	RegisterWaapiTextureCallback(textureID);
#endif
}

void UAkSettings::ClearTextureParamsMap()
{
	AcousticTextureParamsMap.Empty();

#if AK_SUPPORT_WAAPI
	ClearWaapiTextureCallbacks();
#endif
}

#if AK_SUPPORT_WAAPI
void UAkSettings::WaapiProjectLoaded()
{
	TArray<FGuid> keys;
	AcousticTextureParamsMap.GetKeys(keys);
	for (auto key : keys)
	{
		UpdateTextureParams(key);
		UpdateTextureColor(key);
		RegisterWaapiTextureCallback(key);
	}
}

void UAkSettings::WaapiDisconnected()
{
	ClearWaapiTextureCallbacks();
}

void UAkSettings::RegisterWaapiTextureCallback(const FGuid& textureID)
{
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr && waapiClient->IsConnected())
	{
		auto absorptionCallback = WampEventCallback::CreateLambda([this](uint64_t id, TSharedPtr<FJsonObject> jsonObject)
		{
			const TSharedPtr<FJsonObject> itemObj = jsonObject->GetObjectField(WwiseWaapiHelper::OBJECT);
			if (itemObj != nullptr)
			{
				const FString itemIdString = itemObj->GetStringField(WwiseWaapiHelper::ID);
				FGuid itemID = FGuid::NewGuid();
				FGuid::ParseExact(itemIdString, EGuidFormats::DigitsWithHyphensInBraces, itemID);
				if (AcousticTextureParamsMap.Find(itemID) != nullptr)
				{
					AsyncTask(ENamedThreads::GameThread, [this, itemID]
					{
						UpdateTextureParams(itemID);
					});
				}
			}
		});


		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
		options->SetStringField(WwiseWaapiHelper::OBJECT, textureID.ToString(EGuidFormats::DigitsWithHyphensInBraces));

		TArray<FString> absorptionStrings{ "AbsorptionLow", "AbsorptionMidLow", "AbsorptionMidHigh", "AbsorptionHigh" };
		TSharedPtr<FJsonObject> jsonResult;
		TSharedPtr<FJsonObject> unsubscribeResult;
		bool unsubscribeNeeded = WaapiTextureSubscriptions.Find(textureID) != nullptr;
		TArray<uint64> subscriptionIDs{ 0,0,0,0 };
		for (int i = 0; i < absorptionStrings.Num(); ++i)
		{
			options->SetStringField(WwiseWaapiHelper::PROPERTY, absorptionStrings[i]);
			if (unsubscribeNeeded)
			{
				waapiClient->Unsubscribe(WaapiTextureSubscriptions[textureID][i], unsubscribeResult);
			}
			if (!waapiClient->Subscribe(ak::wwise::core::object::propertyChanged, options, absorptionCallback, subscriptionIDs[i], jsonResult))
			{
				UE_LOG(LogAkAudio, Warning, TEXT("AkSettings: WAAPI: Acoustic texture propertyChanged subscription failed."));
			}
		}
		WaapiTextureSubscriptions.Add(textureID, subscriptionIDs);


		auto colorCallback = WampEventCallback::CreateLambda([this](uint64_t id, TSharedPtr<FJsonObject> jsonObject)
		{
			const TSharedPtr<FJsonObject> itemObj = jsonObject->GetObjectField(WwiseWaapiHelper::OBJECT);
			if (itemObj != nullptr)
			{
				const FString itemIdString = itemObj->GetStringField(WwiseWaapiHelper::ID);
				FGuid itemID = FGuid::NewGuid();
				FGuid::ParseExact(itemIdString, EGuidFormats::DigitsWithHyphensInBraces, itemID);
				if (AcousticTextureParamsMap.Find(itemID) != nullptr)
				{
					AsyncTask(ENamedThreads::GameThread, [this, itemID]
					{
						UpdateTextureColor(itemID);
					});
				}
			}
		});

		options = MakeShareable(new FJsonObject());
		options->SetStringField(WwiseWaapiHelper::OBJECT, textureID.ToString(EGuidFormats::DigitsWithHyphensInBraces));
		unsubscribeNeeded = WaapiTextureColorSubscriptions.Find(textureID) != nullptr;
		uint64 subscriptionID = 0;
		options->SetStringField(WwiseWaapiHelper::PROPERTY, "Color");
		if (unsubscribeNeeded)
		{
			waapiClient->Unsubscribe(WaapiTextureColorSubscriptions[textureID], unsubscribeResult);
		}
		if (!waapiClient->Subscribe(ak::wwise::core::object::propertyChanged, options, colorCallback, subscriptionID, jsonResult))
		{
			UE_LOG(LogAkAudio, Warning, TEXT("AkSettings: WAAPI: Acoustic texture Color propertyChanged subscription failed."));
		}
		WaapiTextureColorSubscriptions.Add(textureID, subscriptionID);

		unsubscribeNeeded = WaapiTextureColorOverrideSubscriptions.Find(textureID) != nullptr;
		subscriptionID = 0;
		options->SetStringField(WwiseWaapiHelper::PROPERTY, "OverrideColor");
		if (unsubscribeNeeded)
		{
			waapiClient->Unsubscribe(WaapiTextureColorOverrideSubscriptions[textureID], unsubscribeResult);
		}
		if (!waapiClient->Subscribe(ak::wwise::core::object::propertyChanged, options, colorCallback, subscriptionID, jsonResult))
		{
			UE_LOG(LogAkAudio, Warning, TEXT("AkSettings: WAAPI: Acoustic texture OverrideColor propertyChanged subscription failed."));
		}
		WaapiTextureColorOverrideSubscriptions.Add(textureID, subscriptionID);
	}
}

void UAkSettings::UnregisterWaapiTextureCallback(const FGuid& textureID)
{
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr && waapiClient->IsConnected())
	{
		if (WaapiTextureSubscriptions.Find(textureID) != nullptr)
		{
			TSharedPtr<FJsonObject> unsubscribeResult;
			for (int i = 0; i < WaapiTextureSubscriptions[textureID].Num(); ++i)
				waapiClient->Unsubscribe(WaapiTextureSubscriptions[textureID][i], unsubscribeResult);
			WaapiTextureSubscriptions.Remove(textureID);
		}
		if (WaapiTextureColorSubscriptions.Find(textureID) != nullptr)
		{
			TSharedPtr<FJsonObject> unsubscribeResult;
			waapiClient->Unsubscribe(WaapiTextureColorSubscriptions[textureID], unsubscribeResult);
			WaapiTextureColorSubscriptions.Remove(textureID);
		}
		if (WaapiTextureColorOverrideSubscriptions.Find(textureID) != nullptr)
		{
			TSharedPtr<FJsonObject> unsubscribeResult;
			waapiClient->Unsubscribe(WaapiTextureColorOverrideSubscriptions[textureID], unsubscribeResult);
			WaapiTextureColorOverrideSubscriptions.Remove(textureID);
		}
	}
}

void UAkSettings::ClearWaapiTextureCallbacks()
{
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr && waapiClient->IsConnected())
	{
		for (auto it = WaapiTextureSubscriptions.CreateIterator(); it; ++it)
		{
			TSharedPtr<FJsonObject> unsubscribeResult;
			for (int i = 0; i < it.Value().Num(); ++i)
				waapiClient->Unsubscribe(it.Value()[i], unsubscribeResult);
		}
		for (auto it = WaapiTextureColorSubscriptions.CreateIterator(); it; ++it)
		{
			TSharedPtr<FJsonObject> unsubscribeResult;
			waapiClient->Unsubscribe(it.Value(), unsubscribeResult);
		}
		for (auto it = WaapiTextureColorOverrideSubscriptions.CreateIterator(); it; ++it)
		{
			TSharedPtr<FJsonObject> unsubscribeResult;
			waapiClient->Unsubscribe(it.Value(), unsubscribeResult);
		}
		WaapiTextureSubscriptions.Empty();
		WaapiTextureColorSubscriptions.Empty();
		WaapiTextureColorOverrideSubscriptions.Empty();
	}
}

void UAkSettings::UpdateTextureParams(const FGuid& textureID)
{
	WAAPIGetTextureParams(textureID, AcousticTextureParamsMap[textureID]);
	OnTextureParamsChanged.Broadcast(textureID);
}

void UAkSettings::UpdateTextureColor(const FGuid& textureID)
{
	if (!WAAPIGetObjectOverrideColor(textureID))
	{
		SetTextureColor(textureID, -1);
		return;
	}

	int colorIndex = 0;
	if (WAAPIGetObjectColorIndex(textureID, colorIndex))
	{
		SetTextureColor(textureID, colorIndex);
	}
}

void UAkSettings::SetTextureColor(FGuid textureID, int colorIndex)
{
	TArray<FAssetData> AcousticTextures;
#if UE_5_1_OR_LATER
	AssetRegistryModule->Get().GetAssetsByClass(UAkAcousticTexture::StaticClass()->GetClassPathName(), AcousticTextures);
#else
	AssetRegistryModule->Get().GetAssetsByClass(UAkAcousticTexture::StaticClass()->GetFName(), AcousticTextures);
#endif

	FLinearColor color = FAkAudioStyle::GetWwiseObjectColor(colorIndex);
	for (FAssetData& textureAsset : AcousticTextures)
	{
		if (UAkAcousticTexture* texture = Cast<UAkAcousticTexture>(textureAsset.GetAsset()))
		{
			if (texture->AcousticTextureInfo.WwiseGuid == textureID && texture->EditColor != color)
			{
				texture->Modify();
				texture->EditColor = color;
				break;
			}
		}
	}
}

#endif // AK_SUPPORT_WAAPI

void UAkSettings::OnAssetAdded(const FAssetData& NewAssetData)
{
	if (!bGeometrySurfacePropertiesTableInitialized)
	{
		return;
	}

#if UE_5_1_OR_LATER
	if (NewAssetData.AssetClassPath == UPhysicalMaterial::StaticClass()->GetClassPathName())
#else
	if (NewAssetData.AssetClass == UPhysicalMaterial::StaticClass()->GetFName())
#endif
	{
		if (auto physicalMaterial = Cast<UPhysicalMaterial>(NewAssetData.GetAsset()))
		{
			TArray<FAssetData> PhysicalMaterials, AcousticTextures;
			PhysicalMaterials.Add(NewAssetData);
#if UE_5_1_OR_LATER
			AssetRegistryModule->Get().GetAssetsByClass(UAkAcousticTexture::StaticClass()->GetClassPathName(), AcousticTextures);
#else
			AssetRegistryModule->Get().GetAssetsByClass(UAkAcousticTexture::StaticClass()->GetFName(), AcousticTextures);
#endif

			UpdateGeometrySurfacePropertiesTable(PhysicalMaterials, AcousticTextures);
		}
	} 
#if UE_5_1_OR_LATER
	else if (NewAssetData.AssetClassPath == UAkAcousticTexture::StaticClass()->GetClassPathName())
#else
	else if (NewAssetData.AssetClass == UAkAcousticTexture::StaticClass()->GetFName())
#endif
	{
		if (auto acousticTexture = Cast<UAkAcousticTexture>(NewAssetData.GetAsset()))
		{
			TArray<FAssetData> PhysicalMaterials, AcousticTextures;
#if UE_5_1_OR_LATER
			AssetRegistryModule->Get().GetAssetsByClass(UPhysicalMaterial::StaticClass()->GetClassPathName(), PhysicalMaterials);
#else
			AssetRegistryModule->Get().GetAssetsByClass(UPhysicalMaterial::StaticClass()->GetFName(), PhysicalMaterials);
#endif
			AcousticTextures.Add(NewAssetData);

			UpdateGeometrySurfacePropertiesTable(PhysicalMaterials, AcousticTextures);
			FAkAcousticTextureParams params;
			bool paramsExist = AcousticTextureParamsMap.Contains(acousticTexture->AcousticTextureInfo.WwiseGuid);
			if (paramsExist)
			{
				params = *AcousticTextureParamsMap.Find(acousticTexture->AcousticTextureInfo.WwiseGuid);
				params.shortID = acousticTexture->AcousticTextureInfo.WwiseShortId;
			}
#if AK_SUPPORT_WAAPI
			bool paramsSet = WAAPIGetTextureParams(acousticTexture->AcousticTextureInfo.WwiseGuid, params);
			if (paramsSet && !paramsExist)
				AcousticTextureParamsMap.Add(acousticTexture->AcousticTextureInfo.WwiseGuid, params);
			RegisterWaapiTextureCallback(acousticTexture->AcousticTextureInfo.WwiseGuid);
			int colorIndex = -1;
			if (WAAPIGetObjectColorIndex(acousticTexture->AcousticTextureInfo.WwiseGuid, colorIndex))
			{
				acousticTexture->EditColor = FAkAudioStyle::GetWwiseObjectColor(colorIndex);
			}
#endif
		}
	}
}

void UAkSettings::OnAssetRemoved(const struct FAssetData& AssetData)
{
#if UE_5_1_OR_LATER
	if (AssetData.AssetClassPath == UPhysicalMaterial::StaticClass()->GetClassPathName())
#else
	if (AssetData.AssetClass == UPhysicalMaterial::StaticClass()->GetFName())
#endif
	{
		if (auto physicalMaterial = Cast<UPhysicalMaterial>(AssetData.GetAsset()))
		{
			auto GeometryTable = GeometrySurfacePropertiesTable.LoadSynchronous();
			if (GeometryTable != nullptr)
		{
				GeometryTable->RemoveRow(FName(physicalMaterial->GetPathName()));
			}
		}
	}
#if UE_5_1_OR_LATER
	else if(AssetData.AssetClassPath == UAkAcousticTexture::StaticClass()->GetClassPathName())
#else
	else if(AssetData.AssetClass == UAkAcousticTexture::StaticClass()->GetFName())
#endif
	{
		if(auto acousticTexture = Cast<UAkAcousticTexture>(AssetData.GetAsset()))
		{
			AcousticTextureParamsMap.Remove(acousticTexture->AcousticTextureInfo.WwiseGuid);
#if AK_SUPPORT_WAAPI
			UnregisterWaapiTextureCallback(acousticTexture->AcousticTextureInfo.WwiseGuid);
#endif
		}
	}
}

#if AK_SUPPORT_WAAPI
void UAkSettings::InitWaapiSync()
{
	FAkWaapiClient* waapiClient = FAkWaapiClient::Get();
	if (waapiClient != nullptr)
	{
		if (waapiClient->IsProjectLoaded())
			WaapiProjectLoaded();
		WaapiProjectLoadedHandle = waapiClient->OnProjectLoaded.AddLambda([this]()
			{
				WaapiProjectLoaded();
			});
		WaapiConnectionLostHandle = waapiClient->OnConnectionLost.AddLambda([this]()
			{
				WaapiDisconnected();
			});
	}
}
#endif

void UAkSettings::EnsurePluginContentIsInAlwaysCook() const
{
	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();

	bool packageSettingsNeedUpdate = false;

	TArray<FString> PathsToCheck = { TEXT("/Wwise/WwiseTree"), TEXT("/Wwise/WwiseTypes") };

	for (auto pathToCheck : PathsToCheck)
	{
		if (!PackagingSettings->DirectoriesToAlwaysCook.ContainsByPredicate([pathToCheck](FDirectoryPath PathInArray) { return PathInArray.Path == pathToCheck; }))
		{
			FDirectoryPath newPath;
			newPath.Path = pathToCheck;

			PackagingSettings->DirectoriesToAlwaysCook.Add(newPath);
			packageSettingsNeedUpdate = true;
		}
	}

	if (packageSettingsNeedUpdate)
	{
		AkUnrealEditorHelper::SaveConfigFile(PackagingSettings);
	}
}

void UAkSettings::RemoveSoundDataFromAlwaysCook(const FString& SoundDataPath)
{
	bool changed = false;

	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();

	for (int32 i = PackagingSettings->DirectoriesToAlwaysCook.Num() - 1; i >= 0; --i)
	{
		if (PackagingSettings->DirectoriesToAlwaysCook[i].Path == SoundDataPath)
		{
			PackagingSettings->DirectoriesToAlwaysCook.RemoveAt(i);
			changed = true;
			break;
		}
	}

	if (changed)
	{
		AkUnrealEditorHelper::SaveConfigFile(PackagingSettings);
	}
}

void UAkSettings::SanitizeProjectPath(FString& Path, const FString& PreviousPath, const FText& DialogMessage)
{
	WwiseUnrealHelper::TrimPath(Path);

	FString TempPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*Path);

	FText FailReason;
	if (!FPaths::ValidatePath(TempPath, &FailReason))
	{
		if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, FailReason))
		{
			Path = PreviousPath;
			return;
		}
	}

	auto ProjectDirectory = WwiseUnrealHelper::GetProjectDirectory();
	if (!FPaths::FileExists(TempPath))
	{
		// Path might be a valid one (relative to game) entered manually. Check that.
		TempPath = FPaths::ConvertRelativePathToFull(ProjectDirectory, Path);

		if (!FPaths::FileExists(TempPath))
		{
			if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::Ok, DialogMessage))
			{
				Path = PreviousPath;
				return;
			}
		}
	}

	// Make the path relative to the game dir
	FPaths::MakePathRelativeTo(TempPath, *ProjectDirectory);
	Path = TempPath;

	if (Path != PreviousPath)
	{
#if UE_4_26_OR_LATER
		auto WwiseBrowserTab = FGlobalTabmanager::Get()->TryInvokeTab(FName("WwiseBrowser"));
#else
		TSharedRef<SDockTab> WwiseBrowserTab = FGlobalTabmanager::Get()->InvokeTab(FName("WwiseBrowser"));
#endif
		bRequestRefresh = true;
	}
}

void UAkSettings::OnAudioRoutingUpdate()
{
	// Calculate what is expected
	bool bExpectedCustom = false;
	bool bExpectedSeparate = false;
	bool bExpectedUsingAudioMixer = false;
	bool bExpectedAudioModuleOverride = true;
	bool bExpectedWwiseSoundEngineEnabled = true;
	bool bExpectedWwiseAudioLinkEnabled = false;
	bool bExpectedAkAudioMixerEnabled = false;
	FString ExpectedAudioDeviceModuleName;
	FString ExpectedAudioMixerModuleName;
	switch (AudioRouting)
	{
	case EAkUnrealAudioRouting::Custom:
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("OnAudioRoutingUpdate: Setting for Custom"));
		bExpectedCustom = true;
		break;

	case EAkUnrealAudioRouting::Separate:
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("OnAudioRoutingUpdate: Setting for Separate"));
		bExpectedSeparate = true;
		bExpectedUsingAudioMixer = true;
		bExpectedAudioModuleOverride = false;
		break;

	case EAkUnrealAudioRouting::EnableWwiseOnly:
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("OnAudioRoutingUpdate: Setting for DisableUnreal"));
		bExpectedUsingAudioMixer = false;
		ExpectedAudioDeviceModuleName = TEXT("");
		ExpectedAudioMixerModuleName = TEXT("");
		break;

	case EAkUnrealAudioRouting::EnableUnrealOnly:
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("OnAudioRoutingUpdate: Setting for DisableWwise"));
		bExpectedSeparate = true;
		bExpectedUsingAudioMixer = true;
		bExpectedAudioModuleOverride = false;
		bExpectedWwiseSoundEngineEnabled = false;
		break;

	case EAkUnrealAudioRouting::AudioMixer:
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("OnAudioRoutingUpdate: Setting for AudioMixer"));
		bExpectedUsingAudioMixer = true;
		bExpectedAkAudioMixerEnabled = true;
		ExpectedAudioDeviceModuleName = TEXT("AkAudioMixer");
		ExpectedAudioMixerModuleName = TEXT("AkAudioMixer");
		break;

	case EAkUnrealAudioRouting::AudioLink:
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("OnAudioRoutingUpdate: Setting for AudioLink"));
		bExpectedSeparate = true;
		bExpectedUsingAudioMixer = true;
		bExpectedWwiseAudioLinkEnabled = true;
		bExpectedAudioModuleOverride = false;
		break;

	default:
		UE_LOG(LogAkAudio, Warning, TEXT("OnAudioRoutingUpdate: Unknown AudioRouting"));
		return;
	}

	//
	// Actually update the files
	//

	UE_LOG(LogAkAudio, Verbose, TEXT("OnAudioRoutingUpdate: Updating system settings."));

	{
		bWwiseSoundEngineEnabled = bExpectedWwiseSoundEngineEnabled;
		UE_LOG(LogAkAudio, Log, TEXT("OnAudioRoutingUpdate: Wwise SoundEngine Enabled: %s"), bExpectedWwiseSoundEngineEnabled ? TEXT("true") : TEXT("false"));

		bWwiseAudioLinkEnabled = bExpectedWwiseAudioLinkEnabled;
		UE_LOG(LogAkAudio, Log, TEXT("OnAudioRoutingUpdate: Wwise AudioLink Enabled: %s"), bExpectedWwiseAudioLinkEnabled ? TEXT("true") : TEXT("false"));

		bAkAudioMixerEnabled = bExpectedAkAudioMixerEnabled;
		UE_LOG(LogAkAudio, Log, TEXT("OnAudioRoutingUpdate: Wwise AudioMixer Enabled: %s"), bExpectedAkAudioMixerEnabled ? TEXT("true") : TEXT("false"));
#if UE_5_0_OR_LATER
		TryUpdateDefaultConfigFile();
#else
		UpdateDefaultConfigFile();
#endif
	}

	TArray<FString> IniPlatformNames;

#if UE_5_0_OR_LATER
	for (const auto& PlatformInfo : FDataDrivenPlatformInfoRegistry::GetAllPlatformInfos())
	{
		if (!PlatformInfo.Value.bIsFakePlatform)
		{
			IniPlatformNames.Add(PlatformInfo.Value.IniPlatformName.ToString());
		}
	}
#else
	for (const auto& Platform : GetTargetPlatformManagerRef().GetTargetPlatforms())
	{
		IniPlatformNames.Add(Platform->IniPlatformName());
	}
#endif
	for (const auto& IniPlatformName : IniPlatformNames)
	{
		const auto RelativePlatformEnginePath = FString::Printf(TEXT("%s/%sEngine.ini"), *IniPlatformName, *IniPlatformName);
		auto PlatformEnginePath = FString::Printf(TEXT("%s%s"), *FPaths::SourceConfigDir(), *RelativePlatformEnginePath);

#if UE_5_1_OR_LATER
		PlatformEnginePath = FConfigCacheIni::NormalizeConfigIniPath(PlatformEnginePath);
#else
		FPaths::RemoveDuplicateSlashes(PlatformEnginePath);
		PlatformEnginePath = FPaths::CreateStandardFilename(PlatformEnginePath);
#endif

		const FString FullPlatformEnginePath = FPaths::ConvertRelativePathToFull(PlatformEnginePath);

		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPlatformEnginePath))
		{
			FText ErrorMessage;

			if (ISourceControlModule::Get().IsEnabled())
			{
				if (SourceControlHelpers::CheckoutOrMarkForAdd(FullPlatformEnginePath, FText::FromString(FullPlatformEnginePath), NULL, ErrorMessage))
				{
					ErrorMessage = FText();
				}
			}
			else if (!FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*FullPlatformEnginePath, false))
			{
				ErrorMessage = FText::Format(LOCTEXT("FailedToMakeWritable", "Could not make {0} writable."), FText::FromString(FullPlatformEnginePath));
			}

			if (!ErrorMessage.IsEmpty())
			{
				FNotificationInfo Info(ErrorMessage);
				Info.ExpireDuration = 3.0;

				FSlateNotificationManager::Get().AddNotification(Info);
				continue;
			}
		}

		if (bExpectedUsingAudioMixer)
		{
			UE_LOG(LogAkAudio, Log, TEXT("%s: Removing UseAudioMixer override"), *RelativePlatformEnginePath);
			GConfig->RemoveKey(TEXT("Audio"), TEXT("UseAudioMixer"), PlatformEnginePath);
		}
		else
		{
			UE_LOG(LogAkAudio, Log, TEXT("%s: Updating UseAudioMixer to: %s"), *RelativePlatformEnginePath, bExpectedUsingAudioMixer ? TEXT("true") : TEXT("false"));
			GConfig->SetBool(TEXT("Audio"), TEXT("UseAudioMixer"), bExpectedUsingAudioMixer, PlatformEnginePath);
		}

		if (bExpectedAudioModuleOverride)
		{
			UE_LOG(LogAkAudio, Log, TEXT("%s: Updating AudioDeviceModuleName: %s"), *RelativePlatformEnginePath, ExpectedAudioDeviceModuleName.IsEmpty() ? TEXT("[empty]") : *ExpectedAudioDeviceModuleName);
			UE_LOG(LogAkAudio, Log, TEXT("%s: Updating AudioMixerModuleName: %s"), *RelativePlatformEnginePath, ExpectedAudioMixerModuleName.IsEmpty() ? TEXT("[empty]") : *ExpectedAudioMixerModuleName);
			GConfig->SetString(TEXT("Audio"), TEXT("AudioDeviceModuleName"), *ExpectedAudioDeviceModuleName, PlatformEnginePath);
			GConfig->SetString(TEXT("Audio"), TEXT("AudioMixerModuleName"), *ExpectedAudioMixerModuleName, PlatformEnginePath);
		}
		else
		{
			UE_LOG(LogAkAudio, Log, TEXT("%s: Removing AudioDeviceModuleName override"), *RelativePlatformEnginePath);
			UE_LOG(LogAkAudio, Log, TEXT("%s: Removing AudioMixerModuleName override"), *RelativePlatformEnginePath);
			GConfig->RemoveKey(TEXT("Audio"), TEXT("AudioDeviceModuleName"), PlatformEnginePath);
			GConfig->RemoveKey(TEXT("Audio"), TEXT("AudioMixerModuleName"), PlatformEnginePath);
		}

		GConfig->Flush(false, PlatformEnginePath);
	}
}

void UAkSettings::RemoveSoundDataFromAlwaysStageAsUFS(const FString& SoundDataPath)
{
	bool changed = false;

	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();

	for (int32 i = PackagingSettings->DirectoriesToAlwaysStageAsUFS.Num() - 1; i >= 0; --i)
	{
		if (PackagingSettings->DirectoriesToAlwaysStageAsUFS[i].Path == SoundDataPath)
		{
			PackagingSettings->DirectoriesToAlwaysStageAsUFS.RemoveAt(i);
			changed = true;
			break;
		}
	}

	if (changed)
	{
		AkUnrealEditorHelper::SaveConfigFile(PackagingSettings);
	}
}

void UAkSettings::InitReverbAssignmentTable()
{
	auto DecayTable = ReverbAssignmentTable.LoadSynchronous();
	if (DecayTable && DecayTable->RowStruct)
	{
		if (DecayTable->RowStruct->GetStructCPPName() != "FWwiseDecayAuxBusRow")
		{
			UE_LOG(LogAkAudio, Log, TEXT("ReverbAssignmentTable cannot be assigned to %s. It must be assigned to a Data Table with FWwiseDecayAuxBusRow type rows."), *DecayTable->GetPathName());
			DecayTable = nullptr;
			ReverbAssignmentTable = nullptr;
		}
	}

	if (DecayTable == nullptr)
	{
		// find a valid ReverbAssignmentTable
		TArray<FAssetData> TableAssets;
#if UE_5_1_OR_LATER
		AssetRegistryModule->Get().GetAssetsByClass(UDataTable::StaticClass()->GetClassPathName(), TableAssets);
#else
		AssetRegistryModule->Get().GetAssetsByClass(UDataTable::StaticClass()->GetFName(), TableAssets);
#endif
		for (const auto& TableAsset : TableAssets)
		{
			auto Table = Cast<UDataTable>(TableAsset.GetAsset());
			// verify it has the correct structure
			if (Table && Table->RowStruct && Table->RowStruct->GetStructCPPName() == "FWwiseDecayAuxBusRow")
			{
				UE_LOG(LogAkAudio, Log, TEXT("No ReverbAssignmentTable is assigned in the Integration Settings. Assigning %s."), *Table->GetPathName());
				DecayTable = Table;
				ReverbAssignmentTable = TSoftObjectPtr<UDataTable>(Table);
				AkUnrealEditorHelper::SaveConfigFile(this);
				break;
			}
		}
	}

	if (DecayTable == nullptr)
	{
		// create a new asset
		auto& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		auto NewTable = Cast<UDataTable>(AssetToolsModule.CreateAsset(TEXT("DefaultReverbAssignmentTable"), DefaultAssetCreationPath, UDataTable::StaticClass(), nullptr));
		NewTable->RowStruct = FWwiseDecayAuxBusRow::StaticStruct();
		DecayTable = NewTable;
		ReverbAssignmentTable = TSoftObjectPtr<UDataTable>(NewTable);
		AkUnrealEditorHelper::SaveConfigFile(this);
	}

	if (DecayTable == nullptr)
	{
		UE_LOG(LogAkAudio, Log, TEXT("No ReverbAssignmentTable is assigned in the Integration Settings. Couldn't find a corresponding asset or create a new one."));
		return;
	}

	ReverbAssignmentTableChangedHandle = ReverbAssignmentTable->OnDataTableChanged().AddUObject(this, &UAkSettings::OnReverbAssignmentTableChanged);

	// Migrate the old EnvironmentDecayAuxBusMap if it exists
	if (EnvironmentDecayAuxBusMap.Num() != 0)
	{
		UE_LOG(LogAkAudio, Log, TEXT("EnvironmentDecayAuxBusMap is deprecated. Its contents will be moved to the asset assigned to the ReverbAssignmentTable Integration Setting."));

		for (const auto& MapElement : EnvironmentDecayAuxBusMap)
		{
			DecayTable->AddRow(FName(FString::SanitizeFloat(MapElement.Key)), FWwiseDecayAuxBusRow(MapElement.Key, MapElement.Value));
		}

		EnvironmentDecayAuxBusMap.Empty();
		AkUnrealEditorHelper::SaveConfigFile(this);
	}
}

void UAkSettings::OnReverbAssignmentTableChanged()
{
	OnReverbAssignmentChanged.Broadcast();
}

const FAkAcousticTextureParams* UAkSettings::GetTextureParams(const uint32& shortID) const
{
	for (auto it = AcousticTextureParamsMap.CreateConstIterator(); it; ++it)
	{
		if (it.Value().shortID == shortID)
			return AcousticTextureParamsMap.Find(it.Key());
	}
	return nullptr;
}

#endif // WITH_EDITOR

bool UAkSettings::ReverbRTPCsInUse() const
{
	return DecayRTPCInUse() || DampingRTPCInUse() || PredelayRTPCInUse();
}

bool UAkSettings::DecayRTPCInUse() const
{
	const bool validPath = !DecayEstimateRTPC.ToSoftObjectPath().ToString().IsEmpty();
	return validPath || !DecayEstimateName.IsEmpty();
}

bool UAkSettings::DampingRTPCInUse() const
{
	const bool validPath = !HFDampingRTPC.ToSoftObjectPath().ToString().IsEmpty();
	return validPath || !HFDampingName.IsEmpty();
}

bool UAkSettings::PredelayRTPCInUse() const
{
	const bool validPath = !TimeToFirstReflectionRTPC.ToSoftObjectPath().ToString().IsEmpty();
	return validPath || !TimeToFirstReflectionName.IsEmpty();
}

bool UAkSettings::GetAssociatedAcousticTexture(const UPhysicalMaterial* physMaterial, UAkAcousticTexture*& acousticTexture) const
{
	auto GeometryTable = GeometrySurfacePropertiesTable.LoadSynchronous();
	if (GeometryTable == nullptr)
	{
		return false;
	}

	FName Key = FName(physMaterial->GetPathName());
	auto GeometrySurfacePropertiesFound = GeometryTable->FindRow<FWwiseGeometrySurfacePropertiesRow>(Key, TEXT("Find Physical Material"), false);

	if (!GeometrySurfacePropertiesFound)
	{
		return false;
	}

	acousticTexture = GeometrySurfacePropertiesFound->AcousticTexture.LoadSynchronous();
	return true;
}

bool UAkSettings::GetAssociatedOcclusionValue(const UPhysicalMaterial* physMaterial, float& occlusionValue) const
{
	auto GeometryTable = GeometrySurfacePropertiesTable.LoadSynchronous();
	if (GeometryTable == nullptr)
	{
		return false;
	}

	FName Key = FName(physMaterial->GetPathName());
	auto GeometrySurfacePropertiesFound = GeometryTable->FindRow<FWwiseGeometrySurfacePropertiesRow>(Key, TEXT("Find Physical Material"), false);

	if (!GeometrySurfacePropertiesFound)
	{
		return false;
	}

	occlusionValue = GeometrySurfacePropertiesFound->TransmissionLoss;
	return true;
}

UAkAuxBus* UAkSettings::GetAuxBusForDecayValue(float Decay)
{
	auto DecayTable = ReverbAssignmentTable.LoadSynchronous();

	if (!DecayTable)
	{
		return DefaultReverbAuxBus.LoadSynchronous();
	}

	float MinKey = FLT_MAX;
	TSoftObjectPtr<UAkAuxBus> AuxBus;

	DecayTable->ForeachRow<FWwiseDecayAuxBusRow>("Get decay values",
		[Decay, &MinKey, &AuxBus](const FName& Key, const FWwiseDecayAuxBusRow& Value)
		{
			if (Decay <= Value.Decay && Value.Decay < MinKey)
			{
				MinKey = Value.Decay;
				AuxBus = Value.AuxBus;
			}
		}
	);

	if (MinKey == FLT_MAX)
	{
		return DefaultReverbAuxBus.LoadSynchronous();
	}
	else if (!AuxBus.IsNull())
	{
		auto* Result = AuxBus.LoadSynchronous();
		UE_CLOG(UNLIKELY(!Result), LogAkAudio, Warning, TEXT("UAkSettings::GetAuxBusForDecayValue: Could not load AuxBus for Decay Value (%f)"), MinKey);
		return Result;
	}
	else
	{
		return nullptr;
	}
}

void UAkSettings::GetAudioInputEvent(UAkAudioEvent*& OutInputEvent)
{
	OutInputEvent = AudioInputEvent.LoadSynchronous();
}

#undef LOCTEXT_NAMESPACE
