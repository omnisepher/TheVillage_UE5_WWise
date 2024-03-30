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
	AkEvent.cpp:
=============================================================================*/

#include "AkAudioEvent.h"
#include "AkAudioBank.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "AkComponentCallbackManager.h"
#include "AkGameObject.h"
#include "AkRoomComponent.h"
#include "WwiseUnrealDefines.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/Global.h"
#include "Wwise/Stats/AkAudio.h"

#include <inttypes.h>

#if WITH_EDITORONLY_DATA
#include "Wwise/WwiseProjectDatabase.h"
#include "Wwise/WwiseResourceCooker.h"
#endif

int32 UAkAudioEvent::PostOnActor(const AActor* Actor, const FOnAkPostEventCallback& Delegate, const int32 CallbackMask,
                                 const bool bStopWhenAttachedObjectDestroyed)
{
	return PostOnActor(Actor, &Delegate, nullptr, nullptr,
		AkCallbackTypeHelpers::GetCallbackMaskFromBlueprintMask(CallbackMask), nullptr, bStopWhenAttachedObjectDestroyed);
}

int32 UAkAudioEvent::PostOnComponent(UAkComponent* Component, const FOnAkPostEventCallback& Delegate,
	const int32 CallbackMask, const bool bStopWhenAttachedObjectDestroyed)
{
	return PostOnComponent(Component, &Delegate, nullptr, nullptr,
		AkCallbackTypeHelpers::GetCallbackMaskFromBlueprintMask(CallbackMask), nullptr, bStopWhenAttachedObjectDestroyed);
}

int32 UAkAudioEvent::PostOnGameObject(UAkGameObject* GameObject, const FOnAkPostEventCallback& Delegate, const int32 CallbackMask)
{
	return PostOnGameObject(GameObject, &Delegate, nullptr, nullptr,
		AkCallbackTypeHelpers::GetCallbackMaskFromBlueprintMask(CallbackMask), nullptr);
}

int32 UAkAudioEvent::PostOnActorAndWait(const AActor* Actor, const bool bStopWhenAttachedObjectDestroyed,
	const FLatentActionInfo LatentActionInfo)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostOnActorAndWait"));
	if (UNLIKELY(!IsValid(Actor) || Actor->IsActorBeingDestroyed()))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post latent AkAudioEvent '%s' with an actor that's not valid. The actor needs to be valid in order to wait for it."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto* World = Actor->GetWorld();
	if (UNLIKELY(!World))
	{
		UE_LOG(LogAkAudio, Log, TEXT("Failed to post latent AkAudioEvent '%s' with an actor '%s' world that's not valid."), *GetName(), *Actor->GetName());
		return AK_INVALID_PLAYING_ID;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	FWaitEndOfEventAction* LatentAction = new FWaitEndOfEventAction(LatentActionInfo);
	LatentActionManager.AddNewAction(LatentActionInfo.CallbackTarget, LatentActionInfo.UUID, LatentAction);

	if (UNLIKELY(!World->AllowAudioPlayback()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with an actor '%s' world '%s' that doesn't allow audio playback."), *GetName(), *Actor->GetName(), *World->GetName());
		LatentAction->EventFinished = true;
		return AK_INVALID_PLAYING_ID;
	}

	const auto PlayingID = PostOnActor(Actor, nullptr, nullptr, nullptr, (AkCallbackType)0, LatentAction, bStopWhenAttachedObjectDestroyed);
	if (UNLIKELY(PlayingID == AK_INVALID_PLAYING_ID))
	{
		LatentAction->EventFinished = true;
	}
	return PlayingID;
}

int32 UAkAudioEvent::PostOnComponentAndWait(UAkComponent* Component, const bool bStopWhenAttachedObjectDestroyed,
	const FLatentActionInfo LatentActionInfo)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostOnComponentAndWait"));
	if (UNLIKELY(!IsValid(Component)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post latent AkAudioEvent '%s' with a component that's not valid. The component needs to be valid in order to wait for it."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto* World = Component->GetWorld();
	if (UNLIKELY(!World))
	{
		UE_LOG(LogAkAudio, Log, TEXT("Failed to post latent AkAudioEvent '%s' with a component '%s' world that's not valid."), *GetName(), *Component->GetName());
		return AK_INVALID_PLAYING_ID;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	FWaitEndOfEventAction* LatentAction = new FWaitEndOfEventAction(LatentActionInfo);
	LatentActionManager.AddNewAction(LatentActionInfo.CallbackTarget, LatentActionInfo.UUID, LatentAction);

	if (UNLIKELY(!World->AllowAudioPlayback()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with a component '%s' world '%s' that doesn't allow audio playback."), *GetName(), *Component->GetName(), *World->GetName());
		LatentAction->EventFinished = true;
		return AK_INVALID_PLAYING_ID;
	}

	const auto PlayingID = PostOnComponent(Component, nullptr, nullptr, nullptr, (AkCallbackType)0, LatentAction, bStopWhenAttachedObjectDestroyed);
	if (UNLIKELY(PlayingID == AK_INVALID_PLAYING_ID))
	{
		LatentAction->EventFinished = true;
	}
	return PlayingID;
}

int32 UAkAudioEvent::PostOnGameObjectAndWait(UAkGameObject* GameObject, const FLatentActionInfo LatentActionInfo)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostOnGameObjectAndWait"));
	if (UNLIKELY(!IsValid(GameObject)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post latent AkAudioEvent '%s' with a game object that's not valid. The gane object needs to be valid in order to wait for it."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto* World = GameObject->GetWorld();
	if (UNLIKELY(!World))
	{
		UE_LOG(LogAkAudio, Log, TEXT("Failed to post latent AkAudioEvent '%s' with a game object '%s' world that's not valid."), *GetName(), *GameObject->GetName());
		return AK_INVALID_PLAYING_ID;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	FWaitEndOfEventAction* LatentAction = new FWaitEndOfEventAction(LatentActionInfo);
	LatentActionManager.AddNewAction(LatentActionInfo.CallbackTarget, LatentActionInfo.UUID, LatentAction);

	if (UNLIKELY(!World->AllowAudioPlayback()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with a game object '%s' world '%s' that doesn't allow audio playback."), *GetName(), *GameObject->GetName(), *World->GetName());
		LatentAction->EventFinished = true;
		return AK_INVALID_PLAYING_ID;
	}

	const auto PlayingID = PostOnGameObject(GameObject, nullptr, nullptr, nullptr, (AkCallbackType)0, LatentAction);
	if (UNLIKELY(PlayingID == AK_INVALID_PLAYING_ID))
	{
		LatentAction->EventFinished = true;
	}
	return PlayingID;
}

int32 UAkAudioEvent::PostAtLocation(const FVector Location, const FRotator Orientation, const FOnAkPostEventCallback& Callback,
                                    const int32 CallbackMask, const UObject* WorldContextObject)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostAtLocation"));
	if (UNLIKELY(!IsValid(WorldContextObject)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post AkAudioEvent '%s' at location with a World Context Object that's not valid."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	const auto* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	return PostAtLocation(Location, Orientation, World, &Callback, nullptr, nullptr,
		AkCallbackTypeHelpers::GetCallbackMaskFromBlueprintMask(CallbackMask), nullptr);
}

int32 UAkAudioEvent::ExecuteAction(const AkActionOnEventType ActionType, const AActor* Actor, const int32 PlayingID,
	const int32 TransitionDuration, const EAkCurveInterpolation FadeCurve)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::ExecuteAction"));
	const auto* AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to execute an action on AkAudioEvent '%s' without an Audio Device."), *GetName());
		return AK_NotInitialized;
	}

	if (UNLIKELY(!AudioDevice->IsInitialized()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to execute an action on AkAudioEvent '%s' with the Sound Engine uninitialized."), *GetName());
		return AK_NotInitialized;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to execute an action on AkAudioEvent '%s' without a Sound Engine."), *GetName());
		return AK_NotInitialized;
	}

	if (!Actor)
	{
		return SoundEngine->ExecuteActionOnEvent(GetShortID(),
			static_cast<AK::SoundEngine::AkActionOnEventType>(ActionType),
			AK_INVALID_GAME_OBJECT,
			TransitionDuration,
			static_cast<AkCurveInterpolation>(FadeCurve),
			PlayingID
		);
	}

	if (UNLIKELY(Actor->IsActorBeingDestroyed() || !IsValid(Actor)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to execute on AkAudioEvent '%s' with an actor that's not valid."), *GetName());
		return AK_InvalidParameter;
	}

	UAkComponent* Component = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), nullptr, EAttachLocation::KeepRelativeOffset);
	if (UNLIKELY(!Component))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to execute on AkAudioEvent '%s' with an actor that doesn't have an AkComponent on Root."), *GetName());
		return AK_InvalidParameter;
	}

	return SoundEngine->ExecuteActionOnEvent(GetShortID(),
		static_cast<AK::SoundEngine::AkActionOnEventType>(ActionType),
		Component->GetAkGameObjectID(),
		TransitionDuration,
		static_cast<AkCurveInterpolation>(FadeCurve),
		PlayingID
	);
}

AkPlayingID UAkAudioEvent::PostOnActor(const AActor* Actor, const FOnAkPostEventCallback* Delegate,
	const AkCallbackFunc Callback, void* Cookie, const AkCallbackType CallbackMask, FWaitEndOfEventAction* LatentAction,
	const bool bStopWhenAttachedObjectDestroyed, const EAkAudioContext AudioContext)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostOnActor"));
	const auto* AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' on actor without an Audio Device."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(!AudioDevice->IsInitialized()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with the Sound Engine uninitialized."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (!Actor)
	{
		return PostAmbient(Delegate, Callback, Cookie, CallbackMask, LatentAction, AudioContext);
	}

	if (UNLIKELY(Actor->IsActorBeingDestroyed() || !IsValid(Actor)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post AkAudioEvent '%s' with an actor that's not valid."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	const auto* World = Actor->GetWorld();
	if (UNLIKELY(!World))
	{
		UE_LOG(LogAkAudio, Log, TEXT("Failed to post AkAudioEvent '%s' with an actor '%s' world that's not valid."), *GetName(), *Actor->GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(!World->AllowAudioPlayback()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with an actor '%s' world '%s' that doesn't allow audio playback."), *GetName(), *Actor->GetName(), *World->GetName());
		return AK_INVALID_PLAYING_ID;
	}

	UAkComponent* Component = AudioDevice->GetAkComponent(Actor->GetRootComponent(), FName(), nullptr, EAttachLocation::KeepRelativeOffset);
	if (UNLIKELY(!Component))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent '%s' with an actor that doesn't have an AkComponent on Root."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	return PostOnComponent(Component, Delegate, Callback, Cookie, CallbackMask, LatentAction, bStopWhenAttachedObjectDestroyed, AudioContext);
}

AkPlayingID UAkAudioEvent::PostOnComponent(UAkComponent* Component, const FOnAkPostEventCallback* Delegate,
	const AkCallbackFunc Callback, void* Cookie, const AkCallbackType CallbackMask, FWaitEndOfEventAction* LatentAction,
	const bool bStopWhenAttachedObjectDestroyed, const EAkAudioContext AudioContext)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostOnComponent"));
	if (UNLIKELY(!Component))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with null AkComponent."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(!IsValid(Component)))
	{
		UE_LOG(LogAkAudio, Error, TEXT("Failed to post AkAudioEvent '%s' with an AkComponent that's not valid."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(Component->StopWhenOwnerDestroyed != bStopWhenAttachedObjectDestroyed))
	{
		UE_LOG(LogWwiseHints, VeryVerbose, TEXT("Updating AkComponent(%s) StopWhenOwnerDestroyed (%s->%s) because of AkAudioEvent '%s'. You should not modify the StopWhenOwnerDestroyed through a PostEvent unless you know what you are doing."),
			*Component->GetName(),
			Component->StopWhenOwnerDestroyed ? TEXT("true") : TEXT("false"),
			bStopWhenAttachedObjectDestroyed ? TEXT("true") : TEXT("false"),
			*GetName());
		Component->StopWhenOwnerDestroyed = bStopWhenAttachedObjectDestroyed;
	}
	return PostOnGameObject(Component, Delegate, Callback, Cookie, CallbackMask, LatentAction, AudioContext);
}

AkPlayingID UAkAudioEvent::PostAtLocation(const FVector& Location, const FRotator& Orientation, const UWorld* World,
	const FOnAkPostEventCallback* Delegate, const AkCallbackFunc Callback, void* Cookie, const AkCallbackType CallbackMask,
	FWaitEndOfEventAction* LatentAction, const EAkAudioContext AudioContext)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostAtLocation"));
	auto* AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' at a location without an Audio Device."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(!AudioDevice->IsInitialized()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' at a location with the Sound Engine uninitialized."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent '%s' at a location without a Sound Engine."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(!World))
	{
		UE_LOG(LogAkAudio, Log, TEXT("Failed to post AkAudioEvent '%s' at a location without a world world."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(!World->AllowAudioPlayback()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with a world '%s' that doesn't allow audio playback."), *GetName(), *World->GetName());
		return AK_INVALID_PLAYING_ID;
	}

	const AkGameObjectID ObjectID = (AkGameObjectID)this;
	AKRESULT Result = AudioDevice->RegisterGameObject(ObjectID, GetName());
	if (UNLIKELY(Result != AK_Success))
	{
		return AK_INVALID_PLAYING_ID;
	}

	TArray<AkAuxSendValue> AkReverbVolumes;
	AudioDevice->GetAuxSendValuesAtLocation(Location, AkReverbVolumes, World);
	Result = SoundEngine->SetGameObjectAuxSendValues(ObjectID, AkReverbVolumes.GetData(), AkReverbVolumes.Num());
	UE_CLOG(UNLIKELY(Result != AK_Success), LogAkAudio, Log, TEXT("Could not Set AuxSend Values while PostOnLocation for AkAudioEvent '%s' (ObjId: %" PRIu64 "): (%" PRIu32 ") %s."), *GetName(), ObjectID, Result, WwiseUnrealHelper::GetResultString(Result));

	AkRoomID RoomID = 0;
	auto& RoomIndex = AudioDevice->GetRoomIndex();
	TArray<UAkRoomComponent*> AkRooms = RoomIndex.Query<UAkRoomComponent>(Location, World);
	if (LIKELY(AkRooms.Num() > 0))
	{
		UE_CLOG(AkRooms.Num() > 1, LogAkAudio, Verbose, TEXT("There are %d rooms while PostOnLocation for AkAudioEvent '%s' (ObjId: %" PRIu64 "). Picking the first one."), (int)AkRooms.Num(), *GetName(), ObjectID);
		RoomID = AkRooms[0]->GetRoomID();
		AudioDevice->SetInSpatialAudioRoom(ObjectID, RoomID);
	}
	else
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("No Spatial Audio Room while PostOnLocation AkAudioEvent '%s' (ObjId: %" PRIu64 ")"), *GetName(), ObjectID);
	}

	AkSoundPosition SoundPosition;
	FQuat OrientationQuat(Orientation);
	AudioDevice->FVectorsToAKWorldTransform(Location, OrientationQuat.GetForwardVector(), OrientationQuat.GetUpVector(), SoundPosition);
	Result = SoundEngine->SetPosition(ObjectID, SoundPosition);
	UE_CLOG(UNLIKELY(Result != AK_Success), LogAkAudio, Log, TEXT("Could not Set Position for AkAudioEvent '%s' (ObjId: %" PRIu64 "): (%" PRIu32 ") %s."), *GetName(), ObjectID, Result, WwiseUnrealHelper::GetResultString(Result));

	const auto PlayingID = PostOnGameObjectID(ObjectID, Delegate, Callback, Cookie, CallbackMask, LatentAction, AudioContext);

	Result = SoundEngine->UnregisterGameObj( ObjectID );
	UE_CLOG(UNLIKELY(Result != AK_Success), LogAkAudio, Log, TEXT("Could not Unregister GameObject after PostOnLocation for AkAudioEvent '%s' (ObjId: %" PRIu64 "): (%" PRIu32 ") %s."), *GetName(), ObjectID, Result, WwiseUnrealHelper::GetResultString(Result));

	return PlayingID;
}

AkPlayingID UAkAudioEvent::PostAmbient(const FOnAkPostEventCallback* Delegate, AkCallbackFunc Callback, void* Cookie,
	const AkCallbackType CallbackMask, FWaitEndOfEventAction* LatentAction, const EAkAudioContext AudioContext)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostAmbient"));
	return PostOnGameObjectID(DUMMY_GAMEOBJ, Delegate, Callback, Cookie, CallbackMask, LatentAction, AudioContext);
}

AkPlayingID UAkAudioEvent::PostOnGameObject(UAkGameObject* GameObject, const FOnAkPostEventCallback* Delegate,
                                      const AkCallbackFunc Callback, void* Cookie, const AkCallbackType CallbackMask, FWaitEndOfEventAction* LatentAction,
                                      const EAkAudioContext AudioContext)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostOnGameObject"));
	if (UNLIKELY(!GameObject))
	{
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("Failed to post AkAudioEvent without a GameObject."))
		return AK_INVALID_PLAYING_ID;
	}

	UE_CLOG(LIKELY(GameObject->AkAudioEvent) && UNLIKELY(GameObject->AkAudioEvent != this),
		LogWwiseHints, VeryVerbose, TEXT("Posting AkAudioEvent(%s) that is different than the one associated (%s) in the GameObject(%s). You should not have an associated AkAudioEvent if you want to play different Events on the same GameObject."),
		*GetName(),
		*GameObject->AkAudioEvent->GetName(),
		*GameObject->GetName());

	GameObject->UpdateObstructionAndOcclusion();
	const auto GameObjectID = GameObject->GetAkGameObjectID();
	const AkPlayingID PlayingID = PostOnGameObjectID(GameObjectID, Delegate, Callback, Cookie, CallbackMask, LatentAction, AudioContext);
	if (PlayingID != AK_INVALID_PLAYING_ID)
	{
		GameObject->EventPosted();
	}
	return PlayingID;
}

AkPlayingID UAkAudioEvent::PostOnGameObjectID(const AkGameObjectID GameObjectID, const FOnAkPostEventCallback* Delegate,
	const AkCallbackFunc Callback, void* Cookie, const AkCallbackType CallbackMask, FWaitEndOfEventAction* LatentAction,
	const EAkAudioContext AudioContext)
{
	SCOPED_AKAUDIO_EVENT(TEXT("UAkAudioEvent::PostOnGameObjectID"));
	if (Delegate)
	{
		UE_CLOG(UNLIKELY(Callback), LogAkAudio, Error, TEXT("Both Delegate and Callback used for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 "). Ignoring AkCallback."), *GetName(), GameObjectID);
		UE_CLOG(UNLIKELY(LatentAction), LogAkAudio, Error, TEXT("Both Delegate and LatentAction used for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 "). Ignoring LatentAction."), *GetName(), GameObjectID);
		UE_CLOG(UNLIKELY(Cookie), LogAkAudio, Error, TEXT("Blueprint Delegate ignores Cookie for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 ")."), *GetName(), GameObjectID);

		const auto Result = PostEvent(GameObjectID,
			[this, Delegate, CallbackMask](AkGameObjectID GameObjectID)
			{
				auto* AudioDevice = FAkAudioDevice::Get();
				auto* CallbackManager = AudioDevice->GetCallbackManager();
				return CallbackManager->CreateCallbackPackage(*Delegate, CallbackMask, GameObjectID, true);
			},
			AudioContext);
		return Result;
	}
	else if (LatentAction)
	{
		UE_CLOG(UNLIKELY(Callback), LogAkAudio, Error, TEXT("Both LatentAction and Callback used for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 "). Ignoring AkCallback."), *GetName(), GameObjectID);
		UE_CLOG(UNLIKELY(CallbackMask), LogAkAudio, Error, TEXT("LatentAction ignores CallbackMask for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 ")."), *GetName(), GameObjectID);
		UE_CLOG(UNLIKELY(Cookie), LogAkAudio, Error, TEXT("LatentAction ignores Cookie for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 ")."), *GetName(), GameObjectID);

		const auto Result = PostEvent(GameObjectID,
			[this, LatentAction](AkGameObjectID GameObjectID)
			{
				auto* AudioDevice = FAkAudioDevice::Get();
				auto* CallbackManager = AudioDevice->GetCallbackManager();
				return CallbackManager->CreateCallbackPackage(LatentAction, GameObjectID, true);
			},
			AudioContext);
		return Result;
	}
	else
	{
		UE_CLOG(UNLIKELY(!Callback && CallbackMask), LogAkAudio, Error, TEXT("Callback is not set, but there's a CallbackMask for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 ")."), *GetName(), GameObjectID);
		UE_CLOG(UNLIKELY(!Callback && Cookie), LogAkAudio, Error, TEXT("Callback is not set, but there's a Cookie for AkAudioEvent('%s').Post(GameObjectID=%" PRIu64 ")."), *GetName(), GameObjectID);

		const auto Result = PostEvent(GameObjectID,
			[this, Callback, Cookie, CallbackMask](AkGameObjectID GameObjectID)
			{
				auto* AudioDevice = FAkAudioDevice::Get();
				auto* CallbackManager = AudioDevice->GetCallbackManager();
				return CallbackManager->CreateCallbackPackage(Callback, Cookie, CallbackMask, GameObjectID, true);
			},
			AudioContext);
		return Result;
	}
}

template <typename FCreateCallbackPackage>
AkPlayingID UAkAudioEvent::PostEvent(const AkGameObjectID GameObjectID, FCreateCallbackPackage&& CreateCallbackPackage,
	const EAkAudioContext AudioContext)
{
	SCOPED_AKAUDIO_EVENT_2(TEXT("UAkAudioEvent::PostEvent"));
	auto* AudioDevice = FAkAudioDevice::Get();
	if (UNLIKELY(!AudioDevice))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' without an Audio Device."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (UNLIKELY(!AudioDevice->IsInitialized()))
	{
		UE_LOG(LogAkAudio, Verbose, TEXT("Failed to post AkAudioEvent '%s' with the Sound Engine uninitialized."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	if (!IsLoaded())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent: Data for '%s' wasn't found. Make sure the GeneratedSoundBanks folder (%s) exists and is properly set in the project settings."), *GetName(), *WwiseUnrealHelper::GetSoundBankDirectory());
		return AK_INVALID_PLAYING_ID;
	}

	if (!IsDataFullyLoaded())
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent: Not all localization data for '%s' are loaded. Consider using PostEventAsync()."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto* CallbackManager = AudioDevice->GetCallbackManager();
	if (UNLIKELY(!CallbackManager))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent '%s' without a Callback Manager."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent '%s' without a Sound Engine."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto* ExternalSourceManager = IWwiseExternalSourceManager::Get();
	if (UNLIKELY(!ExternalSourceManager))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent '%s' without the External Source Manager."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	auto CallbackPackage = CreateCallbackPackage(GameObjectID);
	if (UNLIKELY(!CallbackPackage))
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Failed to post AkAudioEvent '%s': Could not create CallbackPackage."), *GetName());
		return AK_INVALID_PLAYING_ID;
	}

	TArray<AkExternalSourceInfo> ExternalSources;
	const TArray<uint32> ExternalSourceMedia = ExternalSourceManager->PrepareExternalSourceInfos(ExternalSources, GetAllExternalSources());

	const auto PlayingID = SoundEngine->PostEvent(
		  GetShortID()
		, GameObjectID
		, CallbackPackage->uUserFlags | AK_EndOfEvent
		, &FAkComponentCallbackManager::AkComponentCallback
		, CallbackPackage
		, ExternalSources.Num()
		, ExternalSources.GetData()
	);

	ExternalSourceManager->BindPlayingIdToExternalSources(PlayingID, ExternalSourceMedia);

	if (UNLIKELY(PlayingID == AK_INVALID_PLAYING_ID))
	{
		UE_LOG(LogAkAudio, Log, TEXT("Failed to post AkAudioEvent '%s'."), *GetName());
		CallbackManager->RemoveCallbackPackage(CallbackPackage, GameObjectID);
		return AK_INVALID_PLAYING_ID;
	}

	AudioDevice->AddPlayingID(GetShortID(), PlayingID, AudioContext);

	UE_LOG(LogAkAudio, VeryVerbose, TEXT("Posted AkAudioEvent '%s' as PlayingId %" PRIu32 "."), *GetName(), PlayingID);
	return PlayingID;
}

void UAkAudioEvent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

#if !UE_SERVER
#if WITH_EDITORONLY_DATA 
	if (Ar.IsCooking() && Ar.IsSaving() && !Ar.CookingTarget()->IsServerOnly())
	{
		FWwiseLocalizedEventCookedData CookedDataToArchive;
		if (auto* ResourceCooker = FWwiseResourceCooker::GetForArchive(Ar))
		{
			ResourceCooker->PrepareCookedData(CookedDataToArchive, GetValidatedInfo(EventInfo));
			FillMetadata(ResourceCooker->GetProjectDatabase());
		}
		CookedDataToArchive.Serialize(Ar);
		Ar << MaximumDuration;
		Ar << MinimumDuration;
		Ar << IsInfinite;
		Ar << MaxAttenuationRadius;
	}
#else
	EventCookedData.Serialize(Ar);
	Ar << MaximumDuration;
	Ar << MinimumDuration;
	Ar << IsInfinite;
	Ar << MaxAttenuationRadius;
#endif
#endif
}

#if WITH_EDITORONLY_DATA
void UAkAudioEvent::FillInfo()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAudioEvent::FillInfo: ResourceCooker not initialized"));
		return;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAudioEvent::FillInfo: ProjectDatabase not initialized"));
		return;
	}

	const TSet<FWwiseRefEvent> EventRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetEvent(GetValidatedInfo(EventInfo));
	if (UNLIKELY(EventRef.Num() == 0))
	{
		UE_LOG(LogAkAudio, Log, TEXT("UAkAudioEvent::FillInfo (%s): Cannot fill Asset Info - Event is not loaded"), *GetName());
		return;
	}

	const FWwiseMetadataEvent* EventMetadata = EventRef.Array()[0].GetEvent();
	if (EventMetadata->Name.ToString().IsEmpty() || !EventMetadata->GUID.IsValid() || EventMetadata->Id == AK_INVALID_UNIQUE_ID) 
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkAudioEvent::FillInfo: Valid object not found in Project Database"));
		return;
	}
	EventInfo.WwiseGuid = EventMetadata->GUID;
	EventInfo.WwiseShortId = EventMetadata->Id;
	EventInfo.WwiseName = EventMetadata->Name;
}

void UAkAudioEvent::FillMetadata(FWwiseProjectDatabase* ProjectDatabase)
{
	Super::FillMetadata(ProjectDatabase);

	const TSet<FWwiseRefEvent> EventRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetEvent(GetValidatedInfo(EventInfo));
	if (UNLIKELY(EventRef.Num() == 0))
	{
		UE_LOG(LogAkAudio, Log, TEXT("UAkAudioEvent::FillMetadata (%s): Cannot fill Metadata - Event not found in Project Database"), *GetName());
		return;
	}

	const FWwiseMetadataEvent* EventMetadata = EventRef.Array()[0].GetEvent();
	if (EventMetadata->Name.ToString().IsEmpty() || !EventMetadata->GUID.IsValid() || EventMetadata->Id == AK_INVALID_UNIQUE_ID)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkAudioEvent::FillMetadata: Valid object not found in Project Database"));
		return;
	}

	MaximumDuration = EventMetadata->DurationMax;
	MinimumDuration = EventMetadata->DurationMin;
	IsInfinite = EventMetadata->DurationType == EWwiseMetadataEventDurationType::Infinite;
	MaxAttenuationRadius = EventMetadata->MaxAttenuation;
}

#endif

void UAkAudioEvent::LoadEventData()
{
	SCOPED_AKAUDIO_EVENT_2(TEXT("LoadEventData"));
	auto* ResourceLoader = FWwiseResourceLoader::Get();
	if (UNLIKELY(!ResourceLoader))
	{
		return;
	}

	UnloadEventData(false);
	
#if WITH_EDITORONLY_DATA
	if (!IWwiseProjectDatabaseModule::ShouldInitializeProjectDatabase())
	{
		return;
	}
	auto* ProjectDatabase = FWwiseProjectDatabase::Get();
	if (!ProjectDatabase || !ProjectDatabase->IsProjectDatabaseParsed())
	{
		UE_LOG(LogAkAudio, VeryVerbose, TEXT("UAkAudioEvent::LoadEventData: Not loading '%s' because project database is not parsed."), *GetName())
		return;
	}
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		return;
	}
	if (UNLIKELY(!ResourceCooker->PrepareCookedData(EventCookedData, GetValidatedInfo(EventInfo))))
	{
		return;
	}
	FillMetadata(ResourceCooker->GetProjectDatabase());
#endif

	UE_LOG(LogAkAudio, Verbose, TEXT("%s - LoadEventData"), *GetName());
	
	const auto NewlyLoadedEvent = ResourceLoader->LoadEvent(EventCookedData);
	auto PreviouslyLoadedEvent = LoadedEvent.exchange(NewlyLoadedEvent);
	if (UNLIKELY(PreviouslyLoadedEvent))
	{
		ResourceLoader->UnloadEvent(MoveTemp(PreviouslyLoadedEvent));
	}
}

#if WITH_EDITOR
void UAkAudioEvent::LoadEventDataForContentBrowserPreview()
{
	if(!bAutoLoad)
	{
		OnBeginPIEDelegateHandle = FEditorDelegates::BeginPIE.AddUObject(this, &UAkAudioEvent::OnBeginPIE);
	}

	LoadEventData();
}

void UAkAudioEvent::OnBeginPIE(const bool bIsSimulating)
{
	FEditorDelegates::BeginPIE.Remove(OnBeginPIEDelegateHandle);
	OnBeginPIEDelegateHandle.Reset();
	UnloadEventData(false);
}

#endif

void UAkAudioEvent::BeginDestroy()
{
	Super::BeginDestroy();

#if WITH_EDITOR
	if (OnBeginPIEDelegateHandle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(OnBeginPIEDelegateHandle);
		OnBeginPIEDelegateHandle.Reset();
	}
#endif
}

void UAkAudioEvent::UnloadEventData(bool bAsync)
{
	auto PreviouslyLoadedEvent = LoadedEvent.exchange(nullptr);
	if (PreviouslyLoadedEvent)
	{
		auto* ResourceLoader = FWwiseResourceLoader::Get();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		UE_LOG(LogAkAudio, Verbose, TEXT("%s - UnloadEventData"), *GetName());
		if (bAsync)
		{
			FWwiseLoadedEventPromise Promise;
			Promise.EmplaceValue(MoveTemp(PreviouslyLoadedEvent));
			ResourceUnload = ResourceLoader->UnloadEventAsync(Promise.GetFuture());
		}
		else
		{
			ResourceLoader->UnloadEvent(MoveTemp(PreviouslyLoadedEvent));
		}
	}
}

bool UAkAudioEvent::IsDataFullyLoaded() const
{
	auto CurrentLoadedEvent = LoadedEvent.load();
	if (!CurrentLoadedEvent)
	{
		return false;
	}

	return CurrentLoadedEvent->GetValue().LoadedData.IsLoaded();
}

bool UAkAudioEvent::IsLoaded() const
{
	return LoadedEvent.load() != nullptr;
}

#if WITH_EDITORONLY_DATA
bool UAkAudioEvent::ObjectIsInSoundBanks()
{
	auto* ResourceCooker = FWwiseResourceCooker::GetDefault();
	if (UNLIKELY(!ResourceCooker))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAudioEvent::GetWwiseRef: ResourceCooker not initialized"));
		return false;
	}

	auto ProjectDatabase = ResourceCooker->GetProjectDatabase();
	if (UNLIKELY(!ProjectDatabase))
	{
		UE_LOG(LogAkAudio, Error, TEXT("UAkAudioEvent::GetWwiseRef: ProjectDatabase not initialized"));
		return false;
	}

	const TSet<FWwiseRefEvent> EventRef = FWwiseDataStructureScopeLock(*ProjectDatabase).GetEvent(GetValidatedInfo(EventInfo));
	if (UNLIKELY(EventRef.Num() == 0))
	{
		UE_LOG(LogAkAudio, Log, TEXT("UAkAudioEvent::GetWwiseRef (%s): Event is not loaded"), *GetName());
		return false;
	}

	return EventRef.Array()[0].IsValid();
}
#endif

TArray<FWwiseExternalSourceCookedData> UAkAudioEvent::GetAllExternalSources() const
{
	auto CurrentLoadedEvent = LoadedEvent.load();
	if (!CurrentLoadedEvent)
	{
		return {};
	}

	const auto& EventData = CurrentLoadedEvent->GetValue();
	if (!EventData.LoadedData.IsLoaded())
	{
		return {};
	}

	const auto& LoadedLanguage = EventData.LanguageRef;
	const FWwiseEventCookedData* CookedData = EventCookedData.EventLanguageMap.Find(LoadedLanguage);
	if (UNLIKELY(!CookedData))
	{
		return {};
	}

	auto Result = CookedData->ExternalSources;
	for (const auto& Leaf : CookedData->SwitchContainerLeaves)
	{
		Result.Append(Leaf.ExternalSources);
	}
	return Result;
}

#if WITH_EDITORONLY_DATA
void UAkAudioEvent::CookAdditionalFilesOverride(const TCHAR* PackageFilename, const ITargetPlatform* TargetPlatform,
	TFunctionRef<void(const TCHAR* Filename, void* Data, int64 Size)> WriteAdditionalFile)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	FWwiseResourceCooker* ResourceCooker = FWwiseResourceCooker::GetForPlatform(TargetPlatform);
	if (!ResourceCooker)
	{
		return;
	}
	ResourceCooker->SetSandboxRootPath(PackageFilename);
	ResourceCooker->CookEvent(GetValidatedInfo(EventInfo), WriteAdditionalFile);
}
#endif
