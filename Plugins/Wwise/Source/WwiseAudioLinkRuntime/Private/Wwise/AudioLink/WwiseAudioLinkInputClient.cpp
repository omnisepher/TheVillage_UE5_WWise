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

#include "Wwise/AudioLink/WwiseAudioLinkInputClient.h"
#include "Wwise/AudioLink/WwiseAudioLinkSettings.h"
#include "Wwise/AudioLink/WwiseAudioLinkComponent.h"
#include "Wwise/AudioLink/WwiseAudioLinkFactory.h"
#include "Wwise/AudioLink/WwiseAudioLinkSynchronizer.h"

#include "AkAudioInputManager.h"
#include "AkAudioEvent.h"
#include "AkAudioDevice.h"
#include "WwiseUnrealHelper.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AudioLink.h"
#include "Wwise/Stats/Global.h"

#include "AudioDevice.h"
#include "Async/Async.h"
#include "HAL/PlatformMisc.h"

#include <inttypes.h>

FWwiseAudioLinkInputClient::FWwiseAudioLinkInputClient(
	const FSharedBufferedOutputPtr& InToConsumeFrom, 
	const UAudioLinkSettingsAbstract::FSharedSettingsProxyPtr& InSettingsProxy,
	FName InNameOfProducingSource )
	: WeakProducer{ InToConsumeFrom }
	, SettingsProxy{ InSettingsProxy }
	, ProducerName{ InNameOfProducingSource }
{
	Register(InNameOfProducingSource);
}

FWwiseAudioLinkInputClient::~FWwiseAudioLinkInputClient()
{
	Unregister();
}

void FWwiseAudioLinkInputClient::Start(UWwiseAudioLinkComponent* InAkComponent)
{
	SCOPED_NAMED_EVENT(WwiseAudioLink_StartComponent, FColor::Red);
	check(IsInGameThread());

	if (UNLIKELY(ObjectId == AK_INVALID_AUDIO_OBJECT_ID))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Start: Unregistered Object (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
		return;
	}

	if (UNLIKELY(PlayId != AK_INVALID_PLAYING_ID))
	{
		UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Start: Reusing an already playing %" PRIu32 " ObjectID %" PRIu64 " (%s). Stopping previous instance."), PlayId.load(), this, *ProducerName.ToString());
		Stop();
	}

	if (UNLIKELY(!IsValid(InAkComponent)))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Start: Invalid component (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
		return;
	}

	FWwiseAudioLinkSettingsProxy* Settings = static_cast<FWwiseAudioLinkSettingsProxy*>(SettingsProxy.Get());
	if (UNLIKELY(!Settings))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Start: Invalid Settings (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
		return;
	}

	const auto StartEvent = Settings->GetStartEvent();		// Might not be loaded at this point, only the pointer should be valid.
	
	IsLoadedHandle = Settings->CallOnEventLoaded([this, SelfSP = AsShared(), StartEvent, InAkComponent]() mutable
	{
        if (UNLIKELY(ObjectId == UINT64_MAX))
        {
            UE_LOG(LogWwiseAudioLink, Error, TEXT("WwiseAudioLinkInputClient::OnEventLoaded: Invalid Object Id On event loaded. Sound was deleted before the load could be completed. No sound will be played."));
            return;
        }
            
		if (UNLIKELY(!IsValid(InAkComponent)))
		{
			UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Start: Invalid component (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
			return;
		}

		if (UNLIKELY(PlayId != AK_INVALID_PLAYING_ID))
		{
			UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Start (OnEventLoaded): Reusing an already playing %" PRIu32 " Component %" PRIu64 " (%s). Stopping previous instance."), PlayId.load(), InAkComponent->GetAkGameObjectID(), *InAkComponent->GetName());
			Stop();
		}

		PlayId = FAkAudioInputManager::PostAudioInputEvent(
			StartEvent.Get(),
			InAkComponent,
			FAkGlobalAudioInputDelegate::CreateSP(SelfSP, &FWwiseAudioLinkInputClient::GetSamples),
			FAkGlobalAudioFormatDelegate::CreateSP(SelfSP, &FWwiseAudioLinkInputClient::GetFormat)
		);
		UE_CLOG(UNLIKELY(PlayId.load() == AK_INVALID_PLAYING_ID), LogWwiseAudioLink, Error,
			TEXT("FWwiseAudioLinkInputClient::Start: Error playing Component %" PRIu64 " (%s) in client %" PRIu64 " (%s)"),
			InAkComponent->GetAkGameObjectID(), *InAkComponent->GetName(), ObjectId, *ProducerName.ToString());
		UE_CLOG(LIKELY(PlayId.load() != AK_INVALID_PLAYING_ID), LogWwiseAudioLink, Verbose,
			TEXT("FWwiseAudioLinkInputClient::Start: Playing %" PRIu32 " Component %" PRIu64 " (%s) in client %" PRIu64 " (%s)"),
			PlayId.load(), InAkComponent->GetAkGameObjectID(), *InAkComponent->GetName(), ObjectId, *ProducerName.ToString());
	});
}

void FWwiseAudioLinkInputClient::Start()
{
	SCOPED_NAMED_EVENT(WwiseAudioLink_Start, FColor::Red);

	if (UNLIKELY(ObjectId == AK_INVALID_AUDIO_OBJECT_ID))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Start: Unregistered Object (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
		return;
	}

	if (UNLIKELY(PlayId != AK_INVALID_PLAYING_ID))
	{
		UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Start: Reusing an already playing %" PRIu32 " ObjectID %" PRIu64 " (%s). Stopping previous instance."), PlayId.load(), this, *ProducerName.ToString());
		Stop();
	}

	FWwiseAudioLinkSettingsProxy* Settings = static_cast<FWwiseAudioLinkSettingsProxy*>(SettingsProxy.Get());
	if (UNLIKELY(!Settings))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Start: Invalid Settings (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
		return;
	}

	const auto StartEvent = Settings->GetStartEvent();		// Might not be loaded at this point, only the pointer should be valid.
	
	IsLoadedHandle = Settings->CallOnEventLoaded([this, SelfSP = AsShared(), StartEvent]() mutable
	{
		if (UNLIKELY(ObjectId == UINT64_MAX))
		{
			UE_LOG(LogWwiseAudioLink, Error, TEXT("WwiseAudioLinkInputClient::OnEventLoaded: Invalid Object Id On event loaded. Sound was deleted before the load could be completed. No sound will be played."));
			return;
		}
        
		if (UNLIKELY(!StartEvent || !IsValid(StartEvent.Get())))
		{
			UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Start: Invalid StartEvent (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
			return;
		}

		if (UNLIKELY(PlayId != AK_INVALID_PLAYING_ID))
		{
			UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Start (OnEventLoaded): Reusing an already playing %" PRIu32 " ObjectID %" PRIu64 " (%s). Stopping previous instance."), PlayId.load(), this, *ProducerName.ToString());
			Stop();
		}

		PlayId = FAkAudioInputManager::PostAudioInputEvent(
			StartEvent.Get(),
			ObjectId,
			FAkGlobalAudioInputDelegate::CreateSP(SelfSP, &FWwiseAudioLinkInputClient::GetSamples),
			FAkGlobalAudioFormatDelegate::CreateSP(SelfSP, &FWwiseAudioLinkInputClient::GetFormat)
		);

		UE_CLOG(UNLIKELY(PlayId.load() == AK_INVALID_PLAYING_ID), LogWwiseAudioLink, Error,
			TEXT("FWwiseAudioLinkInputClient::Start: Error playing Event '%s' in client %" PRIu64 " (%s)"),
			*StartEvent->GetName(), ObjectId, *ProducerName.ToString());
		UE_CLOG(LIKELY(PlayId.load() != AK_INVALID_PLAYING_ID), LogWwiseAudioLink, Verbose,
			TEXT("FWwiseAudioLinkInputClient::Start: Playing %" PRIu32 " Event '%s' in client %" PRIu64 " (%s)"),
			PlayId.load(), *StartEvent->GetName(), ObjectId, *ProducerName.ToString());
	});
}

void FWwiseAudioLinkInputClient::Stop()
{
	SCOPED_NAMED_EVENT(WwiseAudioLink_Stop, FColor::Red);

	if (IsLoadedHandle.IsValid())
	{
		FWwiseAudioLinkSettingsProxy* Settings = static_cast<FWwiseAudioLinkSettingsProxy*>(SettingsProxy.Get());
		if (UNLIKELY(!Settings))
		{
			UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Stop: Invalid Settings (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
		}
		else
		{
			Settings->UnregisterEventLoadedDelegate(IsLoadedHandle);
		}

		IsLoadedHandle.Reset();
	}

	if (PlayId.load() != AK_INVALID_PLAYING_ID)
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (UNLIKELY(!AudioDevice))
		{
			UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Stop: Invalid AudioDevice (PlayId=%" PRIu32 ", this=%" PRIu64 " %s)"), PlayId.load(), this, *ProducerName.ToString());
		}
		else
		{
			UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkInputClient::Stop: PlayId=%" PRIu32 ", ObjectID=%" PRIu64 " %s"), PlayId.load(), ObjectId, *ProducerName.ToString());
			AudioDevice->StopPlayingID(PlayId);
		}

		PlayId = AK_INVALID_PLAYING_ID;
	}	
}

void FWwiseAudioLinkInputClient::Register(const FName& InNameOfProducingSource)
{
	const auto Name = InNameOfProducingSource.GetPlainNameString();

	if (UNLIKELY(!SettingsProxy.IsValid()))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Register: Invalid settings registering %" PRIu64 " %s."), ObjectId, this, *Name);
		return;
	}

	IWwiseSoundEngineAPI* SoundEngine = IWwiseSoundEngineAPI::Get();
	if(UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Register: No Wwise SoundEngine registering %" PRIu64 " %s."), this, *Name);
		return;
	}

	if (UNLIKELY(ObjectId != AK_INVALID_AUDIO_OBJECT_ID))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Register: ObjectId %" PRIu64 " already registered for %" PRIu64 " %s."), ObjectId, this, *Name);
		return;
	}
	ObjectId = reinterpret_cast<AkAudioObjectID>(this);

	AKRESULT Result;
#ifdef AK_OPTIMIZED
	Result = SoundEngine->RegisterGameObj(ObjectId);
#else
	if (Name.Len() > 0)
	{
		Result = SoundEngine->RegisterGameObj(ObjectId, TCHAR_TO_ANSI(*Name));
	}
	else
	{
		Result = SoundEngine->RegisterGameObj(ObjectId);
	}
#endif
	UE_CLOG(LIKELY(Result == AK_Success), LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkInputClient::Register: Registered Object ID %" PRIu64 " (%s)"), ObjectId, *Name);
	UE_CLOG(UNLIKELY(Result != AK_Success), LogWwiseAudioLink, Warning, TEXT("FWwiseAudioLinkInputClient::Register: Error registering Object ID %" PRIu64 " (%s): (%" PRIu32 ") %s"), ObjectId, *Name, Result, WwiseUnrealHelper::GetResultString(Result));

	// Sanity checks
#ifndef AK_OPTIMIZED
	AsyncTask(ENamedThreads::GameThread, []
	{
		const auto AudioDeviceManager = FAudioDeviceManager::Get();
		if (UNLIKELY(!AudioDeviceManager))
		{
			UE_LOG(LogWwiseAudioLink, Warning, TEXT("FWwiseAudioLinkInputClient::Register: No AudioDeviceManager at registration."));
			return;
		}
		const auto AudioDevice = AudioDeviceManager->GetActiveAudioDevice();
		if (UNLIKELY(!AudioDevice))
		{
			UE_LOG(LogWwiseAudioLink, Warning, TEXT("FWwiseAudioLinkInputClient::Register: No active AudioDevice at registration."));
			return;
		}

		UE_CLOG(UNLIKELY(AudioDevice->GetMaxChannels() == 0),
			LogWwiseHints, Warning, TEXT("WwiseAudioLink: The current AudioDevice %" PRIu32 " has 0 MaxChannel. Consider setting AudioMaxChannels to a sensible value in the Engine config file's TargetSettings for your platform."),
			AudioDevice->DeviceID);
	});
#endif
}

void FWwiseAudioLinkInputClient::Unregister()
{
	IWwiseSoundEngineAPI* SoundEngine = IWwiseSoundEngineAPI::Get();
	if(UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Unregister: No Wwise SoundEngine unregistering %" PRIu64 " (this=%" PRIu64 " %s)."), ObjectId, this, *ProducerName.ToString());
		return;
	}
	if (UNLIKELY(ObjectId == AK_INVALID_AUDIO_OBJECT_ID))
	{
		UE_LOG(LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient::Unregister: Unregistering an unregistered object (this=%" PRIu64 " %s)"), this, *ProducerName.ToString());
		return;
	}

	const auto Result = SoundEngine->UnregisterGameObj(ObjectId);
	UE_CLOG(LIKELY(Result == AK_Success), LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkInputClient::Unregister: Unregistered Object ID %" PRIu64 " (%s)"), ObjectId, *ProducerName.ToString());
	UE_CLOG(UNLIKELY(Result != AK_Success), LogWwiseAudioLink, Warning, TEXT("FWwiseAudioLinkInputClient::Unregister: Error Unregistering Object ID %" PRIu64 " (%s): (%" PRIu32 ") %s"), ObjectId, *ProducerName.ToString(), Result, WwiseUnrealHelper::GetResultString(Result));
	ObjectId = AK_INVALID_AUDIO_OBJECT_ID;
}

/** Called from the Consumer Thread, at Game-tick rate */
void FWwiseAudioLinkInputClient::UpdateWorldState(const FWorldState& InParams)
{
	IWwiseSoundEngineAPI* SoundEngine = IWwiseSoundEngineAPI::Get();
	if(UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::Register: No Wwise SoundEngine."));
		return;
	}

	UE_LOG(LogWwiseAudioLinkLowLevel, VeryVerbose, TEXT("FWwiseAudioLinkInputClient::UpdateWorldState, PlayId=%d, Name=%s, This=0x%p"),
		PlayId.load(), *ProducerName.GetPlainNameString(), this )
	
	const FTransform& T = InParams.WorldTransform;
	AkSoundPosition Pos;
	const FQuat& Q = T.GetRotation();
	FAkAudioDevice::FVectorsToAKWorldTransform(
		T.GetLocation(),
		Q.GetForwardVector(),
		Q.GetUpVector(),
		Pos
	);
	SoundEngine->SetPosition(ObjectId, Pos);
}

/** Called from the Wwise Renderer Thread */
bool FWwiseAudioLinkInputClient::GetSamples(uint32 InNumChannels, uint32 InNumFrames, float** InChannelBuffers)
{
	SCOPED_NAMED_EVENT(WwiseAudioLink_GetSamples, FColor::Red);

	FSharedBufferedOutputPtr StrongBufferProducer{ WeakProducer.Pin() };
	if (!StrongBufferProducer.IsValid())
	{
		// Return false, to indicate no more data.
		return false;
	}

	if (UNLIKELY(UnrealFormat.NumChannels == 0))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::GetSamples: UnrealFormat's NumSamples == 0"));
		return false;
	}

	if (UNLIKELY(UnrealFormat.NumSamplesPerSec == 0))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::GetSamples: UnrealFormat's NumSamplesPerSec == 0"));
		return false;
	}

	int32 NumFramesPopped = 0;
	bool bMoreDataRemaining = false;

	// Always zero the buffer before we start.
	for (uint32 Channel = 0; Channel < InNumChannels; ++Channel)
	{
		FMemory::Memzero(InChannelBuffers[Channel], InNumFrames * sizeof(float));
	}

	if (UnrealFormat.NumChannels == 1)
	{
		// Pop the data directly onto Wwise buffers.
		// Keep record if the Producer has told us there's no more data.
		bMoreDataRemaining = StrongBufferProducer->PopBuffer(InChannelBuffers[0], InNumFrames, NumFramesPopped);
	}
	else
	{
		// Make sure we have enough space in our temp buffer
		int32 NumInterleavedSamplesNeeded = InNumFrames * UnrealFormat.NumChannels;
		if (NumInterleavedSamplesNeeded > InterleavedBuffer.Num())
		{
			InterleavedBuffer.SetNumUninitialized(
				NumInterleavedSamplesNeeded,
				true		// bAllowShrinking
			);
		}

		// Pop the data onto an intermediate buffer for deinterleaving.
		// Keep record if the Producer has told us there's no more data.
		int32 NumInterleavedSamplesPopped = 0;
		bMoreDataRemaining = StrongBufferProducer->PopBuffer(InterleavedBuffer.GetData(), NumInterleavedSamplesNeeded, NumInterleavedSamplesPopped);
		NumFramesPopped = NumInterleavedSamplesPopped / UnrealFormat.NumChannels;

		if (NumFramesPopped > 0)
		{
			// De-interleave.
			switch (UnrealFormat.NumChannels)
			{
				// Stereo.
				case 2:
				{
					Audio::BufferDeinterleave2ChannelFast(InterleavedBuffer.GetData(), InChannelBuffers[0], InChannelBuffers[1], NumFramesPopped);
					break;
				}

				// Generic version. 
				default:
				{
					const float* InterleavedPtr = InterleavedBuffer.GetData();
					for (int32 Frame = 0; Frame < NumFramesPopped; ++Frame)
					{
						for (int32 Channel = 0; Channel < UnrealFormat.NumChannels; ++Channel)
						{
							InChannelBuffers[Channel][Frame] = *InterleavedPtr++;
						}
					}
					break;
				}
			}
		}
	}

	const int32 UnwrittenFrames = InNumFrames - NumFramesPopped;

	UE_LOG(LogWwiseAudioLinkLowLevel, VeryVerbose, TEXT("FWwiseAudioLinkInputClient::GetSamples() (post-pop), SamplesPopped=%d, SamplesNeeded=%d, UnwrittenFrames=%d, This=0x%p"),
		NumFramesPopped * UnrealFormat.NumChannels, InNumFrames * UnrealFormat.NumChannels, UnwrittenFrames, this);

	// We are permissive in Editor (debugging, more elements in single-threads), however, it still is an audio glitch, so we log all of them outside the editor.
#if WITH_EDITOR
	static constexpr int32 NumStarvedBuffersBeforeLogging = 10;
	static constexpr int32 NumStarvedBuffersBeforeForciblyClosing = 100;
#else
	static constexpr int32 NumStarvedBuffersBeforeLogging = 1;
	static constexpr int32 NumStarvedBuffersBeforeForciblyClosing = 10;
#endif

	if (UnwrittenFrames > 0)
	{
		NumStarvedBuffersInARow++;

		UE_CLOG(NumStarvedBuffersInARow == NumStarvedBuffersBeforeLogging,
			LogWwiseAudioLink, Log, TEXT("FWwiseAudioLinkInputClient: Starving input object, PlayID=%u, Needed=%d, Read=%d, StarvedCount=%d, This=0x%p"),
			PlayId.load(), InNumFrames, NumFramesPopped, NumStarvedBuffersInARow, this);

		if (NumStarvedBuffersInARow == NumStarvedBuffersBeforeForciblyClosing)
		{
			if (!IsEngineExitRequested())
			{
#if WITH_EDITOR
				UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkInputClient: Forcibly closing defunct AudioLink for object, PlayID=%u, StarvedCount=%d, This=0x%p"),
					PlayId.load(), NumStarvedBuffersInARow, this);
#else
				UE_LOG(LogWwiseAudioLink, Warning, TEXT("FWwiseAudioLinkInputClient: Forcibly closing defunct AudioLink for object, PlayID=%u, StarvedCount=%d, This=0x%p"),
					PlayId.load(), NumStarvedBuffersInARow, this);
#endif
			}
			bMoreDataRemaining = false;
		}
	}
	else
	{
		UE_CLOG(NumStarvedBuffersInARow > NumStarvedBuffersBeforeLogging,
			LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkInputClient: Got input from previously starving input object, PlayID=%u, Needed=%d, Read=%d, StarvedCount=%d, This=0x%p"),
			PlayId.load(), InNumFrames, NumFramesPopped, NumStarvedBuffersInARow, this);

		NumStarvedBuffersInARow = 0;
	}

	// Tell Wwise if this is the last buffer which will stop if it is.
	return bMoreDataRemaining;
}

/** Called from the Wwise Renderer Thread */
void FWwiseAudioLinkInputClient::GetFormat(AkAudioFormat& io_AudioFormat)
{
	SCOPED_NAMED_EVENT(WwiseAudioLink_GetFormat, FColor::Red);
	
	// Ensure we're still listening to a sub mix that exists.
	FSharedBufferedOutputPtr StrongPtr{ WeakProducer.Pin() };
	if (!StrongPtr.IsValid())
	{
		return;
	}
    // Cache the format from the Consumer.
    // Ensure the format is known at this point.
    if(!StrongPtr->GetFormat(UnrealFormat))
    {
        UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkInputClient::GetFormat: UnrealFormat is invalid"));
        return;
    }
    
	AkChannelConfig ChannelConfig(
		UnrealFormat.NumChannels,									// Num Channels
		AK::ChannelMaskFromNumChannels(UnrealFormat.NumChannels)	// Channel mask
	);

	// Tell Wwise our format.
	io_AudioFormat.SetAll(
		UnrealFormat.NumSamplesPerSec,	// Number of samples per second
		ChannelConfig, 					// Channel configuration (above).
		32,								// Number of bits per sample
		4,								// Block alignment
		AK_FLOAT,						// Data sample format (Float or Integer)
		AK_NONINTERLEAVED				// Interleaved type (NOTE: Interleaved does not work with float, per the docs, sad times).
	);
}
