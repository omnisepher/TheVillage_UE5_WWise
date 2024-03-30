// Fill out your copyright notice in the Description page of Project Settings.


#include "FireBase.h"


// Sets default values
AFireBase::AFireBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void AFireBase::BeginPlay()
{
	Super::BeginPlay();

	static FName CBName("BlueFireCallback");
	BindCallback.BindUFunction(this, CBName);

	BlueFireFX->Deactivate();
	
}

//void AFireBase::BlueFireCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
//{
//	const UAkMarkerCallbackInfo* CBInfo = Cast<UAkMarkerCallbackInfo>(CallbackInfo);
//	if (CBInfo == nullptr) return;
//
//	if (CBInfo->Label == "BlueFireOn") {
//		BlueFireFX->Activate();
//	}
//	else if (CBInfo->Label == "BlueFireOff") {
//		BlueFireFX->Deactivate();
//	}
//}