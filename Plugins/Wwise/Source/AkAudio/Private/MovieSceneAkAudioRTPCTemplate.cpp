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

#include "MovieSceneAkAudioRTPCTemplate.h"
#include "AkAudioDevice.h"
#include "MovieSceneAkAudioRTPCSection.h"
#include "MovieSceneExecutionToken.h"
#include "IMovieScenePlayer.h"


FMovieSceneAkAudioRTPCSectionData::FMovieSceneAkAudioRTPCSectionData(const UMovieSceneAkAudioRTPCSection& Section)
	: RTPCName(Section.GetRTPCName())
	, RTPCChannel(Section.GetChannel())
{
}


struct FAkAudioRTPCEvaluationData : IPersistentEvaluationData
{
	TSharedPtr<FMovieSceneAkAudioRTPCSectionData> SectionData;
};


struct FAkAudioRTPCExecutionToken : IMovieSceneExecutionToken
{
	virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{
		auto AudioDevice = FAkAudioDevice::Get();
		if (!AudioDevice)
		{
			return;
		}

		TSharedPtr<FMovieSceneAkAudioRTPCSectionData> SectionData = PersistentData.GetSectionData<FAkAudioRTPCEvaluationData>().SectionData;
		if (SectionData.IsValid())
		{
			auto RTPCNameString = *(SectionData->RTPCName);
			float Value;
			SectionData->RTPCChannel.Evaluate(Context.GetTime(), Value);

			if (Operand.ObjectBindingID.IsValid())
			{	// Object binding audio track
				for (auto ObjectPtr : Player.FindBoundObjects(Operand))
				{
					auto Object = ObjectPtr.Get();
					if (Object)
					{
						auto Actor = CastChecked<AActor>(Object);
						if (Actor)
						{
							AudioDevice->SetRTPCValue(RTPCNameString, Value, 0, Actor);
						}
					}
				}
			}
			else
			{	// Master audio track
				AudioDevice->SetRTPCValue(RTPCNameString, Value, 0, nullptr);
			}
		}
	}
};


FMovieSceneAkAudioRTPCTemplate::FMovieSceneAkAudioRTPCTemplate(const UMovieSceneAkAudioRTPCSection& InSection)
	: Section(&InSection)
{
	if (!Section->GetRTPCName().Len())
	{
		Section = nullptr;
	}
}

void FMovieSceneAkAudioRTPCTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	ExecutionTokens.Add(FAkAudioRTPCExecutionToken());
}

void FMovieSceneAkAudioRTPCTemplate::Setup(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	if (Section != nullptr)
	{
		PersistentData.AddSectionData<FAkAudioRTPCEvaluationData>().SectionData = MakeShareable(new FMovieSceneAkAudioRTPCSectionData(*Section));
	}
}
