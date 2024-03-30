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

#include "AudioMixerInputComponent.h"
#include "AkAudioDevice.h"
#include "AkAudioInputManager.h"
#include "AkAudioEvent.h"

#define AK_AUDIO_INPUT_EVENT_NAME "Play_UnrealAudio"

FAudioMixerInputComponent::FAudioMixerInputComponent() :
	PlayingID(AK_INVALID_PLAYING_ID)
{
	auto Device = FAkAudioDevice::Get();
	if (Device != nullptr)
	{
		Device->RegisterComponent(GetAkGameObjectID());
	}
}

FAudioMixerInputComponent::~FAudioMixerInputComponent()
{
	if (OnNextBuffer.IsBound())
	{
		OnNextBuffer.Unbind();
	}
	auto Device = FAkAudioDevice::Get();
	Device->UnregisterComponent(GetAkGameObjectID());
}

bool FAudioMixerInputComponent::FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill)
{
	if (OnNextBuffer.IsBound())
	{
		OnNextBuffer.Execute(NumChannels, NumSamples, BufferToFill);
	}
	return true;
}

/** This callback is used to provide the Wwise sound engine with the required audio format. */
void FAudioMixerInputComponent::GetChannelConfig(AkAudioFormat& OutAudioFormat)
{
	const int sampleRate = 48000;
	OutAudioFormat.uSampleRate = sampleRate;
	OutAudioFormat.channelConfig.SetStandard(AK_SPEAKER_SETUP_STEREO);

	UE_LOG(LogAkAudio, Log, TEXT("Wwise Channel configuration:"));
	UE_LOG(LogAkAudio, Log, TEXT("Wwise Input Sample Rate: %d"), OutAudioFormat.uSampleRate);
	UE_LOG(LogAkAudio, Log, TEXT("Wwise Channel num: %d"), 2);
}

AkPlayingID FAudioMixerInputComponent::PostAssociatedAudioInputEvent(UAkAudioEvent* InputEvent)
{
	PlayingID = FAkAudioInputManager::PostAudioInputEvent(
		InputEvent,
		OnNextBuffer,
		FAkGlobalAudioFormatDelegate::CreateRaw(this, &FAudioMixerInputComponent::GetChannelConfig),
		EAkAudioContext::AlwaysActive);

	return PlayingID;
}

void FAudioMixerInputComponent::PostUnregisterGameObject()
{
	auto Device = FAkAudioDevice::Get();
	if (Device != nullptr)
	{
		if (PlayingID != AK_INVALID_PLAYING_ID)
		{
			Device->StopPlayingID(PlayingID);
			PlayingID = AK_INVALID_PLAYING_ID;
		}
	}
}

AkGameObjectID FAudioMixerInputComponent::GetAkGameObjectID() const
{
	return (AkGameObjectID)this;
}
