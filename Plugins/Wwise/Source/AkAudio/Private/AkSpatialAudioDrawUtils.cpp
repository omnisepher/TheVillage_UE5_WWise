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
	AkSpatialAudioDrawUtils.cpp:
=============================================================================*/
#include "AkSpatialAudioDrawUtils.h"

#if WITH_EDITOR

#include "WwiseUEFeatures.h"
#include "AkAcousticPortal.h"
#include "AkSpatialAudioVolume.h"
#include "AkSurfaceReflectorSetComponent.h"

AkDrawBounds::AkDrawBounds(const FTransform& T, const FVector& Extent) : Transform(T), BoxExtent(Extent) {}

FVector AkDrawBounds::FRU() const { return Transform.TransformPosition(FVector(BoxExtent)); }
FVector AkDrawBounds::BLD() const { return Transform.TransformPosition(FVector(-BoxExtent)); }
FVector AkDrawBounds::FLD() const { return Transform.TransformPosition(FVector(BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z)); }
FVector AkDrawBounds::BRU() const { return Transform.TransformPosition(FVector(-BoxExtent.X, BoxExtent.Y, BoxExtent.Z)); }
FVector AkDrawBounds::FLU() const { return Transform.TransformPosition(FVector(BoxExtent.X, -BoxExtent.Y, BoxExtent.Z)); }
FVector AkDrawBounds::BLU() const { return Transform.TransformPosition(FVector(-BoxExtent.X, -BoxExtent.Y, BoxExtent.Z)); }
FVector AkDrawBounds::FRD() const { return Transform.TransformPosition(FVector(BoxExtent.X, BoxExtent.Y, -BoxExtent.Z)); }
FVector AkDrawBounds::BRD() const { return Transform.TransformPosition(FVector(-BoxExtent.X, BoxExtent.Y, -BoxExtent.Z)); }
FVector AkDrawBounds::RU()  const { return Transform.TransformPosition(FVector(0.0, BoxExtent.Y, BoxExtent.Z)); }
FVector AkDrawBounds::LU()  const { return Transform.TransformPosition(FVector(0.0, -BoxExtent.Y, BoxExtent.Z)); }
FVector AkDrawBounds::RD()  const { return Transform.TransformPosition(FVector(0.0, BoxExtent.Y, -BoxExtent.Z)); }
FVector AkDrawBounds::LD()  const { return Transform.TransformPosition(FVector(0.0, -BoxExtent.Y, -BoxExtent.Z)); }

namespace AkSpatialAudioColors
{	
	float kAlphaValue = 0.35f;
	float kDraggingAlphaValue = 0.05f;

	void GetPortalColors(const UAkPortalComponent* Portal, FLinearColor& FrontColor, FLinearColor& BackColor)
	{
		FLinearColor ConnectedColor = FAkAppStyle::Get().GetSlateColor("SelectionColor").GetSpecifiedColor();
		FrontColor = ConnectedColor;
		BackColor = ConnectedColor;
		if (!Portal->PortalPlacementValid())
		{
			FLinearColor ErrorColor = FLinearColor::Red;
			FrontColor = ErrorColor;
			BackColor = ErrorColor;
		}
		else if (!Portal->GetFrontRoomComponent().IsValid())
		{
			FLinearColor DisconnectedColor = FLinearColor::Gray;
			FrontColor = DisconnectedColor;
		}
		else if (!Portal->GetBackRoomComponent().IsValid())
		{
			FLinearColor DisconnectedColor = FLinearColor::Gray;
			BackColor = DisconnectedColor;
		}
		FrontColor.A = kAlphaValue;
		BackColor.A = kAlphaValue;
	}

	FLinearColor GetPortalOutlineColor(const UAkPortalComponent* Portal)
	{
		FLinearColor OutlineColor = FAkAppStyle::Get().GetSlateColor("SelectionColor").GetSpecifiedColor();
		if (false == Portal->PortalPlacementValid())
		{
			OutlineColor = FLinearColor::Red;
			OutlineColor.A = 0.85f;
		}
		return OutlineColor;
	}
	
	FLinearColor GetRoomColor()
	{
		return FAkAppStyle::Get().GetSlateColor("SelectionColor").GetSpecifiedColor();
	}
	
	FLinearColor GetRadialEmitterOutlineColor()
	{
		return FAkAppStyle::Get().GetSlateColor("SelectionColor").GetSpecifiedColor();
	}

	FLinearColor GetRadialEmitterColor()
	{
		return GetRadialEmitterOutlineColor();
	}

	float GetRadialEmitterInnerOpacity() { return kAlphaValue; }

	float GetRadialEmitterOuterOpacity() { return kAlphaValue * 0.15f; }

	FLinearColor GetSurfaceReflectorColor(const UAkSurfaceReflectorSetComponent* SurfaceReflectorSet, int NodeIdx, bool IsDragging)
	{
		const FLinearColor DefaultColor(FColor(0x4280AF));

		FLinearColor Color = FLinearColor::Gray;

		FAkSurfacePoly AcousticProperties = SurfaceReflectorSet->AcousticPolys[NodeIdx];
		if (AcousticProperties.EnableSurface)
		{
			Color = DefaultColor;

			if (AcousticProperties.Texture != nullptr)
			{
				Color = AcousticProperties.Texture->EditColor;
			}
		}

		Color.A = IsDragging ? kDraggingAlphaValue : kAlphaValue;

		return Color;
	}

	FLinearColor GetSpatialAudioVolumeOutlineColor()
	{
		return FAkAppStyle::Get().GetSlateColor("SelectionColor").GetSpecifiedColor();
	}

	FLinearColor GetBadFitSpatialAudioVolumeOutlineColor()
	{
		return FLinearColor::Red;
	}

	FLinearColor GetDiffractionEdgeColor()
	{
		return FLinearColor(FColor(0x09558F));
	}

	FLinearColor GetBoundaryDiffractionEdgeColor()
	{
		return FLinearColor(FColor(0x480D97));
	}
}

#endif // WITH_EDITOR
