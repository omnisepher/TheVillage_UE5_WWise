// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WizardChar.generated.h"

UCLASS()
class THEVILLAGE_API AWizardChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWizardChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		float Health;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		float HealthMax;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		float HealthRegen;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		float Mana;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		float ManaMax;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		float ManaRegen;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Generic")
		bool IsAttacking;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill")
		float SkillIceManaCost;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill")
		float SkillIceManaCostPerSecond;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
		int ExtinguishedFlameCount;

};
