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

#include "Components/TextRenderComponent.h"
#include "GameFramework/Volume.h"
#include "AkGameplayTypes.h"

#include "Wwise/AkPortalObstructionAndOcclusionService.h"

#if WITH_EDITOR
#include "AkSettings.h"
#endif

#include "AkAcousticPortal.generated.h"

class UAkRoomComponent;
class UAkLateReverbComponent;
class FAkEnvironmentIndex;

UCLASS(ClassGroup = Audiokinetic, hidecategories = (Advanced, Attachment, Volume), BlueprintType, meta = (BlueprintSpawnableComponent))
class AKAUDIO_API UAkPortalComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAkPortalComponent(const class FObjectInitializer& ObjectInitializer);

	/**
	 * Enables the portal. Emitters positioned in the AkRoomComponent in front of and behind the portal emit through it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkPortalComponent")
	void EnablePortal();

	/**
	 * Disables the portal. Emitters positioned in the AkRoomComponent in front of and behind the portal do not emit through it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkPortalComponent")
	void DisablePortal();

	/**
	 * Returns an AkAcousticPortalState, which represents current the state of the portal: Enabled or Disabled.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkPortalComponent")
	AkAcousticPortalState GetCurrentState() const;

	/**
	 * Returns a floating point number between 0 and 1 that represents the occlusion value applied to the portal. A value of 0 indicates that the portal is not occluded and a value of 1 indicates that it is completely occluded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkPortalComponent")
	float GetPortalOcclusion() const;

	/**
	 * Sets a new portal occlusion value. A value of 0 indicates that the portal is not occluded and a value of 1 indicates that it is completely occluded.
	 * The occlusion value is applied to the portal with AK::SpatialAudio::SetPortalObstructionAndOcclusion.
	 * Portal occlusion can be used to modulate sound in response to a door opening or closing.
	 *
	 * @param InPortalOcclusion	The new portal occlusion value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkPortalComponent")
	void SetPortalOcclusion(float InPortalOcclusion);

	/**
	 * Returns the UPrimitiveComponent to which this Ak Portal Component is attached.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkPortalComponent")
	UPrimitiveComponent* GetPrimitiveParent() const;

	/**
	 * Returns true if the portal position and orientation are valid.
	 * Portals have a front and a back room. They must have at least one connected room,
	 * the front room must be different than the back room
	 * and both Rooms cannot be part of the same Reverb Zone hierarchy.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkPortalComponent")
	bool PortalPlacementValid() const;

	/**
	* If true, the room connections for this portal can change during runtime when this portal moves.
	* For worlds containing many rooms, this can be expensive.
	* Note that this portal's room connections may still change, even when bDynamic = false,
	* when dynamic rooms are moved (i.e. when rooms move who have bDynamic = true),
	* or rooms are enabled or disabled.
	*/
	UPROPERTY(EditAnywhere, BlueprintSetter = SetDynamic, Category = "AkPortalComponent", meta = (DisplayName = "Is Dynamic"))
	bool bDynamic = false;

	UFUNCTION(BlueprintSetter, Category = "AkPortalComponent")
	void SetDynamic(bool bInDynamic);

	/**
	 * Initially enables or disables the portal. When the portal is enabled, emitters positioned in the AkRoomComponent in front of and behind the portal emit through it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkPortalComponent")
	AkAcousticPortalState InitialState = AkAcousticPortalState::Open;

	/**
	* The initial occlusion value applied to the portal. When the occlusion value is set to 0, the portal is not occluded, and when it is set to 1, the portal is completely occluded.
	* The occlusion value is directly applied to the portal with AK::SpatialAudio::SetPortalObstructionAndOcclusion.
	* Portal occlusion can be used to modulate sound in response to a door opening or closing.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkPortalComponent|Obstruction Occlusion", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float InitialOcclusion = 0.f;

	/** Time interval between obstruction checks; a direct line of sight between the current portal and an emitter, a listener, or another portal. Set to 0 to disable obstruction checks. Valid range [0, [.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkPortalComponent|Obstruction Occlusion", meta = (ClampMin = 0.f))
	float ObstructionRefreshInterval = .0f;

	/** Collision channel for obstruction checks; a direct line of sight between the current portal and an emitter, a listener, or another portal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkPortalComponent|Obstruction Occlusion")
	TEnumAsByte<ECollisionChannel> ObstructionCollisionChannel = ECollisionChannel::ECC_Visibility;

	void ResetPortalState();
	void ResetPortalOcclusion();

	FVector GetExtent() const;
	AkRoomID GetFrontRoomID() const;
	AkRoomID GetBackRoomID() const;
	AkPortalID GetPortalID() const { return AkPortalID(this); }

	/** Update the room connections for the portal, given the portals current transform. 
		Return true if the room connections have changed.
	*/
	bool UpdateConnectedRooms(bool in_bForceUpdate = false);
	void RemovePortalConnections();

	const TWeakObjectPtr<UAkRoomComponent> GetFrontRoomComponent() const { return FrontRoom; }
	const TWeakObjectPtr<UAkRoomComponent> GetBackRoomComponent() const { return BackRoom; }

	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void BeginDestroy() override;
	virtual void InitializeComponent() override;
	virtual void OnComponentCreated() override;
	virtual void PostLoad() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	void UpdateTextRotations() const;
	void UpdateRoomNames();
#endif // WITH_EDITOR
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual bool MoveComponentImpl(
		const FVector & Delta,
		const FQuat & NewRotation,
		bool bSweep,
		FHitResult * Hit,
		EMoveComponentFlags MoveFlags,
		ETeleportType Teleport) override;

	FString GetPortalName();

private:
	TWeakObjectPtr<class UPrimitiveComponent> Parent;

	void InitializeParent();
	void SetSpatialAudioPortal();

	void FindConnectedComponents(FAkEnvironmentIndex& RoomQuery, TWeakObjectPtr<UAkRoomComponent>& out_pFront, TWeakObjectPtr<UAkRoomComponent>& out_pBack);

	AkAcousticPortalState PortalState = AkAcousticPortalState::Open;
	float PortalOcclusion = 0.f;

	static const float RoomsRefreshIntervalGame;
	static const float RoomsRefreshDistanceThreshold;
	static const float RoomsRefreshMinRotationThreshold_Degrees;
	float RoomsRefreshIntervalSeconds = 0.5f;
	float LastRoomsUpdate = 0.0f;
	FVector PreviousLocation;
	FRotator PreviousRotation;

	bool PortalNeedsUpdate = false;
	bool PortalOcclusionChanged = false;
	bool PortalRoomsNeedUpdate = false;
	TWeakObjectPtr<UAkRoomComponent> FrontRoom;
	TWeakObjectPtr<UAkRoomComponent> BackRoom;

	AkPortalObstructionAndOcclusionService ObstructionServiceFrontRoom;
	AkPortalObstructionAndOcclusionService ObstructionServiceBackRoom;

#if WITH_EDITOR
	static const float RoomsRefreshIntervalEditor;
	void HandleObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap);
	class UDrawPortalComponent* DrawPortalComponent = nullptr;
	void RegisterVisEnabledCallback();
	void InitializeDrawComponent();
	void DestroyDrawComponent();
	FDelegateHandle ShowPortalsChangedHandle;

	bool AreTextVisualizersInitialized() const;
	void InitTextVisualizers();
	void DestroyTextVisualizers();
	void UpdateTextVisibility();
	// Updates the location, rotation and visibility of the text visualizers
	void UpdateTextLocRotVis();
	bool bWasSelected = false;
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY(SkipSerialization, NonTransactional)
	mutable UTextRenderComponent* FrontRoomText = nullptr;

	UPROPERTY(SkipSerialization, NonTransactional)
	mutable UTextRenderComponent* BackRoomText = nullptr;
#endif
};

UCLASS(ClassGroup = Audiokinetic, hidecategories = (Advanced, Attachment, Volume), BlueprintType)
class AKAUDIO_API AAkAcousticPortal : public AVolume
{
	GENERATED_BODY()

public:
	AAkAcousticPortal(const class FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkAcousticPortal")
	void EnablePortal();

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkAcousticPortal")
	void DisablePortal();

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkAcousticPortal")
	AkAcousticPortalState GetCurrentState() const;

	UPROPERTY(VisibleAnywhere, Category = "AcousticPortal", BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	UAkPortalComponent* Portal = nullptr;

	virtual void PostRegisterAllComponents() override;
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;

#if WITH_EDITOR
	void FitRaycast();
	void FitPortal();
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	bool GetBestHits(FVector& Start0, FVector& End0, FVector& Start1, FVector& End1) 
	{ 
		if (BestFitValid)
		{
			Start0 = BestFit[0];
			End0 = BestFit[1];
			Start1 = BestFit[2];
			End1 = BestFit[3];
			
			return true;
		}
		return false;
	}
	float GetDetectionRadius() const { return DetectionRadius; }
	bool GetFitToGeometry() const { return FitToGeometry; }
	bool GetIsDragging() const { return IsDragging;  }

	virtual FName GetCustomIconName() const override
	{
		static const FName IconName("ClassIcon.AkAcousticPortal");
		return IconName;
	}
#endif

protected:
	static const int kNumRaycasts = 128;

#if WITH_EDITORONLY_DATA
	void ClearBestFit();

	/**
	Automatically fit the Ak Acoustic Portal to surrounding geometry. The fitting operation is performed after enabling this property, or after moving the actor to a new location.
	To find portals in surrounding geometry, rays emanating spherically outwards are cast from the origin of the actor in an attempt to detect sets of parallel surfaces.
	The "best" detected parallel surfaces are indicated with yellow outline when dragging the actor to a new location.
	*/
	UPROPERTY(EditAnywhere, Category = "Fit to Geometry")
	bool FitToGeometry = false;

	/**
	Sets the collision channel for the ray traces performed to fit the portal to the surrounding geometry. When set to 'Use Integration Settings Default', the value will be taken from the DefaultFitToGeometryCollisionChannel in the Wwise Integration Settings.
	*/
	UPROPERTY(EditAnywhere, Category = "Fit to Geometry")
	TEnumAsByte<EAkCollisionChannel> CollisionChannel = { EAkCollisionChannel::EAKCC_UseIntegrationSettingsDefault };

#if WITH_EDITOR
	/**
	Converts between EAkCollisionChannel and ECollisionChannel. Returns Wwise Integration Settings default if CollisionChannel == UseIntegrationSettingsDefault. Otherwise, casts CollisionChannel to ECollisionChannel.
	*/
	UFUNCTION(BlueprintCallable, Category = "Fit to Geometry")
	ECollisionChannel GetCollisionChannel();
#endif

	/**
	Limits the effective portal opening size that can be detected when fitting the portal to surrounding geometry.
	Increase this value to find larger openings; decrease it if large portals are erroneously detected, for example ones that span whole rooms.
	The slider range can be expanded by entering a text value into this field.
	*/
	UPROPERTY(EditAnywhere, Category = "Fit to Geometry", meta = (ClampMin = 1.0f, ClampMax = 100000.0f, UIMin = 100.0f, UIMax = 5000.0f))
	float DetectionRadius = 500.0f;

	FVector SavedRaycastOrigin;
	bool bUseSavedRaycastOrigin = false;
	FVector BestFit[4];
	bool BestFitValid = false;
	bool IsDragging = false;
#endif

private:
	/** As of Wwise 2020.1, the InitialState is contained in the AkPortalComponent */
	UPROPERTY()
	AkAcousticPortalState InitialState;

	UPROPERTY(Transient)
	bool bRequiresStateMigration = false;
	bool bRequiresTransformMigration = false;
};
