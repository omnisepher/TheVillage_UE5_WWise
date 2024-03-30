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

#include "AkInclude.h"
#include "WwiseUnrealDefines.h"
#include "AkRtpc.h"
#include "Curves/RichCurve.h"
#include "MovieSceneSection.h"
#include "MovieSceneFloatChannelSerializationHelper.h"

#include "Channels/MovieSceneFloatChannel.h"
#include "MovieSceneAkAudioRTPCSection.generated.h"

/**
* A single floating point section
*/
UCLASS()
class AKAUDIO_API UMovieSceneAkAudioRTPCSection
	: public UMovieSceneSection
{
	GENERATED_BODY()
	UMovieSceneAkAudioRTPCSection(const FObjectInitializer& Init);
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;

public:
	FMovieSceneFloatChannel GetChannel() const {	return RTPCChannel;	}
	float GetStartTime() const;
	float GetEndTime() const;

#if !UE_4_26_OR_LATER
	virtual FMovieSceneEvalTemplatePtr GenerateTemplate() const override;
#endif

	/** @return the name of the RTPC being modified by this track */
	FString GetRTPCName() const { return RTPC ? RTPC->GetName() : Name; }

	/**
	* Sets the name of the RTPC being modified by this track
	*
	* @param InRTPCName The RTPC being modified
	*/
	void SetRTPCName(const FString& InRTPCName) { Name = InRTPCName; }
	void SetRTPC(UAkRtpc* InRTPC) { RTPC = InRTPC; }

#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UPROPERTY(EditAnywhere, Category = "AkAudioRTPC", meta = (NoResetToDefault))
	UAkRtpc* RTPC = nullptr;

	/** Name of the RTPC to modify. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "AkAudioRTPC", meta = (NoResetToDefault))
	FString Name;

	/** Curve data */
	UPROPERTY()
	FRichCurve FloatCurve;

	// Enabled serialization of RTPCChannel when 4.24 support was added. 
	UPROPERTY()
	FMovieSceneFloatChannelSerializationHelper FloatChannelSerializationHelper;

	UPROPERTY()
	FMovieSceneFloatChannel RTPCChannel;

private:
#if WITH_EDITOR
	bool IsRTPCNameValid();

	FString PreviousName;
#endif
};
