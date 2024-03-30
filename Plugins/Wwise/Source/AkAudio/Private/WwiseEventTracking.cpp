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

#include "WwiseEventTracking.h"
#include "AkAudioEvent.h"
#include "Wwise/WwiseExternalSourceManager.h"

void FWwiseEventTracker::PostEventCallbackHandler(AkCallbackType in_eType, AkCallbackInfo * in_pCallbackInfo)
{
	if (in_pCallbackInfo == nullptr)
		return;

	auto Tracker = (FWwiseEventTracker*)in_pCallbackInfo->pCookie;
	if (Tracker == nullptr)
		return;

	/* Event end */
	if (in_eType == AkCallbackType::AK_EndOfEvent)
	{
		const auto CBInfo = (AkEventCallbackInfo*)in_pCallbackInfo;
		const auto IDToStop = CBInfo->playingID;
		Tracker->RemovePlayingID(IDToStop);
		Tracker->RemoveScheduledStop(IDToStop);
	}/* Received close to the beginning of the event */
	else if (in_eType == AkCallbackType::AK_Duration)
	{
		const auto CBInfo = (AkDurationCallbackInfo*)in_pCallbackInfo;
		Tracker->CurrentDurationEstimation = (CBInfo->fEstimatedDuration * Tracker->CurrentDurationProportionRemaining) / 1000.0f;
	}
}

void FWwiseEventTracker::RemoveScheduledStop(AkPlayingID InID)
{
	FScopeLock autoLock(&ScheduledStopsLock);

	for (auto PlayingID : ScheduledStops)
	{
		if (PlayingID == InID)
		{
			ScheduledStops.Remove(PlayingID);
			break;
		}
	}
}

void FWwiseEventTracker::RemovePlayingID(AkPlayingID InID)
{
	FScopeLock autoLock(&PlayingIDsLock);

	for (auto PlayingID : PlayingIDs)
	{
		if (PlayingID == InID)
		{
			PlayingIDs.Remove(PlayingID);
			break;
		}
	}
}

void FWwiseEventTracker::TryAddPlayingID(const AkPlayingID & PlayingID)
{
	if (PlayingID != AK_INVALID_PLAYING_ID)
	{
		FScopeLock autoLock(&PlayingIDsLock);
		PlayingIDs.Add(PlayingID);
	}
}

void FWwiseEventTracker::EmptyPlayingIDs()
{
	FScopeLock autoLock(&PlayingIDsLock);
	PlayingIDs.Empty();
}

void FWwiseEventTracker::EmptyScheduledStops()
{
	FScopeLock autoLock(&ScheduledStopsLock);
	ScheduledStops.Empty();
}

bool FWwiseEventTracker::PlayingIDHasScheduledStop(AkPlayingID InID)
{
	FScopeLock autoLock(&ScheduledStopsLock);

	for (auto PlayingID : ScheduledStops)
	{
		if (PlayingID == InID)
		{
			return true;
		}
	}

	return false;
}

void FWwiseEventTracker::AddScheduledStop(AkPlayingID InID)
{
	FScopeLock autoLock(&ScheduledStopsLock);
	ScheduledStops.Add(InID);
}

namespace WwiseEventTriggering
{
	TArray<AkPlayingID, TInlineAllocator<16>> GetPlayingIds(FWwiseEventTracker & EventTracker)
	{
		FScopeLock autoLock(&EventTracker.PlayingIDsLock);
		return TArray<AkPlayingID, TInlineAllocator<16>> { EventTracker.PlayingIDs };
	}

	void LogDirtyPlaybackWarning()
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Playback occurred from sequencer section with new changes. You may need to save your diry work units and re-generate your soundbanks."));
	}

	void StopAllPlayingIDs(FAkAudioDevice * AudioDevice, FWwiseEventTracker & EventTracker)
	{
		ensure(AudioDevice != nullptr);
		if (AudioDevice)
		{
			for (auto PlayingID : GetPlayingIds(EventTracker))
			{
				AudioDevice->StopPlayingID(PlayingID);
			}
		}
	}

	AkPlayingID PostEventOnDummyObject(FAkAudioDevice * AudioDevice, FWwiseEventTracker & EventTracker, float CurrentTime)
	{
		ensure(AudioDevice != nullptr);
		if (EventTracker.EventName.IsEmpty())
		{
			UE_LOG(LogAkAudio, Warning, TEXT("Attempted to post an AkEvent from an empty Sequencer section."));
			return AK_INVALID_PLAYING_ID;
		}

		if (AudioDevice)
		{
			AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;
			if (EventTracker.Event)
			{
				PlayingID = EventTracker.Event->PostAmbient(nullptr, &FWwiseEventTracker::PostEventCallbackHandler, &EventTracker,
					(AkCallbackType)(AK_EndOfEvent | AK_Duration), nullptr);
			}
			if (LIKELY(PlayingID != AK_INVALID_PLAYING_ID))
			{
				EventTracker.TryAddPlayingID(PlayingID);
				if (EventTracker.IsDirty)
					LogDirtyPlaybackWarning();
				return PlayingID;
			}
		}
		return AK_INVALID_PLAYING_ID;
	}

	AkPlayingID PostEvent(UObject * Object, FAkAudioDevice * AudioDevice, FWwiseEventTracker & EventTracker, float CurrentTime)
	{
		ensure(AudioDevice != nullptr);

		if (EventTracker.EventName.IsEmpty())
		{
			UE_LOG(LogAkAudio, Warning, TEXT("Attempted to post an AkEvent from an empty Sequencer section."));
			return AK_INVALID_PLAYING_ID;
		}

		if (Object && AudioDevice)
		{
			auto AkComponent = Cast<UAkComponent>(Object);

			if (!IsValid(AkComponent))
			{
				auto Actor = CastChecked<AActor>(Object);
				if (IsValid(Actor))
				{
					AkComponent = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
				}
			}

			if (IsValid(AkComponent))
			{
				AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;
				if (EventTracker.Event)
				{
					PlayingID = EventTracker.Event->PostOnComponent(AkComponent, nullptr, &FWwiseEventTracker::PostEventCallbackHandler, &EventTracker, (AkCallbackType)(AK_EndOfEvent | AK_Duration), nullptr, AkComponent->StopWhenOwnerDestroyed);
				}
				EventTracker.TryAddPlayingID(PlayingID);
				if (EventTracker.IsDirty)
					LogDirtyPlaybackWarning();
				return PlayingID;
			}
		}
		return AK_INVALID_PLAYING_ID;
	}

	void StopEvent(FAkAudioDevice * AudioDevice, AkPlayingID InPlayingID, FWwiseEventTracker * EventTracker)
	{
		ensure(AudioDevice != nullptr);
		if (AudioDevice)
			AudioDevice->StopPlayingID(InPlayingID);
	}

	void TriggerStopEvent(FAkAudioDevice * AudioDevice, FWwiseEventTracker & EventTracker, AkPlayingID PlayingID)
	{
		AudioDevice->StopPlayingID(PlayingID, (float)EventTracker.ScrubTailLengthMs, AkCurveInterpolation::AkCurveInterpolation_Log1);
		EventTracker.AddScheduledStop(PlayingID);
	}

	void ScheduleStopEventsForCurrentlyPlayingIDs(FAkAudioDevice * AudioDevice, FWwiseEventTracker & EventTracker)
	{
		ensure(AudioDevice != nullptr);
		if (AudioDevice)
		{
			for (auto PlayingID : GetPlayingIds(EventTracker))
			{
				if (!EventTracker.PlayingIDHasScheduledStop(PlayingID))
				{
					TriggerStopEvent(AudioDevice, EventTracker, PlayingID);
				}
			}
		}
	}

	void TriggerScrubSnippetOnDummyObject(FAkAudioDevice * AudioDevice, FWwiseEventTracker & EventTracker)
	{
		ensure(AudioDevice != nullptr);
		if (EventTracker.EventName.IsEmpty())
		{
			return;
		}

		if (AudioDevice)
		{
			AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;
			if (EventTracker.Event)
			{
				PlayingID = EventTracker.Event->PostAmbient(nullptr, &FWwiseEventTracker::PostEventCallbackHandler, &EventTracker,
					(AkCallbackType)(AK_EndOfEvent | AK_Duration), nullptr);
			}
			if (LIKELY(PlayingID != AK_INVALID_PLAYING_ID))
			{
				EventTracker.TryAddPlayingID(PlayingID);
				if (EventTracker.IsDirty)
					LogDirtyPlaybackWarning();
				TriggerStopEvent(AudioDevice, EventTracker, PlayingID);
			}
		}
	}

	void TriggerScrubSnippet(UObject * Object, FAkAudioDevice * AudioDevice, FWwiseEventTracker & EventTracker)
	{
		ensure(AudioDevice != nullptr);

		if (EventTracker.EventName.IsEmpty())
		{
			return;
		}

		if (Object && AudioDevice)
		{
			auto AkComponent = Cast<UAkComponent>(Object);

			if (!IsValid(AkComponent))
			{
				auto Actor = CastChecked<AActor>(Object);
				if (IsValid(Actor))
				{
					AkComponent = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
				}
			}

			if (IsValid(AkComponent))
			{
				AkPlayingID PlayingID = AK_INVALID_PLAYING_ID;
				if (EventTracker.Event)
				{
					PlayingID = EventTracker.Event->PostOnComponent(AkComponent, nullptr, &FWwiseEventTracker::PostEventCallbackHandler, &EventTracker,
						(AkCallbackType)(AK_EndOfEvent | AK_Duration), nullptr, AkComponent->StopWhenOwnerDestroyed);
				}
				if (LIKELY(PlayingID != AK_INVALID_PLAYING_ID))
				{
					EventTracker.TryAddPlayingID(PlayingID);
					if (EventTracker.IsDirty)
						LogDirtyPlaybackWarning();
					TriggerStopEvent(AudioDevice, EventTracker, PlayingID);
				}
			}
		}
	}

	void SeekOnEvent(UObject * Object, FAkAudioDevice * AudioDevice, AkReal32 in_fPercent, FWwiseEventTracker & EventTracker, AkPlayingID InPlayingID)
	{
		ensure(AudioDevice != nullptr);

		if (EventTracker.EventName.IsEmpty())
		{
			return;
		}

		if (Object && AudioDevice)
		{
			auto AkComponent = Cast<UAkComponent>(Object);
			if (!IsValid(AkComponent))
			{
				auto Actor = CastChecked<AActor>(Object);
				if (IsValid(Actor))
				{
					AkComponent = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), NULL, EAttachLocation::KeepRelativeOffset);
				}
			}

			if (IsValid(AkComponent))
			{
				const AkUInt32 ShortID = AudioDevice->GetShortID(EventTracker.Event, EventTracker.EventName);
				AudioDevice->SeekOnEvent(ShortID, AkComponent, in_fPercent, false, InPlayingID);
			}
		}
	}

	void SeekOnEvent(UObject * Object, FAkAudioDevice * AudioDevice, AkReal32 in_fPercent, FWwiseEventTracker & EventTracker)
	{
		for (auto PlayingID : GetPlayingIds(EventTracker))
		{
			WwiseEventTriggering::SeekOnEvent(Object, AudioDevice, in_fPercent, EventTracker, PlayingID);
		}
	}

	void SeekOnEventWithDummyObject(FAkAudioDevice * AudioDevice, AkReal32 ProportionalTime, FWwiseEventTracker & EventTracker, AkPlayingID InPlayingID)
	{
		ensure(AudioDevice != nullptr);
		if (EventTracker.EventName.IsEmpty())
		{
			return;
		}

		if (AudioDevice)
		{
			if (ProportionalTime < 1.0f && ProportionalTime >= 0.0f)
			{
				AActor* DummyActor = nullptr;
				const AkUInt32 ShortID = AudioDevice->GetShortID(EventTracker.Event, EventTracker.EventName);
				AudioDevice->SeekOnEvent(ShortID, DummyActor, ProportionalTime, false, InPlayingID);
				// Update the duration proportion remaining property of the event tracker, rather than updating the current duration directly here.
				// This way, we ensure that the current duration is updated first by any PostEvent callback, 
				// before it is then multiplied by the remaining proportion.
				EventTracker.CurrentDurationProportionRemaining = 1.0f - ProportionalTime;
			}
		}
	}

	void SeekOnEventWithDummyObject(FAkAudioDevice * AudioDevice, AkReal32 ProportionalTime, FWwiseEventTracker & EventTracker)
	{
		for (auto PlayingID : GetPlayingIds(EventTracker))
		{
			WwiseEventTriggering::SeekOnEventWithDummyObject(AudioDevice, ProportionalTime, EventTracker, PlayingID);
		}
	}
}
