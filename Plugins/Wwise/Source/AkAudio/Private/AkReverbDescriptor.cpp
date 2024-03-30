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

#include "AkReverbDescriptor.h"
#include "AkAudioDevice.h"
#include "AkAcousticTextureSetComponent.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkComponentHelpers.h"
#include "AkSettings.h"
#include "Wwise/API/WwiseSpatialAudioAPI.h"

#include "Components/PrimitiveComponent.h"
#include "Rendering/PositionVertexBuffer.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BrushComponent.h"

#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#if AK_USE_PHYSX
#include "PhysXIncludes.h"
#endif

/*=============================================================================
	Volume & Area Helpers
=============================================================================*/

// FKBoxElem has its own GetVolume function but it is inaccurate. It uses scale.GetMin() rather than using element-wise multiplication.
float BoxVolume(const FKBoxElem& box, const FVector& scale)
{
	return (box.X * scale.X) * (box.Y * scale.Y) * (box.Z * scale.Z);
}
// This is the volume calculated by Unreal in UBodySetup::GetVolume, for box elements. We'll use this to subtract from the total volume, and add the more accurate volume calculated by BoxVolume, above.
float InaccurateBoxVolume(const FKBoxElem& box, const FVector& scale)
{
	float MinScale = scale.GetMin();
	return (box.X * MinScale) * (box.Y * MinScale) * (box.Z * MinScale);
}

float BoxSurfaceArea(const FKBoxElem& box, const FVector& scale)
{
	return box.X * scale.X * box.Y * scale.Y * 2.0f /* top & bottom */
		+ box.X * scale.X * box.Z * scale.Z * 2.0f /* left & right */
		+ box.Y * scale.Y * box.Z * scale.Z * 2.0f; /* front & back */
}

float SphereSurfaceArea(const FKSphereElem& sphere, const FVector& scale)
{
	return 4.0f * PI * FMath::Pow(sphere.Radius * scale.GetMin(), 2.0f);
}

float CapsuleSurfaceArea(const FKSphylElem& capsule, const FVector& scale)
{
	const float r = capsule.Radius * FMath::Min(scale.X, scale.Y);
	return 2.0f * PI * r * (2.0f * r + capsule.Length * scale.Z);
}

bool HasSimpleCollisionGeometry(UBodySetup* bodySetup)
{
	FKAggregateGeom geometry = bodySetup->AggGeom;
	return geometry.BoxElems.Num() > 0 || geometry.ConvexElems.Num() > 0 || geometry.SphereElems.Num() > 0 || geometry.TaperedCapsuleElems.Num() > 0 || geometry.SphylElems.Num() > 0;

}

#if AK_USE_CHAOS
// Copied from BodySetup.cpp
// References: 
// http://amp.ece.cmu.edu/Publication/Cha/icip01_Cha.pdf
// http://stackoverflow.com/questions/1406029/how-to-calculate-the-volume-of-a-3d-mesh-object-the-surface-of-which-is-made-up
float FAkReverbDescriptor::SignedVolumeOfTriangle(const FVector& p1, const FVector& p2, const FVector& p3)
{
	return FVector::DotProduct(p1, FVector::CrossProduct(p2, p3)) / 6.0f;
}
#endif

void UpdateVolumeAndArea(UBodySetup* bodySetup, const FVector& scale, float& volume, float& surfaceArea)
{
	surfaceArea = 0.0f;
	// Initially use the Unreal UBodySetup::GetVolume function to calculate volume...
#if UE_5_1_OR_LATER
	volume = bodySetup->GetScaledVolume(scale);
#else
	volume = bodySetup->GetVolume(scale);
#endif
	FKAggregateGeom& geometry = bodySetup->AggGeom;

	for (const FKBoxElem& box : geometry.BoxElems)
	{
		surfaceArea += BoxSurfaceArea(box, scale);
#if (!UE_5_1_OR_LATER)
		// ... correct for any FKBoxElem elements in the geometry.
		// UBodySetup::GetVolume has an inaccuracy for box elements. It is scaled uniformly by the minimum scale dimension (see FKBoxElem::GetVolume).
		// For our purposes we want to scale by each dimension individually.
		volume -= InaccurateBoxVolume(box, scale);
		volume += BoxVolume(box, scale);
#endif
	}
	for (const FKConvexElem& convexElem : geometry.ConvexElems)
	{
		FTransform ScaleTransform = FTransform(FQuat::Identity, FVector::ZeroVector, scale);

		int32 numTriangles = convexElem.IndexData.Num() / 3;
		for (int32 triIdx = 0; triIdx < numTriangles; ++triIdx)
		{
			FVector v0 = ScaleTransform.TransformPosition(convexElem.VertexData[convexElem.IndexData[3 * triIdx]]);
			FVector v1 = ScaleTransform.TransformPosition(convexElem.VertexData[convexElem.IndexData[3 * triIdx + 1]]);
			FVector v2 = ScaleTransform.TransformPosition(convexElem.VertexData[convexElem.IndexData[3 * triIdx + 2]]);

			surfaceArea += FAkReverbDescriptor::TriangleArea(v0, v1, v2);
#if AK_USE_CHAOS && !(UE_5_1_OR_LATER)
			// FKConvexElem::GetVolume is not implemented with Chaos before UE 5.1
			volume += FAkReverbDescriptor::SignedVolumeOfTriangle(v0, v1, v2);
#endif
		}
	}
	for (const FKSphereElem& sphere : geometry.SphereElems)
	{
		surfaceArea += SphereSurfaceArea(sphere, scale);
	}
	for (const FKSphylElem& capsule : geometry.SphylElems)
	{
		surfaceArea += CapsuleSurfaceArea(capsule, scale);
	}
}

bool ConvertToAkAcousticTextures(TArray<FAkAcousticTextureParams>& InTexturesParams, TArray<AkAcousticTexture>& OutTextures)
{
	bool bAreAbsorptionValuesZero = true;
	for (const FAkAcousticTextureParams& params : InTexturesParams)
	{
		AkAcousticTexture Texture;
		Texture.fAbsorptionLow = params.AbsorptionLow();
		Texture.fAbsorptionMidLow = params.AbsorptionMidLow();
		Texture.fAbsorptionMidHigh = params.AbsorptionMidHigh();
		Texture.fAbsorptionHigh = params.AbsorptionHigh();
		OutTextures.Add(Texture);

		if (Texture.fAbsorptionLow != 0 ||
			Texture.fAbsorptionMidLow != 0 ||
			Texture.fAbsorptionMidHigh != 0 ||
			Texture.fAbsorptionHigh != 0)
		{
			bAreAbsorptionValuesZero = false;
		}
	}
	return bAreAbsorptionValuesZero;
}


/*=============================================================================
	FAkReverbDescriptor:
=============================================================================*/
double FAkReverbDescriptor::TriangleArea(const FVector& v1, const FVector& v2, const FVector& v3)
{
#if UE_5_0_OR_LATER
	double Mag = 0.0;
#else
	float Mag = 0.0f;
#endif
	FVector Dir;
	FVector::CrossProduct(v2 - v1, v3 - v1).ToDirectionAndLength(Dir, Mag);
	return 0.5 * Mag;
}

bool FAkReverbDescriptor::ShouldEstimateDecay() const
{
	if (IsValid(ReverbComponent) && ReverbComponent->AutoAssignAuxBus)
		return true;
	if (!IsValid(Primitive) || AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive) == nullptr)
		return false;

	return true;
}

bool FAkReverbDescriptor::ShouldEstimateDamping() const
{
	if (!IsValid(Primitive) || AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive) == nullptr)
		return false;

	return true;
}

bool FAkReverbDescriptor::ShouldEstimatePredelay() const
{
	if (!IsValid(Primitive) || AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive) == nullptr)
		return false;

	return true;
}

bool FAkReverbDescriptor::RequiresUpdates() const
{
	return ShouldEstimateDecay() || ShouldEstimateDamping() || ShouldEstimatePredelay();
}

void FAkReverbDescriptor::SetPrimitive(UPrimitiveComponent* primitive)
{
	Primitive = primitive;
}

void FAkReverbDescriptor::SetReverbComponent(UAkLateReverbComponent* InReverbComp)
{
	ReverbComponent = InReverbComp;
}

void FAkReverbDescriptor::CalculateT60(UAkLateReverbComponent* InReverbComp)
{
	if (IsValid(Primitive))
	{
		PrimitiveVolume = 0.0f;
		PrimitiveSurfaceArea = 0.0f;
		T60Decay = 0.0f;
		if (Primitive != nullptr)
		{
			FVector scale = Primitive->GetComponentScale();
			UBodySetup* primitiveBody = Primitive->GetBodySetup();
			if (primitiveBody != nullptr && HasSimpleCollisionGeometry(primitiveBody))
			{
				UpdateVolumeAndArea(primitiveBody, scale, PrimitiveVolume, PrimitiveSurfaceArea);
			}
			else
			{
				if (UBrushComponent* brush = Cast<UBrushComponent>(Primitive))
				{
					brush->BuildSimpleBrushCollision();
				}
				else
				{
					FString PrimitiveName = "";
					Primitive->GetName(PrimitiveName);
					FString ActorName = "";
					AActor* owner = Primitive->GetOwner();
					if (owner != nullptr)
						owner->GetName(ActorName);
					UE_LOG(LogAkAudio, Warning,
						TEXT("Primitive component %s on actor %s has no simple collision geometry.%sCalculations for reverb aux bus assignment will use component bounds. This could be less accurate than using simple collision geometry."),
						*PrimitiveName, *ActorName, LINE_TERMINATOR);
					// only apply scale to local bounds to calculate volume and surface area.
					FTransform transform = Primitive->GetComponentTransform();
					transform.SetRotation(FQuat::Identity);
					transform.SetLocation(FVector::ZeroVector);
					FBoxSphereBounds bounds = Primitive->CalcBounds(transform);
					FVector boxDimensions = bounds.BoxExtent * 2.0f;
					PrimitiveVolume = boxDimensions.X * boxDimensions.Y * boxDimensions.Z;
					PrimitiveSurfaceArea += boxDimensions.X * boxDimensions.Y * 2.0f;
					PrimitiveSurfaceArea += boxDimensions.X * boxDimensions.Z * 2.0f;
					PrimitiveSurfaceArea += boxDimensions.Y * boxDimensions.Z * 2.0f;
				}
			}

			PrimitiveVolume = FMath::Abs(PrimitiveVolume) / AkComponentHelpers::UnrealUnitsPerCubicMeter(Primitive);
			PrimitiveSurfaceArea /= AkComponentHelpers::UnrealUnitsPerSquaredMeter(Primitive);

			auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
			if (SpatialAudio && PrimitiveVolume > 0.0f && PrimitiveSurfaceArea > 0.0f)
			{
				float Absorption = AK_SA_MIN_ENVIRONMENT_ABSORPTION;

				TArray<FAkAcousticTextureParams> TexturesParams;
				TArray<float> SurfaceAreas;

				auto TextureSetComponent = InReverbComp->GetAttachedTextureSetComponent();
				if (TextureSetComponent.IsValid())
				{
					TextureSetComponent->GetTexturesAndSurfaceAreas(TexturesParams, SurfaceAreas);
					checkf(TexturesParams.Num() == SurfaceAreas.Num(), TEXT("FAkReverbDescriptor::CalculateT60: TexturesParams.Num (%d) != SurfaceAreas.Num (%d)"), (int)TexturesParams.Num(), (int)SurfaceAreas.Num());
				}
				
				// If we have at least one texture specified, we compute an average absorption value.
				if(TexturesParams.Num() != 0)
				{
					TArray<AkAcousticTexture> Textures;
					bool bAreAbsorptionValuesZero = ConvertToAkAcousticTextures(TexturesParams, Textures);

					if (!bAreAbsorptionValuesZero)
					{
						AkAcousticTexture AverageTextures;
						SpatialAudio->ReverbEstimation->GetAverageAbsorptionValues(&Textures[0], &SurfaceAreas[0], Textures.Num(), AverageTextures);
						float AverageAbsorption = (AverageTextures.fAbsorptionLow + AverageTextures.fAbsorptionMidLow + AverageTextures.fAbsorptionMidHigh + AverageTextures.fAbsorptionHigh) / 4.f;

						// We only update the absorption value if the value is above 1
						if (AverageAbsorption > AK_SA_MIN_ENVIRONMENT_ABSORPTION)
						{
							Absorption = AverageAbsorption;
						}
					}
				}
				// Else we use the Global Decay Absorption Value
				else {
					UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
					if (AkSettings != nullptr)
					{
						Absorption = AkSettings->GlobalDecayAbsorption;
					}
				}

				//calcuate t60 using the Sabine equation
				SpatialAudio->ReverbEstimation->EstimateT60Decay(PrimitiveVolume, PrimitiveSurfaceArea, Absorption, T60Decay);
			}
		}
	}

	if (IsValid(ReverbComponent))
		ReverbComponent->UpdateDecayEstimation(T60Decay, PrimitiveVolume, PrimitiveSurfaceArea);

	UpdateDecayRTPC();
}

void FAkReverbDescriptor::CalculateTimeToFirstReflection()
{
	auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
	if (SpatialAudio && IsValid(Primitive))
	{
		FTransform transform = Primitive->GetComponentTransform();
		transform.SetRotation(FQuat::Identity);
		transform.SetLocation(FVector::ZeroVector);
		FBoxSphereBounds bounds = Primitive->CalcBounds(transform);
		AkVector extentMeters = FAkAudioDevice::FVectorToAKVector(bounds.BoxExtent / AkComponentHelpers::UnrealUnitsPerMeter(Primitive));
		SpatialAudio->ReverbEstimation->EstimateTimeToFirstReflection(extentMeters, TimeToFirstReflection);
	}
#if WITH_EDITOR
	if (IsValid(ReverbComponent))
		ReverbComponent->UpdatePredelayEstimation(TimeToFirstReflection);
#endif
	UpdatePredelaytRTPC();
}

void FAkReverbDescriptor::CalculateHFDamping(const UAkAcousticTextureSetComponent* acousticTextureSetComponent)
{
	HFDamping = 0.0f;

	if (IsValid(Primitive))
	{
		auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (SpatialAudio && AkSettings)
		{
			TArray<FAkAcousticTextureParams> texturesParams;
			TArray<float> surfaceAreas;
			acousticTextureSetComponent->GetTexturesAndSurfaceAreas(texturesParams, surfaceAreas);

			if (texturesParams.Num() == 0)
			{
				HFDamping = 0.0f;
			}
			else
			{
				bool bAreAbsorptionValuesZero = true;
				bool bAreSurfaceAreasZero = true;

				TArray<AkAcousticTexture> textures;
				bAreAbsorptionValuesZero = ConvertToAkAcousticTextures(texturesParams, textures);

				for (int idx=0; idx<surfaceAreas.Num(); idx++)
				{
					if (surfaceAreas[idx] != 0)
					{
						bAreSurfaceAreasZero = false;
					}
				}

				if (bAreAbsorptionValuesZero || bAreSurfaceAreasZero)
				{
					HFDamping = 0.0f;
				}
				else
				{
					HFDamping = SpatialAudio->ReverbEstimation->EstimateHFDamping(&textures[0], &surfaceAreas[0], textures.Num());
				}
			}
		}
	}
#if WITH_EDITOR
	if (IsValid(ReverbComponent))
		ReverbComponent->UpdateHFDampingEstimation(HFDamping);
#endif
	UpdateDampingRTPC();
}

bool FAkReverbDescriptor::GetRTPCRoom(UAkRoomComponent*& room) const
{
	if (!IsValid(Primitive))
		return false;

	room = AkComponentHelpers::GetChildComponentOfType<UAkRoomComponent>(*Primitive);
	if (!CanSetRTPCOnRoom(room))
	{
		room = nullptr;
	}

	return room != nullptr;
}

bool FAkReverbDescriptor::CanSetRTPCOnRoom(const UAkRoomComponent* room) const
{
	if (FAkAudioDevice::Get() == nullptr
		|| room == nullptr
		|| !room->HasBeenRegisteredWithWwise()
		|| room->GetWorld() == nullptr
		|| (room->GetWorld()->WorldType != EWorldType::Game && room->GetWorld()->WorldType != EWorldType::PIE))
	{
		return false;
	}
	return true;
}

void FAkReverbDescriptor::UpdateDecayRTPC() const
{
	UAkRoomComponent* room = nullptr;
	if (GetRTPCRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->DecayRTPCInUse())
		{
			room->SetRTPCValue(AkSettings->DecayEstimateRTPC.LoadSynchronous(), T60Decay, 0, AkSettings->DecayEstimateName);
		}
	}
}

void FAkReverbDescriptor::UpdateDampingRTPC() const
{
	UAkRoomComponent* room = nullptr;
	if (GetRTPCRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->DampingRTPCInUse())
		{
			room->SetRTPCValue(AkSettings->HFDampingRTPC.LoadSynchronous(), HFDamping, 0, *AkSettings->HFDampingName);
		}
	}
}

void FAkReverbDescriptor::UpdatePredelaytRTPC() const
{
	UAkRoomComponent* room = nullptr;
	if (GetRTPCRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->PredelayRTPCInUse())
		{
			room->SetRTPCValue(AkSettings->TimeToFirstReflectionRTPC.LoadSynchronous(), TimeToFirstReflection, 0, *AkSettings->TimeToFirstReflectionName);
		}
	}
}

void FAkReverbDescriptor::UpdateAllRTPCs(const UAkRoomComponent* room) const
{
	checkf(room, TEXT("FAkReverbDescriptor::UpdateAllRTPCs: room is nullptr."));

	if (CanSetRTPCOnRoom(room))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();
		if (AkSettings != nullptr && AkSettings->ReverbRTPCsInUse())
		{
			if (AkSettings->DecayRTPCInUse())
			{
				room->SetRTPCValue(AkSettings->DecayEstimateRTPC.LoadSynchronous(), T60Decay, 0, AkSettings->DecayEstimateName);
			}

			if (AkSettings->DampingRTPCInUse())
			{
				room->SetRTPCValue(AkSettings->HFDampingRTPC.LoadSynchronous(), HFDamping, 0, *AkSettings->HFDampingName);
			}

			if (AkSettings->PredelayRTPCInUse())
			{
				room->SetRTPCValue(AkSettings->TimeToFirstReflectionRTPC.LoadSynchronous(), TimeToFirstReflection, 0, *AkSettings->TimeToFirstReflectionName);
			}
		}
	}
}