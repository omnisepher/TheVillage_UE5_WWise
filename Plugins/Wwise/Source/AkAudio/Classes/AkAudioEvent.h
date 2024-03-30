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

#include "AkAudioType.h"
#include "AkGameplayTypes.h"
#include "Wwise/CookedData/WwiseLocalizedEventCookedData.h"
#include "Wwise/Loaded/WwiseLoadedEvent.h"

#if WITH_EDITORONLY_DATA
#include "Wwise/Info/WwiseEventInfo.h"
#endif

#include "AkAudioEvent.generated.h"

class UAkGameObject;
class UAkGroupValue;
class UAkAuxBus;
class UAkAudioBank;
class UAkTrigger;


UCLASS(BlueprintType)
class AKAUDIO_API UAkAudioEvent : public UAkAudioType
{
	GENERATED_BODY()

	//
	// Unreal properties
	//
public:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		float MaxAttenuationRadius = .0f;

	/** Whether this event is infinite (looping) or finite (duration parameters are valid) */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		bool IsInfinite = false;

	/** Minimum duration */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		float MinimumDuration = .0f;

	/** Maximum duration */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "AkAudioEvent")
		float MaximumDuration = .0f;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AkAudioEvent")
		FWwiseEventInfo EventInfo;
#endif

	UPROPERTY(Transient, VisibleAnywhere, Category = "AkAudioEvent")
		FWwiseLocalizedEventCookedData EventCookedData;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Used for migration"))
		UAkAudioBank* RequiredBank_DEPRECATED = nullptr;

public:
	/**
	 * @brief Posts the Wwise Event on the root component of the specified actor.
	 *
	 * @param Actor Actor on which to play the event. This actor gets followed automatically by the Event. If the Actor is left empty,
	 *			the Event will be played as an Ambient sound.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param bStopWhenAttachedObjectDestroyed Specifies whether the sound should stop playing when the owner of the attach to component
	 *			 is destroyed. This parameter modifies the AkComponent itself, you can only have one behavior per actor's root component.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|Actor", meta=(AdvancedDisplay="1", AutoCreateRefTerm = "Delegate"))
	int32 PostOnActor(const AActor* Actor, 
		const FOnAkPostEventCallback& Delegate,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) const int32 CallbackMask,
		const bool bStopWhenAttachedObjectDestroyed);

	/**
	 * @brief Posts the Wwise Event on the specified component.
	 *
	 * @param Component Component on which to play the event.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param bStopWhenAttachedObjectDestroyed Specifies whether the sound should stop playing when the owner of the attach to component
	 *			 is destroyed. This parameter modifies the AkComponent itself, you can only have one behavior per actor's root component.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkComponent", meta=(AdvancedDisplay="1", AutoCreateRefTerm = "Delegate"))
	int32 PostOnComponent(UAkComponent* Component, 
		const FOnAkPostEventCallback& Delegate,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) const int32 CallbackMask,
		const bool bStopWhenAttachedObjectDestroyed);

	/**
	 * @brief Posts the Wwise Event on the specified game object.
	 *
	 * @param GameObject Game object on which to play the event.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkGameObject", meta=(AdvancedDisplay="1", AutoCreateRefTerm = "Delegate"))
	int32 PostOnGameObject(UAkGameObject* GameObject, 
		const FOnAkPostEventCallback& Delegate,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) const int32 CallbackMask);

	/**
	 * @brief Posts the Wwise Event on the root component of the specified actor, and waits for the end of the event to continue execution.
	 *
	 * Additional calls made while an event is active on a particular actor's root component are ignored.
	 *
	 * @param Actor Actor on which to play the event. This actor gets followed automatically by the Event.
	 * @param bStopWhenAttachedObjectDestroyed Specifies whether the sound should stop playing when the owner of the attach to component is destroyed.
	 *        This parameter modifies the AkComponent itself, you can only have one behavior per actor's root component.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|Actor", meta = (Latent, LatentInfo = "LatentActionInfo", AdvancedDisplay = "1", bStopWhenAttachedObjectDestroyed="false"))
	int32 PostOnActorAndWait(const AActor* Actor,
		const bool bStopWhenAttachedObjectDestroyed,
		const FLatentActionInfo LatentActionInfo);

	/**
	 * @brief Posts the Wwise Event on the specified component, and waits for the end of the event to continue execution.
	 *
	 * Additional calls made while an event is active on a particular component are ignored.
	 *
	 * @param Component component on which to play the event. This component gets followed automatically by the Event.
	 * @param bStopWhenAttachedObjectDestroyed Specifies whether the sound should stop playing when the owner of the attach to component is destroyed.
	 *        This parameter modifies the AkComponent itself, you can only have one behavior per actor's root component.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkComponent", meta = (Latent, LatentInfo = "LatentActionInfo", AdvancedDisplay = "1", bStopWhenAttachedObjectDestroyed="false"))
	int32 PostOnComponentAndWait(UAkComponent* Component,
		const bool bStopWhenAttachedObjectDestroyed,
		const FLatentActionInfo LatentActionInfo);

	/**
	 * @brief Posts the Wwise Event on the specified game object, and waits for the end of the event to continue execution.
	 *
	 * Additional calls made while an event is active on a particular game object are ignored.
	 *
	 * @param GameObject Game object on which to play the event. This game object gets followed automatically by the Event.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audiokinetic|AkGameObject", meta = (Latent, LatentInfo = "LatentActionInfo", AdvancedDisplay = "1", bStopWhenAttachedObjectDestroyed="false"))
	int32 PostOnGameObjectAndWait(UAkGameObject* GameObject,
		const FLatentActionInfo LatentActionInfo);

	/**
	 * @brief Posts a Wwise Event at the specified location.
	 *
	 * This is a fire and forget sound, created on a temporary Wwise Game Object. Replication is also not handled at this point.
	 *
	 * @param Location Location from which to post the Wwise Event.
	 * @param Orientation Orientation of the event.
	 * @param Callback Function that gets called every time the operation defined by CallbackMask is processed.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param WorldContextObject An object having the world we target as context.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic", meta=(WorldContext="WorldContextObject", AdvancedDisplay = "2"))
	int32 PostAtLocation(const FVector Location,
						 const FRotator Orientation,
					     const FOnAkPostEventCallback& Callback,
					     UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) const int32 CallbackMask,
						 const UObject* WorldContextObject);

	/**
	 * @brief Executes action on the different playing IDs from this event that were previously posted on the
	 * Actor's root component.
	 *
	 * @param ActionType What action to do.
	 * @param Actor The actor that initially got some event posted.
	 * @param PlayingID Use the return value of a Post Event to act only on this specific instance of an event.
	 *			Use 0 for all the posted operations from this event.
	 * @param TransitionDuration Transition duration in milliseconds.
	 * @param FadeCurve The interpolation curve of the transition.
	 * @return AKRESULT for the operation. AK_Success (0) if successful.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|Actor", meta=(AdvancedDisplay = "2"))
	int32 ExecuteAction(const AkActionOnEventType ActionType,
					    const AActor* Actor,
					    const int32 PlayingID = 0,
					    const int32 TransitionDuration = 0,
					    const EAkCurveInterpolation FadeCurve = EAkCurveInterpolation::Linear);

public:
	/**
	 * @brief Posts the Wwise Event on the root component of the specified actor.
	 *
	 * @param Actor Actor on which to play the event. This actor gets followed automatically by the Event. If the Actor is left empty,
	 *			the Event will be played as an Ambient sound.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 *			This version is useful for Blueprint callbacks. This is mutually exclusive with AkCallback and LatentAction.
	 * @param Callback Function that gets called every time the operation defined by CallbackMask is processed,
	 *			with the Cookie parameter to provide data. This is mutually exclusive with Delegate and LatentAction.
	 * @param Cookie Parameter provided for AkCallback to provide data.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param LatentAction Function called when the posted event is done playing for latent Blueprint operation.
	 *			This is mutually exclusive with Delegate and Callback.
	 * @param bStopWhenAttachedObjectDestroyed Specifies whether the sound should stop playing when the owner of the attach
	 *			to component is destroyed. This parameter modifies the AkComponent itself, you can only have one behavior per
	 *			actor's root component.
	 * @param AudioContext Context in which the Event's sounds are played. Only GamePlayAudio sounds are affected by PIE pause/stop.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	AkPlayingID PostOnActor(const AActor* Actor,
		const FOnAkPostEventCallback* Delegate,
		AkCallbackFunc Callback,
		void* Cookie,
		const AkCallbackType CallbackMask,
		FWaitEndOfEventAction* LatentAction,
		const bool bStopWhenAttachedObjectDestroyed,
		const EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio);

	/**
	 * @brief Posts the Wwise Event on the specified component.
	 *
	 * @param Component Component on which to play the event.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 *			This version is useful for Blueprint callbacks. This is mutually exclusive with AkCallback and LatentAction.
	 * @param Callback Function that gets called every time the operation defined by CallbackMask is processed,
	 *			with the Cookie parameter to provide data. This is mutually exclusive with Delegate and LatentAction.
	 * @param Cookie Parameter provided for AkCallback to provide data.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param LatentAction Function called when the posted event is done playing for latent Blueprint operation.
	 *			This is mutually exclusive with Delegate and Callback.
	 * @param bStopWhenAttachedObjectDestroyed Specifies whether the sound should stop playing when the owner of the attach
	 *			to component is destroyed. This parameter modifies the AkComponent itself, you can only have one behavior per
	 *			actor's root component.
	 * @param AudioContext Context in which the Event's sounds are played. Only GamePlayAudio sounds are affected by PIE pause/stop.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	AkPlayingID PostOnComponent(UAkComponent* Component,
		const FOnAkPostEventCallback* Delegate,
		AkCallbackFunc Callback,
		void* Cookie,
		const AkCallbackType CallbackMask,
		FWaitEndOfEventAction* LatentAction,
		const bool bStopWhenAttachedObjectDestroyed,
		const EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio);

	/**
	 * @brief Posts a Wwise Event at the specified location.
	 *
	 * This is a fire and forget sound, created on a temporary Wwise Game Object. Replication is also not handled at this point.
	 *
	 * @param Location Location from which to post the Wwise Event.
	 * @param Orientation Orientation of the event.
	 * @param World The world that defines the Location's coordinates.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 *			This version is useful for Blueprint callbacks. This is mutually exclusive with AkCallback and LatentAction.
	 * @param Callback Function that gets called every time the operation defined by CallbackMask is processed,
	 *			with the Cookie parameter to provide data. This is mutually exclusive with Delegate and LatentAction.
	 * @param Cookie Parameter provided for AkCallback to provide data.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param LatentAction Function called when the posted event is done playing for latent Blueprint operation.
	 *			This is mutually exclusive with Delegate and Callback.
	 * @param AudioContext Context in which the Event's sounds are played. Only GamePlayAudio sounds are affected by PIE pause/stop.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	AkPlayingID PostAtLocation(const FVector& Location, const FRotator& Orientation, const UWorld* World,
		const FOnAkPostEventCallback* Delegate,
		AkCallbackFunc Callback,
		void* Cookie,
		const AkCallbackType CallbackMask,
		FWaitEndOfEventAction* LatentAction,
		const EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio);

	/**
	 * @brief Posts a Wwise Event onto a dummy object.
	 *
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 *			This version is useful for Blueprint callbacks. This is mutually exclusive with AkCallback and LatentAction.
	 * @param Callback Function that gets called every time the operation defined by CallbackMask is processed,
	 *			with the Cookie parameter to provide data. This is mutually exclusive with Delegate and LatentAction.
	 * @param Cookie Parameter provided for AkCallback to provide data.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param LatentAction Function called when the posted event is done playing for latent Blueprint operation.
	 *			This is mutually exclusive with Delegate and Callback.
	 * @param AudioContext Context in which the Event's sounds are played. Only GamePlayAudio sounds are affected by PIE pause/stop.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	AkPlayingID PostAmbient(
		const FOnAkPostEventCallback* Delegate,
		AkCallbackFunc Callback,
		void* Cookie,
		const AkCallbackType CallbackMask,
		FWaitEndOfEventAction* LatentAction,
		const EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio);

	/**
	 * @brief Posts the Wwise Event on the specified game object.
	 *
	 * @param GameObject Game object on which to play the event.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 *			This version is useful for Blueprint callbacks. This is mutually exclusive with AkCallback and LatentAction.
	 * @param Callback Function that gets called every time the operation defined by CallbackMask is processed,
	 *			with the Cookie parameter to provide data. This is mutually exclusive with Delegate and LatentAction.
	 * @param Cookie Parameter provided for AkCallback to provide data.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param LatentAction Function called when the posted event is done playing for latent Blueprint operation.
	 *			This is mutually exclusive with Delegate and Callback.
	 * @param AudioContext Context in which the Event's sounds are played. Only GamePlayAudio sounds are affected by PIE pause/stop.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	AkPlayingID PostOnGameObject(UAkGameObject* GameObject,
		const FOnAkPostEventCallback* Delegate,
		AkCallbackFunc Callback,
		void* Cookie,
		const AkCallbackType CallbackMask,
		FWaitEndOfEventAction* LatentAction,
		const EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio);

	/**
	 * @brief Posts the Wwise Event on the specified game object ID.
	 *
	 * @param GameObjectID Game object's ID on which to play the event.
	 * @param Delegate Function that gets called every time the operation defined by CallbackMask is processed.
	 *			This version is useful for Blueprint callbacks. This is mutually exclusive with AkCallback and LatentAction.
	 * @param Callback Function that gets called every time the operation defined by CallbackMask is processed,
	 *			with the Cookie parameter to provide data. This is mutually exclusive with Delegate and LatentAction.
	 * @param Cookie Parameter provided for AkCallback to provide data.
	 * @param CallbackMask Bitmask defining all the operations that will call the Callback. See \ref AkCallbackType.
	 * @param LatentAction Function called when the posted event is done playing for latent Blueprint operation.
	 *			This is mutually exclusive with Delegate and Callback.
	 * @param AudioContext Context in which the Event's sounds are played. Only GamePlayAudio sounds are affected by PIE pause/stop.
	 * @return The Playing ID returned by the SoundEngine's PostEvent, or AK_INVALID_PLAYING_ID (0) if invalid.
	 */
	AkPlayingID PostOnGameObjectID(const AkGameObjectID GameObjectID,
		const FOnAkPostEventCallback* Delegate,
		AkCallbackFunc Callback,
		void* Cookie,
		const AkCallbackType CallbackMask,
		FWaitEndOfEventAction* LatentAction,
		const EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio);

private:
	template<typename FCreateCallbackPackage>
	AkPlayingID PostEvent(const AkGameObjectID GameObjectID,
		FCreateCallbackPackage&& CreateCallbackPackage,
		const EAkAudioContext AudioContext
	);

public:
	void Serialize(FArchive& Ar) override;
	void BeginDestroy() override;

	virtual void LoadData()   override {LoadEventData();}
	virtual void UnloadData(bool bAsync = false) override {UnloadEventData(bAsync);}
	virtual AkUInt32 GetShortID() const override {return EventCookedData.EventId;}
	bool IsDataFullyLoaded() const;
	bool IsLoaded() const;

#if WITH_EDITOR
	// Allow for content browser preview to work, even if asset is not auto-loaded.
	// This method will load the content, and register to BeginPIE. When a PIE session
	// begins, data will be unloaded, allowing to replicate in-game behaviour more
	// closely.
	void LoadEventDataForContentBrowserPreview();

private:
	void OnBeginPIE(const bool bIsSimulating);
	FDelegateHandle OnBeginPIEDelegateHandle;
#endif
#if WITH_EDITORONLY_DATA
public:
	virtual void FillInfo() override;
	virtual void FillMetadata(FWwiseProjectDatabase* ProjectDatabase) override;
	void CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
		TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile) override;
	virtual FWwiseObjectInfo* GetInfoMutable() override {return &EventInfo;}
	virtual FWwiseObjectInfo GetInfo() const override{return EventInfo;}
	virtual bool ObjectIsInSoundBanks() override;
#endif

	TArray<FWwiseExternalSourceCookedData> GetAllExternalSources() const;

private:
	void LoadEventData();
	void UnloadEventData(bool bAsync);
	FWwiseLoadedEventPtrAtomic LoadedEvent{nullptr};
};

