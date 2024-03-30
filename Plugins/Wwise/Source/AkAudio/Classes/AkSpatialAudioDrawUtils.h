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
	AkSpatialAudioDrawUtils.h:
=============================================================================*/
#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "AkAudioDevice.h"

namespace AkDrawConstants
{
	const float CullDepth = 100.0f;
	const float PortalOutlineThickness = 5.0f;
	const float PortalRoomConnectionThickness = 3.0f;
	const float RoomIconThickness = 2.0f;
	const float RoomIconRadius = 10.0f;
	const int	RoomIconSides = 16;
	const float RoomAxisLength = 30.0f;
	const float RoomAxisThickness = 1.0f;
	const float RoomOutlineThickness = 1.0f; // Slightly thicker lines for spatial audio volumes when surface reflectors are disabled, since they are less visible.
	const float SpatialAudioVolumeOutlineThickness = 0.2f; // Shows the outline of disabled surfaces and non-diffraction edges.
	const float RadialEmitterThickness = 2.0f;
	const float DiffractionEdgeThickness = 2.5f;
}

/** A utility struct to transform the points on a local axis-aligned bounding box to world space using the given transform.
	Used for drawing rotated bounding boxes around portals.
*/
struct AKAUDIO_API AkDrawBounds
{
	AkDrawBounds(const FTransform& T, const FVector& Extent);

private:
	const FTransform& Transform;
	const FVector& BoxExtent;

public:
	/** FrontRightUp */		FVector FRU() const;
	/** BackLeftDown */		FVector BLD() const;
	/** FrontLeftDown */	FVector FLD() const;
	/** BackRightUp */		FVector BRU() const;
	/** FrontLeftDown */	FVector FLU() const;
	/** BackLeftUp */		FVector BLU() const;
	/** FrontRightDown */	FVector FRD() const;
	/** BackRightDown */	FVector BRD() const;
	/** RightUp */			FVector RU() const;
	/** LeftUp */			FVector LU() const;
	/** RightDown */		FVector RD() const;
	/** LeftDown */			FVector LD() const;
};

class UAkPortalComponent;
class UAkSurfaceReflectorSetComponent;
namespace AkSpatialAudioColors
{
	AKAUDIO_API void GetPortalColors(const UAkPortalComponent* Portal, FLinearColor& FrontColor, FLinearColor& BackColor);
	AKAUDIO_API FLinearColor GetPortalOutlineColor(const UAkPortalComponent* Portal);
	AKAUDIO_API FLinearColor GetRoomColor();
	AKAUDIO_API FLinearColor GetRadialEmitterOutlineColor();
	AKAUDIO_API FLinearColor GetRadialEmitterColor();
	AKAUDIO_API float GetRadialEmitterOuterOpacity();
	AKAUDIO_API float GetRadialEmitterInnerOpacity();
	AKAUDIO_API FLinearColor GetSurfaceReflectorColor(const UAkSurfaceReflectorSetComponent* SurfaceReflectorSet, int NodeIdx, bool IsDragging);
	AKAUDIO_API FLinearColor GetSpatialAudioVolumeOutlineColor();
	AKAUDIO_API FLinearColor GetBadFitSpatialAudioVolumeOutlineColor();
	AKAUDIO_API FLinearColor GetDiffractionEdgeColor();
	AKAUDIO_API FLinearColor GetBoundaryDiffractionEdgeColor();
}

#endif // WITH_EDITOR
