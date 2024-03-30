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
#include "AkAcousticTextureSetComponent.h"
#include "AkGameObject.h"
#include "AkSurfaceReflectorSetUtils.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "AkSurfaceReflectorSetComponent.generated.h"

class UAkRoomComponent;
struct FAkReverbDescriptor;

DECLARE_DELEGATE(FOnRefreshDetails);

UCLASS(ClassGroup = Audiokinetic, BlueprintType, hidecategories = (Transform, Rendering, Mobility, LOD, Component, Activation, Tags), meta = (BlueprintSpawnableComponent))
class AKAUDIO_API UAkSurfaceReflectorSetComponent : public UAkAcousticTextureSetComponent
{
	GENERATED_BODY()

public:
	UAkSurfaceReflectorSetComponent(const class FObjectInitializer& ObjectInitializer);

	/** Convert the brush to a geometry set consisting of vertices, triangles, surfaces, acoustic textures and transmission loss values.
	* Send it to Wwise with the rest of the AkGeometryParams to add or update a geometry in Spatial Audio.
	* It is necessary to create at least one geometry instance for each geometry set that is to be used for diffraction and reflection simulation. See UpdateSurfaceReflectorSet(). */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkSurfaceReflectorSet")
	void SendSurfaceReflectorSet();

	/** Add or update an instance of the geometry by sending the transform of this component to Wwise.
	* A geometry instance is a unique instance of a geometry set with a specified transform (position, rotation and scale).
	* It is necessary to create at least one geometry instance for each geometry set that is to be used for diffraction and reflection simulation. */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkSurfaceReflectorSet")
	void UpdateSurfaceReflectorSet();

	/** Remove the geometry and the corresponding instance from Wwise. */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkSurfaceReflectorSet")
	void RemoveSurfaceReflectorSet();

	/** Enable Surface Reflector Set to send the geometry for reflection and diffraction use. Additional properties are available in the Surface Reflector Set and Surface Properties categories.
	* Disable Surface Reflector Set to send a geometry that is not used for reflection and diffraction. The complete Surface Reflector Set category and the Transmission Loss property of the Surface Properties category are removed from the details panel.
	* When Surface Reflector Set is re-enabled after being disabled, the previously set values are restored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnableComponent", meta = (DisplayName = "Enable Surface Reflector Set"))
	bool bEnableSurfaceReflectors = false;
	
	/** The surface properties of each face on the brush geometry. */
	UPROPERTY(EditAnywhere, Category = "SurfaceReflectorSet", BlueprintSetter = UpdateAcousticProperties)
	TArray<FAkSurfacePoly> AcousticPolys;

	/** Set AcousticPolys with an input array, compute the surface areas of each poly and notify damping needs updating. */
	UFUNCTION(BlueprintSetter, Category = "Audiokinetic|AkSurfaceReflectorSet")
	void UpdateAcousticProperties(TArray<FAkSurfacePoly> in_AcousticPolys);

	/** Enable or disable geometric diffraction for this mesh. Check this box to have Wwise Spatial Audio generate diffraction edges on the geometry. The diffraction edges will be visible in the Wwise game object viewer when connected to the game. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurfaceReflectorSet")
	bool bEnableDiffraction = false;

	/** Enable or disable geometric diffraction on boundary edges for this Geometry. Boundary edges are edges that are connected to only one triangle. Depending on the specific shape of the geometry, boundary edges may or may not be useful and it is beneficial to reduce the total number of diffraction edges to process.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurfaceReflectorSet", meta = (EditCondition = "bEnableDiffraction"))
	bool bEnableDiffractionOnBoundaryEdges = false;

	/** (Deprecated) Associate this Surface Reflector Set component with a Room.
	* This property is deprecated and will be removed in a future version. We recommend not using it by leaving it set to None.
	* Associating a Surface Reflector Set component with a particular Room limits the scope in which the geometry is accessible. Doing so reduces the search space for ray casting performed by reflection and diffraction calculations.
	* When set to None, this geometry has a global scope.
	* Note if one or more geometry sets are associated with a room, that room can no longer access geometry that is in the global scope.
	*/ 
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "SurfaceReflectorSet")
	AActor* AssociatedRoom = nullptr;

	UModel* ParentBrush;

#if WITH_EDITORONLY_DATA
	UPROPERTY(SkipSerialization, NonTransactional)
	mutable TArray<UTextRenderComponent*> TextVisualizers;

	FText GetPolyText(int32 PolyIdx) const;

	void SetOnRefreshDetails(const FOnRefreshDetails& OnRefreshDetailsDelegate) { OnRefreshDetails = OnRefreshDetailsDelegate; }
	void ClearOnRefreshDetails() { OnRefreshDetails.Unbind(); }
	const FOnRefreshDetails* GetOnRefreshDetails() { return &OnRefreshDetails; }

	void AssignAcousticTexturesFromSamples(const TArray<FVector>& Points, const TArray<FVector>& Normals, const TArray< TWeakObjectPtr<class UPhysicalMaterial> >& Materials, int Num);
#endif

#if WITH_EDITOR
	/**
	* Check for errors
	*/
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	virtual void PreEditUndo() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);

	/** Tracks whether the user is interacting with a UI element in the details panel (e.g. a slider) */
	bool UserInteractionInProgress = false;
	FDelegateHandle PropertyChangedHandle;
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	void SchedulePolysUpdate();
	void UpdatePolys();
	void UpdateText(bool Visible);
	/** Align all of the text components (1 for each face) along one of the edges on the face */
	void UpdateTextPositions() const;
	void SurfacePropertiesChanged();
	void DestroyTextVisualizers();

	bool WasSelected;

	TSet<int> GetSelectedFaceIndices() const;
	bool TexturesDiffer() const;

	/** Used to delay the polys update by one frame when editing geometry */
	bool PolysNeedUpdate = false;
	/** Store the current acoustic properties for each face. Store them in PreviousPolys.
		If bUpdateEdges == true, also store the current surface edges.
		Transform the edges, normals and midpoints of the surfaces from world space to local space. */
	void CacheAcousticProperties();
	/** Store the current acoustic properties and geometry for each face. Store them in PreviousPolys
		Transform the edges, normals and midpoints of the surfaces from world space to local space. */
	void CacheLocalSpaceSurfaceGeometry();
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TMap<int64, FAkSurfaceEdgeInfo> EdgeMap;
#endif

	virtual void BeginDestroy() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual bool MoveComponentImpl(
		const FVector& Delta,
		const FQuat& NewRotation,
		bool bSweep,
		FHitResult* Hit,
		EMoveComponentFlags MoveFlags,
		ETeleportType Teleport) override;

	void GetTexturesAndSurfaceAreas(TArray<FAkAcousticTextureParams>& textures, TArray<float>& surfaceAreas) const override;
	void ComputeAcousticPolySurfaceArea();

private:
	virtual bool ShouldSendGeometry() const override;
	void InitializeParentBrush(bool fromTick = false);

#if WITH_EDITOR
	/** Used to keep track of the surfaces and their acoustic properties so that we can
		restore acoustic properties to the appropriate faces when the brush geometry is changed.*/
	TArray<FAkSurfacePoly> PreviousPolys;
	virtual void HandleObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap) override;
	virtual bool ContainsTexture(const FGuid& textureID) override;
	virtual void RegisterAllTextureParamCallbacks() override;
	/* Sort the edges of a face such that they form a continuous loop */
	void SortFaceEdges(int FaceIndex);
	/* Recalculate the normals for the face at FaceIndex, taking world scaling into account. */
	void UpdateFaceNormals(int FaceIndex);
	/** Identify the edges in the brush geometry and store in EdgeMap */
	void UpdateEdgeMap();
	/* Compare AcousticPolys to PreviousPolys, carrying over the acoustic properties from PreviousPolys for those faces whose edges and normals have not changed. */
	void EdgeMapChanged();
	void AlignTextWithEdge(int FaceIndex) const;
	/* Choose the edge upon which to align the text. The 'optimal' edge is that which aligns the 
	   up vector of the text closest to the up vector of the view camera. */
	int ChooseAlignmentEdge(int FaceIndex) const;
	/* Positions the text at the beginning of the AlignmentEdge (V0).
	   Shifts the text along the AlignmentEdge by a certain amount.
	   The amount of shift is proportional to the dot product between AlignmentEdge and the edge that connects to V0. */
	FVector GetTextAnchorPosition(int FaceIndex, const FAkSurfaceEdgeInfo& AlignmentEdge, int AlignmentEdgeIndex) const;
	/* Progressively scale down the text visualizer at FaceIndex until it is completely contained within the face.  */
	void SetTextScale(UTextRenderComponent* TextComp, int FaceIndex, int AlignmentEdgeIndex, const FVector& TextAnchorPosition, const struct FFacePlane& FacePlane, const FTransform& AttachTransform) const;
#endif

#if WITH_EDITORONLY_DATA
	FOnRefreshDetails OnRefreshDetails;
#endif
};
