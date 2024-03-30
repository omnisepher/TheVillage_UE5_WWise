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

#include "NiagaraCommon.h"
#include "NiagaraShared.h"
#include "NiagaraDataInterface.h"
#include "AkAudioEvent.h"
#include "AkRtpc.h"
#include "WwiseUEFeatures.h"
#include "WwiseUnrealHelper.h"

#include "UObject/WeakObjectPtr.h"
#include "NiagaraDataInterfaceWwiseEvent.generated.h"

#if UE_5_0_OR_LATER
using FUnrealVectorVMContext = FVectorVMExternalFunctionContext;
#else
using FUnrealVectorVMContext = FVectorVMContext;
#endif

class FAkComponentCallbackManager;
class USoundConcurrency;

struct FWwiseEventParticleData
{
	FVector Position;
	FRotator Rotation;
	float StartTime = 1;
};

struct FPersistentWwiseParticleData
{
	int32 AudioHandle = 0;

	/** The update callback is executed in PerInstanceTickPostSimulate, which runs on the game thread */
	TFunction<void(struct FWwiseEventInterface_InstanceData*,FNiagaraSystemInstance*)> UpdateCallback;
};

struct FWwiseEventInterface_InstanceData
{
	/** We use a lock-free queue here because multiple threads might try to push data to it at the same time. */
	TQueue<FWwiseEventParticleData, EQueueMode::Mpsc> OneShotQueue;
	TQueue<FPersistentWwiseParticleData, EQueueMode::Mpsc> PersistentAudioActionQueue;
	FThreadSafeCounter HandleCount;

	TSortedMap<int32, TWeakObjectPtr<UAkComponent>> PersistentComponents;
	TSortedMap<int32, int32 > PlayingIDs;

	TWeakObjectPtr<UAkAudioEvent> EventToPost;
	TArray<TWeakObjectPtr<UAkRtpc>> GameParameters;

#if UE_5_0_OR_LATER
	FNiagaraLWCConverter LWCConverter;
#endif
	int32 MaxPlaysPerTick = 0;
	bool bStopWhenComponentIsDestroyed = true;
	bool bStopWhenNotUpdated = true;

	bool bValidOneShotSound = true;

#if WITH_EDITORONLY_DATA
	bool bOnlyActiveDuringGameplay = false;
#endif
};

/** This Data Interface can be used to post Wwise events driven by particle data. */
UCLASS(EditInlineNew, Category = "WwiseAudio", meta = (DisplayName = "Niagara Wwise Event"))
class WWISENIAGARA_API UNiagaraDataInterfaceWwiseEvent : public UNiagaraDataInterface
{
	GENERATED_BODY()

public:
	UNiagaraDataInterfaceWwiseEvent(FObjectInitializer const& ObjectInitializer);

	/** The AkAudioEvent asset to post. */
	UPROPERTY(EditAnywhere, Category = "Audio")
	UAkAudioEvent* EventToPost;

	/** A set of Game Parameters updated (via their index) in the Set Wwise Persistent Event Game Parameter module */
	UPROPERTY(EditAnywhere, Category = "Parameters")
	TArray<UAkRtpc*> GameParameters;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Audio", meta = (InlineEditConditionToggle))
	bool bLimitPostsPerTick;

	/** This sets the max number of events posted on each tick.
	 *  If more particles try to play a sound in a given tick, then it will play sounds until the limit is reached and discard the rest. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Audio", meta=(EditCondition="bLimitPostsPerTick", ClampMin="0", UIMin="0"))
	int32 MaxPostsPerTick;

	/** If false then the event keeps playing after the Niagara component was destroyed (particle death, or system is stopped/destroyed).
	Looping sounds are always stopped when the component is destroyed. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Audio")
	bool bStopWhenComponentIsDestroyed = true;

#if WITH_EDITORONLY_DATA
	/** If true, this data interface only processes sounds during active gameplay, and not while using Realtime Rendering in the open viewport.
	 * This is useful when you are working in the preview window and the sounds annoy you. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Audio")
	bool bOnlyActiveDuringGameplay = false;

	virtual bool UpgradeFunctionCall(FNiagaraFunctionSignature& FunctionSignature) override;
#endif
	
	//UObject Interface
	virtual void PostInitProperties() override;
	//UObject Interface End

	//UNiagaraDataInterface Interface
	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc) override;
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual int32 PerInstanceDataSize() const override { return sizeof(FWwiseEventInterface_InstanceData); }
	virtual bool PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return Target == ENiagaraSimTarget::CPUSim; }

	virtual bool HasPreSimulateTick() const override { return true; }
	virtual bool HasPostSimulateTick() const override { return true; }
	virtual bool PostSimulateCanOverlapFrames() const { return false; }

	//UNiagaraDataInterface Functions
	virtual void PostEventAtLocation(FUnrealVectorVMContext& Context);
	virtual void PostPersistentEvent(FUnrealVectorVMContext& Context);
	
	virtual void UpdatePosition(FUnrealVectorVMContext& Context);
	virtual void UpdateRotation(FUnrealVectorVMContext& Context);
	virtual void SetGameParameter(FUnrealVectorVMContext& Context);

	virtual void SetPausedState(FUnrealVectorVMContext& Context);
	virtual void StopPersistentEvent(FUnrealVectorVMContext& Context);


protected:
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
	
private:
	static const FName PostEventAtLocationName;
	static const FName PostPersistentWwiseEventName;
	static const FName SetPersistentWwiseEventPositionName;
	static const FName SetPersistentWwiseEventRotationName;
	static const FName SetPersistenEventGameParameterName;
	static const FName PausePersistentWwiseEventName; 
	static const FName StopPersistentWwiseEventName;
};
