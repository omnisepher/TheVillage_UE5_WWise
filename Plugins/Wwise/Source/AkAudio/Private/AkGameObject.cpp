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

/*=============================================================================
	AkGameObject.cpp:
=============================================================================*/

#include "AkGameObject.h"
#include "AkAudioEvent.h"
#include "AkComponentCallbackManager.h"
#include "AkRtpc.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"

class FPostAssociatedEventAction : public FAkPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink = 0;
	FWeakObjectPtr CallbackTarget;
	int32* PlayingID = nullptr;
	TFuture<AkPlayingID> FuturePlayingID;
	UAkAudioEvent* AkEvent = nullptr;
	bool* bGameObjectStarted= nullptr;

	FPostAssociatedEventAction(const FLatentActionInfo& LatentInfo, int32* PlayingID, UAkAudioEvent* Event, bool* bStarted)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, PlayingID(PlayingID)
		, AkEvent(Event)
		, bGameObjectStarted(bStarted)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		bool futureIsReady = FuturePlayingID.IsReady();
		if (futureIsReady)
		{
			*PlayingID = FuturePlayingID.Get();
			if (bGameObjectStarted!=nullptr)
			{
				*bGameObjectStarted = true;
			}
		}

		Response.FinishAndTriggerIf(futureIsReady, ExecutionFunction, OutputLink, CallbackTarget);
	}

#if WITH_EDITOR
	virtual FString GetDescription() const override
	{
		return TEXT("Waiting for posted AkEvent to load media.");
	}
#endif
};

UAkGameObject::UAkGameObject(const class FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer)
{
	bEventPosted = false;
}

int32 UAkGameObject::PostAssociatedAkEvent(int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback)
{
	return PostAkEvent(AkAudioEvent, CallbackMask, PostEventCallback);
}

int32 UAkGameObject::PostAkEvent(UAkAudioEvent* AkEvent, int32 CallbackMask,
	const FOnAkPostEventCallback& PostEventCallback
)
{
	if (LIKELY(IsValid(AkEvent)))
	{
		return AkEvent->PostOnGameObject(this, PostEventCallback, CallbackMask);
	}

	AkPlayingID playingID = AK_INVALID_PLAYING_ID;

	auto AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AkEvent->PostOnGameObject(this, PostEventCallback, CallbackMask);
	}
	
	return playingID;
}

AkPlayingID UAkGameObject::PostAkEvent(UAkAudioEvent* AkEvent, AkUInt32 Flags, AkCallbackFunc UserCallback,
	void* UserCookie)
{
	if (UNLIKELY(!IsValid(AkEvent)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post invalid AkAudioEvent on game object '%s'."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}
	return AkEvent->PostOnGameObject(this, nullptr, UserCallback, UserCookie, static_cast<AkCallbackType>(Flags), nullptr);
}

void UAkGameObject::PostAssociatedAkEventAsync(const UObject* WorldContextObject, int32 CallbackMask, const FOnAkPostEventCallback& PostEventCallback, FLatentActionInfo LatentInfo, int32& PlayingID)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	FLatentActionManager& LatentActionManager = DeviceAndWorld.CurrentWorld->GetLatentActionManager();
	FPostAssociatedEventAction* NewAction = LatentActionManager.FindExistingAction<FPostAssociatedEventAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	if (!NewAction)
	{
		NewAction = new FPostAssociatedEventAction(LatentInfo, &PlayingID, AkAudioEvent, &bEventPosted);
		AkAudioEvent->PostOnGameObject(this, PostEventCallback, CallbackMask);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}
}

void UAkGameObject::PostAkEventAsync(const UObject* WorldContextObject,
	UAkAudioEvent* AkEvent,
	int32& PlayingID,
	int32 CallbackMask,
	const FOnAkPostEventCallback& PostEventCallback,
	FLatentActionInfo LatentInfo
)
{
	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	FLatentActionManager& LatentActionManager = DeviceAndWorld.CurrentWorld->GetLatentActionManager();
	FPostAssociatedEventAction* NewAction = LatentActionManager.FindExistingAction<FPostAssociatedEventAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	if (!NewAction)
	{
		NewAction = new FPostAssociatedEventAction(LatentInfo, &PlayingID, AkEvent, &bEventPosted);
		AkEvent->PostOnGameObject(this, PostEventCallback, CallbackMask);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}
}

void UAkGameObject::SetRTPCValue(const UAkRtpc* RTPCValue, float Value, int32 InterpolationTimeMs, FString RTPC) const
{
	if (FAkAudioDevice::Get())
	{
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine))
		{
			return;
		}

		auto GameObjectID = GetAkGameObjectID();
		if (UNLIKELY(GameObjectID == AK_INVALID_GAME_OBJECT || GameObjectID == 0))
		{
			return;
		}

		if (RTPCValue)
		{
			SoundEngine->SetRTPCValue(RTPCValue->GetShortID(), Value, GameObjectID, InterpolationTimeMs);
		}
		else
		{
			SoundEngine->SetRTPCValue(TCHAR_TO_AK(*RTPC), Value, GameObjectID, InterpolationTimeMs);
		}
	}
}

void UAkGameObject::GetRTPCValue(const UAkRtpc* RTPCValue, ERTPCValueType InputValueType, float& Value, ERTPCValueType& OutputValueType, FString RTPC, int32 PlayingID) const
{
	if (FAkAudioDevice::Get())
	{
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine)) return;

		AK::SoundEngine::Query::RTPCValue_type RTPCType = (AK::SoundEngine::Query::RTPCValue_type)InputValueType;

		if (RTPCValue)
		{
			SoundEngine->Query->GetRTPCValue(RTPCValue->GetShortID(), GetAkGameObjectID(), PlayingID, Value, RTPCType);
		}
		else
		{
			SoundEngine->Query->GetRTPCValue(TCHAR_TO_AK(*RTPC), GetAkGameObjectID(), PlayingID, Value, RTPCType);
		}

		OutputValueType = (ERTPCValueType)RTPCType;
	}
}

bool UAkGameObject::VerifyEventName(const FString& InEventName) const
{
	const bool IsEventNameEmpty = InEventName.IsEmpty();
	if (IsEventNameEmpty)
	{
		FString OwnerName = FString(TEXT(""));
		FString ObjectName = GetName();

		const auto owner = GetOwner();
		if (owner)
			OwnerName = owner->GetName();

		UE_LOG(LogAkAudio, Warning, TEXT("[%s.%s] AkGameObject: Attempted to post an empty AkEvent name."), *OwnerName, *ObjectName);
	}

	return !IsEventNameEmpty;
}

bool UAkGameObject::AllowAudioPlayback() const
{
	UWorld* CurrentWorld = GetWorld();
	return (CurrentWorld && CurrentWorld->AllowAudioPlayback() && !IsBeingDestroyed());
}

AkGameObjectID UAkGameObject::GetAkGameObjectID() const
{
	return (AkGameObjectID)this;
}

void UAkGameObject::Stop()
{
	if (HasActiveEvents() && FAkAudioDevice::Get() && IsRegisteredWithWwise)
	{
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine)) return;

		SoundEngine->StopAll(GetAkGameObjectID());
		SoundEngine->RenderAudio();
	}
}

bool UAkGameObject::HasActiveEvents() const
{
	auto CallbackManager = FAkComponentCallbackManager::GetInstance();
	return (CallbackManager != nullptr) && CallbackManager->HasActiveEvents(GetAkGameObjectID());
}