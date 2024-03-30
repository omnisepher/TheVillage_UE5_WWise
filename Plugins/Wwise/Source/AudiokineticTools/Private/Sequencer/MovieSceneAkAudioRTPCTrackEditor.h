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

#include "KeyframeTrackEditor.h"
#include "WwiseUEFeatures.h"
#include "MovieSceneAkAudioRTPCTrack.h"
#include "MovieSceneAkAudioRTPCSection.h"


/**
 * Tools for AkAudioRTPC tracks
 */
class FMovieSceneAkAudioRTPCTrackEditor
	: public FKeyframeTrackEditor<UMovieSceneAkAudioRTPCTrack>
{
public:

	/**
	* Constructor
	*
	* @param InSequencer	The sequencer instance to be used by this tool
	*/
	FMovieSceneAkAudioRTPCTrackEditor(TSharedRef<ISequencer> InSequencer);

	/**
	* Creates an instance of this class.  Called by a sequencer
	*
	* @param OwningSequencer The sequencer instance to be used by this tool
	* @return The new instance of this class
	*/
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer);

public:

	// ISequencerTrackEditor interface

	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass) override;
	virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	virtual bool SupportsSequence(UMovieSceneSequence* InSequence) const override;

	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;

	virtual const FSlateBrush* GetIconBrush() const override;

private:

	DECLARE_DELEGATE_RetVal_OneParam(UMovieSceneAkAudioRTPCTrack*, FCreateAkAudioRTPCTrack, UMovieScene*);

	void TryAddAkAudioRTPCTrack(FCreateAkAudioRTPCTrack DoCreateAkAudioRTPCTrack);
};
