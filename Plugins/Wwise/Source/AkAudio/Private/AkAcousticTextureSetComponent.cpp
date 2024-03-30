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

#include "AkAcousticTextureSetComponent.h"
#include "AkAudioDevice.h"
#include "AkRoomComponent.h"
#include "AkReverbDescriptor.h"
#include "AkComponentHelpers.h"
#include "AkLateReverbComponent.h"
#include "AkSpatialAudioHelper.h"

UAkAcousticTextureSetComponent::UAkAcousticTextureSetComponent(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
#if WITH_EDITOR
	if (AkSpatialAudioHelper::GetObjectReplacedEvent())
	{
		AkSpatialAudioHelper::GetObjectReplacedEvent()->AddUObject(this, &UAkAcousticTextureSetComponent::HandleObjectsReplaced);
	}
#endif
}

void UAkAcousticTextureSetComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	RegisterAllTextureParamCallbacks();
	RegisterReverbRTPCChangedCallback();
#endif
	// In the case where a blueprint class has a texture set component and a late reverb component as siblings, We can't know which will be registered first.
	// We need to check for the sibling in each OnRegister function and associate the texture set component to the late reverb when they are both registered.
	if (USceneComponent* parent = GetAttachParent())
	{
		if (UAkLateReverbComponent* reverbComp = AkComponentHelpers::GetChildComponentOfType<UAkLateReverbComponent>(*parent))
		{
			reverbComp->AssociateAkTextureSetComponent(this);
		}
	}
	DampingEstimationNeedsUpdate = true;
}

void UAkAcousticTextureSetComponent::OnUnregister()
{
#if WITH_EDITOR
	UnregisterTextureParamChangeCallbacks();
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (AkSettings != nullptr)
	{
		if (RTPCChangedHandle.IsValid())
			AkSettings->OnReverbRTPCChanged.Remove(RTPCChangedHandle);
	}
#endif
	Super::OnUnregister();
}

void UAkAcousticTextureSetComponent::BeginPlay()
{
	Super::BeginPlay();
	DampingEstimationNeedsUpdate = true;
}

void UAkAcousticTextureSetComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SecondsSinceDampingUpdate < PARAM_ESTIMATION_UPDATE_PERIOD)
	{
		SecondsSinceDampingUpdate += DeltaTime;
	}
	if (DampingEstimationNeedsUpdate && SecondsSinceDampingUpdate >= PARAM_ESTIMATION_UPDATE_PERIOD)
	{
		RecalculateHFDamping();

		if (USceneComponent* parent = GetAttachParent())
		{
			if (UAkLateReverbComponent* ReverbComp = AkComponentHelpers::GetChildComponentOfType<UAkLateReverbComponent>(*parent))
			{
				ReverbComp->TextureSetUpdated(); // We notify the late reverb component so it can recompute the Decay value.
			}
		}

		DampingEstimationNeedsUpdate = false;
	}
}

void UAkAcousticTextureSetComponent::SetReverbDescriptor(FAkReverbDescriptor* reverbDescriptor)
{
	ReverbDescriptor = reverbDescriptor;
#if WITH_EDITOR
	UnregisterTextureParamChangeCallbacks();
	if (reverbDescriptor != nullptr)
		RegisterAllTextureParamCallbacks();
#endif
	if (reverbDescriptor != nullptr)
		DampingEstimationNeedsUpdate = true;
}

void UAkAcousticTextureSetComponent::RecalculateHFDamping()
{
	if (ReverbDescriptor != nullptr && ReverbDescriptor->ShouldEstimateDamping())
	{
		ReverbDescriptor->CalculateHFDamping(this);
		SecondsSinceDampingUpdate = 0.0f;
	}
}

#if WITH_EDITOR
void UAkAcousticTextureSetComponent::BeginDestroy()
{
	Super::BeginDestroy();
	if (AkSpatialAudioHelper::GetObjectReplacedEvent())
	{
		AkSpatialAudioHelper::GetObjectReplacedEvent()->RemoveAll(this);
	}
}

void UAkAcousticTextureSetComponent::HandleObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	auto ValuePtr = ReplacementMap.Find(this);
	if (ValuePtr && *ValuePtr)
	{
		UAkAcousticTextureSetComponent* NewTextureSetComponent = Cast<UAkAcousticTextureSetComponent>(*ValuePtr);
		if (USceneComponent* Parent = NewTextureSetComponent->GetAttachParent())
		{
			if (UAkLateReverbComponent* ReverbComp = AkComponentHelpers::GetChildComponentOfType<UAkLateReverbComponent>(*Parent))
			{
				ReverbComp->AssociateAkTextureSetComponent(NewTextureSetComponent);
			}
		}
	}
}

void UAkAcousticTextureSetComponent::RegisterReverbRTPCChangedCallback()
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (AkSettings != nullptr)
	{
		if (RTPCChangedHandle.IsValid())
			AkSettings->OnReverbRTPCChanged.Remove(RTPCChangedHandle);
		RTPCChangedHandle = AkSettings->OnReverbRTPCChanged.AddLambda([this]()
		{
			DampingEstimationNeedsUpdate = true;
		});
	}
}

void UAkAcousticTextureSetComponent::RegisterTextureParamChangeCallback(FGuid textureID)
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (AkSettings != nullptr)
	{
		if (TextureDelegateHandles.Find(textureID) != nullptr)
		{
			if (TextureDelegateHandles[textureID].IsValid())
			{
				AkSettings->OnTextureParamsChanged.Remove(TextureDelegateHandles[textureID]);
			}
			TextureDelegateHandles.Remove(textureID);
		}
		TextureDelegateHandles.Add(textureID, AkSettings->OnTextureParamsChanged.AddLambda([&](const FGuid& textureID)
		{
			if (ContainsTexture(textureID) && ReverbDescriptor != nullptr)
				DampingEstimationNeedsUpdate = ReverbDescriptor->ShouldEstimateDamping();
		}));
	}
}

void UAkAcousticTextureSetComponent::UnregisterTextureParamChangeCallbacks()
{
	UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
	if (AkSettings != nullptr)
	{
		for (auto it = TextureDelegateHandles.CreateIterator(); it; ++it)
		{
			if (it->Value.IsValid())
				AkSettings->OnTextureParamsChanged.Remove(it->Value);
		}
		TextureDelegateHandles.Empty();
	}
}
#endif

bool UAkAcousticTextureSetComponent::ShouldSendGeometry() const
{
	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && !IsRunningCommandlet())
	{
		return CurrentWorld->WorldType == EWorldType::Game || CurrentWorld->WorldType == EWorldType::PIE;
	}
	return false;
}

void UAkAcousticTextureSetComponent::SendGeometryToWwise(const AkGeometryParams& params)
{
	if (ShouldSendGeometry())
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice != nullptr && AkAudioDevice->SetGeometry(GetGeometrySetID(), params) == AK_Success)
			GeometryHasBeenSent = true;
	}
}

void UAkAcousticTextureSetComponent::SendGeometryInstanceToWwise(const FRotator& rotation, const FVector& location, const FVector& scale, const AkRoomID roomID, bool useForReflectionAndDiffraction)
{
	if (ShouldSendGeometry() && GeometryHasBeenSent)
	{
		AkVector front, up;
		AkVector64 position;
		FAkAudioDevice::FVectorToAKVector(rotation.RotateVector(FVector::ForwardVector), front);
		FAkAudioDevice::FVectorToAKVector(rotation.RotateVector(FVector::UpVector), up); 
		FAkAudioDevice::FVectorToAKVector64(location, position);

		AkGeometryInstanceParams params;
		params.PositionAndOrientation.Set(position, front, up);
		FAkAudioDevice::FVectorToAKVector(scale, params.Scale);
		params.GeometrySetID = GetGeometrySetID();
		params.RoomID = roomID;
#if WWISE_2023_1_OR_LATER
		params.UseForReflectionAndDiffraction = useForReflectionAndDiffraction;
#endif

		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice != nullptr && AkAudioDevice->SetGeometryInstance(GetGeometrySetID(), params) == AK_Success)
			GeometryInstanceHasBeenSent = true;
	}
}

void UAkAcousticTextureSetComponent::RemoveGeometryFromWwise()
{
	if (GeometryHasBeenSent)
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice != nullptr && AkAudioDevice->RemoveGeometrySet(GetGeometrySetID()) == AK_Success)
		{
			GeometryHasBeenSent = false;
			GeometryInstanceHasBeenSent = false;
		}
	}
}

void UAkAcousticTextureSetComponent::RemoveGeometryInstanceFromWwise()
{
	if (GeometryInstanceHasBeenSent)
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice != nullptr && AkAudioDevice->RemoveGeometrySet(GetGeometrySetID()) == AK_Success)
			GeometryInstanceHasBeenSent = false;
	}
}
