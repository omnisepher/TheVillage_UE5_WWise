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

#include "IAudioLink.h"
#include "IAudioLinkFactory.h"
#include "Templates/SharedPointer.h"
#include "IBufferedAudioOutput.h"

#include "AK/SoundEngine/Common/AkTypes.h"
#include "AkAudioDevice.h"
#include "DSP/BufferVectorOperations.h"

class UWwiseAudioLinkComponent;

class FWwiseAudioLinkInputClient : public TSharedFromThis<FWwiseAudioLinkInputClient, ESPMode::ThreadSafe>
{
public:	
	FWwiseAudioLinkInputClient(const FSharedBufferedOutputPtr& InToConsumeFrom, const UAudioLinkSettingsAbstract::FSharedSettingsProxyPtr& InSettingsProxy, FName InNameOfProducingSource={});
	virtual ~FWwiseAudioLinkInputClient();

	void Start(UWwiseAudioLinkComponent* InComponent);
	void Start();
	void Stop();

	struct FWorldState
	{
		FTransform WorldTransform;		
	};
	// Called from Consumer thread at game tick rate.
	void UpdateWorldState(const FWorldState&);

private:
	void Register(const FName& InNameOfProducingSource);
	void Unregister();


	// Called from WWise thread.
	bool GetSamples(uint32 InNumChannels, uint32 InNumFrames, float** InChannelBuffers);

	// Called from WWise thread.
	void GetFormat(AkAudioFormat& io_AudioFormat);
	
	Audio::AlignedFloatBuffer InterleavedBuffer;
	TWeakObjectPtr<AActor> UnsafeAttachment;
	FWeakBufferedOutputPtr WeakProducer;
	UAudioLinkSettingsAbstract::FSharedSettingsProxyPtr SettingsProxy;
	IBufferedAudioOutput::FBufferFormat UnrealFormat;

	std::atomic<AkInt32> PlayId = AK_INVALID_PLAYING_ID;
	AkGameObjectID ObjectId = AK_INVALID_AUDIO_OBJECT_ID;
	FName ProducerName;
	int32 NumStarvedBuffersInARow = 0;
	FDelegateHandle IsLoadedHandle;
};

using FSharedWwiseAudioLinkInputClientPtr = TSharedPtr<FWwiseAudioLinkInputClient, ESPMode::ThreadSafe>;