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

#include "MovieSceneAkTrack.h"
#include "AkInclude.h"
#include "AkAudioEvent.h"
#include "WwiseUnrealDefines.h"
#if UE_4_26_OR_LATER
#include "Compilation/IMovieSceneTrackTemplateProducer.h"
#else
#include "MovieSceneBackwardsCompatibility.h"
#endif
#include "MovieSceneAkAudioRTPCTrack.generated.h"

/**
 * Handles manipulation of float properties in a movie scene
 */
UCLASS(MinimalAPI)
class UMovieSceneAkAudioRTPCTrack
	: public UMovieSceneAkTrack
	, public IMovieSceneTrackTemplateProducer
{
	GENERATED_BODY()

public:

	UMovieSceneAkAudioRTPCTrack()
	{
#if WITH_EDITORONLY_DATA
		SetColorTint(FColor(58, 111, 143, 65));
#endif
	}

	AKAUDIO_API virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;

	AKAUDIO_API virtual UMovieSceneSection* CreateNewSection() override;

	AKAUDIO_API virtual FName GetTrackName() const override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;

#if WITH_EDITORONLY_DATA
	AKAUDIO_API virtual FText GetDisplayName() const override;
#endif
};
