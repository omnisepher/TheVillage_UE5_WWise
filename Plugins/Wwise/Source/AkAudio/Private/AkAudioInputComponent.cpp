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
AkAudioInputComponent.cpp:
=============================================================================*/

#include "AkAudioInputComponent.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"

UAkAudioInputComponent::UAkAudioInputComponent(const class FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{}

int32 UAkAudioInputComponent::PostAssociatedAudioInputEvent()
{
	AudioInputDelegate = FAkGlobalAudioInputDelegate::CreateLambda(
		[this](uint32 NumChannels, uint32 NumSamples, float** BufferToFill) -> bool
		{
			return FillSamplesBuffer(NumChannels, NumSamples, BufferToFill);
		});

	AudioFormatDelegate = FAkGlobalAudioFormatDelegate::CreateLambda([this](AkAudioFormat& AudioFormat)
	{
		return GetChannelConfig(AudioFormat);
	});


	AkPlayingID PlayingID = FAkAudioInputManager::PostAudioInputEvent(
		 AkAudioEvent, this, AudioInputDelegate, AudioFormatDelegate);

	if (PlayingID != AK_INVALID_PLAYING_ID)
	{
		CurrentlyPlayingIDs.Add(PlayingID);
	}
	return PlayingID;
}

void UAkAudioInputComponent::PostUnregisterGameObject()
{
	if (AudioInputDelegate.IsBound())
	{
		AudioInputDelegate.Unbind();
	}

	if (AudioFormatDelegate.IsBound())
	{
		AudioFormatDelegate.Unbind();
	}

	auto Device = FAkAudioDevice::Get();
	if (Device != nullptr)
	{
		for (int i = 0; i < CurrentlyPlayingIDs.Num(); ++i)
		{
			Device->StopPlayingID(CurrentlyPlayingIDs[i]);
		}
	}
	CurrentlyPlayingIDs.Empty();
}