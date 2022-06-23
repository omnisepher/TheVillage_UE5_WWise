// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Plugins/Wwise/Source/AkAudio/Classes/AkGameplayStatics.h"
#include "Components/ActorComponent.h"
#include "WWiseEvent.generated.h"


UCLASS(Blueprintable, ClassGroup=WWiseCustom, meta=(BlueprintSpawnableComponent) )
class THEVILLAGE_API UWWiseEvent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWWiseEvent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
		class UAkAudioEvent* TestEvent;

		
};
