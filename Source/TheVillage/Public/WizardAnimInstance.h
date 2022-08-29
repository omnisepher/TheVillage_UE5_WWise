// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Plugins/Wwise/Source/AkAudio/Classes/AkGameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WizardChar.h"
#include "Animation/AnimInstance.h"
#include "WizardAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class THEVILLAGE_API UWizardAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	AWizardChar* OwningCharacter;

public:
	UWizardAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void PlayFootsteps(AActor* StepActor);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generic")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generic")
	float Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generic")
		bool IsFalling;

	
};
