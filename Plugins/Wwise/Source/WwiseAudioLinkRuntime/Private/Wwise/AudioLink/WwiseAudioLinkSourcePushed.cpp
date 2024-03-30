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

#include "Wwise/AudioLink/WwiseAudioLinkSourcePushed.h"
#include "Wwise/AudioLink/WwiseAudioLinkSettings.h"
#include "Wwise/Stats/AudioLink.h"
#include "AkAudioEvent.h"

FWwiseAudioLinkSourcePushed::FWwiseAudioLinkSourcePushed(const IAudioLinkFactory::FAudioLinkSourcePushedCreateArgs& InArgs, IAudioLinkFactory* InFactory)
	: CreateArgs(InArgs)
{
	const FWwiseAudioLinkSettingsProxy* WwiseSettings = static_cast<FWwiseAudioLinkSettingsProxy*>(InArgs.Settings.Get());
	
	IAudioLinkFactory::FPushedBufferListenerCreateParams Params;
	Params.SizeOfBufferInFrames = InArgs.NumFramesPerBuffer;
	Params.bShouldZeroBuffer = WwiseSettings->ShouldClearBufferOnReceipt();

	ProducerSP = InFactory->CreatePushableBufferListener(Params);
	ConsumerSP = MakeShared<FWwiseAudioLinkInputClient, ESPMode::ThreadSafe>(ProducerSP, InArgs.Settings, InArgs.OwnerName);

	int32 BufferSizeInSamples = WwiseSettings->GetReceivingBufferSizeInFrames() * InArgs.NumChannels;
	int32 ReserveSizeInSamples = (float)BufferSizeInSamples * WwiseSettings->GetProducerConsumerBufferRatio();
	int32 SilenceToAddToFirstBuffer = FMath::Min((float)BufferSizeInSamples * WwiseSettings->GetInitialSilenceFillRatio(), ReserveSizeInSamples);

	// Set circular buffer ahead of first buffer. 
	ProducerSP->Reserve(ReserveSizeInSamples, SilenceToAddToFirstBuffer);

	UE_LOG(LogWwiseAudioLinkLowLevel, Verbose,
		TEXT("FWwiseAudioLinkSourcePushed::Ctor() Name=%s, Producer=0x%p, Consumer=0x%p, p2c%%=%2.2f, PlayEvent=%s, TotalFramesForSource=%d, This=0x%p"),
		*InArgs.OwnerName.GetPlainNameString(), ProducerSP.Get(), 
			ConsumerSP.Get(), WwiseSettings->GetProducerConsumerBufferRatio(), *WwiseSettings->GetStartEvent()->GetName(), CreateArgs.TotalNumFramesInSource, this);
}
FWwiseAudioLinkSourcePushed::~FWwiseAudioLinkSourcePushed()
{
	UE_LOG(LogWwiseAudioLinkLowLevel, Verbose,
		TEXT("FWwiseAudioLinkSourcePushed::Dtor() Name=%s, Producer=0x%p, Consumer=0x%p, RecievedFrames=%d/%d, This=0x%p"),
		*CreateArgs.OwnerName.GetPlainNameString(), ProducerSP.Get(), ConsumerSP.Get(), NumFramesReceivedSoFar, CreateArgs.TotalNumFramesInSource,this);

	ConsumerSP->Stop();
}
void FWwiseAudioLinkSourcePushed::OnNewBuffer(const FOnNewBufferParams& InArgs)
{
	UE_LOG(LogWwiseAudioLinkLowLevel, VeryVerbose,
		TEXT("FWwiseAudioLinkSourcePushed::OnNewBuffer() Name=%s, Producer=0x%p, Consumer=0x%p, SourceID=%d, RecievedFrames=%d/%d, This=0x%p"),
		*CreateArgs.OwnerName.GetPlainNameString(), ProducerSP.Get(), ConsumerSP.Get(), SourceId, NumFramesReceivedSoFar, CreateArgs.TotalNumFramesInSource, 
		this);

	NumFramesReceivedSoFar += CreateArgs.NumFramesPerBuffer;
	
	if (SourceId == INDEX_NONE)
	{
		SourceId = InArgs.SourceId;
		ProducerSP->Start(nullptr /*AudioDevice*/);	
		ConsumerSP->Start();
	}
	check(SourceId == InArgs.SourceId);

	IPushableAudioOutput* Pushable = ProducerSP->GetPushableInterface();
	if (ensure(Pushable))
	{
		IPushableAudioOutput::FOnNewBufferParams Params;
		Params.AudioData = InArgs.Buffer.GetData();
		Params.NumSamples = InArgs.Buffer.Num();
		Params.Id = InArgs.SourceId;
		Params.NumChannels = CreateArgs.NumChannels;
		Params.SampleRate = CreateArgs.SampleRate;
		Pushable->PushNewBuffer(Params);
	}
}

void FWwiseAudioLinkSourcePushed::OnSourceDone(const int32 InSourceId)
{
	UE_LOG(LogWwiseAudioLinkLowLevel, Verbose,
		TEXT("FWwiseAudioLinkSourcePushed::OnSourceDone() Name=%s, Producer=0x%p, Consumer=0x%p, RecievedFrames=%d/%d, This=0x%p"),
		*CreateArgs.OwnerName.GetPlainNameString(), ProducerSP.Get(), ConsumerSP.Get(), NumFramesReceivedSoFar, CreateArgs.TotalNumFramesInSource, this);

	check(SourceId == InSourceId);
	IPushableAudioOutput* Pushable = ProducerSP->GetPushableInterface();
	if (ensure(Pushable))
	{
		Pushable->LastBuffer(SourceId);
	}
	SourceId = INDEX_NONE;
}

void FWwiseAudioLinkSourcePushed::OnSourceReleased(const int32 InSourceId)
{
	UE_LOG(LogWwiseAudioLinkLowLevel, Verbose,
		TEXT("FWwiseAudioLinkSourcePushed::OnSourceReleased() Name=%s, Producer=0x%p, Consumer=0x%p, RecievedFrames=%d/%d, This=0x%p"),
		*CreateArgs.OwnerName.GetPlainNameString(), ProducerSP.Get(), ConsumerSP.Get(), NumFramesReceivedSoFar, CreateArgs.TotalNumFramesInSource,this);
}

// Called by the AudioThread, not the AudioRenderThread
void FWwiseAudioLinkSourcePushed::OnUpdateWorldState(const FOnUpdateWorldStateParams& InParams)
{
	FWwiseAudioLinkInputClient::FWorldState UpdateParams;
	UpdateParams.WorldTransform = InParams.WorldTransform;
	ConsumerSP->UpdateWorldState(UpdateParams);
}