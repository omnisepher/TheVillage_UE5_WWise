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

#include "Wwise/AudioLink/WwiseAudioLinkComponent.h"
#include "Wwise/AudioLink/WwiseAudioLinkFactory.h"
#include "Components/AudioComponent.h"

void UWwiseAudioLinkComponent::CreateLink()
{
	if (!Settings)
	{
		Settings = GetMutableDefault<UWwiseAudioLinkSettings>();
	}
	
	IAudioLinkFactory* Factory = IAudioLinkFactory::FindFactory(FWwiseAudioLinkFactory::GetFactoryNameStatic());
	if (ensure(Factory))
	{
		IAudioLinkFactory::FAudioLinkSourceCreateArgs CreateArgs;
		CreateArgs.OwningComponent = this;
		CreateArgs.AudioComponent = AudioComponent;
		CreateArgs.Settings = Settings;
		LinkInstance = Factory->CreateSourceAudioLink(CreateArgs);	
	}
}

void UWwiseAudioLinkComponent::CreateAudioComponent()
{
	if (!AudioComponent)
	{
		// Create the audio component which will be used to play the procedural sound wave
		AudioComponent = NewObject<UAudioComponent>(this);

		if (!AudioComponent->GetAttachParent() && !AudioComponent->IsAttachedTo(this))
		{
			AActor* Owner = GetOwner();

			// If the media component has no owner or the owner doesn't have a world
			if (!Owner || !Owner->GetWorld())
			{
				// Attempt to retrieve the component's world and register the audio component with it
				// This ensures that the component plays on the correct world in cases where there isn't an owner
				if (UWorld* World = GetWorld())
				{
					AudioComponent->RegisterComponentWithWorld(World);
					AudioComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
				}
				else
				{
					AudioComponent->SetupAttachment(this);
				}
			}
			else
			{
				AudioComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
				AudioComponent->RegisterComponent();
			}
		}
	}

	if (AudioComponent)
	{
		AudioComponent->bAutoActivate = false;
		AudioComponent->bStopWhenOwnerDestroyed = true;
		AudioComponent->bShouldRemainActiveIfDropped = true;
		AudioComponent->Mobility = EComponentMobility::Movable;
		AudioComponent->bOverrideAttenuation = 1;
		AudioComponent->AttenuationOverrides.bEnableSendToAudioLink = 1;
		AudioComponent->AttenuationOverrides.AudioLinkSettingsOverride = Settings;

#if WITH_EDITORONLY_DATA
		AudioComponent->bVisualizeComponent = false;
#endif
	}
}

void UWwiseAudioLinkComponent::OnRegister()
{
	Super::OnRegister();

	CreateAudioComponent();

	if (ensure(AudioComponent))
	{
		check(LinkInstance == nullptr);
		CreateLink();		
	}
	if(bAutoPlay)
	{
		PlayLink(0);
	}
}

void UWwiseAudioLinkComponent::OnUnregister()
{
	LinkInstance.Reset();
	AudioComponent = nullptr;
	
	Super::OnUnregister();
}

void UWwiseAudioLinkComponent::SetLinkSound(USoundBase* InSound)
{
	Sound = InSound;

	if (AudioComponent)
	{
		AudioComponent->SetSound(InSound);
	}
}

void UWwiseAudioLinkComponent::PlayLink(float StartTime)
{	
	if (AudioComponent)
	{
		// Set the audio component's sound to be our procedural sound wave
		AudioComponent->SetSound(Sound);
		AudioComponent->Play(StartTime);

		SetActiveFlag(AudioComponent->IsActive());
	}
}

void UWwiseAudioLinkComponent::StopLink()
{
	if (IsActive())
	{
		if (AudioComponent)
		{
			AudioComponent->Stop();
		}

		SetActiveFlag(false);
	}
}

bool UWwiseAudioLinkComponent::IsLinkPlaying() const
{
	return AudioComponent && AudioComponent->IsPlaying();
}