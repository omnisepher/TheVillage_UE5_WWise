// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Plugins/Wwise/Source/AkAudio/Classes/AkGameplayStatics.h"
#include "Kismet/GameplayStatics.h"
#include "WizardChar.h"
#include "GameFramework/Actor.h"
#include "Jukebox.generated.h"

UCLASS()
class THEVILLAGE_API AJukebox : public AActor
{
	GENERATED_BODY()

	AWizardChar* MainCharacter;
	
public:	
	// Sets default values for this actor's properties
	AJukebox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
		UAkAudioEvent* m_HeartBeat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
		UAkAudioEvent* m_GameTheme;

};
