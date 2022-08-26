// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardAnimInstance.h"

void UWizardAnimInstance::PlayFootsteps(AActor* StepActor)
{
	if (StepActor != nullptr) {
		if (StepActor->ActorHasTag("Rock")) {
			UAkGameplayStatics::SetSwitch(nullptr, GetOwningActor(), "Footsteps", "Rock");
		}
		else {
			UAkGameplayStatics::SetSwitch(nullptr, GetOwningActor(), "Footsteps", "Grass");
		}

		FOnAkPostEventCallback nullCallback;
		TArray<FAkExternalSourceInfo> nullSources;

		UAkGameplayStatics::PostEvent(nullptr, GetOwningActor(), int32(0), nullCallback, nullSources,false,(FString)("Footsteps"));

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Test steps from C++!"));
	}

}
