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

#include "AkAcousticTexture.h"
#include "Engine/EngineTypes.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "WwiseDefines.h"
#include "AkRtpc.h"
#include "Engine/DataTable.h"
#include "AkSettings.generated.h"

class UAkInitBank;
class UAkAcousticTexture;

DECLARE_MULTICAST_DELEGATE(FOnSoundBanksPathChangedDelegate);


/** Custom Collision Channel enum with an option to take the value from the Wwise Integration Settings (this follows a similar approach to that of EActorUpdateOverlapsMethod in Actor.h). */
UENUM(BlueprintType)
enum EAkCollisionChannel
{
	EAKCC_WorldStatic UMETA(DisplayName = "WorldStatic"),
	EAKCC_WorldDynamic UMETA(DisplayName = "WorldDynamic"),
	EAKCC_Pawn UMETA(DisplayName = "Pawn"),
	EAKCC_Visibility UMETA(DisplayName = "Visibility", TraceQuery = "1"),
	EAKCC_Camera UMETA(DisplayName = "Camera", TraceQuery = "1"),
	EAKCC_PhysicsBody UMETA(DisplayName = "PhysicsBody"),
	EAKCC_Vehicle UMETA(DisplayName = "Vehicle"),
	EAKCC_Destructible UMETA(DisplayName = "Destructible"),
	EAKCC_UseIntegrationSettingsDefault UMETA(DisplayName = "Use Integration Settings Default"), // Use the default value specified by Wwise Integration Settings.
};

UENUM()
enum class EAkUnrealAudioRouting
{
	Custom UMETA(DisplayName = "Default", ToolTip = "Custom Unreal audio settings set up by the developer"),
	Separate UMETA(DisplayName = "Both Wwise and Unreal audio", ToolTip = "Use default Unreal audio at the same time than Wwise SoundEngine (might be incompatible with some platforms)"),
	AudioLink UMETA(DisplayName = "Route through AudioLink [UE5.1+]", ToolTip = "Use WwiseAudioLink to route all Unreal audio sources to Wwise SoundEngine Inputs (requires Unreal Engine 5.1 or higher)"),
	AudioMixer UMETA(DisplayName = "Route through AkAudioMixer", ToolTip = "Use AkAudioMixer to route Unreal submixes to a Wwise SoundEngine Input"),
	EnableWwiseOnly UMETA(DisplayName = "Enable Wwise SoundEngine only", ToolTip = "Only use Wwise SoundEngine, and disable Unreal audio"),
	EnableUnrealOnly UMETA(DisplayName = "Enable Unreal Audio only", ToolTip = "Only use Unreal audio, and disable Wwise SoundEngine")
};

USTRUCT()
struct FAkGeometrySurfacePropertiesToMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "AkGeometry Surface Properties Map")
	TSoftObjectPtr<class UAkAcousticTexture> AcousticTexture = nullptr;

	UPROPERTY(EditAnywhere, DisplayName = "Transmission Loss", Category = "AkGeometry Surface Properties Map", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OcclusionValue = 1.f;

	bool operator==(const FAkGeometrySurfacePropertiesToMap& Rhs) const
	{
		if (OcclusionValue != Rhs.OcclusionValue)
		{
			return false;
		}
		if (!AcousticTexture.IsValid() != !Rhs.AcousticTexture.IsValid())
		{
			return false;
		}
		if (!AcousticTexture.IsValid())
		{
			return true;
		}
		return AcousticTexture->GetFName() == Rhs.AcousticTexture->GetFName();
	}
};

USTRUCT()
struct FWwiseGeometrySurfacePropertiesRow : public FTableRowBase
{
	GENERATED_BODY()

	FWwiseGeometrySurfacePropertiesRow() {}

	FWwiseGeometrySurfacePropertiesRow(TSoftObjectPtr<class UAkAcousticTexture> InAcousticTexture, float InTransmissionLoss)
	{
		AcousticTexture = InAcousticTexture;
		TransmissionLoss = InTransmissionLoss;
	}

	// The Acoustic Texture associated with this row's Physical Material.
	// A sound reflected on a surface is filtered according to the acoustic texture's absorption values.
	// When estimating the Reverb of an environment, acoustic textures applied to the surfaces are used to estimate the environment's Decay and HF Damping.
	// The default value is set to None. A surface with no acoustic texture is considered completely reflective.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry Surface Properties")
	TSoftObjectPtr<class UAkAcousticTexture> AcousticTexture = nullptr;

	// The Transmission Loss value associated with this row's Physical Material.
	// A sound going through a surface is filtered according to the amount of transmission loss.
	// A surface with a transmission loss value of 0 is considered transparent and lets sound pass through without any filtering. Sound cannot reflect on such surfaces.
	// The default value is set to 1, which is also the maximum possible value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry Surface Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TransmissionLoss = 1.f;
};

USTRUCT()
struct FWwiseDecayAuxBusRow : public FTableRowBase
{
	GENERATED_BODY()

	FWwiseDecayAuxBusRow() {}

	FWwiseDecayAuxBusRow(float InDecay, TSoftObjectPtr<class UAkAuxBus> InAuxBus)
	{
		Decay = InDecay;
		AuxBus = InAuxBus;
	}

	// The number of seconds it takes for the sound reverberation in an environment to decay by 60 dB.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reverb Assignment", meta = (ClampMin = "0.0"))
	float Decay = 0.f;

	// The Auxiliary Bus with a reverb effect to use for a chosen Decay value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reverb Assignment")
	TSoftObjectPtr<class UAkAuxBus> AuxBus = nullptr;
};

USTRUCT()
struct FAkAcousticTextureParams
{
	GENERATED_BODY()
	UPROPERTY()
	FVector4 AbsorptionValues = FVector4(FVector::ZeroVector, 0.0f);
	uint32 shortID = 0;

	float AbsorptionLow() const { return AbsorptionValues[0]; }
	float AbsorptionMidLow() const { return AbsorptionValues[1]; }
	float AbsorptionMidHigh() const { return AbsorptionValues[2]; }
	float AbsorptionHigh() const { return AbsorptionValues[3]; }

	TArray<float> AsTArray() const { return { AbsorptionLow(), AbsorptionMidLow(), AbsorptionMidHigh(), AbsorptionHigh() }; }
};

#define AK_MAX_AUX_PER_OBJ	4

DECLARE_EVENT(UAkSettings, ActivatedNewAssetManagement);
DECLARE_EVENT(UAkSettings, ReverbAssignmentChanged);
DECLARE_EVENT(UAkSettings, GlobalDecayAbsorptionChanged);
DECLARE_EVENT(UAkSettings, ReverbRTPCChanged);
DECLARE_EVENT_TwoParams(UAkSettings, SoundDataFolderChanged, const FString&, const FString&);
DECLARE_EVENT_OneParam(UAkSettings, AcousticTextureParamsChanged, const FGuid&)

UCLASS(config = Game, defaultconfig)
class AKAUDIO_API UAkSettings : public UObject
{
	GENERATED_BODY()

public:
	UAkSettings(const FObjectInitializer& ObjectInitializer);
	~UAkSettings();

	/**
	Converts between EAkCollisionChannel and ECollisionChannel. Returns Wwise Integration Settings default if CollisionChannel == UseIntegrationSettingsDefault. Otherwise, casts CollisionChannel to ECollisionChannel.
	*/
	static ECollisionChannel ConvertFitToGeomCollisionChannel(EAkCollisionChannel CollisionChannel);

	/**
	Converts between EAkCollisionChannel and ECollisionChannel. Returns Wwise Integration Settings default if CollisionChannel == UseIntegrationSettingsDefault. Otherwise, casts CollisionChannel to ECollisionChannel.
	*/
	static ECollisionChannel ConvertOcclusionCollisionChannel(EAkCollisionChannel CollisionChannel);

	// The maximum number of reverb auxiliary sends that will be simultaneously applied to a sound source
	// Reverbs from a Spatial Audio room will be active even if this maximum is reached.
	UPROPERTY(Config, EditAnywhere, DisplayName = "Max Simultaneous Reverb", Category="Reverb")
	uint8 MaxSimultaneousReverbVolumes = AK_MAX_AUX_PER_OBJ;

	// Wwise Project Path
	UPROPERTY(Config, EditAnywhere, Category="Installation", meta=(FilePathFilter="wproj", AbsolutePath))	
	FFilePath WwiseProjectPath;

	// Where the Sound Data will be generated in the Content Folder
	UPROPERTY()
 	FDirectoryPath WwiseSoundDataFolder;

	// The location of the folder where Wwise project metadata will be generated. This should be the same as the Root Output Path in the Wwise Project Settings.
	UPROPERTY(Config, EditAnywhere, Category="Installation", meta=( AbsolutePath))
	FDirectoryPath RootOutputPath;

	UPROPERTY(Config)
	FDirectoryPath GeneratedSoundBanksFolder_DEPRECATED;
	
	//Where wwise .bnk and .wem files will be copied to when staging files during cooking
	UPROPERTY(Config, EditAnywhere, Category = "Cooking", meta=(RelativeToGameContentDir))
	FDirectoryPath WwiseStagingDirectory = {TEXT("WwiseAudio")};

	//Used to track whether SoundBanks have been transferred to Wwise after migration to 2022.1 (or later)
	UPROPERTY(Config)
	bool bSoundBanksTransfered = false;

	//Used after migration to track whether assets have been re-serialized after migration to 2022.1 (or later)
	UPROPERTY(Config)
	bool bAssetsMigrated = false;

	//Used after migration to track whether project settings have been updated after migration to 2022.1 (or later)
	UPROPERTY(Config)
	bool bProjectMigrated = false;

	UPROPERTY(Config)
	bool bAutoConnectToWAAPI_DEPRECATED = false;

	// Default value for the Collision Channel when creating a new Ak Component.
	UPROPERTY(Config, EditAnywhere, Category = "Obstruction Occlusion", meta = (DisplayName = "DefaultCollisionChannel"))
	TEnumAsByte<ECollisionChannel> DefaultOcclusionCollisionChannel = ECollisionChannel::ECC_Visibility;
	
	// Default value for Collision Channel when fitting Ak Acoustic Portals and Ak Spatial Audio Volumes to surrounding geometry.
	UPROPERTY(Config, EditAnywhere, Category = "Fit To Geometry")
	TEnumAsByte<ECollisionChannel> DefaultFitToGeometryCollisionChannel = ECollisionChannel::ECC_WorldStatic;

	// PhysicalMaterial to AcousticTexture and Occlusion Value Map
	// @deprecated Use GeometrySurfacePropertiesTable instead.
	UPROPERTY(Config)
	TMap<TSoftObjectPtr<UPhysicalMaterial>, FAkGeometrySurfacePropertiesToMap> AkGeometryMap;

	// The default Acoustic Texture set on a surface of a Spatial Audio Volume actor when Fit to Geometry is used and no geometry is hit.
	// Default value is None, which indicates a completely reflective surface.
	UPROPERTY(Config, EditAnywhere, Category = "Geometry Surface Properties")
	TSoftObjectPtr<class UAkAcousticTexture> DefaultAcousticTexture = nullptr;

	// The default Transmission Loss value set on a surface of a Spatial Audio Volume actor when Fit to Geometry is used and no geometry is hit. The valid range is between 0 and 1.
	// The default value is 0, which indicates that sound can pass through the surface without any loss. A surface with 0 transmission loss is considered transparent. It disables any reflections and does not use the Acoustic Texture.
	UPROPERTY(Config, EditAnywhere, Category = "Geometry Surface Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultTransmissionLoss = 0.0f;

	// Table that associates Geometry Surface Properties (Acoustic Texture and Transmission Loss) with Physical Materials.
	// This table is used to retrieve the Geometry Surface Properties according to the Static Mesh's Physical Materials when using the AkGeometry component or when using Fit to Geometry with the AkSpatialAudioVolume.
	// Rows must be of type FWwiseGeometrySurfacePropertiesRow. We recommend that you do not add or remove rows.
	// Rows are updated when Physical Material assets are added to or removed from the project.
	// Rows are also updated when an Acoustic Texture with a name similar to a Physical Material is added to the project.
	UPROPERTY(Config, EditAnywhere, Category = "Geometry Surface Properties")
	TSoftObjectPtr<UDataTable> GeometrySurfacePropertiesTable;

	// Default surface absorption value to use when estimating environment Decay value. It is used for the decay estimations of environments without Acoustic Texture information. The default value is 0.5.
	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment", DisplayName = "Default Surface Absorption", meta = (ClampMin = 0.1f, ClampMax = 1.0f, UIMin = 0.1f, UIMax = 1.0f))
	float GlobalDecayAbsorption = .5f;

	// The default Auxiliary Bus to choose for Automatic Reverb Assignment.
	// Automatic Reverb Assignment can be enabled on Late Reverb components. When their Decay values exceed the highest Decay value in the Reverb Assignment Table, or if the table is empty or nonexistant, the default Auxiliary Bus is chosen.
	// This Auxiliary Bus must have a reverb effect.
	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment")
	TSoftObjectPtr<class UAkAuxBus> DefaultReverbAuxBus = nullptr;
	
	// RoomDecay to AuxBus Map.
	// @deprecated Use ReverbAssignmentTable instead.
	UPROPERTY(Config)
	TMap<float, TSoftObjectPtr<class UAkAuxBus>> EnvironmentDecayAuxBusMap;

	// Table that associates Auxiliary Busses with Reverb Decay values. Rows must be of type FWwiseDecayAuxBusRow.
	// The Decay value represents the number of seconds it takes for the sound reverberation in an environment to decay by 60 dB.
	// The Auxiliary Busses are Auxiliary Busses in Wwise Authoring that have reverb effects.
	// If Automatic Reverb Assignment is enabled on a Late Reverb component, its Decay value is compared to the table's Decay values. The chosen Auxiliary Bus is the one associated with the closest and highest Decay value in the table.
	// If the given Decay value exceeds the highest Decay value in the table, or if the table is empty or nonexistant, the Default Reverb Aux Bus is chosen.
	// Decay values are represented with floating point numbers. We recommend that consecutive Decay values differ by at least 0.01 to ensure the correct Auxiliary Bus is chosen for a given Decay value.
	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment")
	TSoftObjectPtr<UDataTable> ReverbAssignmentTable;

	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment|RTPCs")
	FString HFDampingName = "";

	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment|RTPCs")
	FString DecayEstimateName = "";

	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment|RTPCs")
	FString TimeToFirstReflectionName = "";

	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment|RTPCs")
	TSoftObjectPtr<UAkRtpc> HFDampingRTPC = nullptr;

	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment|RTPCs")
	TSoftObjectPtr<UAkRtpc> DecayEstimateRTPC = nullptr;

	UPROPERTY(Config, EditAnywhere, Category = "Reverb Assignment|RTPCs")
	TSoftObjectPtr<UAkRtpc> TimeToFirstReflectionRTPC = nullptr;

	// Input event associated with the Wwise Audio Input
	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	TSoftObjectPtr<class UAkAudioEvent> AudioInputEvent = nullptr;

	UPROPERTY(Config, meta = (Deprecated, DeprecationMessage = "AcousticTextureParamsMap is now an internal map."))
	TMap<FGuid, FAkAcousticTextureParams> AcousticTextureParamsMap_DEPRECATED;

	// When generating the event data, the media contained in switch containers will be split by state/switch value
	// and only loaded if the state/switch value are currently loaded
	UPROPERTY(Config, meta = (Deprecated, DeprecationMessage="Setting now exists for each AK Audio Event"))
	bool SplitSwitchContainerMedia = false;

	//Deprecated in 2022.1
	//Used in migration from previous versions
	UPROPERTY(Config)
	bool SplitMediaPerFolder= false;

	// Deprecated in 2022.1
	//Used in migration from previous versions
	UPROPERTY(Config)
	bool UseEventBasedPackaging= false;

	// Commit message that GenerateSoundBanksCommandlet will use
	UPROPERTY()
	FString CommandletCommitMessage = TEXT("Unreal Wwise Sound Data auto-generation");
	
	UPROPERTY(Config, EditAnywhere, Category = "Localization")
	TMap<FString, FString> UnrealCultureToWwiseCulture;

	// When an asset is dragged from the Wwise Browser, assets are created by default in this path.
	UPROPERTY(Config, EditAnywhere, Category = "Asset Creation")
	FString DefaultAssetCreationPath = "/Game/WwiseAudio";

	// The unique Init Bank for the Wwise project. This contains the basic information necessary for properly setting up the SoundEngine.
	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	TSoftObjectPtr<UAkInitBank> InitBank;

	// Routing Audio from Unreal Audio to Wwise Sound Engine
	UPROPERTY(Config, EditAnywhere, Category = "Initialization", DisplayName = "Unreal Audio Routing", meta=(ConfigRestartRequired=true))
	EAkUnrealAudioRouting AudioRouting = EAkUnrealAudioRouting::Custom;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", meta=(ConfigRestartRequired=true, EditCondition="AudioRouting == EAkUnrealAudioRouting::Custom"))
	bool bWwiseSoundEngineEnabled = true;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", meta=(ConfigRestartRequired=true, EditCondition="AudioRouting == EAkUnrealAudioRouting::Custom"))
	bool bWwiseAudioLinkEnabled = false;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", meta=(ConfigRestartRequired=true, EditCondition="AudioRouting == EAkUnrealAudioRouting::Custom"))
	bool bAkAudioMixerEnabled = false;

	UPROPERTY(Config)
	bool AskedToUseNewAssetManagement_DEPRECATED = false;

	UPROPERTY(Config)
	bool bEnableMultiCoreRendering_DEPRECATED = false;

	UPROPERTY(Config)
	bool MigratedEnableMultiCoreRendering = false;

	UPROPERTY(Config)
	bool FixupRedirectorsDuringMigration = false;

	UPROPERTY(Config)
	FDirectoryPath WwiseWindowsInstallationPath_DEPRECATED;

	UPROPERTY(Config)
	FFilePath WwiseMacInstallationPath_DEPRECATED;

	static FString DefaultSoundDataFolder;

	virtual void PostInitProperties() override;

	bool ReverbRTPCsInUse() const;
	bool DecayRTPCInUse() const;
	bool DampingRTPCInUse() const;
	bool PredelayRTPCInUse() const;

	bool GetAssociatedAcousticTexture(const UPhysicalMaterial* physMaterial, UAkAcousticTexture*& acousticTexture) const;
	bool GetAssociatedOcclusionValue(const UPhysicalMaterial* physMaterial, float& occlusionValue) const;

#if WITH_EDITOR
	bool UpdateGeneratedSoundBanksPath();
	bool UpdateGeneratedSoundBanksPath(FString Path);
	bool GeneratedSoundBanksPathExists() const;
	bool AreSoundBanksGenerated() const;
	void RefreshAcousticTextureParams() const;
#if AK_SUPPORT_WAAPI
	/** This needs to be called after the waapi client has been initialized, which happens after AkSettings is constructed. */
	void InitWaapiSync();
	/** Set the color of a UAkAcousticTexture asset using a color from the UnrealWwiseObjectColorPalette (this is the same as the 'dark theme' in Wwise Authoring). Send a colorIndex of -1 to use the 'unset' color. */
	void SetTextureColor(FGuid textureID, int colorIndex);
#endif
	void RemoveSoundDataFromAlwaysStageAsUFS(const FString& SoundDataPath);
	void RemoveSoundDataFromAlwaysCook(const FString& SoundDataPath);
	void EnsurePluginContentIsInAlwaysCook() const;

	AK_DEPRECATED(2023.1, "Use InitGeometrySurfacePropertiesTable instead.")
	void InitAkGeometryMap() { InitGeometrySurfacePropertiesTable(); }

	void InitGeometrySurfacePropertiesTable();
	void VerifyAndUpdateGeometrySurfacePropertiesTable();

	void InitReverbAssignmentTable();
#endif

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void OnReverbAssignmentTableChanged();
#endif

private:
#if WITH_EDITOR
	FString PreviousWwiseProjectPath;
	FString PreviousWwiseGeneratedSoundBankFolder;
	bool bTextureMapInitialized = false;
	TMap< UPhysicalMaterial*, UAkAcousticTexture* > TextureMapInternal;
	FAssetRegistryModule* AssetRegistryModule;

	void OnAssetAdded(const FAssetData& NewAssetData);
	void OnAssetRemoved(const struct FAssetData& AssetData);
	void UpdateGeometrySurfacePropertiesTable(const TArray<FAssetData>& PhysicalMaterials, const TArray<FAssetData>& AcousticTextureAssets);
	void FillGeometrySurfacePropertiesTable();

	void SanitizeProjectPath(FString& Path, const FString& PreviousPath, const FText& DialogMessage);
	void OnAudioRoutingUpdate();
	
	bool bGeometrySurfacePropertiesTableInitialized = false;

	FDelegateHandle ReverbAssignmentTableChangedHandle;

#if AK_SUPPORT_WAAPI
	TMap<FGuid, TArray<uint64>> WaapiTextureSubscriptions;
	TMap<FGuid, uint64> WaapiTextureColorSubscriptions;
	TMap<FGuid, uint64> WaapiTextureColorOverrideSubscriptions;
	FDelegateHandle WaapiProjectLoadedHandle;
	FDelegateHandle WaapiConnectionLostHandle;
#endif
#endif

	TMap<FGuid, FAkAcousticTextureParams> AcousticTextureParamsMap;

public:
	bool bRequestRefresh = false;
#if WITH_EDITOR
	const FAkAcousticTextureParams* GetTextureParams(const uint32& shortID) const;
	void SetAcousticTextureParams(const FGuid& textureID, const FAkAcousticTextureParams& params);
	void ClearTextureParamsMap();
#if AK_SUPPORT_WAAPI
	void WaapiProjectLoaded();
	void WaapiDisconnected();
	void RegisterWaapiTextureCallback(const FGuid& textureID);
	void UnregisterWaapiTextureCallback(const FGuid& textureID);
	void ClearWaapiTextureCallbacks();
	/** Use WAAPI to query the absorption params for a given texture and store them in the texture params map. */
	void UpdateTextureParams(const FGuid& textureID);
	/** Use WAAPI to query the color for a given texture and Update the corresponding UAkAcousticTexture asset. */
	void UpdateTextureColor(const FGuid& textureID);
#endif // AK_SUPPORT_WAAPI
	mutable AcousticTextureParamsChanged OnTextureParamsChanged;

#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
	ReverbAssignmentChanged OnReverbAssignmentChanged;
	GlobalDecayAbsorptionChanged OnGlobalDecayAbsorptionChanged;
	ReverbRTPCChanged OnReverbRTPCChanged;
	FOnSoundBanksPathChangedDelegate OnGeneratedSoundBanksPathChanged;
#endif

	/** Get the associated AuxBus for the given environment decay value.
	 * Return the AuxBus associated with the next highest decay value in the ReverbAssignmentTable, after the given value. 
	 */
	UAkAuxBus* GetAuxBusForDecayValue(float Decay);

	void GetAudioInputEvent(class UAkAudioEvent*& OutInputEvent);

};
