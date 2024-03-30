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
	AkDrawPortalComponent.h:
=============================================================================*/
#pragma once

#if WITH_EDITOR
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AkAcousticPortal.h"
#endif // WITH_EDITOR

#include "Components/PrimitiveComponent.h"
#include "AkDrawPortalComponent.generated.h"

class FMeshElementCollector;

/**
 *	Utility component for drawing a portal in a scene.
 */

UCLASS(collapsecategories, hidecategories = Object, editinlinenew, MinimalAPI)
class UDrawPortalComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UDrawPortalComponent(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	void DrawPortalOutline(const FSceneView* View, FPrimitiveDrawInterface* PDI, FMeshElementCollector& Collector, int32 ViewIndex) const;
	const UAkPortalComponent* GetPortalParent() const;

private:
	virtual class FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

#endif // WITH_EDITOR
};