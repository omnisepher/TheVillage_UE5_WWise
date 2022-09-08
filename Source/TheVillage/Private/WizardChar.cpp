// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardChar.h"

// Sets default values
AWizardChar::AWizardChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Name = "Darwin";

}

// Called when the game starts or when spawned
void AWizardChar::BeginPlay()
{
	Super::BeginPlay();
	IsAttacking = false;
	Health = 100.f;
	HealthMax = 100.f;
	HealthRegen = 2.f;

	Mana = 100.f;
	ManaMax = 100.f;
	ManaRegen = 1.5f;

	SkillIceManaCost = 10.f;
	SkillIceManaCostPerSecond = 4.f;

	ExtinguishedFlameCount = 0;
}

// Called every frame
void AWizardChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Health = (Health >= HealthMax) ? HealthMax : Health + (HealthRegen * DeltaTime);
	Mana = (Mana >= ManaMax) ? ManaMax : Mana + (ManaRegen * DeltaTime);

	if (IsAttacking) {
		if (Mana >= SkillIceManaCostPerSecond) {
			Mana = (Mana <= 0) ? 0 : Mana - (SkillIceManaCostPerSecond * DeltaTime);
		}
		else {
			StopSpellCasting();
		}
	}

}

// Called to bind functionality to input
void AWizardChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Attack",IE_Pressed,this,&AWizardChar::StartSpellCasting);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AWizardChar::StopSpellCasting);

}


void AWizardChar::StartSpellCasting()
{
	if (GetCharacterMovement()->IsFalling() != true && Mana > SkillIceManaCost) {
		IsAttacking = true;
		GetCharacterMovement()->DisableMovement();

		StartSpellEffect();

		FOnAkPostEventCallback nullCallback;
		TArray<FAkExternalSourceInfo> nullSources;

		IceSkillEventID = UAkGameplayStatics::PostEvent(nullptr, this, int32(0), nullCallback, nullSources, false, (FString)("IceSkill"));
		Mana = Mana - SkillIceManaCost;
	}
}

void AWizardChar::StopSpellCasting()
{
	IsAttacking = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	StopSpellEffect();

	UAkGameplayStatics::ExecuteActionOnPlayingID(AkActionOnEventType::Stop, IceSkillEventID, int32(200));
}

