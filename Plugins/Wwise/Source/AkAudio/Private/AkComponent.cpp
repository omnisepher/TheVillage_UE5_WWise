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
	AkComponent.cpp:
=============================================================================*/

#include "AkComponent.h"

#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkLateReverbComponent.h"
#include "AkRoomComponent.h"
#include "AkGameplayTypes.h"
#include "AkSettings.h"
#include "AkSpotReflector.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectIterator.h"
#include "Wwise/WwiseExternalSourceManager.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/API/WwiseSpatialAudioAPI.h"

#if WITH_EDITOR
#include "LevelEditorViewport.h"
#include "Editor.h"
#endif

/*------------------------------------------------------------------------------------
Component Helpers
------------------------------------------------------------------------------------*/
namespace UAkComponentUtils
{
	APlayerController* GetAPlayerController(const UActorComponent* Component)
	{
		const APlayerCameraManager* AsPlayerCameraManager = Cast<APlayerCameraManager>(Component->GetOwner());
		return AsPlayerCameraManager ? AsPlayerCameraManager->GetOwningPlayerController() : nullptr;
	}

	void GetListenerPosition(const UAkComponent* Component, FVector& Location, FVector& Front, FVector& Up)
	{
		APlayerController* pPlayerController = GetAPlayerController(Component);
		if (pPlayerController != nullptr)
		{
			FVector Right;
			pPlayerController->GetAudioListenerPosition(Location, Front, Right);
			Up = FVector::CrossProduct(Front, Right);
			return;
		}

#if WITH_EDITORONLY_DATA
		auto& Clients = GEditor->GetAllViewportClients();
		static FTransform LastKnownEditorTransform;
		for (int i = 0; i < Clients.Num(); i++)
		{
			FEditorViewportClient* ViewportClient = Clients[i];
			UWorld* World = ViewportClient->GetWorld();
			if (ViewportClient->Viewport && ViewportClient->Viewport->HasFocus() && World->AllowAudioPlayback())
			{
				EWorldType::Type WorldType = World->WorldType;
				if (WorldType == EWorldType::Editor || WorldType == EWorldType::PIE)
				{
					LastKnownEditorTransform = FAkAudioDevice::Get()->GetEditorListenerPosition(i);
					Location = LastKnownEditorTransform.GetLocation();
					Front = LastKnownEditorTransform.GetRotation().GetForwardVector();
					Up = LastKnownEditorTransform.GetRotation().GetUpVector();
					return;
				}
				else if (WorldType != EWorldType::Game && WorldType != EWorldType::GamePreview)
				{
					Location = ViewportClient->GetViewLocation();
					Front = ViewportClient->GetViewRotation().Quaternion().GetForwardVector();
					Up = ViewportClient->GetViewRotation().Quaternion().GetUpVector();
					LastKnownEditorTransform.SetLocation(Location);
					LastKnownEditorTransform.SetRotation(ViewportClient->GetViewRotation().Quaternion());
					return;
				}
			}
		}

		Location = LastKnownEditorTransform.GetLocation();
		Front = LastKnownEditorTransform.GetRotation().GetForwardVector();
		Up = LastKnownEditorTransform.GetRotation().GetUpVector();
#endif
	}

	void GetLocationFrontUp(const UAkComponent* Component, FVector& Location, FVector& Front, FVector& Up)
	{
		if (Component->IsDefaultListener)
		{
			GetListenerPosition(Component, Location, Front, Up);
		}
		else
		{
			auto& Transform = Component->GetComponentTransform();
			Location = Transform.GetTranslation();
			Front = Transform.GetUnitAxis(EAxis::X);
			Up = Transform.GetUnitAxis(EAxis::Z);
		}
	}
}

AkReverbFadeControl::AkReverbFadeControl(const UAkLateReverbComponent& LateReverbComponent)
	: AuxBusId(LateReverbComponent.GetAuxBusId())
	, bIsFadingOut(false)
	, FadeControlUniqueId((void*)&LateReverbComponent)
	, CurrentControlValue(0.f)
	, TargetControlValue(LateReverbComponent.SendLevel)
	, FadeRate(LateReverbComponent.FadeRate)
	, Priority(LateReverbComponent.Priority)
{}

void AkReverbFadeControl::UpdateValues(const UAkLateReverbComponent& LateReverbComponent)
{
	AuxBusId = LateReverbComponent.GetAuxBusId();
	TargetControlValue = LateReverbComponent.SendLevel;
	FadeRate = LateReverbComponent.FadeRate;
	Priority = LateReverbComponent.Priority;
}

bool AkReverbFadeControl::Update(float DeltaTime)
{
	if (CurrentControlValue != TargetControlValue || bIsFadingOut)
	{
		// Rate (%/s) * Delta (s) = % for given delta, apply to target.
		const float Increment = DeltaTime * FadeRate * TargetControlValue;
		if (bIsFadingOut)
		{
			CurrentControlValue -= Increment;
			if (CurrentControlValue <= 0.f)
				return false;
		}
		else
			CurrentControlValue = FMath::Min(CurrentControlValue + Increment, TargetControlValue);
	}

	return true;
}

AkAuxSendValue AkReverbFadeControl::ToAkAuxSendValue() const
{
	AkAuxSendValue ret;
	ret.listenerID = AK_INVALID_GAME_OBJECT;
	ret.auxBusID = AuxBusId;
	ret.fControlValue = CurrentControlValue;
	return ret;
}

bool AkReverbFadeControl::Prioritize(const AkReverbFadeControl& A, const AkReverbFadeControl& B)
{
	if (A.bIsFadingOut == B.bIsFadingOut)
	{
		if (A.Priority == B.Priority)
		{
			// Sort by bus id if priority and fade are equal, to ensure comparisons in UAkComponent::NeedToUpdateAuxSends dont lead to continuous aux sends updates, when there are overlapping reverbs.
			return A.AuxBusId < B.AuxBusId;
		}
		return A.Priority > B.Priority;
	}
	// Ensure the fading out buffers are sent to the end of the array.
	return A.bIsFadingOut < B.bIsFadingOut;
}

/*------------------------------------------------------------------------------------
	UAkComponent
------------------------------------------------------------------------------------*/

UAkComponent::UAkComponent(const class FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer)
{
	// Property initialization

	DrawFirstOrderReflections = false;
	DrawSecondOrderReflections = false;
	DrawHigherOrderReflections = false;
	DrawDiffraction = false;

	EarlyReflectionBusSendGain = 1.f;

 	StopWhenOwnerDestroyed = true;
	bUseReverbVolumes = true;
	OcclusionRefreshInterval = 0.2f;

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	bTickInEditor = true;

	bAutoActivate = true;
	bNeverNeedsRenderUpdate = true;
	bWantsOnUpdateTransform = true;

#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif

	AttenuationScalingFactor = 1.0f;
	bAutoDestroy = false;
	bUseDefaultListeners = true;

	OcclusionCollisionChannel = EAkCollisionChannel::EAKCC_UseIntegrationSettingsDefault;

	outerRadius = 0.0f;
	innerRadius = 0.0f;
}

ECollisionChannel UAkComponent::GetOcclusionCollisionChannel()
{
	return UAkSettings::ConvertOcclusionCollisionChannel(OcclusionCollisionChannel.GetValue());
}

int32 UAkComponent::PostAssociatedAkEventAndWaitForEnd(FLatentActionInfo LatentInfo)
{
	return PostAkEventAndWaitForEnd(AkAudioEvent, LatentInfo);
}

int32 UAkComponent::PostAkEventAndWaitForEnd(class UAkAudioEvent * AkEvent, FLatentActionInfo LatentInfo)
{
	if (LIKELY(IsValid(AkEvent)))
	{
		return AkEvent->PostOnComponentAndWait(this, StopWhenOwnerDestroyed, LatentInfo);
	}

	UE_LOG(LogAkAudio, Error, TEXT("Failed to post invalid latent AkAudioEvent on component '%s'"), *GetName());
	return AK_INVALID_PLAYING_ID;
}

int32 UAkComponent::PostAkEvent(UAkAudioEvent* AkEvent, int32 CallbackMask,
	const FOnAkPostEventCallback& PostEventCallback)
{
	if (LIKELY(IsValid(AkEvent)))
	{
		return AkEvent->PostOnComponent(this, PostEventCallback, CallbackMask, StopWhenOwnerDestroyed);
	}

	UE_LOG(LogAkAudio, Error, TEXT("Failed to post invalid AkAudioEvent on component '%s'"), *GetName());
	return AK_INVALID_PLAYING_ID;
}

AkPlayingID UAkComponent::PostAkEvent(UAkAudioEvent* AkEvent, AkUInt32 Flags, AkCallbackFunc UserCallback,
	void* UserCookie)
{
	if (LIKELY(IsValid(AkEvent)))
	{
		return AkEvent->PostOnComponent(this, nullptr, UserCallback, UserCookie, static_cast<AkCallbackType>(Flags), nullptr, StopWhenOwnerDestroyed);
	}

	UE_LOG(LogAkAudio, Error, TEXT("Failed to post invalid AkAudioEvent on component '%s'"), *GetName());
	return AK_INVALID_PLAYING_ID;
}

AkRoomID UAkComponent::GetSpatialAudioRoomID() const
{
	AkRoomID RoomID;
	if (CurrentRoom.IsValid())
	{
		RoomID = CurrentRoom->GetRoomID();
	}
	return RoomID;
}

void UAkComponent::UpdateObstructionAndOcclusion()
{
	SCOPED_AKAUDIO_EVENT_2(TEXT("UAkComponent::UpdateObstructionAndOcclusion"));
	auto World = GetWorld();
	auto AudioDevice = FAkAudioDevice::Get();

	if (World && AudioDevice && AudioDevice->ShouldNotifySoundEngine(World->WorldType))
	{
		FScopeLock Lock(&ListenerCriticalSection);
		AkObstructionAndOcclusionService::ListenerMap ObsOccListenerMap;
		for (auto& Listener : Listeners)
		{
			AkObstructionAndOcclusionService::FListenerInfo ListenerInfo(Listener->GetPosition(), Listener->GetSpatialAudioRoomID());
			ObsOccListenerMap.Add(Listener->GetAkGameObjectID(), ListenerInfo);
		}

		AkObstructionAndOcclusionService::PortalMap ObsOccPortalMap;
		AudioDevice->GetObsOccServicePortalMap(GetSpatialAudioRoom(), GetWorld(), ObsOccPortalMap);

		ObstructionService.UpdateObstructionAndOcclusion(ObsOccListenerMap, ObsOccPortalMap, GetPosition(), GetOwner(), GetSpatialAudioRoomID(), GetOcclusionCollisionChannel(), OcclusionRefreshInterval);
	}
}

void UAkComponent::PostTrigger(const UAkTrigger* TriggerValue, FString Trigger)
{
	if (FAkAudioDevice::Get())
	{
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine)) return;

		if (TriggerValue)
		{
			SoundEngine->PostTrigger(TriggerValue->TriggerCookedData.TriggerId, GetAkGameObjectID());
		}
		else
		{
			SoundEngine->PostTrigger(TCHAR_TO_AK(*Trigger), GetAkGameObjectID());
		}
	}
}

void UAkComponent::SetSwitch(const UAkSwitchValue* SwitchValue, FString SwitchGroup, FString SwitchState)
{
	if (FAkAudioDevice::Get())
	{
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		if (UNLIKELY(!SoundEngine)) return;

		if (SwitchValue)
		{
			SoundEngine->SetSwitch(SwitchValue->GroupValueCookedData.GroupId, SwitchValue->GroupValueCookedData.Id, GetAkGameObjectID());
		}
		else
		{
			uint32 SwitchGroupID = SoundEngine->GetIDFromString(TCHAR_TO_AK(*SwitchGroup));
			uint32 SwitchStateID = SoundEngine->GetIDFromString(TCHAR_TO_AK(*SwitchState));

			SoundEngine->SetSwitch(SwitchGroupID, SwitchStateID, GetAkGameObjectID());
		}
	}
}

void UAkComponent::SetStopWhenOwnerDestroyed(bool bStopWhenOwnerDestroyed)
{
	StopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
}

void UAkComponent::SetListeners(const TArray<UAkComponent*>& NewListeners)
{
	auto AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		FScopeLock Lock(&ListenerCriticalSection);
		bUseDefaultListeners = false;

		for(auto& Listener : Listeners)
		{
			Listener->IsListener = false;
		}
		Listeners.Reset();

		for (UAkComponent* AkComponent : NewListeners)
		{
			if(AkComponent)
			{
				Listeners.Add(AkComponent);
			}
		}
		
		for(auto& Listener : Listeners)
		{
			Listener->IsListener = true;
		}
		AudioDevice->SetListeners(this, Listeners.Array());
	}
}

void UAkComponent::SetEarlyReflectionsAuxBus(const FString& AuxBusName)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetEarlyReflectionsAuxBus(this, FAkAudioDevice::GetShortID(nullptr, AuxBusName));
	}
}

void UAkComponent::SetEarlyReflectionsVolume(float SendVolume)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->SetEarlyReflectionsVolume(this, SendVolume);
	}
}

float UAkComponent::GetAttenuationRadius() const
{
	return AkAudioEvent ? AttenuationScalingFactor * AkAudioEvent->MaxAttenuationRadius : 0.f;
}

void UAkComponent::SetOutputBusVolume(float BusVolume)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		FScopeLock Lock(&ListenerCriticalSection);
		for (auto It = Listeners.CreateIterator(); It; ++It)
		{
			AudioDevice->SetGameObjectOutputBusVolume(this, It->Get(), BusVolume);
		}
	}
}

void UAkComponent::OnRegister()
{
	UWorld* CurrentWorld = GetWorld();
	if(!IsRegisteredWithWwise && CurrentWorld->WorldType != EWorldType::Inactive && CurrentWorld->WorldType != EWorldType::None)
		RegisterGameObject(); // Done before parent so that OnUpdateTransform follows registration and updates position correctly.

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		ObstructionService.Init(GetAkGameObjectID(), CurrentWorld, OcclusionRefreshInterval, AudioDevice->UsingSpatialAudioRooms(CurrentWorld));
	}

	// It's possible for OnRegister to be called while the WorldType is inactive.
	// The game object will be registered again later when the WorldType is active.
	if (AudioDevice && IsRegisteredWithWwise)
	{
		if (EarlyReflectionAuxBus || !EarlyReflectionAuxBusName.IsEmpty())
		{
			AkUInt32 AuxBusID = FAkAudioDevice::GetShortID(EarlyReflectionAuxBus, EarlyReflectionAuxBusName);
			if (AuxBusID != AK_INVALID_UNIQUE_ID)
				AudioDevice->SetEarlyReflectionsAuxBus(this, AuxBusID);
		}
		if (EarlyReflectionBusSendGain != 1.0)
			AudioDevice->SetEarlyReflectionsVolume(this, EarlyReflectionBusSendGain);
	}

	Super::OnRegister();

#if WITH_EDITORONLY_DATA
	if (bVisualizeComponent)
	{
		UpdateSpriteTexture();
	}
#endif
}

#if WITH_EDITORONLY_DATA
void UAkComponent::UpdateSpriteTexture()
{
	if (SpriteComponent)
	{
		SpriteComponent->SetSprite(LoadObject<UTexture2D>(NULL, TEXT("/Wwise/S_AkComponent.S_AkComponent")));
	}
}
#endif

void UAkComponent::OnUnregister()
{
	// Route OnUnregister event.
	Super::OnUnregister();

	// Don't stop audio and clean up component if owner has been destroyed (default behaviour). This function gets
	// called from AActor::ClearComponents when an actor gets destroyed which is not usually what we want for one-
	// shot sounds.
	AActor* Owner = GetOwner();
	UWorld* CurrentWorld = GetWorld();
	if( !Owner || !CurrentWorld || StopWhenOwnerDestroyed || CurrentWorld->bIsTearingDown || (Owner->GetClass() == APlayerController::StaticClass() && CurrentWorld->WorldType == EWorldType::PIE))
	{
		Stop();
	}
}

void UAkComponent::OnComponentDestroyed( bool bDestroyingHierarchy )
{
	UnregisterGameObject();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UAkComponent::ShutdownAfterError( void )
{
	UnregisterGameObject();

	Super::ShutdownAfterError();
}

bool UAkComponent::NeedToUpdateAuxSends(const TArray<AkAuxSendValue>& NewValues)
{
	if (NewValues.Num() != CurrentAuxSendValues.Num())
		return true;

	for (int32 i = 0; i < NewValues.Num(); i++)
	{
		if (NewValues[i].listenerID != CurrentAuxSendValues[i].listenerID ||
			NewValues[i].auxBusID != CurrentAuxSendValues[i].auxBusID ||
			NewValues[i].fControlValue != CurrentAuxSendValues[i].fControlValue)
		{
			return true;
		}
	}

	return false;
}

void UAkComponent::ApplyAkReverbVolumeList(float DeltaTime)
{
	for (int32 Idx = 0; Idx < ReverbFadeControls.Num(); )
	{
		if (!ReverbFadeControls[Idx].Update(DeltaTime))
			ReverbFadeControls.RemoveAt(Idx);
		else
			++Idx;
	}

	if (ReverbFadeControls.Num() > 1)
		ReverbFadeControls.Sort(AkReverbFadeControl::Prioritize);

	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (AkAudioDevice)
	{
		TArray<AkAuxSendValue> NewAuxSendValues;
		for (int32 Idx = 0; Idx < ReverbFadeControls.Num() && Idx < AkAudioDevice->GetMaxAuxBus(); Idx++)
		{
			AkAuxSendValue* FoundAuxSend = NewAuxSendValues.FindByPredicate([this, Idx](const AkAuxSendValue& ItemInArray) { return ItemInArray.auxBusID == ReverbFadeControls[Idx].AuxBusId; });
			if (FoundAuxSend)
			{
				FoundAuxSend->fControlValue += ReverbFadeControls[Idx].ToAkAuxSendValue().fControlValue;
			}
			else
			{
				NewAuxSendValues.Add(ReverbFadeControls[Idx].ToAkAuxSendValue());
			}
		}

		if (NeedToUpdateAuxSends(NewAuxSendValues))
		{
			AkAudioDevice->SetAuxSends(this, NewAuxSendValues);
			CurrentAuxSendValues = NewAuxSendValues;
		}
	}
}

void UAkComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return;

	if (SoundEngine->IsInitialized())
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

		auto World = GetWorld();
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		// If we're a listener, update our position here instead of in OnUpdateTransform. 
		// This is because PlayerController->GetAudioListenerPosition caches its value, and it can be out of sync
		if (IsDefaultListener && HasMoved())
			UpdateGameObjectPosition();

		if (AkAudioDevice && AkAudioDevice->WorldSpatialAudioVolumesUpdated(World))
		{
			UpdateSpatialAudioRoom(GetComponentLocation());
			// Find and apply all AkReverbVolumes at this location
			if (bUseReverbVolumes && AkAudioDevice->GetMaxAuxBus() > 0)
			{
				UpdateAkLateReverbComponentList(GetComponentLocation());
			}
		}

		if (AkAudioDevice && bUseReverbVolumes && AkAudioDevice->GetMaxAuxBus() > 0)
			ApplyAkReverbVolumeList(DeltaTime);

		if (World && AkAudioDevice && AkAudioDevice->ShouldNotifySoundEngine(World->WorldType))
		{
			FScopeLock Lock(&ListenerCriticalSection);
			AkObstructionAndOcclusionService::ListenerMap ObsOccListenerMap;
			for (auto& Listener : Listeners)
			{
				AkObstructionAndOcclusionService::FListenerInfo ListenerInfo(Listener->GetPosition(), Listener->GetSpatialAudioRoomID());
				ObsOccListenerMap.Add(Listener->GetAkGameObjectID(), ListenerInfo);
			}

			AkObstructionAndOcclusionService::PortalMap ObsOccPortalMap;
			if (AkAudioDevice)
			{
				AkAudioDevice->GetObsOccServicePortalMap(GetSpatialAudioRoom(), GetWorld(), ObsOccPortalMap);
			}

			ObstructionService.Tick(ObsOccListenerMap, ObsOccPortalMap, GetPosition(), GetOwner(), GetSpatialAudioRoomID(), GetOcclusionCollisionChannel(), DeltaTime, OcclusionRefreshInterval);
		}

		if (bAutoDestroy && bEventPosted && !HasActiveEvents())
		{
			DestroyComponent();
		}

#if !UE_BUILD_SHIPPING
		if (DrawFirstOrderReflections || DrawSecondOrderReflections || DrawHigherOrderReflections)
		{
			DebugDrawReflections();
		}
		if (DrawDiffraction)
		{
			DebugDrawDiffraction();
		}
#endif
	}
}

void UAkComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateGameObjectPosition();

	// If spawned inside AkReverbVolume(s), we do not want the fade in effect to kick in.
	UpdateAkLateReverbComponentList(GetComponentLocation());
	for (auto& ReverbFadeControl : ReverbFadeControls)
		ReverbFadeControl.ForceCurrentToTargetValue();

	SetAttenuationScalingFactor(AttenuationScalingFactor);

	if (EnableSpotReflectors)
		AAkSpotReflector::UpdateSpotReflectors(this);
}

void UAkComponent::SetAttenuationScalingFactor(float Value)
{
	AttenuationScalingFactor = Value;
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
		AudioDevice->SetAttenuationScalingFactor(this, AttenuationScalingFactor);
}

void UAkComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);

	// If we're a listener, our position will be updated from Tick instead of here.
	// This is because PlayerController->GetAudioListenerPosition caches its value, and it can be out of sync
	if(!IsDefaultListener)
		UpdateGameObjectPosition();
}

UAkComponent* UAkComponent::GetAkComponent(AkGameObjectID GameObjectID)
{ 
	return GameObjectID == DUMMY_GAMEOBJ ? nullptr : (UAkComponent*)GameObjectID;
}

void UAkComponent::GetAkGameObjectName(FString& Name) const
{
	AActor* parentActor = GetOwner();
	if (parentActor)
	{
#if WITH_EDITOR
		Name = parentActor->GetActorLabel() + ".";
#else
		Name = parentActor->GetName() + ".";
#endif
	}

	Name += GetName();

	UWorld* CurrentWorld = GetWorld();
	switch (CurrentWorld->WorldType)
	{
	case  EWorldType::Editor:
		Name += "(Editor)";
		break;
	case  EWorldType::EditorPreview:
		Name += "(EditorPreview)";
		break;
	case  EWorldType::GamePreview:
		Name += "(GamePreview)";
		break;
	case  EWorldType::Inactive:
		Name += "(Inactive)";
		break;
	}
}

void UAkComponent::PostRegisterGameObject() {}

void UAkComponent::PostUnregisterGameObject() {}

void UAkComponent::RegisterGameObject()
{
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if ( AkAudioDevice )
	{
		if ( bUseDefaultListeners )
		{
			FScopeLock Lock(&ListenerCriticalSection);
			const auto& DefaultListeners = AkAudioDevice->GetDefaultListeners();
			Listeners.Empty(DefaultListeners.Num());
			
			for (auto Listener : DefaultListeners)
			{
				Listeners.Add(Listener);
			}
		}

		AkAudioDevice->RegisterComponent(this);
		IsRegisteredWithWwise = true;

		AkAudioDevice->SetGameObjectRadius(this, outerRadius, innerRadius);
	}

	PostRegisterGameObject();
}

void UAkComponent::UnregisterGameObject()
{
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (AkAudioDevice)
	{
		AkAudioDevice->UnregisterComponent(this);
		IsRegisteredWithWwise = false;
	}

	if(IsListener)
	{
		for (TObjectIterator<UAkComponent> Emitter; Emitter; ++Emitter)
		{
			Emitter->OnListenerUnregistered(this);
		}
	}
	
	PostUnregisterGameObject();
}

void UAkComponent::UpdateAkLateReverbComponentList( FVector Loc )
{
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (!AkAudioDevice)
		return;

	TArray<UAkLateReverbComponent*> FoundComponents = AkAudioDevice->FindLateReverbComponentsAtLocation(Loc, GetWorld());

	// Add the new volumes to the current list
	for (const auto& LateReverbComponent : FoundComponents)
	{
		const auto AuxBusId = LateReverbComponent->GetAuxBusId();
		const int32 FoundIdx = ReverbFadeControls.IndexOfByPredicate([&LateReverbComponent](const AkReverbFadeControl& Candidate)
		{
			return Candidate.FadeControlUniqueId == (void*)LateReverbComponent;
		});

		if (FoundIdx == INDEX_NONE)
		{
			// The volume was not found, add it to the list
			ReverbFadeControls.Add(AkReverbFadeControl(*LateReverbComponent));
		}
		else
		{
			// The volume was found. We still have to check if it is currently fading out, in case we are
			// getting back in a volume we just exited.
			ReverbFadeControls[FoundIdx].bIsFadingOut = false;
			// We need to update the late reverb values in case they have changed on the reverb component.
			ReverbFadeControls[FoundIdx].UpdateValues(*LateReverbComponent);
		}
	}

	// Fade out the current volumes not found in the new list
	for (auto& ReverbFadeControl : ReverbFadeControls)
	{
		const int32 FoundIdx = FoundComponents.IndexOfByPredicate([&ReverbFadeControl](const UAkLateReverbComponent* const Candidate)
		{
			return ReverbFadeControl.FadeControlUniqueId == (void*)Candidate;
		});

		if (FoundIdx == INDEX_NONE)
			ReverbFadeControl.bIsFadingOut = true;
	}
}

FVector UAkComponent::GetPosition() const
{
	return FAkAudioDevice::AKVector64ToFVector(CurrentSoundPosition.Position());
}

bool UAkComponent::HasMoved()
{
	AkSoundPosition soundpos;
	FVector Location, Front, Up;
	UAkComponentUtils::GetLocationFrontUp(this, Location, Front, Up);
	FAkAudioDevice::FVectorsToAKWorldTransform(Location, Front, Up, soundpos);

	return CurrentSoundPosition.Position().X != soundpos.Position().X || CurrentSoundPosition.Position().Y != soundpos.Position().Y || CurrentSoundPosition.Position().Z != soundpos.Position().Z ||
		CurrentSoundPosition.OrientationTop().X != soundpos.OrientationTop().X || CurrentSoundPosition.OrientationTop().Y != soundpos.OrientationTop().Y || CurrentSoundPosition.OrientationTop().Z != soundpos.OrientationTop().Z ||
		CurrentSoundPosition.OrientationFront().X != soundpos.OrientationFront().X || CurrentSoundPosition.OrientationFront().Y != soundpos.OrientationFront().Y || CurrentSoundPosition.OrientationFront().Z != soundpos.OrientationFront().Z;
}

void UAkComponent::UpdateGameObjectPosition()
{
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (IsActive() && AkAudioDevice)
	{
		if (AllowAudioPlayback())
		{
			AkSoundPosition soundpos;
			FVector Location, Front, Up;
			UAkComponentUtils::GetLocationFrontUp(this, Location, Front, Up);
			FAkAudioDevice::FVectorsToAKWorldTransform(Location, Front, Up, soundpos);

			UpdateSpatialAudioRoom(Location);

			AkAudioDevice->SetPosition(this, soundpos);
			CurrentSoundPosition = soundpos;
		}
		 
		// Find and apply all AkReverbVolumes at this location
		if (bUseReverbVolumes && AkAudioDevice->GetMaxAuxBus() > 0)
		{
			UpdateAkLateReverbComponentList(GetComponentLocation());
		}
	}
}

void UAkComponent::UpdateSpatialAudioRoom(FVector Location)
{
	if (!IsRegisteredWithWwise)
	{
		return;
	}

	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (!AkAudioDevice)
	{
		return;
	}

	if (!AkAudioDevice->WorldHasActiveRooms(GetWorld()))
	{
		if (CurrentRoom.IsValid())
		{
			// Edge case where component was previously in a room and has somehow persisted between world changes 
			CurrentRoom.Reset();
		}
		return;
	}

	AKRESULT result = AK_Fail;
	TArray<UAkRoomComponent*> RoomComponents = AkAudioDevice->FindRoomComponentsAtLocation(Location, GetWorld());
	if (RoomComponents.Num() == 0 && CurrentRoom.IsValid())
	{
		// No longer in room
		CurrentRoom.Reset();
		result = AkAudioDevice->SetInSpatialAudioRoom(GetAkGameObjectID(), AK::SpatialAudio::kOutdoorRoomID);
	}
	else if (RoomComponents.Num() > 0 && CurrentRoom.Get() != RoomComponents[0])
	{
		// In a new room
		CurrentRoom = RoomComponents[0];
		result = AkAudioDevice->SetInSpatialAudioRoom(GetAkGameObjectID(), GetSpatialAudioRoomID());
	}

	if (EnableSpotReflectors && result == AK_Success)
	{
		AAkSpotReflector::UpdateSpotReflectors(this);
	}
}

void UAkComponent::_DebugDrawReflections( const AkVector64& akEmitterPos, const AkVector64& akListenerPos, const AkReflectionPathInfo* paths, AkUInt32 uNumPaths) const
{
	::FlushDebugStrings(GWorld);

	for (AkInt32 idxPath = uNumPaths-1; idxPath >= 0; --idxPath)
	{
		const AkReflectionPathInfo& path = paths[idxPath];

		unsigned int order = path.numReflections;

		if ((DrawFirstOrderReflections && order == 1) ||
			(DrawSecondOrderReflections && order == 2) ||
			(DrawHigherOrderReflections && order > 2))
		{
			FColor colorLight;
			FColor colorMed;
			FColor colorDark;

			switch ((order - 1))
			{
			case 0:
				colorLight = FColor(0x9DEBF3);
				colorMed = FColor(0x318087);
				colorDark = FColor(0x186067);
				break;
			case 1:
				colorLight = FColor(0xFCDBA2);
				colorMed = FColor(0xDEAB4E);
				colorDark = FColor(0xA97B27);
				break;
			case 2:
			default:
				colorLight = FColor(0xFCB1A2);
				colorMed = FColor(0xDE674E);
				colorDark = FColor(0xA93E27);
				break;
			}

			FColor colorLightGrey(75, 75, 75);
			FColor colorMedGrey(50, 50, 50);
			FColor colorDarkGrey(35, 35, 35);

			const int kPathThickness = 5.f;
			const float kRadiusSphere = 25.f;
			const int kNumSphereSegments = 8;

			const FVector emitterPos = FAkAudioDevice::AKVector64ToFVector(akEmitterPos);
			FVector listenerPt = FAkAudioDevice::AKVector64ToFVector(akListenerPos);

			for (int idxSeg = path.numPathPoints-1; idxSeg >= 0; --idxSeg)
			{
				const FVector reflectionPt = FAkAudioDevice::AKVector64ToFVector(path.pathPoint[idxSeg]);
				
				if (idxSeg != path.numPathPoints - 1)
				{
					// Note: Not drawing the first leg of the path from the listener.  Often hard to see because it is typically the camera position.
					::DrawDebugLine(GWorld, listenerPt, reflectionPt, path.isOccluded ? colorLightGrey : colorLight, false, -1.f, (uint8)'\000', kPathThickness / order);

					::DrawDebugSphere(GWorld, reflectionPt, (kRadiusSphere/2) / order, kNumSphereSegments, path.isOccluded ? colorLightGrey : colorLight);
				}
				else
				{
					::DrawDebugSphere(GWorld, reflectionPt, kRadiusSphere / order, kNumSphereSegments, path.isOccluded ? colorMedGrey : colorMed);
				}
				
				// Draw image source point.  Not as useful as I had hoped.
				//const FVector imageSrc = FAkAudioDevice::AKVectorToFVector(path.imageSource);
				//::DrawDebugSphere(GWorld, imageSrc, kRadiusSphere/order, kNumSphereSegments, colorDark);

				listenerPt = reflectionPt;
			}

			if (!path.isOccluded)
			{
				// Finally the last path segment towards the emitter.
				::DrawDebugLine(GWorld, listenerPt, emitterPos, path.isOccluded ? colorLightGrey : colorLight, false, -1.f, (uint8)'\000', kPathThickness / order);
			}
		}
	}
	
}

void UAkComponent::_DebugDrawDiffraction(const AkVector64& akEmitterPos, const AkVector64& akListenerPos, const AkDiffractionPathInfo* paths, AkUInt32 uNumPaths) const
{
	::FlushDebugStrings(GWorld);

	for (AkInt32 idxPath = uNumPaths - 1; idxPath >= 0; --idxPath)
	{
		const AkDiffractionPathInfo& path = paths[idxPath];
		
		FColor purple(0x492E74);
		FColor green(0x267158);

		if (path.nodeCount > 0)
		{
			const int kPathThickness = 5.f;
			const float kRadiusSphereMax = 35.f;
			const float kRadiusSphereMin = 2.f;

			const FVector emitterPos = FAkAudioDevice::AKVector64ToFVector(akEmitterPos);
			const FVector listenerPos = FAkAudioDevice::AKVector64ToFVector(akListenerPos);
			FVector prevPt = FAkAudioDevice::AKVector64ToFVector(akListenerPos);

			for (int idxSeg = 0; idxSeg < (int)path.nodeCount; ++idxSeg)
			{
				const FVector pt = FAkAudioDevice::AKVector64ToFVector(path.nodes[idxSeg]);

				if (idxSeg != 0)
				{
					::DrawDebugLine(GWorld, prevPt, pt, green, false, -1.f, (uint8)'\000', kPathThickness);
				}

				float rad = kRadiusSphereMin + (1.f - path.angles[idxSeg] / PI) * (kRadiusSphereMax - kRadiusSphereMin);
				::DrawDebugSphere(GWorld, pt, rad, 8, path.portals[idxSeg].IsValid() ? green : purple );

				prevPt = pt;
			}

			// Finally the last path segment towards the emitter.
			::DrawDebugLine(GWorld, prevPt, emitterPos, green, false, -1.f, (uint8)'\000', kPathThickness);
		}
	}
}

void UAkComponent::DebugDrawReflections() const
{
	auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
	if (UNLIKELY(!SpatialAudio)) return;

	enum { kMaxPaths = 64 };
	AkReflectionPathInfo paths[kMaxPaths];
	AkUInt32 uNumPaths = kMaxPaths;
	AkVector64 listenerPos, emitterPos;
	 
	if (SpatialAudio->QueryReflectionPaths(GetAkGameObjectID(), 0, listenerPos, emitterPos, paths, uNumPaths) == AK_Success && uNumPaths > 0)
		_DebugDrawReflections(emitterPos, listenerPos, paths, uNumPaths);
}

void UAkComponent::DebugDrawDiffraction() const
{
	auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
	if (UNLIKELY(!SpatialAudio)) return;

	enum { kMaxPaths = 16 };
	AkDiffractionPathInfo paths[kMaxPaths];
	AkUInt32 uNumPaths = kMaxPaths;

	AkVector64 listenerPos, emitterPos;

	if (SpatialAudio->QueryDiffractionPaths(GetAkGameObjectID(), 0, listenerPos, emitterPos, paths, uNumPaths) == AK_Success)
	{
		if (uNumPaths > 0)
			_DebugDrawDiffraction(emitterPos, listenerPos, paths, uNumPaths);
	}
}

void UAkComponent::SetGameObjectRadius(float in_outerRadius, float in_innerRadius)
{
	outerRadius = in_outerRadius;
	innerRadius = in_innerRadius;
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (AkAudioDevice && IsRegisteredWithWwise)
	{
		AkAudioDevice->SetGameObjectRadius(this, outerRadius, innerRadius);
	}
}

void UAkComponent::SetEnableSpotReflectors(bool in_enable)
{
	if (EnableSpotReflectors != in_enable)
	{
		EnableSpotReflectors = in_enable;
		AAkSpotReflector::UpdateSpotReflectors(this);
	}
}

#if WITH_EDITOR
void UAkComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		if ((PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAkComponent, outerRadius) ||
			PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAkComponent, innerRadius)) &&
			PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
		{
			if (innerRadius > outerRadius)
				innerRadius = outerRadius;

			SetGameObjectRadius(outerRadius, innerRadius);
		}
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAkComponent, EnableSpotReflectors) &&
			PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
		{
			AAkSpotReflector::UpdateSpotReflectors(this);
		}
	}
}
#endif

