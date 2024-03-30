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

#include "Wwise/AudioLink/WwiseAudioLinkSynchronizer.h"

#include "AkAudioDevice.h"
#include "Wwise/WwiseGlobalCallbacks.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AudioLink.h"

FWwiseAudioLinkSynchronizer::FWwiseAudioLinkSynchronizer()
{}

FWwiseAudioLinkSynchronizer::~FWwiseAudioLinkSynchronizer()
{
	Unbind();
}

void FWwiseAudioLinkSynchronizer::ExecuteBeginRender(AK::IAkGlobalPluginContext* InContext)
{
	FOnRenderParams Params;
	Params.BufferTickID = InContext->GetBufferTick();
	Params.NumFrames = InContext->GetMaxBufferLength();
	OnBeginRender.Broadcast(Params);
}

void FWwiseAudioLinkSynchronizer::ExecuteEndRender(AK::IAkGlobalPluginContext* InContext)
{
	FOnRenderParams Params;
	Params.BufferTickID = InContext->GetBufferTick();
	Params.NumFrames = InContext->GetMaxBufferLength();
	OnEndRender.Broadcast(Params);
}

void FWwiseAudioLinkSynchronizer::ExecuteOpenStream()
{
	UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkSynchronizer::ExecuteOpenStream: Opening stream between Unreal and Wwise."));

	auto* SoundEngine = IWwiseSoundEngineModule::SoundEngine;
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkSynchronizer::ExecuteOpenStream: No Sound Engine."));
		return;
	}

	AkAudioSettings Settings;
	SoundEngine->GetAudioSettings(Settings);					
	const AkChannelConfig SinkConfig = SoundEngine->GetSpeakerConfiguration();

	FOnOpenStreamParams Params;
	Params.NumChannels = SinkConfig.uNumChannels;
	Params.SampleRate = Settings.uNumSamplesPerSecond;
	Params.NumFrames = Settings.uNumSamplesPerFrame;
	Params.Name = TEXT("AudioLink for Wwise");
	OnOpenStream.Broadcast(Params);
}

void FWwiseAudioLinkSynchronizer::ExecuteCloseStream()
{
	UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkSynchronizer::ExecuteCloseStream: Closing stream between Unreal and Wwise."));

	OnCloseStream.Broadcast();
}

void FWwiseAudioLinkSynchronizer::Bind()
{
	if (UNLIKELY(bIsBound))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkSynchronizer::Bind: Binding an already bound SoundEngine."));
		return;
	}

	auto* Callbacks = FWwiseGlobalCallbacks::Get();
	if (UNLIKELY(!Callbacks))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkSynchronizer::Bind: No Callbacks."));
		return;
	}

	auto* SoundEngine = IWwiseSoundEngineModule::SoundEngine;
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkSynchronizer::ExecuteOpenStream: No Sound Engine."));
		return;
	}

	UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkSynchronizer::Bind: Binding SoundEngine."));
	Callbacks->BeginRenderSync([WeakThis = AsWeak()](AK::IAkGlobalPluginContext* InContext) mutable
	{
		auto This = WeakThis.Pin();
		if (UNLIKELY(!This.IsValid() || !This->bIsBound))
		{
			return EWwiseDeferredAsyncResult::Done;
		}

		This->ExecuteBeginRender(InContext);
		return EWwiseDeferredAsyncResult::KeepRunning;
	});

	Callbacks->EndRenderSync([WeakThis = AsWeak()](AK::IAkGlobalPluginContext* InContext) mutable
	{
		auto This = WeakThis.Pin();
		if (UNLIKELY(!This.IsValid() || !This->bIsBound))
		{
			return EWwiseDeferredAsyncResult::Done;
		}

		This->ExecuteEndRender(InContext);
		return EWwiseDeferredAsyncResult::KeepRunning;
	});

	if (SoundEngine->IsInitialized())
	{
		ExecuteOpenStream();
	}
	Callbacks->InitAsync([WeakThis = AsWeak()]() mutable
	{
		auto This = WeakThis.Pin();
		if (UNLIKELY(!This.IsValid() || !This->bIsBound))
		{
			return EWwiseDeferredAsyncResult::Done;
		}

		This->ExecuteOpenStream();
		return EWwiseDeferredAsyncResult::KeepRunning;
	});

	Callbacks->TermAsync([WeakThis = AsWeak()]() mutable
	{
		auto This = WeakThis.Pin();
		if (UNLIKELY(!This.IsValid() || !This->bIsBound))
		{
			return EWwiseDeferredAsyncResult::Done;
		}

		This->ExecuteCloseStream();
		return EWwiseDeferredAsyncResult::KeepRunning;
	});
	bIsBound = true;
}

void FWwiseAudioLinkSynchronizer::Unbind()
{
	if (UNLIKELY(!bIsBound))
	{
		return;
	}

	UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkSynchronizer::Bind: Unbinding SoundEngine."));
	bIsBound = false;

	ExecuteCloseStream();
}
