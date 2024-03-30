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

#include "AkInclude.h"
#include "AkAudioDevice.h"
#include "Templates/Function.h"

/*------------------------------------------------------------------------------------
AkAudioInput Delegates
------------------------------------------------------------------------------------*/

DECLARE_DELEGATE_RetVal_ThreeParams(bool, FAkGlobalAudioInputDelegate, uint32, uint32, float**);
DECLARE_DELEGATE_OneParam(FAkGlobalAudioFormatDelegate, AkAudioFormat&);

/*------------------------------------------------------------------------------------
FAkAudioInputManager
------------------------------------------------------------------------------------*/

class AKAUDIO_API FAkAudioInputManager
{
public:

    /**
     * Post an Input event to Wwise SoundEngine linked to an actor
     *
     * @param Event Event to post
     * @param Actor Actor on which to play the event
     * @param AudioSamplesDelegate Callback that fills the audio samples buffer
     * @param AudioFormatDelegate Callback that sets the audio format
     * @param AudioContext Context where this input is used (Editor, Player, or other)
     * @return ID assigned by Wwise SoundEngine
     */
    static AkPlayingID PostAudioInputEvent(
	    UAkAudioEvent* Event,
	    AActor* Actor,
	    FAkGlobalAudioInputDelegate AudioSamplesDelegate,
	    FAkGlobalAudioFormatDelegate AudioFormatDelegate,
        EAkAudioContext AudioContext = EAkAudioContext::Foreign
    );

	/**
	 * Post an Input event to Wwise SoundEngine linked to a Component
	 *
	 * @param Event Event to post
	 * @param Component Component on which to play the event
	 * @param AudioSamplesDelegate Callback that fills the audio samples buffer
	 * @param AudioFormatDelegate Callback that sets the audio format
     * @param AudioContext Context where this input is used (Editor, Player, or other)
     * @return ID assigned by Wwise SoundEngine
	 */
	static AkPlayingID PostAudioInputEvent(
		UAkAudioEvent* Event,
		UAkComponent* Component,
		FAkGlobalAudioInputDelegate AudioSamplesDelegate,
		FAkGlobalAudioFormatDelegate AudioFormatDelegate,
        EAkAudioContext AudioContext = EAkAudioContext::Foreign
	);

	/**
	 * Post an Input event to Wwise SoundEngine linked to a GameObject
	 *
	 * @param Event Event to post
	 * @param GameObject GameObject on which to play the event
	 * @param AudioSamplesDelegate Callback that fills the audio samples buffer
	 * @param AudioFormatDelegate Callback that sets the audio format
     * @param AudioContext Context where this input is used (Editor, Player, or other)
     * @return ID assigned by Wwise SoundEngine
	 */
	static AkPlayingID PostAudioInputEvent(
		UAkAudioEvent* Event,
		AkGameObjectID GameObject,
		FAkGlobalAudioInputDelegate AudioSamplesDelegate,
		FAkGlobalAudioFormatDelegate AudioFormatDelegate,
        EAkAudioContext AudioContext = EAkAudioContext::Foreign
	);

	/**
	 * Post an Input event to Wwise SoundEngine not linked to any GameObject
	 *
	 * @param Event Event to post
	 * @param AudioSamplesDelegate Callback that fills the audio samples buffer
	 * @param AudioFormatDelegate Callback that sets the audio format
     * @param AudioContext Context where this input is used (Editor, Player, or other)
     * @return ID assigned by Wwise SoundEngine
	 */
	static AkPlayingID PostAudioInputEvent(
		UAkAudioEvent* Event,
		FAkGlobalAudioInputDelegate AudioSamplesDelegate,
		FAkGlobalAudioFormatDelegate AudioFormatDelegate,
        EAkAudioContext AudioContext = EAkAudioContext::Foreign
	);
};