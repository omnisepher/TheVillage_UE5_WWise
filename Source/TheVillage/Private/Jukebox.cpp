// Fill out your copyright notice in the Description page of Project Settings.


#include "Jukebox.h"

// Sets default values
AJukebox::AJukebox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJukebox::BeginPlay()
{
	Super::BeginPlay();
	FOnAkPostEventCallback nullCallback;
	TArray<FAkExternalSourceInfo> nullSources;

	UAkGameplayStatics::PostEvent(m_HeartBeat, this, int32(0), nullCallback, false);

	MainCharacter = Cast<AWizardChar>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));

	if (MainCharacter != nullptr) {
		UAkGameplayStatics::SetOcclusionRefreshInterval(0.f, this);

		UAkGameplayStatics::PostEvent(m_GameTheme, this, int32(0), nullCallback, false);
	}
	
}

// Called every frame
void AJukebox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MainCharacter != nullptr) {
		if (MainCharacter->ExtinguishedFlameCount >=2) {
			UAkGameplayStatics::SetState(nullptr, "GameTheme", "Success");
			UAkGameplayStatics::PostTrigger(nullptr, this, "Stinger"); // every 5 seconds after success, it will send
		}
		else {
			UAkGameplayStatics::SetState(nullptr, "GameTheme", "Normal");
		}
	}

}

