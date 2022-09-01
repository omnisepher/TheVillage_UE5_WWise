// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardAnimInstance.h"

UWizardAnimInstance::UWizardAnimInstance()
{
	Speed = 0.0f;
}

void UWizardAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<AWizardChar>(GetOwningActor());
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

		UAkGameplayStatics::PostEvent(nullptr, GetOwningActor(), int32(0), nullCallback, nullSources,false,(FString)("Footsteps"));
	}

}
