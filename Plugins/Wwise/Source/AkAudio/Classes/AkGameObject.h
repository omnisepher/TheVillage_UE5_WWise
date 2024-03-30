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
	AkGameObject.h:
=============================================================================*/

#pragma once

#include "AkAudioDevice.h"
#include "AkGameplayTypes.h"
#include "Components/SceneComponent.h"
#include "AkGameObject.generated.h"


UCLASS(ClassGroup=Audiokinetic, BlueprintType, Blueprintable, hidecategories=(Transform,Rendering,Mobility,LOD,Component,Activation), AutoExpandCategories=AkComponent, meta=(BlueprintSpawnableComponent))
class AKAUDIO_API UAkGameObject: public USceneComponent
{
	GENERATED_BODY()

public:
	UAkGameObject(const class FObjectInitializer& ObjectInitializer);

	/** Associated Wwise Event to be posted on this game object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AkEvent")
	UAkAudioEvent* AkAudioEvent = nullptr;

	/**
	 * Posts this game object's AkAudioEvent to Wwise, using this as the game object source
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkGameObject", meta = (AdvancedDisplay = "2", AutoCreateRefTerm = "PostEventCallback,ExternalSources"))
	virtual int32 PostAssociatedAkEvent(
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) int32 CallbackMask,
		const FOnAkPostEventCallback& PostEventCallback);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkGameObject", meta = (AutoCreateRefTerm = "PostEventCallback,ExternalSources", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	virtual void PostAssociatedAkEventAsync(const UObject* WorldContextObject,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) int32 CallbackMask,
		const FOnAkPostEventCallback& PostEventCallback,
		FLatentActionInfo LatentInfo,
		int32& PlayingID);

	/**
	 * Posts an event to Wwise, using this as the game object source
	 *
	 * @param AkEvent			The event to post
	 * @param CallbackMask		Mask of desired callbacks
	 * @param PostEventCallback	Blueprint Event to execute on callback
	 *
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkGameObject", meta = (AdvancedDisplay = "1", AutoCreateRefTerm = "PostEventCallback,ExternalSources"))
	virtual int32 PostAkEvent(
		class UAkAudioEvent * AkEvent,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) int32 CallbackMask,
		const FOnAkPostEventCallback& PostEventCallback
	);

	virtual AkPlayingID PostAkEvent(UAkAudioEvent* AkEvent, AkUInt32 Flags = 0, AkCallbackFunc UserCallback = nullptr, void* UserCookie = nullptr);

	/**
	 * Posts an event to Wwise, using this as the game object source
	 *
	 * @param AkEvent		The event to post
	 * @param CallbackMask	Mask of desired callbacks
	 * @param PostEventCallback	Blueprint Event to execute on callback
	 *
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkGameObject", meta = (AdvancedDisplay = "3", AutoCreateRefTerm = "PostEventCallback,ExternalSources", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	virtual void PostAkEventAsync(const UObject* WorldContextObject,
			class UAkAudioEvent* AkEvent,
			int32& PlayingID,
			UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) int32 CallbackMask,
			const FOnAkPostEventCallback& PostEventCallback,
			FLatentActionInfo LatentInfo
	);

	/**
	 * Stops playback using this game object as the game object to stop
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkComponent")
	virtual void Stop();

	/**
	* Sets an RTPC value, using this game object as the game object source
	*
	* @param RTPC			The name of the RTPC to set
	* @param Value			The value of the RTPC
	* @param InterpolationTimeMs - Duration during which the RTPC is interpolated towards Value (in ms)
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkGameObject", meta = (AdvancedDisplay = "3"))
	void SetRTPCValue(class UAkRtpc const* RTPCValue, float Value, int32 InterpolationTimeMs, FString RTPC) const;

	/**
	* Gets an RTPC value that was set on this game object as the game object source
	*
	* @param RTPC				The name of the RTPC to set
	* @param InputValueType		The input value type
	* @param Value				The value of the RTPC
	* @param OutputValueType	The output value type
	* @param PlayingID			The playing ID of the posted event (Set to zero to ignore)
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkGameObject", meta = (AdvancedDisplay = "RTPC"))
	void GetRTPCValue(class UAkRtpc const* RTPCValue, ERTPCValueType InputValueType, float& Value, ERTPCValueType& OutputValueType, FString RTPC, int32 PlayingID = 0) const;

#if CPP
	bool VerifyEventName(const FString& InEventName) const;
	bool AllowAudioPlayback() const;
	AkGameObjectID GetAkGameObjectID() const;
	virtual void UpdateObstructionAndOcclusion() {};
	bool HasActiveEvents() const;
#endif

	bool HasBeenRegisteredWithWwise() const { return IsRegisteredWithWwise; }
	void EventPosted() {bEventPosted = true;}
protected:
	// Whether an event was posted on the game object. Never reset to false. 
	bool bEventPosted;
	bool IsRegisteredWithWwise = false;
};
