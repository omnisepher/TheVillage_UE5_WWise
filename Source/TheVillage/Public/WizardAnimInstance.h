// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Plugins/Wwise/Source/AkAudio/Classes/AkGameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "WizardAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class THEVILLAGE_API UWizardAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void PlayFootsteps(AActor* StepActor);
	
};
