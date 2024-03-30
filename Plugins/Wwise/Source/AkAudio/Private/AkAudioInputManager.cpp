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

#include "AkAudioInputManager.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#if WITH_EDITOR
#include "Editor.h"
#endif
#include "Wwise/API/WwiseSoundEngineAPI.h"

#include "Misc/ScopeLock.h"

#include <inttypes.h>

#include "AkComponent.h"

/*------------------------------------------------------------------------------------
FAudioInputDelegates
Helper struct that contains an audio samples delegate and an audio format delegate
------------------------------------------------------------------------------------*/

struct FAudioInputDelegates
{
	FAkGlobalAudioInputDelegate AudioSamplesDelegate;
	FAkGlobalAudioFormatDelegate AudioFormatDelegate;
};

/*------------------------------------------------------------------------------------
FAkAudioInputHelpers
------------------------------------------------------------------------------------*/

namespace FAkAudioInputHelpers
{
	static FCriticalSection MapSection;
	static TArray<float*> AudioData = TArray<float*>();
	/* A Map of playing ids to input delegates */
	static TMap<uint32, FAudioInputDelegates> AudioInputDelegates = TMap<uint32, FAudioInputDelegates>();

	static AkSampleType* GetChannel(AkAudioBuffer* Buffer, AkUInt32 in_uIndex)
	{
		check(Buffer);
		check( in_uIndex < Buffer->NumChannels() );
		return (AkSampleType*)((AkUInt8*)(Buffer->GetInterleavedData()) + ( in_uIndex * sizeof(AkSampleType) * Buffer->MaxFrames() ));
	}

	static void UpdateDataPointers(AkAudioBuffer* BufferToFill)
	{
		AkUInt32 NumChannels = BufferToFill->NumChannels();
		for (AkUInt32 c = 0; c < NumChannels; ++c)
		{
			AudioData[c] = GetChannel(BufferToFill, c);
		}
	}

	// This is the equivalent of AkAudioBuffer::ZeroPadToMaxFrames, but doesn't use global variables.
	static void ZeroPadToMaxFrames(AkAudioBuffer* Buffer)
	{
		check(Buffer);
		auto pData = Buffer->GetInterleavedData();
		check(pData || Buffer->MaxFrames() == 0)

		// The following members MUST be copied locally due to multi-core calls to this function.
		const AkUInt32 uNumChannels = Buffer->NumChannels();
		const AkUInt32 uNumCurrentFrames = FMath::Min(Buffer->uValidFrames, Buffer->MaxFrames());
		const AkUInt32 uNumZeroFrames = Buffer->MaxFrames() - uNumCurrentFrames;
		if ( uNumZeroFrames )
		{
			check(pData);
			for ( AkUInt32 i = 0; i < uNumChannels; ++i )
			{
				FPlatformMemory::Memset( GetChannel(Buffer, i) + uNumCurrentFrames, 0, uNumZeroFrames * sizeof(AkSampleType) );
			}
			Buffer->uValidFrames = Buffer->MaxFrames();
		}
	}

	/* The global audio samples callback that searches AudioInputDelegates for
	   the key PlayingID and executes the corresponding delegate*/
	static void GetAudioSamples(AkPlayingID PlayingID, AkAudioBuffer* BufferToFill)
	{
		if (!BufferToFill)
		{
			return;
		}

		BufferToFill->eState = AK_NoMoreData;

		AkUInt32 NumChannels = BufferToFill->NumChannels();
		const AkUInt16 NumFrames = BufferToFill->MaxFrames();

		BufferToFill->uValidFrames = NumFrames;

		FAkGlobalAudioInputDelegate SamplesCallback;

		{
			FScopeLock MapLock(&MapSection);
			auto Delegates = AudioInputDelegates.Find((uint32)PlayingID);
			if (Delegates)
			{
				SamplesCallback = Delegates->AudioSamplesDelegate;
			}
		}
		
		if (SamplesCallback.IsBound())
		{
			UpdateDataPointers(BufferToFill);
			if(auto AudioDataPtr = AudioData.GetData())
			{
				if (SamplesCallback.Execute((int)NumChannels, (int)NumFrames, AudioDataPtr))
				{
					BufferToFill->eState = AK_DataReady;
				}
			}
		}
		else
		{
			ZeroPadToMaxFrames(BufferToFill);

		}
	}

	/* The global audio format callback that searches AudioInputDelegates for
	the key PlayingID and executes the corresponding delegate*/
	static void GetAudioFormat(AkPlayingID PlayingID, AkAudioFormat& AudioFormat)
	{
		FAkGlobalAudioFormatDelegate FormatCallback;

		{
			FScopeLock MapLock(&MapSection);
			auto Delegates = AudioInputDelegates.Find((uint32)PlayingID);
			if (Delegates)
			{
				FormatCallback = Delegates->AudioFormatDelegate;
			}
		}

		if (FormatCallback.IsBound())
		{
			FormatCallback.Execute(AudioFormat);
		}
		const uint32 NumChannels = AudioFormat.channelConfig.uNumChannels;
		if (AudioData.Max() < (int32)NumChannels)
		{
			AudioData.Reserve(NumChannels);
			AudioData.AddUninitialized(AudioData.GetSlack());
		}
	}

	/**
	* Sets the main callbacks for the Wwise engine audio input plugin.
	*
	*/
	static void SetAkAudioInputCallbacks()
	{
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->AudioInputPlugin->SetAudioInputCallbacks(
			&FAkAudioInputHelpers::GetAudioSamples,
			&FAkAudioInputHelpers::GetAudioFormat,
			nullptr);
	}
	/* Protects against calling Wwise sound engine SetAudioInputCallbacks function more than once */
	static bool bIsInitialized = false;
	/* Calls the Wwise sound engine SetAudioInputCallbacks function*/
	static void TryInitialize()
	{
		if (!bIsInitialized)
		{
			SetAkAudioInputCallbacks();
			bIsInitialized = true;
		}
#if WITH_EDITOR
		FEditorDelegates::EndPIE.AddLambda([](const bool bIsSimulating) 
		{
			bIsInitialized = false;
		});
#endif
	}

	static void AddAudioInputPlayingID(AkPlayingID PlayingID,
		FAkGlobalAudioInputDelegate AudioSamplesDelegate,
		FAkGlobalAudioFormatDelegate AudioFormatDelegate)
	{
		FScopeLock MapLock(&MapSection);
		AudioInputDelegates.Add((uint32)PlayingID, { AudioSamplesDelegate, AudioFormatDelegate });
	}

	/* Posts an event and associates the AudioSamplesDelegate and AudioFormatDelegate delegates with the resulting playing id. */
	AkPlayingID PostAudioInputEvent(TFunction<AkPlayingID(FAkAudioDevice* AkDevice)> PostEventCall,
							        FAkGlobalAudioInputDelegate AudioSamplesDelegate,
							        FAkGlobalAudioFormatDelegate AudioFormatDelegate)
	{
		TryInitialize();
		AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;
		FAkAudioDevice* AkDevice = FAkAudioDevice::Get();
		if (AkDevice != nullptr)
		{
			PlayingID = PostEventCall(AkDevice);
			if (PlayingID != AK_INVALID_PLAYING_ID)
			{
				AddAudioInputPlayingID(PlayingID, AudioSamplesDelegate, AudioFormatDelegate);
			}
		}
		return PlayingID;
	}

    static void EventCallback(AkCallbackType CallbackType, AkCallbackInfo *CallbackInfo)
	{
		if (CallbackType == AkCallbackType::AK_EndOfEvent)
		{
			AkEventCallbackInfo* EventInfo = (AkEventCallbackInfo*)CallbackInfo;
			if (EventInfo != nullptr)
			{
				uint32 PlayingID = (uint32)EventInfo->playingID;

				{
					FScopeLock MapLock(&MapSection);
					AudioInputDelegates.Remove(PlayingID);
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------------
FAkAudioInputManager
------------------------------------------------------------------------------------*/

AkPlayingID FAkAudioInputManager::PostAudioInputEvent(
    UAkAudioEvent * Event,
    AActor * Actor,
    FAkGlobalAudioInputDelegate AudioSamplesDelegate,
    FAkGlobalAudioFormatDelegate AudioFormatDelegate,
	EAkAudioContext AudioContext
)
{
	if (!IsValid(Event))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioInputManager::PostAudioInputEvent: Invalid AkEvent."))
		return AK_INVALID_PLAYING_ID;
	}
	if (!IsValid(Actor))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioInputManager::PostAudioInputEvent: Invalid Actor playing AkEvent %s."), *Event->GetName())
		return AK_INVALID_PLAYING_ID;
	}

	return FAkAudioInputHelpers::PostAudioInputEvent([Event, Actor, AudioContext](FAkAudioDevice* AkDevice)
	{
		const auto Result = Event->PostOnActor(
			Actor,
			nullptr,
			&FAkAudioInputHelpers::EventCallback,
			nullptr,
			AkCallbackType::AK_EndOfEvent,
			nullptr,
			false,
			AudioContext);
		UE_CLOG(UNLIKELY(Result == AK_INVALID_PLAYING_ID), LogAkAudio, Warning,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Failed posting input event %s to actor %s."), *Event->GetName(), *Actor->GetName());
		UE_CLOG(LIKELY(Result != AK_INVALID_PLAYING_ID), LogAkAudio, VeryVerbose,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Posted input event %s to actor %s. PlayId=%" PRIu32), *Event->GetName(), *Actor->GetName(), Result);
		return Result;
	}, AudioSamplesDelegate, AudioFormatDelegate);
}

AkPlayingID FAkAudioInputManager::PostAudioInputEvent(
	UAkAudioEvent* Event,
	UAkComponent* Component,
	FAkGlobalAudioInputDelegate AudioSamplesDelegate,
	FAkGlobalAudioFormatDelegate AudioFormatDelegate,
	EAkAudioContext AudioContext)
{
	if (!IsValid(Event))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioInputManager::PostAudioInputEvent: Invalid AkEvent."))
		return AK_INVALID_PLAYING_ID;
	}
	if (!Component)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioInputManager::PostAudioInputEvent: Invalid Component playing AkEvent %s."), *Event->GetName())
		return AK_INVALID_PLAYING_ID;
	}

	return FAkAudioInputHelpers::PostAudioInputEvent([Event, Component, AudioContext](FAkAudioDevice* AkDevice)
	{
		const auto Result = Event->PostOnComponent(
			Component,
			nullptr,
			&FAkAudioInputHelpers::EventCallback,
			nullptr,
			AkCallbackType::AK_EndOfEvent,
			nullptr,
			false,
			AudioContext);
		UE_CLOG(UNLIKELY(Result == AK_INVALID_PLAYING_ID), LogAkAudio, Warning,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Failed posting input event %s to component %s."), *Event->GetName(), *Component->GetName());
		UE_CLOG(LIKELY(Result != AK_INVALID_PLAYING_ID), LogAkAudio, VeryVerbose,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Posted input event %s to component %s. PlayId=%" PRIu32), *Event->GetName(), *Component->GetName(), Result);
		return Result;
	}, AudioSamplesDelegate, AudioFormatDelegate);
}

AkPlayingID FAkAudioInputManager::PostAudioInputEvent(
	UAkAudioEvent* Event,
	AkGameObjectID GameObject,
	FAkGlobalAudioInputDelegate AudioSamplesDelegate,
	FAkGlobalAudioFormatDelegate AudioFormatDelegate,
	EAkAudioContext AudioContext)
{
	if (!IsValid(Event))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioInputManager::PostAudioInputEvent: Invalid AkEvent."))
		return AK_INVALID_PLAYING_ID;
	}
	if (GameObject == AK_INVALID_GAME_OBJECT)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioInputManager::PostAudioInputEvent: Invalid GameObject playing AkEvent %s."), *Event->GetName())
		return AK_INVALID_PLAYING_ID;
	}
	return FAkAudioInputHelpers::PostAudioInputEvent([Event, GameObject, AudioContext](FAkAudioDevice* AkDevice)
	{
		const auto Result = Event->PostOnGameObjectID(
			GameObject,
			nullptr,
			&FAkAudioInputHelpers::EventCallback,
			nullptr,
			AkCallbackType::AK_EndOfEvent,
			nullptr,
			AudioContext);
		UE_CLOG(UNLIKELY(Result == AK_INVALID_PLAYING_ID), LogAkAudio, Warning,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Failed posting input event %s to %" PRIu64 "."), *Event->GetName(), GameObject);
		UE_CLOG(LIKELY(Result != AK_INVALID_PLAYING_ID), LogAkAudio, VeryVerbose,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Posted input event %s to %" PRIu64 ". PlayId=%" PRIu32), *Event->GetName(), GameObject, Result);
		return Result;
	}, AudioSamplesDelegate, AudioFormatDelegate);
}

AkPlayingID FAkAudioInputManager::PostAudioInputEvent(UAkAudioEvent* Event,
	FAkGlobalAudioInputDelegate AudioSamplesDelegate, FAkGlobalAudioFormatDelegate AudioFormatDelegate,
	EAkAudioContext AudioContext)
{
	if (!IsValid(Event))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("FAkAudioInputManager::PostAudioInputEvent: Invalid AkEvent."))
		return AK_INVALID_PLAYING_ID;
	}
	return FAkAudioInputHelpers::PostAudioInputEvent([Event, AudioContext](FAkAudioDevice* AkDevice)
	{
		const auto Result = Event->PostAmbient(
			nullptr,
			&FAkAudioInputHelpers::EventCallback,
			nullptr,
			AkCallbackType::AK_EndOfEvent,
			nullptr,
			AudioContext);
		UE_CLOG(UNLIKELY(Result == AK_INVALID_PLAYING_ID), LogAkAudio, Warning,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Failed posting ambient input event %s."), *Event->GetName());
		UE_CLOG(LIKELY(Result != AK_INVALID_PLAYING_ID), LogAkAudio, VeryVerbose,
			TEXT("FAkAudioInputManager::PostAudioInputEvent: Posted ambient input event %s. PlayId=%" PRIu32), *Event->GetName(), Result);
		return Result;
	}, AudioSamplesDelegate, AudioFormatDelegate);
}
