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

/*=============================================================================
	AkSpatialAudioVolume.h:
=============================================================================*/
#pragma once

#include "GameFramework/Volume.h"
#include "AkSurfaceReflectorSetComponent.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkSpatialAudioVolume.generated.h"

UENUM()
enum class EAkFitToGeometryMode : uint32
{
	/**
	Oriented Box: The Ak Spatial Audio Volume is fit to the surrounding geometry using a minimum volume oriented bounding box. 
	Use for shoe box shaped rooms with and arbitrary extent and rotation.
	*/
	OrientedBox,
	
	/**
	Aligned Box: The Ak Spatial Audio Volume is fit to the surrounding geometry using an aligned bounding box, aligned to the local axes of the Actor. 
	Use for shoe box shaped rooms with an arbitrary extent, but with the rotation supplied by the user.
	The actor is rotated manually in the editor to achieve desired alignment.
	*/
	AlignedBox,
	
	/**
	Convex Polyhedron: The Ak Spatial Audio Volume is fit to the surrounding geometry using a convex polyhedron. Use for arbitrary-shaped convex rooms. 
	Will likely result in a more complex (higher poly-count) shape, and will possibly resulting in greater CPU and memory usage than oriented or aligned box shapes.
	When using convex polyhedron, a room must be fully enclosed; open ceilings or walls are not permitted and will cause a failure to fit to geometry.
	*/
	ConvexPolyhedron
};

/*------------------------------------------------------------------------------------
	AAkSpatialAudioVolume
------------------------------------------------------------------------------------*/
UCLASS(ClassGroup = Audiokinetic, BlueprintType, hidecategories = (Advanced, Attachment, Volume))
class AKAUDIO_API AAkSpatialAudioVolume : public AVolume
{
	GENERATED_BODY()

public:
	AAkSpatialAudioVolume(const class FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	void FitRaycast();
	void FitBox(bool bPreviewOnly = false);
	bool bBrushNeedsRebuild = false;
	virtual bool ShouldTickIfViewportsOnly() const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	const TArray<FVector>& GetRaycastHits() const { return FitPoints; }
	float GetFitScale() const { return FilterHitPoints; }
	static const int kNumRaycasts = 32;

	virtual FName GetCustomIconName() const override
	{
		static const FName IconName("ClassIcon.AkSpatialAudioVolume");
		return IconName;
	}

	void PostRebuildBrush();
	void ClearTextComponents();
	void UpdatePreviewTextComponents(TArray<FVector> positions);
	void UpdatePreviewPolys(TArray<TMap<TWeakObjectPtr<UPhysicalMaterial>, int>> materialVotes);
	TArray<FAkSurfacePoly> PreviewPolys;
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpatialAudioVolume", meta = (ShowOnlyInnerProperties))
	UAkSurfaceReflectorSetComponent* SurfaceReflectorSet = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpatialAudioVolume", meta = (ShowOnlyInnerProperties))
	UAkLateReverbComponent* LateReverb = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpatialAudioVolume", meta = (ShowOnlyInnerProperties))
	UAkRoomComponent* Room = nullptr;

#if WITH_EDITORONLY_DATA
	/**
	Automatically fit the Ak Spatial Audio Volume to the surrounding geometry. The fitting operation is performed after this property is enabled. It is performed again when the actor is moved to a new location.
	The fitting operation casts rays emanating spherically outwards from the origin of the actor. The points where the rays hit the surrounding geometry (drawn in the editor as green dots) are fit to a shape (box or convex polyhedron), and the actor is then resized appropriately.
	This operation modifies properties in the Surface Properties category. The Physical Materials of the surfaces hit are used to choose a corresponding Acoustic Texture and Transmission Loss value, as determined by the DefaultGeometrySurfacePropertiesTable in the Integration Settings.
	When this property is disabled, the fitting operation is not performed but the shape and surface properties of the Spatial Audio Volume are retained.
	*/
	UPROPERTY(EditAnywhere, Category = "Fit to Geometry" )
	bool FitToGeometry = false;

	/**
	Sets the collision channel for the ray traces performed to fit the spatial audio volume to the surrounding geometry. When set to 'Use Integration Settings Default', the value will be taken from the DefaultFitToGeometryCollisionChannel in the Wwise Integration Settings.
	*/
	UPROPERTY(EditAnywhere, Category = "Fit to Geometry")
	TEnumAsByte<EAkCollisionChannel> CollisionChannel;

#if WITH_EDITOR
	/**
	Converts between EAkCollisionChannel and ECollisionChannel. Returns Wwise Integration Settings default if CollisionChannel == UseIntegrationSettingsDefault. Otherwise, casts CollisionChannel to ECollisionChannel.
	*/
	UFUNCTION(BlueprintCallable, Category = "Fit to Geometry")
	ECollisionChannel GetCollisionChannel();
#endif

	/** 
	Choose the shape with which to fit to the surrounding geometry. 
	*/
	UPROPERTY(EditAnywhere, Category = "Fit to Geometry" )
	EAkFitToGeometryMode Shape = EAkFitToGeometryMode::AlignedBox;

	/**
	Set to a value less then 1.0 to filter out a percentage of the ray cast hits for use in fitting to surrounding geometry. 
	Points that have been rejected by the filter are drawn in red, and points accepted drawn in green. 
	Particularly useful when rays happen to escape through windows or other openings, resulting in undesirable points.
	*/
	UPROPERTY(EditAnywhere, Category = "Fit to Geometry", meta = (ClampMin = 0.1875f, ClampMax = 1.0f))
	float FilterHitPoints = 1.0f;

	UPROPERTY()
	TArray<FVector> FitPoints;
	
	UPROPERTY()
	TArray<FVector> FitNormals;

	UPROPERTY()
	TArray< TWeakObjectPtr<class UPhysicalMaterial> > FitMaterials;
	
	UPROPERTY()
	FRotator SavedRotation;

	TArray< TPair< FVector, FVector> > PreviewOutline;
	bool IsDragging = false;

	UPROPERTY()
	bool FitFailed = false;

	// Used by the visualizer when scaling the preview text components.
	float LongestEdgeLength = 0.0f;
	mutable TArray<UTextRenderComponent*> PreviewTextureNameComponents;

private:
	FBoxSphereBounds PreviousBounds = FBoxSphereBounds();
#endif
};
