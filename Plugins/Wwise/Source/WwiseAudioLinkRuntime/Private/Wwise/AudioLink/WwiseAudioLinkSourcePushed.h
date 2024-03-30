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
#include "Wwise/AudioLink/WwiseAudioLinkInputClient.h"

#include "IAudioLink.h"
#include "IBufferedAudioOutput.h"

struct FWwiseAudioLinkSourcePushed : IAudioLinkSourcePushed
{
	int32 SourceId = INDEX_NONE;
	int32 NumFramesReceivedSoFar = INDEX_NONE;
	FSharedBufferedOutputPtr ProducerSP;
	FSharedWwiseAudioLinkInputClientPtr ConsumerSP;
	IAudioLinkFactory::FAudioLinkSourcePushedCreateArgs CreateArgs;
	
	FWwiseAudioLinkSourcePushed(const IAudioLinkFactory::FAudioLinkSourcePushedCreateArgs& InArgs, IAudioLinkFactory* InFactory);
	virtual ~FWwiseAudioLinkSourcePushed() override;
	void OnNewBuffer(const FOnNewBufferParams& InArgs) override;
	void OnSourceDone(const int32 InSourceId) override;
	void OnSourceReleased(const int32 InSourceId) override;
	void OnUpdateWorldState(const FOnUpdateWorldStateParams& InParams) override;
};	
