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

#include "MovieSceneAkAudioEventTrack.h"
#include "AkAudioDevice.h"

#include "IMovieScenePlayer.h"
#include "MovieScene.h"
#include "MovieSceneAkAudioEventSection.h"
#include "MovieSceneAkAudioEventTemplate.h"

FMovieSceneEvalTemplatePtr UMovieSceneAkAudioEventTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
#if UE_4_26_OR_LATER
	return FMovieSceneAkAudioEventTemplate(CastChecked<UMovieSceneAkAudioEventSection>(&InSection));
#else
	return InSection.GenerateTemplate();
#endif
}

UMovieSceneSection* UMovieSceneAkAudioEventTrack::CreateNewSection()
{
	return NewObject<UMovieSceneSection>(this, UMovieSceneAkAudioEventSection::StaticClass(), NAME_None, RF_Transactional);
}

#if WITH_EDITOR
bool UMovieSceneAkAudioEventTrack::AddNewEvent(FFrameNumber Time, UAkAudioEvent* Event, const FString& EventName)
{
    UMovieSceneAkAudioEventSection* NewSection = NewObject<UMovieSceneAkAudioEventSection>(this);
	ensure(NewSection);

	bool eventSet = NewSection->SetEvent(Event, EventName);
	if (eventSet)
	{
		const auto Duration = NewSection->GetMaxEventDuration();
		NewSection->InitialPlacement(GetAllSections(), Time, Duration, SupportsMultipleRows());
		AddSection(*NewSection);
	}
	return true;
}

void UMovieSceneAkAudioEventTrack::WorkUnitChangesDetectedFromSection(UMovieSceneAkAudioEventSection* in_pSection)
{
    for (auto Section : Sections)
    {
        if (UMovieSceneAkAudioEventSection* AkSection = Cast<UMovieSceneAkAudioEventSection>(Section))
        {
            if (AkSection != in_pSection)
            {
                AkSection->CheckForWorkunitChanges();
            }
        }
    }
}
#endif

#if WITH_EDITORONLY_DATA
FText UMovieSceneAkAudioEventTrack::GetDisplayName() const
{
	return NSLOCTEXT("MovieSceneAkAudioEventTrack", "TrackName", "AkAudioEvents");
}
#endif

FName UMovieSceneAkAudioEventTrack::GetTrackName() const
{
	static FName TrackName("AkAudioEvents");
	return TrackName;
}

bool UMovieSceneAkAudioEventTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneAkAudioEventSection::StaticClass();
}

