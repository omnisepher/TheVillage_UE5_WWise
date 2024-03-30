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

#include "AkGameplayTypes.h"

#include "AkReverbDescriptor.generated.h"

class UAkAcousticTextureSetComponent;
class UAkLateReverbComponent;
class UAkRoomComponent;

#define PARAM_ESTIMATION_UPDATE_PERIOD 0.1f

/**
 * FAkReverbDescriptor is used to estimate the reverb parameters of a primitive component, by calculating its volume and surface area, and using the 'sabine equation' to estimate the reverb tail.
 * It also estimates the Time to First Reflection and the HFDamping.
 */
USTRUCT()
struct AKAUDIO_API FAkReverbDescriptor
{
	GENERATED_BODY()
public:
	static double TriangleArea(const FVector& v1, const FVector& v2, const FVector& v3);
	static float SignedVolumeOfTriangle(const FVector& p1, const FVector& p2, const FVector& p3);

	float PrimitiveVolume = 0.0f;
	float PrimitiveSurfaceArea = 0.0f;
	float T60Decay = 0.0f;
	float HFDamping = 0.0f;
	float TimeToFirstReflection = 0.0f;
	
	bool ShouldEstimateDecay() const;
	bool ShouldEstimateDamping() const;
	bool ShouldEstimatePredelay() const;
	bool RequiresUpdates() const;

	void CalculateT60(UAkLateReverbComponent* reverbComp);
	void CalculateTimeToFirstReflection();
	void CalculateHFDamping(const UAkAcousticTextureSetComponent* textureSetComponent);
	
	void SetPrimitive(UPrimitiveComponent* primitive);
	void SetReverbComponent(UAkLateReverbComponent* reverbComp);

	void UpdateAllRTPCs(const UAkRoomComponent* room) const;

private:
	UPROPERTY(Transient)
	UPrimitiveComponent* Primitive = nullptr;
	UAkLateReverbComponent* ReverbComponent = nullptr;
	/* Looks for a room component attached to Primitive, whose room ID has been registered with wwise, and whose world is Game or PIE.
		room will be null if no such room is found, or if there is no valid AkAudioDevice.
		return true if a room is found (and there is a valid AkAudioDevice). */
	bool GetRTPCRoom(UAkRoomComponent*& room) const;
	bool CanSetRTPCOnRoom(const UAkRoomComponent* room) const;
	void UpdateDecayRTPC() const;
	void UpdateDampingRTPC() const;
	void UpdatePredelaytRTPC() const;

};
