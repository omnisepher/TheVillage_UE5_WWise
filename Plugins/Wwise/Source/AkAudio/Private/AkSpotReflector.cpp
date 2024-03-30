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

#include "AkSpotReflector.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "AkAuxBus.h"
#include "AkRoomComponent.h"
#include "Engine/Texture2D.h"
#include "Components/BillboardComponent.h"

AAkSpotReflector::WorldToSpotReflectorsMap AAkSpotReflector::sWorldToSpotReflectors;

// Sets default values
AAkSpotReflector::AAkSpotReflector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AcousticTexture(NULL)
    , DistanceScalingFactor(2.f)
    , Level(1.f)
{
	static const FName ComponentName = TEXT("SpotReclectorRootComponent");
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, ComponentName);

#if WITH_EDITORONLY_DATA
	static const FName SpriteComponentName = TEXT("Sprite");
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(SpriteComponentName);
	if (SpriteComponent) 
	{
		SpriteComponent->SetSprite(LoadObject<UTexture2D>(NULL, TEXT("/Wwise/S_AkSpotReflector.S_AkSpotReflector")));
		SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->SetupAttachment(RootComponent);
	}
#endif

	// AActor properties 
	SetHidden(true);
	SetCanBeDamaged(false);
}

void AAkSpotReflector::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AddToWorld();
}

void AAkSpotReflector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	RemoveFromWorld();
}

void AAkSpotReflector::AddToWorld()
{
	UWorld* world = GetWorld();
	if (world)
	{
		SpotReflectorSet& SRSet = sWorldToSpotReflectors.FindOrAdd(world);
		SRSet.Add(this);
	}
}

void AAkSpotReflector::RemoveFromWorld()
{
	UWorld* world = GetWorld();
	if (world)
	{
		SpotReflectorSet* pSRSet = sWorldToSpotReflectors.Find(world);
		if (pSRSet)
		{
			pSRSet->Remove(this);

			if (pSRSet->Num() == 0)
			{
				sWorldToSpotReflectors.Remove(world);
			}
		}
	}
}

AkImageSourceID AAkSpotReflector::GetImageSourceID() const
{
	return (AkImageSourceID)(uint64)this;
}

AkAuxBusID AAkSpotReflector::GetAuxBusID() const
{
	if (EarlyReflectionAuxBus || !EarlyReflectionAuxBusName.IsEmpty())
	{
		return FAkAudioDevice::GetShortID(EarlyReflectionAuxBus, EarlyReflectionAuxBusName);
	}
	else
	{
		// No early reflection aux bus is set. The one assigned in the Wwise Authoring Tool will be used instead.
		// Skipping call to FAkAudioDevice::GetShortID() to avoid warning.
		return AK_INVALID_UNIQUE_ID;
	}
}

void AAkSpotReflector::SetImageSource(UAkComponent* AkComponent)
{
	FAkAudioDevice* pDev = FAkAudioDevice::Get();
	if (!pDev)
		return;

	const auto& RootTransform = RootComponent->GetComponentTransform();

	if (SameRoomOnly)
	{
		AkRoomID roomID;
		if (EnableRoomOverride)
		{
			if (RoomOverride != nullptr)
			{
				UAkRoomComponent* room = Cast<UAkRoomComponent>(RoomOverride->GetComponentByClass(UAkRoomComponent::StaticClass()));
				if (room != nullptr)
					roomID = room->GetRoomID();
			}
		}
		else
		{
			TArray<UAkRoomComponent*> AkRooms = pDev->FindRoomComponentsAtLocation(RootTransform.GetTranslation(), GetWorld());
			if (AkRooms.Num() > 0)
				roomID = AkRooms[0]->GetRoomID();
		}

		if (roomID != AkComponent->GetSpatialAudioRoomID())
			return;
	}

	AkImageSourceSettings sourceInfo = AkImageSourceSettings(
		FAkAudioDevice::FVectorToAKVector64(RootTransform.GetTranslation()),
		DistanceScalingFactor, Level);

	if (AcousticTexture)
	{
		sourceInfo.SetOneTexture(AcousticTexture->GetShortID());
	}

	pDev->SetImageSource(this, sourceInfo, GetAuxBusID(), AkComponent);
}

void AAkSpotReflector::UpdateSpotReflectors(UAkComponent* AkComponent)
{
	FAkAudioDevice* pDev = FAkAudioDevice::Get();
	if (pDev)
	{
		pDev->ClearImageSources(AK_INVALID_AUX_ID, AkComponent);

		if (AkComponent->EnableSpotReflectors)
		{
			UWorld* world = AkComponent->GetWorld();
			if (world)
			{
				SpotReflectorSet* pSRSet = sWorldToSpotReflectors.Find(world);
				if (pSRSet)
				{
					for (auto It = pSRSet->CreateIterator(); It; ++It)
					{
						(*It)->SetImageSource(AkComponent);
					}
				}
			}
		}
	}
}

const FString AAkSpotReflector::GetSpotReflectorName() const
{
#if WITH_EDITOR
	return GetActorLabel();
#else
	return GetName();
#endif
}
