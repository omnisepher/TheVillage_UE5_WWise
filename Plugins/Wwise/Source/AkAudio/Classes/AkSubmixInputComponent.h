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
AkSubmixInputComponent.h:
=============================================================================*/

#pragma once

#include "AkInclude.h"
#include "AudioDevice.h"
#include "AkAudioInputComponent.h"
#if UE_5_1_OR_LATER
#include "ISubmixBufferListener.h"
#endif
#include "AkSubmixInputComponent.generated.h"


/*------------------------------------------------------------------------------------
UAkSubmixInputComponent
------------------------------------------------------------------------------------*/
UCLASS(ClassGroup = Audiokinetic, BlueprintType, hidecategories = (Transform, Rendering, Mobility, LOD, Component, Activation), meta = (BlueprintSpawnableComponent))
class AKAUDIO_API UAkSubmixInputComponent 
	: public UAkAudioInputComponent 
	, public ISubmixBufferListener
{
    GENERATED_BODY()
public:
	UAkSubmixInputComponent(const class FObjectInitializer& ObjectInitializer);

	virtual void OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 SampleRate, double AudioClock) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubmixInput")
	USoundSubmix* SubmixToRecord = nullptr;

	virtual int32 PostAssociatedAudioInputEvent();
	virtual void Stop();
protected:
	/** The audio callback. This will be called continuously by the Wwise sound engine,
	  * and is used to provide the sound engine with audio samples. If this function returns false, the audio
	  * input event will be stopped and the function will stop being called.
	  */
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) override;
	/** This callback is used to provide the Wwise sound engine with the required audio format. */
	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;

private:
	int32 NumChannels;
	int32 SampleRate;
	int32 BufferLength;
	Audio::TCircularAudioBuffer<float> SampleBuffer;
	TArray<float> PoppedSamples;

	Audio::FMixerDevice* GetAudioMixerDevice();
	int32 PlayingID = AK_INVALID_PLAYING_ID;
};