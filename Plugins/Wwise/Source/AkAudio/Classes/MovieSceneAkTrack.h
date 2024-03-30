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

#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "WwiseDefines.h"

#include "MovieSceneAkTrack.generated.h"


/**
 * Handles manipulation of an Ak track in a movie scene
 */
UCLASS(abstract, MinimalAPI)
class UMovieSceneAkTrack 
	: public UMovieSceneTrack
{
	GENERATED_BODY()

public:

	/** begin UMovieSceneTrack interface */
	
	virtual void RemoveAllAnimationData() override { Sections.Empty(); }
	virtual bool HasSection(const UMovieSceneSection& Section) const override { return Sections.Contains(&Section); }
	virtual void AddSection(UMovieSceneSection& Section) override { Sections.Add(&Section); }
	virtual void RemoveSection(UMovieSceneSection& Section) override { Sections.Remove(&Section); }
	virtual bool IsEmpty() const override { return Sections.Num() == 0; }
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override { return Sections; }

	/** end UMovieSceneTrack interface */

	void SetIsAMasterTrack(bool AMasterTrack) { bIsAMasterTrack = AMasterTrack; }
	bool IsAMasterTrack() const { return bIsAMasterTrack; }

protected:

	/** All the sections in this track */
	UPROPERTY()
	TArray<UMovieSceneSection*> Sections;

	UPROPERTY()
	bool bIsAMasterTrack = false;
};
