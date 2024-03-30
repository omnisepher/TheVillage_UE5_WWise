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

#include "Wwise/Niagara/NiagaraDataInterfaceWwiseEvent.h"

#include "Components/AudioComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Internationalization/Internationalization.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "NiagaraCustomVersion.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraTypes.h"
#include "NiagaraWorldManager.h"

#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "WwiseUEFeatures.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/Niagara.h"

const FName UNiagaraDataInterfaceWwiseEvent::PostEventAtLocationName(TEXT("PostWwiseEventAtLocation"));
const FName UNiagaraDataInterfaceWwiseEvent::PostPersistentWwiseEventName(TEXT("PostPersistentWwiseEvent"));
const FName UNiagaraDataInterfaceWwiseEvent::SetPersistentWwiseEventPositionName(TEXT("UpdatePersistentEventPosition"));
const FName UNiagaraDataInterfaceWwiseEvent::SetPersistentWwiseEventRotationName(TEXT("UpdatePersistentEventRotation"));
const FName UNiagaraDataInterfaceWwiseEvent::PausePersistentWwiseEventName(TEXT("SetWwiseEventPaused"));
const FName UNiagaraDataInterfaceWwiseEvent::SetPersistenEventGameParameterName(TEXT("SetPersistentEventGameParameter"));
const FName UNiagaraDataInterfaceWwiseEvent::StopPersistentWwiseEventName(TEXT("StopPersistentWwiseEvent"));

struct FNiagaraPostEventDIFunctionVersion
{
	enum Type
	{
		InitialVersion = 0,
		VersionUE5,

		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};

namespace NiagaraWwiseParticleHelpers
{
	UAkComponent* SpawnAkComponentAtLocation(class UAkAudioEvent* AkEvent, const USceneComponent* NiagaraComponent, FVector Location, FRotator Orientation, UWorld* World, bool bStopWhenDestroyed, bool bVisualizeComponent = false)
	{
		SCOPED_WWISENIAGARA_EVENT_2(TEXT("NiagaraWwiseParticleHelpers::SpawnAkComponentAtLocation"));
		UAkComponent* AkComponent;
		if (NiagaraComponent)
		{
			AkComponent = NewObject<UAkComponent>(NiagaraComponent->GetOwner());
		}
		else if (World)
		{
			AkComponent = NewObject<UAkComponent>(World->GetWorldSettings());
		}
		else
		{
			AkComponent = NewObject<UAkComponent>();
		}

		if (AkComponent)
		{
#if WITH_EDITORONLY_DATA
			AkComponent->bVisualizeComponent = bVisualizeComponent;
#endif
			AkComponent->AkAudioEvent = AkEvent;
			
			AkComponent->SetWorldLocationAndRotation(Location, Orientation.Quaternion());
			if (World)
			{
				AkComponent->RegisterComponentWithWorld(World);
			}

			AkComponent->SetAutoDestroy(true);
			//Always stop looping events
			if (AkEvent->IsInfinite)
			{
				bStopWhenDestroyed = true;
			}
			AkComponent->SetStopWhenOwnerDestroyed(bStopWhenDestroyed);

#if WITH_EDITORONLY_DATA
			FString EventName = AkEvent->GetWwiseName().ToString();
#else 
			FString EventName = AkEvent->EventCookedData.DebugName.ToString();
#endif

			UE_LOG(LogWwiseNiagara, VeryVerbose, TEXT("Creating AkComponent %s for Event %s, with owner %s"), *AkComponent->GetName(), *EventName, *AkComponent->GetOwner()->GetName())
		}

		return AkComponent;
	}
}

UNiagaraDataInterfaceWwiseEvent::UNiagaraDataInterfaceWwiseEvent(FObjectInitializer const& ObjectInitializer) : Super(ObjectInitializer)
{
	EventToPost = nullptr;
	bLimitPostsPerTick = true;
	MaxPostsPerTick = 10;
}

#if WITH_EDITORONLY_DATA
bool UNiagaraDataInterfaceWwiseEvent::UpgradeFunctionCall(FNiagaraFunctionSignature& FunctionSignature)
{
	bool bChanged = false;

#if UE_5_0_OR_LATER
	// Update out UE4 assets to UE5. We need to make sure that Positions are using the latest type
	if (FunctionSignature.FunctionVersion < FNiagaraPostEventDIFunctionVersion::VersionUE5)
	{
		if (FunctionSignature.Name == PostPersistentWwiseEventName)
		{
			int32 PositionIndex = FunctionSignature.Inputs.IndexOfByPredicate([](const FNiagaraVariable& Param)
			{
				return Param.GetName() == TEXT("Position");
			});

			if (PositionIndex != INDEX_NONE)
			{
				FunctionSignature.Inputs[PositionIndex].SetType(FNiagaraTypeDefinition::GetPositionDef());
				bChanged = true;
			}
		}

		else if (FunctionSignature.Name == SetPersistentWwiseEventPositionName)
		{
			int32 PositionIndex = FunctionSignature.Inputs.IndexOfByPredicate([](const FNiagaraVariable& Param)
			{
				return Param.GetName() == TEXT("Position");
			});

			if (PositionIndex != INDEX_NONE)
			{
				FunctionSignature.Inputs[PositionIndex].SetType(FNiagaraTypeDefinition::GetPositionDef());
				bChanged = true;
			}
		}

		FunctionSignature.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
	}
#endif

	return bChanged;
}
#endif

void UNiagaraDataInterfaceWwiseEvent::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
	}
}

bool UNiagaraDataInterfaceWwiseEvent::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::InitPerInstanceData"));
	FWwiseEventInterface_InstanceData* PIData = new (PerInstanceData) FWwiseEventInterface_InstanceData;
#if UE_5_0_OR_LATER
	PIData->LWCConverter = SystemInstance->GetLWCConverter();
#endif
	if (bLimitPostsPerTick)
	{
		PIData->MaxPlaysPerTick = MaxPostsPerTick;
	}
	PIData->bStopWhenComponentIsDestroyed = bStopWhenComponentIsDestroyed;

#if WITH_EDITORONLY_DATA
	PIData->bOnlyActiveDuringGameplay = bOnlyActiveDuringGameplay;
#endif
	return true;
}

void UNiagaraDataInterfaceWwiseEvent::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::DestroyPerInstanceData"));
	FWwiseEventInterface_InstanceData* InstData = (FWwiseEventInterface_InstanceData*)PerInstanceData;

	for (const auto& Entry : InstData->PersistentComponents)
	{
		if (Entry.Value.IsValid())
		{
			Entry.Value->Stop();
		}
	}
	InstData->~FWwiseEventInterface_InstanceData();
}

bool UNiagaraDataInterfaceWwiseEvent::PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::PerInstanceTick"));
	FWwiseEventInterface_InstanceData* PIData = (FWwiseEventInterface_InstanceData*)PerInstanceData;
	if (!PIData)
	{
		return true;
	}

	if (IsValid(EventToPost) && SystemInstance)
	{
		PIData->EventToPost = EventToPost;

		PIData->GameParameters.Empty();
		for (const auto GameParameter : GameParameters)
		{
			PIData->GameParameters.Add(GameParameter);
		}
		PIData->bValidOneShotSound = !EventToPost->IsInfinite;
	}
	else
	{
		PIData->EventToPost.Reset();
		PIData->bValidOneShotSound = false;
	}

	return false;
}

bool UNiagaraDataInterfaceWwiseEvent::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	FWwiseEventInterface_InstanceData* PIData = (FWwiseEventInterface_InstanceData*) PerInstanceData;
	UNiagaraSystem* System = SystemInstance->GetSystem();

	// Would love to use SystemInstance->GetWorldManager()->GetWorld(), but it's not exposed in the api
	UWorld* World = SystemInstance->GetAttachComponent()->GetWorld();

#if WITH_EDITORONLY_DATA
	if (World->HasBegunPlay() == false && PIData->bOnlyActiveDuringGameplay)
	{
		PIData->OneShotQueue.Empty();
		PIData->PersistentAudioActionQueue.Empty();
		return false;
	}
#endif

	if (!PIData->OneShotQueue.IsEmpty() && !PIData->bValidOneShotSound)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("Niagara PostEventAtLocation: Not posting event at location, because it has infinite duration and there will be no way to stop it."));
		PIData->OneShotQueue.Empty();
	}
	if (!PIData->OneShotQueue.IsEmpty() && System)
	{
		//Drain the queue into an array here
		TArray<FWwiseEventParticleData> Data;
		FWwiseEventParticleData Value;
		while (PIData->OneShotQueue.Dequeue(Value))
		{
			Data.Add(Value);
			if (PIData->MaxPlaysPerTick > 0 && Data.Num() >= PIData->MaxPlaysPerTick)
			{
				// discard the rest of the queue if over the tick limit
				PIData->OneShotQueue.Empty();
				break;
			}
		}
		for (const FWwiseEventParticleData& ParticleData : Data)
		{
			EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio;
#if WITH_EDITOR
			if (GIsEditor && !FApp::IsGame())
			{
				AudioContext = EAkAudioContext::EditorAudio;
			}
#endif
			if (!World)
			{
				UE_LOG(LogAkAudio, Warning, TEXT("Niagara PostEventAtLocation: Cannot post event because world is invalid."));
				break;
			}
			if (!World->AllowAudioPlayback())
			{
				break;
			}
			PIData->EventToPost->PostAtLocation(ParticleData.Position, ParticleData.Rotation,
				World, nullptr, nullptr, nullptr, (AkCallbackType)0, nullptr, AudioContext);
		}
	}

	// process the persistent event updates
	FPersistentWwiseParticleData Value;
	TSet<int32> UpdatedAudioHandles;
	ensure(IsInGameThread());
	while (PIData->PersistentAudioActionQueue.Dequeue(Value))
	{
		// since we are in the game thread here, it is safe for the callback to access the AkComponent
		if (Value.UpdateCallback)
		{
			Value.UpdateCallback(PIData, SystemInstance);
		}
		UpdatedAudioHandles.Add(Value.AudioHandle);
	}

	// destroy all persistent event AkComponents that were not updated this frame
	// It also stops sounds if an emitter is culled by scalability.
	for (auto Iterator = PIData->PersistentComponents.CreateIterator(); Iterator; ++Iterator)
	{
		if (!UpdatedAudioHandles.Contains(Iterator.Key()))
		{
			SCOPE_CYCLE_COUNTER(STAT_WwiseNiagaraStopEvent);
			TWeakObjectPtr<UAkComponent> WeakComponent = Iterator.Value();
			UAkComponent* AudioComponent = WeakComponent.IsValid() ? WeakComponent.Get() : nullptr;
			if (AudioComponent)
			{
				AudioComponent->ConditionalBeginDestroy();
			}
			Iterator.RemoveCurrent();
		}
	}
	return false;
}

bool UNiagaraDataInterfaceWwiseEvent::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}

	const UNiagaraDataInterfaceWwiseEvent* OtherPlayer = CastChecked<UNiagaraDataInterfaceWwiseEvent>(Other);
	return OtherPlayer->EventToPost == EventToPost && OtherPlayer->bLimitPostsPerTick == bLimitPostsPerTick && OtherPlayer->MaxPostsPerTick == MaxPostsPerTick;
}

void UNiagaraDataInterfaceWwiseEvent::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
	SCOPED_WWISENIAGARA_EVENT_3(TEXT("NiagaraDataInterfaceWwiseEvent::GetFunctions"));
	FNiagaraFunctionSignature Sig;
	Sig.Name = PostEventAtLocationName;
#if WITH_EDITORONLY_DATA
	Sig.Description = NSLOCTEXT("Wwise Niagara", "PlayWwiseEventFunctionDescription", "This function plays a sound at the given location after the simulation has ticked.");
	Sig.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.bRequiresExecPin = true;
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wwise Event Interface")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Post Event")));
#if UE_5_0_OR_LATER
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetPositionDef(), TEXT("Position")));
#else
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
#endif
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Rotation")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("StartTime")));
	Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
	OutFunctions.Add(Sig);

	Sig = FNiagaraFunctionSignature();
	Sig.Name = PostPersistentWwiseEventName;
#if WITH_EDITORONLY_DATA
	Sig.Description = NSLOCTEXT("Wwise Niagara", "PostPersistentWwiseEventFunctionDescription", "This function plays a sound at the given location after the simulation has ticked. The returned handle can be used to control the sound in subsequent ticks.");
	Sig.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.bRequiresExecPin = true;
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wwise Event Interface")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Post Event")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Existing Audio Handle")));
#if UE_5_0_OR_LATER
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetPositionDef(), TEXT("Position")));
#else
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
#endif
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Rotation")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Start Time")));
	Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Audio Handle")));
	OutFunctions.Add(Sig);

	Sig = FNiagaraFunctionSignature();
	Sig.Name = SetPersistentWwiseEventPositionName;
#if WITH_EDITORONLY_DATA
	Sig.Description = NSLOCTEXT("Wwise Niagara", "SetPersistentWwisePositionFunctionDescription", "If an active Wwise Event can be found for the given handle then the this will adjust its world position.");
	Sig.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.bRequiresExecPin = true;
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wwise Event Interface")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Audio Handle")));
#if UE_5_0_OR_LATER
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetPositionDef(), TEXT("Position")));
#else
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
#endif
	OutFunctions.Add(Sig);

	Sig = FNiagaraFunctionSignature();
	Sig.Name = SetPersistentWwiseEventRotationName;
#if WITH_EDITORONLY_DATA
	Sig.Description = NSLOCTEXT("Wwise Niagara", "SetPersistentWwiseEventRotationFunctionDescription", "If an active Wwise Event can be found for the given handle then the this will adjust its world rotation.");
	Sig.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.bRequiresExecPin = true;
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wwise Event Interface")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Audio Handle")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Rotation")));

	OutFunctions.Add(Sig);

	Sig = FNiagaraFunctionSignature();
	Sig.Name = PausePersistentWwiseEventName;
#if WITH_EDITORONLY_DATA
	Sig.Description = NSLOCTEXT("Wwise Niagara", "SetPersistentWwiseAudioPausedDescription", "If an active audio effect can be found for the given handle then the this will either pause or unpause the effect.");
	Sig.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.bRequiresExecPin = true;
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wwise Event Interface")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Audio Handle")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Pause Audio")));
	OutFunctions.Add(Sig);


	Sig = FNiagaraFunctionSignature();
	Sig.Name = SetPersistenEventGameParameterName;
#if WITH_EDITORONLY_DATA
	Sig.Description = NSLOCTEXT("Wwise Niagara", "SetPersistentWwiseGameParameterDescription", "If an active Wwise Event can be found for the given handle then set a Game Parameter on it");
	Sig.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.bRequiresExecPin = true;
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wwise Event Interface")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Audio Handle")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Game Parameter Index")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Game Parameter Value")));

	OutFunctions.Add(Sig);


	Sig = FNiagaraFunctionSignature();
	Sig.Name = StopPersistentWwiseEventName;
#if WITH_EDITORONLY_DATA
	Sig.Description = NSLOCTEXT("Wwise Niagara", "StopWwisePesistentEventDescription", "If an active Wwise Event can be found for the given handle then stop it");
	Sig.FunctionVersion = FNiagaraPostEventDIFunctionVersion::LatestVersion;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.bRequiresExecPin = true;
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wwise Event Interface")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Audio Handle")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Stop Condition")));

	OutFunctions.Add(Sig);
}

DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, PostEventAtLocation);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, PostPersistentEvent);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, UpdatePosition);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, UpdateRotation);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, SetPausedState);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, SetGameParameter);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, StopPersistentEvent);

void UNiagaraDataInterfaceWwiseEvent::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc)
{
	SCOPED_WWISENIAGARA_EVENT_3(TEXT("NiagaraDataInterfaceWwiseEvent::GetVMExternalFunction"));
	if (BindingInfo.Name == PostEventAtLocationName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, PostEventAtLocation)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == PostPersistentWwiseEventName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, PostPersistentEvent)::Bind(this, OutFunc);
	}
	
	else if (BindingInfo.Name == SetPersistentWwiseEventPositionName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, UpdatePosition)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetPersistentWwiseEventRotationName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, UpdateRotation)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == PausePersistentWwiseEventName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, SetPausedState)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetPersistenEventGameParameterName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, SetGameParameter)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == StopPersistentWwiseEventName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceWwiseEvent, StopPersistentEvent)::Bind(this, OutFunc);
	}
	else
	{
		UE_LOG(LogAkAudio, Display, TEXT("Could not find data interface external function in %s. Received Name: %s"), *GetPathNameSafe(this), *BindingInfo.Name.ToString());
	}
}


void UNiagaraDataInterfaceWwiseEvent::UpdatePosition(FUnrealVectorVMContext& Context)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::UpdatePosition"));
	VectorVM::FUserPtrHandler<FWwiseEventInterface_InstanceData> InstData(Context);

	FNDIInputParam<int32> AudioHandleInParam(Context);
	FNDIInputParam<FUnrealFloatVector> PositionParam(Context);
	checkfSlow(InstData.Get(), TEXT("Wwise Event interface has invalid instance data. %s"), *GetPathName());

	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		int32 Handle = AudioHandleInParam.GetAndAdvance();
#if UE_5_0_OR_LATER
		FVector Position = InstData->LWCConverter.ConvertSimulationPositionToWorld(PositionParam.GetAndAdvance());
#else
		FVector Position = PositionParam.GetAndAdvance();
#endif
		if (Handle > 0)
		{

			FPersistentWwiseParticleData AudioData;
			AudioData.AudioHandle = Handle;
			AudioData.UpdateCallback = [Position, Handle](FWwiseEventInterface_InstanceData* InstanceData, FNiagaraSystemInstance*)
			{
				TWeakObjectPtr<UAkComponent> AkComponent = InstanceData->PersistentComponents.FindRef(Handle);
				if (AkComponent.IsValid())
				{
					AkComponent->SetWorldLocation(Position);
				}
			};
			InstData->PersistentAudioActionQueue.Enqueue(AudioData);
		}
	}
}

void UNiagaraDataInterfaceWwiseEvent::UpdateRotation(FUnrealVectorVMContext& Context)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::UpdateRotation"));
	VectorVM::FUserPtrHandler<FWwiseEventInterface_InstanceData> InstData(Context);
	FNDIInputParam<int32> AudioHandleInParam(Context);
	FNDIInputParam<FUnrealFloatVector> RotationParam(Context);
	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		int32 Handle = AudioHandleInParam.GetAndAdvance();

		FUnrealFloatVector InRot = RotationParam.GetAndAdvance();
		FRotator Rotation = FRotator(InRot.X, InRot.Y, InRot.Z);
		if (Handle > 0)
		{
			FPersistentWwiseParticleData AudioData;
			AudioData.AudioHandle = Handle;
			AudioData.UpdateCallback = [Rotation, Handle](FWwiseEventInterface_InstanceData* InstanceData, FNiagaraSystemInstance*)
			{
				TWeakObjectPtr<UAkComponent> AkComponent = InstanceData->PersistentComponents.FindRef(Handle);
				if (AkComponent.IsValid())
				{
					AkComponent->SetWorldRotation(Rotation);
				}
			};
			InstData->PersistentAudioActionQueue.Enqueue(AudioData);
		}
	}
}

void UNiagaraDataInterfaceWwiseEvent::SetGameParameter(FUnrealVectorVMContext& Context)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::SetGameParameter"));
	VectorVM::FUserPtrHandler<FWwiseEventInterface_InstanceData> InstData(Context);

	FNDIInputParam<int32> AudioHandleInParam(Context);
	FNDIInputParam<int32> GameParameterIndexParam(Context);
	FNDIInputParam<float> GameParameterValueParam(Context);
	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		int32 Handle = AudioHandleInParam.GetAndAdvance();
		int32 GameParameterIndex = GameParameterIndexParam.GetAndAdvance();
		float GameParameterValue = GameParameterValueParam.GetAndAdvance();

		if (Handle > 0)
		{
			FPersistentWwiseParticleData AudioData;
			AudioData.AudioHandle = Handle;
			AudioData.UpdateCallback = [Handle, GameParameterIndex, GameParameterValue](FWwiseEventInterface_InstanceData* InstanceData, FNiagaraSystemInstance*)
			{
				if (InstanceData->GameParameters.Num() > GameParameterIndex)
				{
					TWeakObjectPtr<UAkRtpc> GameParameter = InstanceData->GameParameters[GameParameterIndex];
					TWeakObjectPtr<UAkComponent> AkComponent = InstanceData->PersistentComponents.FindRef(Handle);
					if (AkComponent.IsValid() && GameParameter.IsValid())
					{
						AkComponent->SetRTPCValue(GameParameter.Get(), GameParameterValue, 0, {});
					}
				}
			};
			InstData->PersistentAudioActionQueue.Enqueue(AudioData);
		}
	}
}


void UNiagaraDataInterfaceWwiseEvent::SetPausedState(FUnrealVectorVMContext& Context)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::SetPausedState"));
	VectorVM::FUserPtrHandler<FWwiseEventInterface_InstanceData> InstData(Context);

	FNDIInputParam<int32> AudioHandleInParam(Context);
	FNDIInputParam<FNiagaraBool> PausedParam(Context);
	checkfSlow(InstData.Get(), TEXT("Wwise Event interface has invalid instance data. %s"), *GetPathName());

	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		int32 Handle = AudioHandleInParam.GetAndAdvance();
		bool IsPaused = PausedParam.GetAndAdvance();

		if (Handle > 0)
		{
			FPersistentWwiseParticleData AudioData;

			auto PlayingID = InstData->PlayingIDs.Find(Handle);
			if (!PlayingID)
			{
				return;
			}

			auto* SoundEngine = IWwiseSoundEngineAPI::Get();
			if (UNLIKELY(!SoundEngine))
			{
				return;
			}
			if (IsPaused)
			{
				SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Pause, *PlayingID);
			}
			else
			{
				SoundEngine->ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Resume, *PlayingID);
			}
		}
	}
}


void UNiagaraDataInterfaceWwiseEvent::StopPersistentEvent(FUnrealVectorVMContext& Context)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::StopPersistentEvent"));
	VectorVM::FUserPtrHandler<FWwiseEventInterface_InstanceData> InstData(Context);

	FNDIInputParam<int32> AudioHandleInParam(Context);
	FNDIInputParam<FNiagaraBool> StopParam(Context);

	checkfSlow(InstData.Get(), TEXT("Wwise Event interface has invalid instance data. %s"), *GetPathName());

	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		int32 Handle = AudioHandleInParam.GetAndAdvance();
		bool DoStop = StopParam.GetAndAdvance();

		if (Handle > 0 && DoStop)
		{
			FPersistentWwiseParticleData AudioData;

			auto AkComponent = InstData->PersistentComponents.FindRef(Handle);
			if (AkComponent.IsValid())
			{
				AkComponent->Stop();
			}
			InstData->PersistentComponents.Remove(Handle);
		}
	}
}


void UNiagaraDataInterfaceWwiseEvent::PostEventAtLocation(FUnrealVectorVMContext& Context)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::PostEventAtLocation"));
	VectorVM::FUserPtrHandler<FWwiseEventInterface_InstanceData> InstData(Context);

	VectorVM::FExternalFuncInputHandler<FNiagaraBool> PlayDataParam(Context);

	FNDIInputParam<FUnrealFloatVector> PositionParam(Context);
	FNDIInputParam<FUnrealFloatVector> RotationParam(Context);

	VectorVM::FExternalFuncInputHandler<float> StartTimeParam(Context);

	VectorVM::FExternalFuncRegisterHandler<FNiagaraBool> OutSample(Context);

	checkfSlow(InstData.Get(), TEXT("Wwise Event interface has invalid instance data. %s"), *GetPathName());
	bool ValidSoundData = InstData->EventToPost.IsValid();

	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		FNiagaraBool ShouldPlay = PlayDataParam.GetAndAdvance();
		FWwiseEventParticleData Data;

#if UE_5_0_OR_LATER
		Data.Position = InstData->LWCConverter.ConvertSimulationVectorToWorld(PositionParam.GetAndAdvance());
#else
		Data.Position = PositionParam.GetAndAdvance();
#endif
		const auto InRot = RotationParam.GetAndAdvance();
		Data.Rotation = FRotator(InRot.X, InRot.Y, InRot.Z);
		Data.StartTime = StartTimeParam.GetAndAdvance();

		FNiagaraBool Valid;
		if (ValidSoundData && ShouldPlay)
		{
			Valid.SetValue(InstData->OneShotQueue.Enqueue(Data));
		}
		*OutSample.GetDestAndAdvance() = Valid;
	}
}

void UNiagaraDataInterfaceWwiseEvent::PostPersistentEvent(FUnrealVectorVMContext& Context)
{
	SCOPED_WWISENIAGARA_EVENT(TEXT("NiagaraDataInterfaceWwiseEvent::PostPersistentEvent"));
	VectorVM::FUserPtrHandler<FWwiseEventInterface_InstanceData> InstData(Context);

	FNDIInputParam<FNiagaraBool> PlayAudioParam(Context);
	FNDIInputParam<int32> AudioHandleInParam(Context);
	FNDIInputParam<FUnrealFloatVector> PositionParam(Context);
	FNDIInputParam<FUnrealFloatVector> RotationParam(Context);
	FNDIInputParam<float> StartTimeParam(Context);

	FNDIOutputParam<int32> AudioHandleOutParam(Context);

	checkfSlow(InstData.Get(), TEXT("Wwise Event interface has invalid instance data. %s"), *GetPathName());

	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		bool ShouldPlay = PlayAudioParam.GetAndAdvance();
		int32 Handle = AudioHandleInParam.GetAndAdvance();
#if UE_5_0_OR_LATER
		FVector Position = InstData->LWCConverter.ConvertSimulationVectorToWorld(PositionParam.GetAndAdvance());
#else
		FVector Position = PositionParam.GetAndAdvance();
#endif
		FUnrealFloatVector InRot = RotationParam.GetAndAdvance();
		float StartTime = StartTimeParam.GetAndAdvance();

		FRotator Rotation = FRotator(InRot.X, InRot.Y, InRot.Z);

		FPersistentWwiseParticleData AudioData;
		if (ShouldPlay)
		{
			if (Handle <= 0)
			{
				// play a new sound
				Handle = InstData->HandleCount.Increment();
				SCOPE_CYCLE_COUNTER(STAT_WwiseNiagaraCreateEvent);
				AudioData.AudioHandle = Handle;
				AudioData.UpdateCallback = [Handle, Position, Rotation, StartTime](FWwiseEventInterface_InstanceData* InstanceData, FNiagaraSystemInstance* SystemInstance)
				{
					TWeakObjectPtr<UAkAudioEvent> Event = InstanceData->EventToPost;
					USceneComponent* NiagaraComponent = SystemInstance->GetAttachComponent();

					if (NiagaraComponent && Event.IsValid())
					{
						UWorld* World = NiagaraComponent->GetWorld();
						if (!World)
						{
							UE_LOG(LogAkAudio, Warning, TEXT("Niagara PostPersistentEvent: Cannot post event because world is invalid."));
							return;
						}
						if (!World->AllowAudioPlayback())
						{
							return;
						}
						auto* AudioDevice = FAkAudioDevice::Get();
						if (UNLIKELY(!AudioDevice))
						{
							UE_LOG(LogAkAudio, Warning, TEXT("Niagara PostPersistentEvent: Failed to post AkAudioEvent '%s' at a location without an Audio Device."), *Event->GetName());
							return;
						}

						UAkComponent* AkComponent = NiagaraWwiseParticleHelpers::SpawnAkComponentAtLocation(Event.Get(), NiagaraComponent, Position, Rotation, World, InstanceData->bStopWhenComponentIsDestroyed, false);

						if (AkComponent == nullptr)
						{
							// failed to create component (perhaps because audio is disabled)
							return;
						}

						EAkAudioContext AudioContext = EAkAudioContext::GameplayAudio;
#if WITH_EDITOR
						if (GIsEditor && !FApp::IsGame())
						{
							AudioContext = EAkAudioContext::EditorAudio;
						}
#endif
						uint32 PlayingId = Event->PostOnComponent(AkComponent, nullptr, nullptr, nullptr, (AkCallbackType)0, nullptr, true, AudioContext);
						if (PlayingId == AK_INVALID_PLAYING_ID )
						{
							AkComponent->ConditionalBeginDestroy();
							return;
						}


						InstanceData->PlayingIDs.Add(Handle,PlayingId);
						InstanceData->PersistentComponents.Add(Handle, AkComponent);
					}
				};
			}

			else
			{
				// add a dummy entry to keep the component alive
				AudioData.AudioHandle = Handle;
			}
			
			InstData->PersistentAudioActionQueue.Enqueue(AudioData);
			AudioHandleOutParam.SetAndAdvance(Handle);
			continue;
		}

		//Should not play
		if (Handle > 0)
		{
			// stop sound
			AudioData.AudioHandle = Handle;
			AudioData.UpdateCallback = [Handle](FWwiseEventInterface_InstanceData* InstanceData,  FNiagaraSystemInstance*)
			{
				SCOPE_CYCLE_COUNTER(STAT_WwiseNiagaraStopEvent);
				TWeakObjectPtr<UAkComponent> AkComponent = InstanceData->PersistentComponents.FindRef(Handle);
				if (AkComponent.IsValid())
				{
					AkComponent->Stop();
					InstanceData->PersistentComponents.Remove(Handle);
				}
			};
			InstData->PersistentAudioActionQueue.Enqueue(AudioData);
		}

		AudioHandleOutParam.SetAndAdvance(0);
	}
}

bool UNiagaraDataInterfaceWwiseEvent::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	SCOPED_WWISENIAGARA_EVENT_3(TEXT("NiagaraDataInterfaceWwiseEvent::CopyToInternal"));
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}

	UNiagaraDataInterfaceWwiseEvent* OtherTyped = CastChecked<UNiagaraDataInterfaceWwiseEvent>(Destination);
	OtherTyped->EventToPost = EventToPost;
	OtherTyped->GameParameters = GameParameters;
	OtherTyped->bLimitPostsPerTick = bLimitPostsPerTick;
	OtherTyped->MaxPostsPerTick = MaxPostsPerTick;
	OtherTyped->bStopWhenComponentIsDestroyed = bStopWhenComponentIsDestroyed;
#if WITH_EDITORONLY_DATA
	OtherTyped->bOnlyActiveDuringGameplay = bOnlyActiveDuringGameplay;
#endif
	return true;
}
