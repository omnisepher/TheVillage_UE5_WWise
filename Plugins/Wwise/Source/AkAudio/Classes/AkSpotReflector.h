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
	AkSpotReflector.h:
=============================================================================*/
#pragma once

#include "AkAudioDevice.h"
#include "AkAcousticTexture.h"
#include "GameFramework/Actor.h"
#include "AkSpotReflector.generated.h"

UCLASS(config = Engine)
class AKAUDIO_API AAkSpotReflector : public AActor
{
	GENERATED_BODY()

public:
	AAkSpotReflector(const FObjectInitializer& ObjectInitializer);

	/**
	*	Send to an Auxiliary Bus containing the Wwise Reflect plugin for early reflections rendering.
	*	Leave unassigned to use the Early Reflections Auxiliary Bus that is assigned in the Wwise Authoring Tool.
	*	Setting a value here will apply only to sounds playing on AK Components with EnableSpotReflectors to true.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkSpotReflector)
	class UAkAuxBus * EarlyReflectionAuxBus = nullptr;

	/**
	*	Send to an Auxiliary Bus containing the Wwise Reflect plugin for early reflections rendering.
	*	Leave unassigned to use the Early Reflections Auxiliary Bus that is assigned in the Wwise Authoring Tool.
	*	Setting a value here will apply only to sounds playing on AK Components with EnableSpotReflectors to true.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = AkSpotReflector)
	FString EarlyReflectionAuxBusName;

	/**
	*	The Acoustic Texture represents sound absorption. It is done by filtering the sound bouncing off the spot reflector.
	*	If left to None, no filtering will be applied to the sound.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkSpotReflector)
	UAkAcousticTexture* AcousticTexture = nullptr;

	/**
	*	This number scales the distance between the listener and the actual image source, preserving orientation.
	*	Set to 1 to position the image source at the position of the spot reflector
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkSpotReflector, meta = (ClampMin = "0.0"))
	float DistanceScalingFactor = .0f;

	/** Game-controlled level for the sound that will emit from the image source. Valid range: (0.0, 4.0)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkSpotReflector, meta = (ClampMin = "0.0", ClampMax = "4.0"))
	float Level = .0f;

	/** Make this spot reflector only reflect emitted sounds in the same Spatial Audio Room.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkSpotReflector)
	bool SameRoomOnly = false;

	/** Override the room this spot reflector is contained in. If disabled, a containment check will be done to find the room.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkSpotReflector, meta = (EditCondition = "SameRoomOnly"))
	bool EnableRoomOverride = false;

	/** The room in which the spot reflector will be virtually placed in. If set to None, the default "Outdoors" room will be used.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkSpotReflector, meta = (EditCondition = "EnableRoomOverride"))
	class AActor* RoomOverride = nullptr;

	AkImageSourceID GetImageSourceID() const;
	AkAuxBusID GetAuxBusID() const;

	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	* Update all spot reflectors in the world for a single ak component.
	* All spot reflectors are cleared first, then they are set
	* If the spot reflector is set as SameRoomOnly, it will be set only if it is in the same room as the ak component.
	*/
	static void UpdateSpotReflectors(UAkComponent* AkComponent);

	const FString GetSpotReflectorName() const;

private:
	void SetImageSource(UAkComponent* AkComponent);
	void AddToWorld();
	void RemoveFromWorld();

#if WITH_EDITORONLY_DATA
	/** Editor only component used to display the sprite so as to be able to see the location of the Spot Reflector  */
	class UBillboardComponent* SpriteComponent;
#endif

	typedef TSet<AAkSpotReflector*> SpotReflectorSet;
	typedef TMap<UWorld*, SpotReflectorSet> WorldToSpotReflectorsMap;
	static WorldToSpotReflectorsMap sWorldToSpotReflectors;
};
