// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardAnimInstance.h"
#include "AkAudio/Classes/AkAudioEvent.h"
#include "AkAudio/Classes/AkGameObject.h"

UWizardAnimInstance::UWizardAnimInstance()
{
	Speed = 0.0f;
}

void UWizardAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<AWizardChar>(GetOwningActor());
	IsAlive = true;
}

void UWizardAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);


	if (OwningCharacter != nullptr) {
		Speed = OwningCharacter->GetVelocity().Size();
		Direction = CalculateDirection(OwningCharacter->GetVelocity(),OwningCharacter->GetActorRotation());
		IsFalling = OwningCharacter->GetMovementComponent()->IsFalling();
		IsAttacking = OwningCharacter->IsAttacking;
		IsAlive = OwningCharacter->Health > 0 ? true : false;
	}

}

void UWizardAnimInstance::PlayFootsteps(AActor* StepActor)
{
	if (StepActor != nullptr) {
		if (StepActor->ActorHasTag("Rock")) {
			UAkGameplayStatics::SetSwitch(nullptr, GetOwningActor(), "Footsteps", "Rock");
		}
		else if (StepActor->ActorHasTag("Terrain")) {
			UAkGameplayStatics::SetSwitch(nullptr, GetOwningActor(), "Footsteps", "Grass");
		}

		FOnAkPostEventCallback nullCallback;
		TArray<FAkExternalSourceInfo> nullSources;

		if (auto* SoundEngine = IWwiseSoundEngineAPI::Get())
		{
			//UAkGameplayStatics::PostEvent(m_FootstepsEvent, GetOwningActor(), int32(0), nullCallback, false);
			UAkGameObject* player_ak_go = Cast<AWizardChar>(GetOwningActor())->GetAkGameObject();
			SoundEngine->PostEvent(m_FootstepsEvent->GetShortID(), player_ak_go->GetAkGameObjectID(), int32(0));
		}
		
	}

}
