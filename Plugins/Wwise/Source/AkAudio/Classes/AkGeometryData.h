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
	AkGeometryData.h:
=============================================================================*/
#pragma once
#include "AkInclude.h"

#include "AkAcousticTexture.h"

#include "AkGeometryData.generated.h"

class UPhysicalMaterial;

USTRUCT()
struct FAkAcousticSurface
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 Texture = AK_INVALID_UNIQUE_ID;

	UPROPERTY(DisplayName = "Transmission Loss")
	float Occlusion = .0f;

	UPROPERTY()
	FString Name;
};

USTRUCT()
struct FAkTriangle
{
	GENERATED_BODY()

	UPROPERTY()
	uint16 Point0 = 0;

	UPROPERTY()
	uint16 Point1 = 0;

	UPROPERTY()
	uint16 Point2 = 0;

	UPROPERTY()
	uint16 Surface = 0;
};

USTRUCT()
struct FAkGeometryData
{
	GENERATED_BODY()

	void Clear()
	{
		Vertices.Empty();
		Surfaces.Empty();
		Triangles.Empty();
		ToOverrideAcousticTexture.Empty();
		ToOverrideOcclusion.Empty();
	}

	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<FAkAcousticSurface> Surfaces;

	UPROPERTY()
	TArray<FAkTriangle> Triangles;

	UPROPERTY()
	TArray<UPhysicalMaterial*> ToOverrideAcousticTexture;

	UPROPERTY(DisplayName = "To Override Transmission Loss")
	TArray<UPhysicalMaterial*> ToOverrideOcclusion;

	void AddBox(AkSurfIdx surfIdx, FVector center, FVector extent, FRotator rotation);
	void AddSphere(AkSurfIdx surfIdx, const FVector& Center, const float Radius, int32 NumSides, int32 NumRings);
	void AddCapsule(AkSurfIdx surfIdx, const FVector& Origin, const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis, float Radius, float HalfHeight, int32 NumSides);
};