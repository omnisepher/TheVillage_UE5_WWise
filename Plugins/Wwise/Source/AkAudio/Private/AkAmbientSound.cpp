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
	AkAmbientSound.cpp:
=============================================================================*/

#include "AkAmbientSound.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "Wwise/WwiseExternalSourceManager.h"

/*------------------------------------------------------------------------------------
	AAkAmbientSound
------------------------------------------------------------------------------------*/

AAkAmbientSound::AAkAmbientSound(const class FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer)
{
	// Property initialization
	StopWhenOwnerIsDestroyed = true;
	CurrentlyPlaying = false;
	
	static const FName ComponentName = TEXT("AkAudioComponent0");
	AkComponent = ObjectInitializer.CreateDefaultSubobject<UAkComponent>(this, ComponentName);
	
	AkComponent->StopWhenOwnerDestroyed = StopWhenOwnerIsDestroyed;

	RootComponent = AkComponent;

	AkComponent->AttenuationScalingFactor = 1.f;

	//bNoDelete = true;
	SetHidden(true);
	AutoPost = false;
}

void AAkAmbientSound::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	if( AkAudioEvent_DEPRECATED )
	{
		AkComponent->AkAudioEvent = AkAudioEvent_DEPRECATED;
	}
#endif
}

void AAkAmbientSound::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AkComponent->UpdateAkLateReverbComponentList(AkComponent->GetComponentLocation());
}

void AAkAmbientSound::BeginPlay()
{
	Super::BeginPlay();
	if (AutoPost)
	{
		StartAmbientSound();
	}
}


#if WITH_EDITOR
void AAkAmbientSound::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if( AkComponent )
	{
		// Reset audio component.
		if( IsCurrentlyPlaying() )
		{
			StartPlaying();
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AAkAmbientSound::StartAmbientSound()
{
	StartPlaying();
}

void AAkAmbientSound::StopAmbientSound()
{
	StopPlaying();
}

void AAkAmbientSound::StartPlaying()
{
	if( !IsCurrentlyPlaying() )
	{
		FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
		if (AkAudioDevice)
		{
			AkAudioDevice->SetAttenuationScalingFactor(this, AkComponent->AttenuationScalingFactor);
		}
		
		if (AkComponent->AkAudioEvent)
		{
			AkComponent->AkAudioEvent->PostOnActor(this, nullptr, nullptr, nullptr, (AkCallbackType)0, nullptr, StopWhenOwnerIsDestroyed);
		}
		else
		{
			UE_LOG(LogAkAudio, Warning, TEXT("AAkAmbientSound::StartPlaying: Failed to post invalid AkAudioEvent on actor '%s'."), *this->GetName());
		}
	}
}

void AAkAmbientSound::StopPlaying()
{
	if( IsCurrentlyPlaying() )
	{
		// State of CurrentlyPlaying gets updated in UAkComponent::Stop() through the EndOfEvent callback.
		AkComponent->Stop();
	}
}

bool AAkAmbientSound::IsCurrentlyPlaying()
{
	return AkComponent != nullptr && AkComponent->HasActiveEvents();
}
