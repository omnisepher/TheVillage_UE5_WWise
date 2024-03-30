// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Plugins/Wwise/Source/AkAudio/Classes/AkGameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Actor.h"
#include "FireBase.generated.h"

UCLASS()
class THEVILLAGE_API AFireBase : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AFireBase();

	FOnAkPostEventCallback BindCallback;

	/*UFUNCTION()
	void BlueFireCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);*/

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FX")
	UParticleSystemComponent* BlueFireFX;

};
