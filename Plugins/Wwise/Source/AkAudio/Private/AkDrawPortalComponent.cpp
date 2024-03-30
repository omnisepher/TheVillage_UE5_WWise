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
	AkDrawPortalComponent.cpp:
=============================================================================*/
#include "AkDrawPortalComponent.h"

#include "PrimitiveSceneProxy.h"

#if WITH_EDITOR
#include "DynamicMeshBuilder.h"
#include "Engine/World.h"
#include "AkRoomComponent.h"
#include "AkSpatialAudioDrawUtils.h"
#include "AkSettingsPerUser.h"
#endif // WITH_EDITOR

UDrawPortalComponent::UDrawPortalComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
const UAkPortalComponent* UDrawPortalComponent::GetPortalParent() const
{
	return Cast<UAkPortalComponent>(GetAttachParent());
}

void UDrawPortalComponent::DrawPortalOutline(const FSceneView* View, FPrimitiveDrawInterface* PDI, FMeshElementCollector& Collector, int32 ViewIndex) const
{
	const UAkPortalComponent* PortalComponent = GetPortalParent();
	if (IsValid(PortalComponent) && IsValid(PortalComponent->GetPrimitiveParent()))
	{
		const UPrimitiveComponent* PrimitiveParent = Cast<UPrimitiveComponent>(PortalComponent->GetPrimitiveParent());
		if (PrimitiveParent == nullptr)
			return;
		// Calculate the unscaled, unrotated box extent of the primitive parent component, at origin.
		FVector BoxExtent = PrimitiveParent->CalcBounds(FTransform()).BoxExtent;
		FLinearColor FrontDrawColor;
		FLinearColor BackDrawColor;
		AkSpatialAudioColors::GetPortalColors(PortalComponent, FrontDrawColor, BackDrawColor);
		FLinearColor OutlineColor = AkSpatialAudioColors::GetPortalOutlineColor(PortalComponent);

		float Thickness = AkDrawConstants::PortalOutlineThickness;
		FTransform T = PrimitiveParent->GetComponentTransform();
		AkDrawBounds DrawBounds(T, BoxExtent);

		/** Draw outline of 'front' (positive X) on portal */
		PDI->DrawLine(DrawBounds.RU(), DrawBounds.FRU(), FrontDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.FRU(), DrawBounds.FRD(), FrontDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.FRD(), DrawBounds.RD(), FrontDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.LU(), DrawBounds.FLU(), FrontDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.FLU(), DrawBounds.FLD(), FrontDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.FLD(), DrawBounds.LD(), FrontDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.FRU(), DrawBounds.FLU(), FrontDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.FRD(), DrawBounds.FLD(), FrontDrawColor, SDPG_MAX, Thickness);
		/** Draw outline of 'back' (negative X) on portal */
		PDI->DrawLine(DrawBounds.RU(), DrawBounds.BRU(), BackDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.BRU(), DrawBounds.BRD(), BackDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.BRD(), DrawBounds.RD(), BackDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.LU(), DrawBounds.BLU(), BackDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.BLU(), DrawBounds.BLD(), BackDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.BLD(), DrawBounds.LD(), BackDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.BLU(), DrawBounds.BRU(), BackDrawColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.BLD(), DrawBounds.BRD(), BackDrawColor, SDPG_MAX, Thickness);
		/** Draw outline around centre of portal (YZ plane) */
		PDI->DrawLine(DrawBounds.LU(), DrawBounds.LD(), OutlineColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.LD(), DrawBounds.RD(), OutlineColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.RD(), DrawBounds.RU(), OutlineColor, SDPG_MAX, Thickness);
		PDI->DrawLine(DrawBounds.RU(), DrawBounds.LU(), OutlineColor, SDPG_MAX, Thickness);

		UWorld* world = GetWorld();
		if (world != nullptr)
		{
			EWorldType::Type worldType = world->WorldType;
			if ((!(worldType == EWorldType::Game || worldType == EWorldType::PIE) && PortalComponent->InitialState == AkAcousticPortalState::Closed) ||
				((worldType == EWorldType::Game || worldType == EWorldType::PIE) && PortalComponent->GetCurrentState() == AkAcousticPortalState::Closed))
			{
				PDI->DrawLine(DrawBounds.FRU(), DrawBounds.BRD(), FrontDrawColor, SDPG_MAX, Thickness);
				PDI->DrawLine(DrawBounds.FLD(), DrawBounds.BLU(), BackDrawColor, SDPG_MAX, Thickness);
			}
		}

		Thickness = AkDrawConstants::PortalRoomConnectionThickness;

		FVector FrontPoint = FVector(BoxExtent.X, 0.0f, 0.0f);
		FVector BackPoint = FVector(-BoxExtent.X, 0.0f, 0.0f);
		if (PortalComponent->GetFrontRoomComponent().IsValid() && PortalComponent->GetFrontRoomComponent()->GetPrimitiveParent() != nullptr)
		{
			// Setup front facing vector to test if line from portal to room points 'backwards' (i.e. if it goes back through the portal). In this case, we extend the 'From' point slightly.
			FVector Front = PrimitiveParent->GetComponentTransform().TransformVector(FVector(1.0f, 0.0f, 0.0f));
			FVector From = PrimitiveParent->GetComponentTransform().TransformPosition(FrontPoint);
			FVector To = PortalComponent->GetFrontRoomComponent()->GetPrimitiveParent()->GetComponentTransform().TransformPosition(FVector(0.0f, 0.0f, 0.0f));
			//PDI->DrawLine(From, To, OutlineColor, SDPG_MAX, Thickness);
			FVector ToRoom = To - From;
			ToRoom.Normalize();
			PDI->DrawLine(From, To, OutlineColor, SDPG_MAX, Thickness);
		}
		Thickness = AkDrawConstants::PortalRoomConnectionThickness;
		if (PortalComponent->GetBackRoomComponent().IsValid() && PortalComponent->GetBackRoomComponent()->GetPrimitiveParent() != nullptr)
		{
			// Setup back facing vector to test if line from portal to room points 'backwards' (i.e. if it goes back through the portal). In this case, we extend the 'From' point slightly
			FVector Back = PrimitiveParent->GetComponentTransform().TransformVector(FVector(-1.0f, 0.0f, 0.0f));
			FVector From = PrimitiveParent->GetComponentTransform().TransformPosition(BackPoint);
			FVector To = PortalComponent->GetBackRoomComponent()->GetPrimitiveParent()->GetComponentTransform().TransformPosition(FVector(0.0f, 0.0f, 0.0f));
			//PDI->DrawLine(From, To, OutlineColor, SDPG_MAX, Thickness);
			FVector ToRoom = To - From;
			ToRoom.Normalize();
			PDI->DrawLine(From, To, OutlineColor, SDPG_MAX, Thickness);
		}
	}
}

/** Represents a portal to the scene manager. 
	Based on FDrawFrustumSceneProxy (in DrawFrustrumComponent.cpp)
*/
class FDrawPortalSceneProxy final : public FPrimitiveSceneProxy
{
public:
	/** Based on FDrawFrustumSceneProxy implementation */
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	/**
	* Initialization constructor.
	* @param	InComponent - game component to draw in the scene
	*/
	FDrawPortalSceneProxy(const UDrawPortalComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, PortalDrawer(InComponent)
	{}

	// FPrimitiveSceneProxy interface.

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		if (GetDefault<UAkSettingsPerUser>()->VisualizeRoomsAndPortals)
		{
			if (PortalDrawer != nullptr)
			{
				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{
					if (VisibilityMap & (1 << ViewIndex))
					{
						FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
						const FSceneView* View = Views[ViewIndex];
						PortalDrawer->DrawPortalOutline(View, PDI, Collector, ViewIndex);
					}
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bDynamicRelevance = true;
		Result.bStaticRelevance = true;
		Result.bEditorNoDepthTestPrimitiveRelevance = true;
		return Result;
	}

	/** Based on FDrawFrustumSceneProxy implementation */
	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:
	const UDrawPortalComponent* PortalDrawer;
};


FPrimitiveSceneProxy* UDrawPortalComponent::CreateSceneProxy()
{
	return new FDrawPortalSceneProxy(this);
}

FBoxSphereBounds UDrawPortalComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(LocalToWorld.TransformPosition(FVector::ZeroVector), FVector(AkDrawConstants::CullDepth), AkDrawConstants::CullDepth);
}

#endif // WITH_EDITOR