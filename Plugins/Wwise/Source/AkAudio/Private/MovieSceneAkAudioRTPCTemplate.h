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

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "MovieSceneAkAudioRTPCTemplate.generated.h"


class UMovieSceneAkAudioRTPCSection;

struct FMovieSceneAkAudioRTPCSectionData
{
	FMovieSceneAkAudioRTPCSectionData() {}

	FMovieSceneAkAudioRTPCSectionData(const UMovieSceneAkAudioRTPCSection& Section);

	FString RTPCName;

	FMovieSceneFloatChannel RTPCChannel;
};


USTRUCT()
struct AKAUDIO_API FMovieSceneAkAudioRTPCTemplate
	: public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

	FMovieSceneAkAudioRTPCTemplate() {}

	FMovieSceneAkAudioRTPCTemplate(const UMovieSceneAkAudioRTPCSection& InSection);

	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }

	virtual void Setup(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;
	virtual void SetupOverrides() override { EnableOverrides(RequiresSetupFlag); }

	UPROPERTY()
	const UMovieSceneAkAudioRTPCSection* Section = nullptr;
};
