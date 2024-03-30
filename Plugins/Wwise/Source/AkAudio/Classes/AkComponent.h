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
	AkComponent.h:
=============================================================================*/

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "AkInclude.h"
#include "AkGameplayTypes.h"
#include "AkSettings.h" // for EAkCollisionChannel
#include "Components/SceneComponent.h"
#include "AkGameObject.h"

#include "Wwise/AkComponentObstructionAndOcclusionService.h"

#include "AkComponent.generated.h"

UENUM(Meta = (Bitflags))
enum class EReflectionFilterBits
{
	Wall,
	Ceiling,
	Floor
};

// PostEvent functions need to return the PlayingID (uint32), but Blueprints only work with int32.
// Make sure AkPlayingID is always 32 bits, or else we're gonna have a bad time.
static_assert(sizeof(AkPlayingID) == sizeof(int32), "AkPlayingID is not 32 bits anymore. Change return value of PostEvent functions!");

struct AkReverbFadeControl
{
public:
	uint32 AuxBusId;
	bool bIsFadingOut;
	void* FadeControlUniqueId; 

private:
	float CurrentControlValue;
	float TargetControlValue;
	float FadeRate;
	float Priority;

public:
	AkReverbFadeControl(const class UAkLateReverbComponent& LateReverbComponent);
	void UpdateValues(const class UAkLateReverbComponent& LateReverbComponent);
	bool Update(float DeltaTime);
	void ForceCurrentToTargetValue() { CurrentControlValue = TargetControlValue; }
	AkAuxSendValue ToAkAuxSendValue() const;

	static bool Prioritize(const AkReverbFadeControl& A, const AkReverbFadeControl& B);
};

/*------------------------------------------------------------------------------------
	UAkComponent
------------------------------------------------------------------------------------*/
UCLASS(ClassGroup=Audiokinetic, BlueprintType, Blueprintable, hidecategories=(Transform,Rendering,Mobility,LOD,Component,Activation), AutoExpandCategories=AkComponent, meta=(BlueprintSpawnableComponent))
class AKAUDIO_API UAkComponent: public UAkGameObject
{
	GENERATED_BODY()

public:
	UAkComponent(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	bool bUseSpatialAudio_DEPRECATED = false;

	int32 ReflectionFilter_DEPRECATED;

	/**
	* The object collision channel to use when doing line of sight traces for obstruction/occlusion calculations.
	* When set to 'Use Integration Settings Default', the value will be taken from the DefaultCollisionChannel in the Wwise Integration Settings.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Obstruction Occlusion", meta = (DisplayName = "Collision Channel"))
	TEnumAsByte<EAkCollisionChannel> OcclusionCollisionChannel = { EAkCollisionChannel::EAKCC_UseIntegrationSettingsDefault };

	/** Get the collision channel used when doing line of sight traces for obstruction/occlusion calculations. */
	UFUNCTION(BlueprintCallable, Category="AkComponent|Obstruction Occlusion", meta = (DisplayName = "Get Collision Channel"))
	ECollisionChannel GetOcclusionCollisionChannel();

	/**
	* Set the time interval between obstruction/occlusion checks (direct line of sight between the listener and this game object). Valid range [0, [.
	* Obstruction is used if Spatial Audio Rooms are present in the map. Otherwise, occlusion is used. Set to 0 to disable obstruction/occlusion on this component.
	* The obstruction/occlusion value is directly applied with AK::SoundEngine::SetObjectObstructionAndOcclusion.
	* When using Spatial Audio, obstruction checks are also done between portals in the same room and this game object.
	* Only use this feature if you plan to obstruct this game object with geometry that is neither \ref features_objects_aksurfacereflectorset nor \ref features_objects_akgeometrycomponent.
	* If not, we recommend that you disable obstruction/occlusion checks and exclusively use Spatial Audio Geometric Diffraction and Transmission.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Obstruction Occlusion", meta = (ClampMin = 0.f, DisplayName = "Refresh Interval"))
	float OcclusionRefreshInterval = .0f;

	/**Enable spot reflectors for this Ak Component **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio")
	bool EnableSpotReflectors = false;

	/**
	*	Define an outer radius around each sound position to simulate a radial sound source.
	*	If the listener is outside the outer radius, the spread is defined by the area that the sphere takes in the listener field of view.
	*	When the listener intersects the outer radius, the spread is exactly 50%. When the listener is in between the inner and outer radius, the spread interpolates linearly from 50% to 100%.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Radial Emitter", meta = (ClampMin = 0.0f) )
	float outerRadius = .0f;

	/**
	*	Define an inner radius around each sound position to simulate a radial sound source.
	*	If the listener is inside the inner radius, the spread is 100%.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AkComponent|Spatial Audio|Radial Emitter", meta = (ClampMin = 0.0f))
	float innerRadius = .0f;

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent")
	void SetGameObjectRadius(float in_outerRadius, float in_innerRadius);

	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent")
	void SetEnableSpotReflectors(bool in_enable);

private:
	/** 
	*	Send to an Auxiliary Bus containing the Wwise Reflect plugin for early reflections rendering.
	*	Note that the Wwise Auxiliary Bus for early reflections can also be set per-sound in the Sound Property Editor in the Wwise Authoring tool. 
	*	Setting a value here will apply only to sounds playing on the AK Component that do not have an Auxiliary Bus set in the Wwise Authoring tool.
	*/
	UPROPERTY(EditAnywhere, Category = "AkComponent|Spatial Audio|Reflect")
	class UAkAuxBus * EarlyReflectionAuxBus = nullptr;

	/**
	*	Send to an Auxiliary Bus containing the Wwise Reflect plugin for early reflections rendering.
	*	Note that the Wwise Auxiliary Bus for early reflections can also be set per-sound in the Sound Property Editor in the Wwise Authoring tool.
	*	Setting a value here will apply only to sounds playing on the AK Component that do not have an Auxiliary Bus set in the Wwise Authoring tool.
	*/
	UPROPERTY(EditAnywhere, Category = "AkComponent|Spatial Audio|Reflect")
	FString EarlyReflectionAuxBusName;

	/**
	*	Set the send volume for the early reflections Auxiliary Bus.
	*	The send volume applied to this AK Component will be applied additively to the Auxiliary Send volume defined per-sound in the Wwise Authoring tool.
	*/
	UPROPERTY(EditAnywhere, Category = "AkComponent|Spatial Audio|Reflect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EarlyReflectionBusSendGain = .0f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	bool DrawFirstOrderReflections = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	bool DrawSecondOrderReflections = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	bool DrawHigherOrderReflections = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent|Spatial Audio|Debug Draw")
	bool DrawDiffraction = false;

	/** Stop sound when owner is destroyed? */
	UPROPERTY()
	bool StopWhenOwnerDestroyed = false;

	/**
	 * Posts this component's AkAudioEvent to Wwise, using this component as the game object source, and wait until the event is 
	 * done playing to continue execution. Extra calls while the event is playing are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent", meta = (AdvancedDisplay = "0", Latent, LatentInfo = "LatentInfo"))
	int32 PostAssociatedAkEventAndWaitForEnd(FLatentActionInfo LatentInfo);

	/**
	 * Posts an event to Wwise, using this component as the game object source, and wait until the event is
	 * done playing to continue execution. Extra calls while the event is playing are ignored.
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent", meta = (AdvancedDisplay = "1", Latent, LatentInfo = "LatentInfo"))
	int32 PostAkEventAndWaitForEnd(
		class UAkAudioEvent * AkEvent,
		FLatentActionInfo LatentInfo
	);

	int32 PostAkEvent(UAkAudioEvent* AkEvent, int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback) override;
	AkPlayingID PostAkEvent(UAkAudioEvent* AkEvent, AkUInt32 Flags = 0, AkCallbackFunc UserCallback = nullptr, void* UserCookie = nullptr) override;
	/**
	 * Posts a trigger to wwise, using this component as the game object source
	 *
	 * @param Trigger		The name of the trigger
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent", meta = (AdvancedDisplay = "1"))
	void PostTrigger(class UAkTrigger const* TriggerValue, FString Trigger);
	
	/**
	 * Sets a switch group in wwise, using this component as the game object source
	 *
	 * @param SwitchGroup	The name of the switch group
	 * @param SwitchState	The new state of the switch
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent", meta = (AdvancedDisplay = "1"))
	void SetSwitch(class UAkSwitchValue const* SwitchValue, FString SwitchGroup, FString SwitchState);

	/**
	 * Sets whether or not to stop sounds when the component's owner is destroyed
	 *
	 * @param bStopWhenOwnerDestroyed	Whether or not to stop sounds when the component's owner is destroyed
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	void SetStopWhenOwnerDestroyed( bool bStopWhenOwnerDestroyed );

	/**
	 * Set a game object's listeners
	 *
	 * @param Listeners	The array of components that listen to this component
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	void SetListeners( const TArray<UAkComponent*>& Listeners );

	// Reverb volumes functions

	/** 
	* Set the early reflections aux bus for this AK Component.
	* Geometrical reflection calculation inside spatial audio is enabled for a game object if any sound playing on the game object has a valid early reflections aux bus specified in the authoring tool,
	* or if an aux bus is specified via SetEarlyReflectionsAuxSend.
	* The AuxBusName parameter of SetEarlyReflectionsAuxSend applies to sounds playing on the Wwise game object which have not specified an early reflection bus in the authoring tool -
	* the parameter specified on individual sounds' reflection bus takes priority over the value passed in to SetEarlyReflectionsAuxSend.
	* @param AuxBusName - Name of aux bus in the Wwise project.
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void SetEarlyReflectionsAuxBus(const FString& AuxBusName);

	/** 
	* Set the early reflections volume for this AK Component. The SendVolume parameter is used to control the volume of the early reflections send. It is combined with the early reflections volume specified in the authoring tool,
	* and is applied to all sounds playing on the Wwise game object.
	* @param SendVolume - Send volume to the early reflections aux bus.
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void SetEarlyReflectionsVolume(float SendVolume);

	/**
	* Set the output bus volume (direct) to be used for the specified game object.
	* The control value is a number ranging from 0.0f to 1.0f.
	*
	* @param BusVolume - Bus volume to set
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void SetOutputBusVolume(float BusVolume);


	/** Modifies the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AkComponent")
	float AttenuationScalingFactor = .0f;

	/** Sets the attenuation scaling factor, which modifies the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	void SetAttenuationScalingFactor(float Value);

	/** Whether to use reverb volumes or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AkComponent")
	bool bUseReverbVolumes = true;


	/**
	 * Return the real attenuation radius for this component (AttenuationScalingFactor * AkAudioEvent->MaxAttenuationRadius)
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent")
	float GetAttenuationRadius() const;

	void UpdateGameObjectPosition();

	void GetAkGameObjectName(FString& Name) const;

	bool IsListener = false;
	bool IsDefaultListener = false;

#if CPP

	/*------------------------------------------------------------------------------------
		UActorComponent interface.
	------------------------------------------------------------------------------------*/
	/**
	 * Called after component is registered
	 */
	virtual void OnRegister();

	/**
	 * Called after component is unregistered
	 */
	virtual void OnUnregister();

	/**
	 * Clean up
	 */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	/**
	 * Clean up after error
	 */
	virtual void ShutdownAfterError();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Begin USceneComponent Interface
	virtual void BeginPlay() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
	// End USceneComponent Interface

	/** Gets all AkLateReverbComponents at the AkComponent's current location, and puts them in a list
	 *
	 * @param Loc					The location of the AkComponent
	 */
	void UpdateAkLateReverbComponentList(FVector Loc);

	/** Gets the current room the AkComponent is in.
	 * 
	 * @param Location			The location of the AkComponent
	 */
	void UpdateSpatialAudioRoom(FVector Location);

	void SetAutoDestroy(bool in_AutoDestroy) { bAutoDestroy = in_AutoDestroy; }

	bool UseDefaultListeners() const { return bUseDefaultListeners; }

	void OnListenerUnregistered(UAkComponent* in_pListener)
	{
		FScopeLock Lock(&ListenerCriticalSection);
		Listeners.Remove(in_pListener);
	}

	void OnDefaultListenerAdded(UAkComponent* in_pListener)
	{
		if(bUseDefaultListeners)
		{
			FScopeLock Lock(&ListenerCriticalSection);
			Listeners.Add(in_pListener);
		}
	}

	static UAkComponent* GetAkComponent(AkGameObjectID GameObjectID);

	AkRoomID GetSpatialAudioRoomID() const;
	const TWeakObjectPtr<UAkRoomComponent> GetSpatialAudioRoom() const { return CurrentRoom; }

	void UpdateObstructionAndOcclusion();

	FVector GetPosition() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/**
	 * Register the component with Wwise
	 */
	void RegisterGameObject();

	/**
	 * Unregister the component from Wwise
	 */
	void UnregisterGameObject();

	/*
	* Called after registering the component with Wwise
	*/
	virtual void PostRegisterGameObject();

	/*
	* Called after unregistering the component from Wwise
	*/
	virtual void PostUnregisterGameObject();

	// Reverb Volume features ---------------------------------------------------------------------

	/** Apply the current list of AkReverbVolumes 
	 *
	 * @param DeltaTime		The given time increment since last fade computation
	 */
	void ApplyAkReverbVolumeList(float DeltaTime);

	AkComponentObstructionAndOcclusionService ObstructionService;

	/** Array of the active AkReverbVolumes at the AkComponent's location */
	TArray<AkReverbFadeControl> ReverbFadeControls;

	/** Aux Send values sent to the SoundEngine in the previous frame */
	TArray<AkAuxSendValue> CurrentAuxSendValues;

	/** Do we need to refresh Aux Send values? */
	bool NeedToUpdateAuxSends(const TArray<AkAuxSendValue>& NewValues);

	/** Room the AkComponent is currently in. nullptr if none */
	TWeakObjectPtr<class UAkRoomComponent> CurrentRoom;

	/** Whether to automatically destroy the component when the event is finished */
	bool bAutoDestroy;

	/** Previous known position. Used to avoid Spamming SetPosition on a listener */
	AkSoundPosition CurrentSoundPosition;
	bool HasMoved();

#endif

#if WITH_EDITORONLY_DATA
	/** Utility function that updates which texture is displayed on the sprite dependent on the properties of the Audio Component. */
	void UpdateSpriteTexture();
#endif

	bool bUseDefaultListeners;
	FCriticalSection ListenerCriticalSection;
	TSet<TWeakObjectPtr<UAkComponent>> Listeners;

	void DebugDrawReflections() const;
	void _DebugDrawReflections(const AkVector64& akEmitterPos, const AkVector64& akListenerPos, const AkReflectionPathInfo* paths, AkUInt32 uNumPaths) const;

	void DebugDrawDiffraction() const;
	void _DebugDrawDiffraction(const AkVector64& akEmitterPos, const AkVector64& akListenerPos, const AkDiffractionPathInfo* paths, AkUInt32 uNumPaths) const;
};
