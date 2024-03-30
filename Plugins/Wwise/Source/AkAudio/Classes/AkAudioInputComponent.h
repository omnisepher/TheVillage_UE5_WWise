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
UAkAudioInputComponent.h:
=============================================================================*/

#pragma once

#include "AkAudioInputManager.h"
#include "AkInclude.h"
#include "AkComponent.h"
#include "AkAudioInputComponent.generated.h"


/*------------------------------------------------------------------------------------
UAkAudioInputComponent
------------------------------------------------------------------------------------*/
UCLASS(ClassGroup = Audiokinetic, abstract, BlueprintType, hidecategories = (Transform, Rendering, Mobility, LOD, Component, Activation), meta = (BlueprintSpawnableComponent))
class AKAUDIO_API UAkAudioInputComponent : public UAkComponent
{
    GENERATED_BODY()

public:
	UAkAudioInputComponent(const class FObjectInitializer& ObjectInitializer);

    /**
    * Posts this component's AkAudioEvent to Wwise along with associated AudioSamples callback and AudioFormat callback, using this component as the game object source
    *
    */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|AkAudioInputComponent")
	virtual int32 PostAssociatedAudioInputEvent();

protected:
	/** This is called after the GameObject that owns this component is unregistered from the Wwise sound engine. */
	virtual void PostUnregisterGameObject() override;
	/** The audio callback. This will be called continuously by the Wwise sound engine,
	  * and is used to provide the sound engine with audio samples. If this function returns false, the audio
	  * input event will be stopped and the function will stop being called.
	  */
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) PURE_VIRTUAL(AkAudioInputComponent::FillSamplesBuffer, return false;);
	/** This callback is used to provide the Wwise sound engine with the required audio format. */
	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) PURE_VIRTUAL(UAkAudioInputComponent::GetChannelConfig,);

	TArray<AkPlayingID> CurrentlyPlayingIDs;

private:
	FAkGlobalAudioInputDelegate AudioInputDelegate;
	FAkGlobalAudioFormatDelegate AudioFormatDelegate;
};