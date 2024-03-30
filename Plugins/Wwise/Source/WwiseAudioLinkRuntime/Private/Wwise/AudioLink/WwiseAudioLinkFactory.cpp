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

#include "Wwise/AudioLink/WwiseAudioLinkFactory.h"

#include "Wwise/AudioLink/WwiseAudioLink.h"
#include "Wwise/AudioLink/WwiseAudioLinkInputClient.h"
#include "Wwise/AudioLink/WwiseAudioLinkSettings.h"
#include "Wwise/AudioLink/WwiseAudioLinkComponent.h"
#include "Wwise/AudioLink/WwiseAudioLinkSourcePushed.h"
#include "Wwise/AudioLink/WwiseAudioLinkSynchronizer.h"

#include "Wwise/Stats/AudioLink.h"

#include "AkAudioModule.h"
#include "AkAudioInputManager.h"
#include "AkAudioEvent.h"
#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"

#include "Async/Async.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundSubmix.h"
#include "Templates/SharedPointer.h"

#include <inttypes.h>

bool FWwiseAudioLinkFactory::bHasSubmix = false;

bool FWwiseAudioLinkFactory::IsWwiseInit()
{
	if (IWwiseSoundEngineModule::IsAvailable())
	{
		IWwiseSoundEngineAPI* WwiseLLSE = IWwiseSoundEngineModule::SoundEngine;
		if (WwiseLLSE)
		{
			return WwiseLLSE->IsInitialized();
		}
	}
	return false;
}

TUniquePtr<IAudioLink> FWwiseAudioLinkFactory::CreateSubmixAudioLink(const FAudioLinkSubmixCreateArgs& InArgs)
{
	if (!IAkAudioModule::IsAvailable())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSubmixAudioLink: No AkAudio module."));
		return {};
	}
	if (!IsWwiseInit())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSubmixAudioLink: AkAudio not initialized."));
		return {};
	}

	if (!InArgs.Settings.IsValid())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSubmixAudioLink: Invalid WwiseAudioLinkSettings."));
		return {};
	}

	if (!InArgs.Submix.IsValid())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSubmixAudioLink: Invalid Submix."));
		return {};
	}

	bHasSubmix = true;

	UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkFactory: Creating AudioLink %s for Submix %s."), *InArgs.Settings->GetName(), *InArgs.Submix->GetName());

	// Downcast to Wwise Settings Proxy.
	const FSharedAudioLinkSettingProxyWwisePtr WwiseSettingsSP = InArgs.Settings->GetCastProxy<FWwiseAudioLinkSettingsProxy>();

	// This is a best guess, as we don't know the format yet, so assume mono.
	const int32 DefaultSizeOfBuffer = WwiseSettingsSP->GetReceivingBufferSizeInFrames();

	// Make buffer listener first, which is our producer.
	FSubmixBufferListenerCreateParams SubmixListenerCreateArgs;			
	SubmixListenerCreateArgs.SizeOfBufferInFrames = DefaultSizeOfBuffer;
	SubmixListenerCreateArgs.bShouldZeroBuffer = WwiseSettingsSP->ShouldClearBufferOnReceipt();
	FSharedBufferedOutputPtr ProducerSP = CreateSubmixBufferListener(SubmixListenerCreateArgs);

	// Create Wwise input client, which is the consumer in the link.
	// This take a Weak Reference to the Producer.
	FSharedWwiseAudioLinkInputClientPtr ConsumerSP = MakeShared<FWwiseAudioLinkInputClient, ESPMode::ThreadSafe>(
		ProducerSP, InArgs.Settings->GetProxy(), InArgs.Submix->GetFName());

	// Setup a delegate to establish the link when we know the format.
	ProducerSP->SetFormatKnownDelegate(IBufferedAudioOutput::FOnFormatKnown::CreateLambda(
		[ProducerSP, ConsumerSP, WwiseSettingsSP](const IBufferedAudioOutput::FBufferFormat& InFormat)
		{
			int32 BufferSizeInSamples = WwiseSettingsSP->GetReceivingBufferSizeInFrames() * InFormat.NumChannels;
			int32 ReserveSizeInSamples = (float)BufferSizeInSamples * WwiseSettingsSP->GetProducerConsumerBufferRatio();
			int32 SilenceToAddToFirstBuffer = FMath::Min((float)BufferSizeInSamples * WwiseSettingsSP->GetInitialSilenceFillRatio(), ReserveSizeInSamples);

			// Set circular buffer ahead of first buffer. 
			ProducerSP->Reserve(ReserveSizeInSamples, SilenceToAddToFirstBuffer);

			ConsumerSP->Stop();

			UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkFactory:CreateSubmixAudioLink: Starting Wwise consumer."));
			ConsumerSP->Start();
		}));

	UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkFactory:CreateSubmixAudioLink: Starting Unreal producer."));
	ProducerSP->Start(InArgs.Device);

	return MakeUnique<FWwiseAudioLink>(ProducerSP, ConsumerSP);
}

TUniquePtr<IAudioLink> FWwiseAudioLinkFactory::CreateSourceAudioLink(const FAudioLinkSourceCreateArgs& InArgs)
{
	if (!IAkAudioModule::IsAvailable())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: No AkAudio module."));
		return {};
	}
	if (!IsWwiseInit())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: AkAudio not initialized."));
		return {};
	}

	if (!FAkAudioDevice::Get())
	{
		UE_LOG(LogWwiseAudioLink, Warning, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: No AkAudioDevice available."));
		return {};
	}

	if (!InArgs.Settings.IsValid())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: Invalid WwiseAudioLinkSettings."));
		return {};
	}

	if (!InArgs.OwningComponent.IsValid())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: Invalid Owning Component."));
		return {};
	}

	if (!InArgs.AudioComponent.IsValid())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: Invalid Audio Component."));
		return {};
	}

	const UWorld* World = InArgs.OwningComponent->GetWorld();
	if (UNLIKELY(!IsValid(World)))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: Invalid World in Owning Component."));
		return {};
	}

	const FAudioDeviceHandle Handle = World->GetAudioDevice();
	if (UNLIKELY(!Handle.IsValid()))
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourceAudioLink: Invalid AudioDevice in World."));
		return {};
	}

	UE_LOG(LogWwiseAudioLink, Verbose, TEXT("FWwiseAudioLinkFactory: Creating AudioLink %s for Audio Compoment %s (Owning=%s; World=%s; AudioDeviceID=%" PRIu32 ")"),
		*InArgs.Settings->GetName(), *InArgs.AudioComponent->GetName(), *InArgs.OwningComponent->GetName(), *World->GetName(), Handle.GetDeviceID());

	// Downcast to Wwise Settings Proxy.
	const FSharedAudioLinkSettingProxyWwisePtr WwiseSettingsSP = InArgs.Settings->GetCastProxy<FWwiseAudioLinkSettingsProxy>();

	// This is a best guess, as we don't know the format yet, so assume mono.
	const int32 DefaultSizeOfBuffer = WwiseSettingsSP->GetReceivingBufferSizeInFrames();

	// Create the Unreal Source Listener, this is our Producer.
	FSourceBufferListenerCreateParams SourceBufferCreateArgs;
	SourceBufferCreateArgs.SizeOfBufferInFrames = DefaultSizeOfBuffer;
	SourceBufferCreateArgs.bShouldZeroBuffer = WwiseSettingsSP->ShouldClearBufferOnReceipt();
	SourceBufferCreateArgs.OwningComponent = InArgs.OwningComponent;
	SourceBufferCreateArgs.AudioComponent = InArgs.AudioComponent;
	FSharedBufferedOutputPtr ProducerSP = CreateSourceBufferListener(SourceBufferCreateArgs);

	static const FName UnknownOwner(TEXT("Unknown"));
	FName OwnerName = InArgs.OwningComponent.IsValid() ? InArgs.OwningComponent->GetFName() : UnknownOwner;
	
	// Create the Wwise input object that will start receiving buffers from us.
	auto ConsumerSP = MakeShared<FWwiseAudioLinkInputClient, ESPMode::ThreadSafe>(ProducerSP, WwiseSettingsSP, OwnerName);

	ProducerSP->SetFormatKnownDelegate(IBufferedAudioOutput::FOnFormatKnown::CreateLambda(
		[ProducerSP, ConsumerSP, WwiseSettingsSP, OwningComponent = InArgs.OwningComponent](const IBufferedAudioOutput::FBufferFormat& InFormat)
		{
			const int32 BufferSizeInSamples = WwiseSettingsSP->GetReceivingBufferSizeInFrames() * InFormat.NumChannels;
			const int32 ReserveSizeInSamples = (float)BufferSizeInSamples * WwiseSettingsSP->GetProducerConsumerBufferRatio();
			const int32 SilenceToAddToFirstBuffer = FMath::Min((float)BufferSizeInSamples * WwiseSettingsSP->GetInitialSilenceFillRatio(), ReserveSizeInSamples);

			// Set circular buffer ahead of first buffer. 
			ProducerSP->Reserve(ReserveSizeInSamples, SilenceToAddToFirstBuffer);

			AsyncTask(ENamedThreads::GameThread, [ConsumerSP,OwningComponent]()
			{
				if (OwningComponent.IsValid())
				{
					ConsumerSP->Stop();

					UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkFactory:CreateSourceAudioLink: Starting Wwise consumer."));
					ConsumerSP->Start(Cast<UWwiseAudioLinkComponent>(OwningComponent.Get()));
				}
			});
		}));

	ProducerSP->SetBufferStreamEndDelegate(IBufferedAudioOutput::FOnBufferStreamEnd::CreateLambda(
		[ConsumerSP](const IBufferedAudioOutput::FBufferStreamEnd& InBufferStreamEndParams)
		{
			UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkFactory:CreateSourceAudioLink: Stopping Wwise consumer."));
			ConsumerSP->Stop();
		}));

	UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkFactory:CreateSourceAudioLink: Starting Unreal producer."));
	ProducerSP->Start(Handle.GetAudioDevice());
	
	AsyncTask(ENamedThreads::GameThread, []
	{
		UE_CLOG(!bHasSubmix,
		LogWwiseAudioLink, Log, TEXT("WwiseAudioLink: No initial submix got routed to AudioLink. Consider creating custom versions of global submixes in Project Settings Audio, and Enable Audio Link in their advanced settings."));
	});

	return MakeUnique<FWwiseAudioLink>(ProducerSP, ConsumerSP);
}

FName FWwiseAudioLinkFactory::GetFactoryNameStatic()
{
	static const FName FactoryName(TEXT("Wwise"));
	return FactoryName;
}

IAudioLinkFactory::FAudioLinkSynchronizerSharedPtr FWwiseAudioLinkFactory::CreateSynchronizerAudioLink()
{
	UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkFactory: Creating AudioLink Synchronizer"));

	auto SynchronizerSP = MakeShared<FWwiseAudioLinkSynchronizer, ESPMode::ThreadSafe>();
	SynchronizerSP->Bind();
	return SynchronizerSP;
}

IAudioLinkFactory::FAudioLinkSourcePushedSharedPtr FWwiseAudioLinkFactory::CreateSourcePushedAudioLink(const FAudioLinkSourcePushedCreateArgs& InArgs)
{
	if (!IAkAudioModule::IsAvailable())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourcePushedAudioLink: No AkAudio module."));
		return {};
	}
	if (!IsWwiseInit())
	{
		UE_LOG(LogWwiseAudioLink, Error, TEXT("FWwiseAudioLinkFactory::CreateSourcePushedAudioLink: AkAudio not initialized."));
		return {};
	}

	UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkFactory: Creating AudioLink SourcePushed"));

	return MakeShared<FWwiseAudioLinkSourcePushed, ESPMode::ThreadSafe>(InArgs,this);
}

FName FWwiseAudioLinkFactory::GetFactoryName() const
{
	return GetFactoryNameStatic();
}

TSubclassOf<UAudioLinkSettingsAbstract> FWwiseAudioLinkFactory::GetSettingsClass() const
{
	return UWwiseAudioLinkSettings::StaticClass();
}