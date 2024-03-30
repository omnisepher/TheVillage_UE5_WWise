/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

/*=============================================================================
	AkReverbVolume.cpp:
=============================================================================*/

#include "AkReverbVolume.h"
#include "AkAudioDevice.h"
#include "AkLateReverbComponent.h"
#include "Components/BrushComponent.h"
#include "Model.h"
#include "AkCustomVersion.h"

/*------------------------------------------------------------------------------------
	AAkReverbVolume
------------------------------------------------------------------------------------*/

AAkReverbVolume::AAkReverbVolume(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// Property initialization
	UBrushComponent* BrushComp = GetBrushComponent();
	if (BrushComp)
	{
		BrushComp->SetGenerateOverlapEvents(false);
		BrushComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BrushComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		BrushComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	bColored = true;
	BrushColor = FColor(0, 255, 255, 255);
	
	bEnabled_DEPRECATED = true;
	SendLevel_DEPRECATED = 1.0f;
	FadeRate_DEPRECATED = 0.5f;
	Priority_DEPRECATED = 1.0f;

	static const FName LateReverbName = TEXT("LateReverb");
	LateReverbComponent = ObjectInitializer.CreateDefaultSubobject<UAkLateReverbComponent>(this, LateReverbName);
	LateReverbComponent->SetupAttachment(BrushComp);
}

void AAkReverbVolume::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FAkCustomVersion::GUID);
}

void AAkReverbVolume::PostLoad()
{
	Super::PostLoad();
	const int32 AkVersion = GetLinkerCustomVersion(FAkCustomVersion::GUID);

	if (LateReverbComponent && AkVersion < FAkCustomVersion::AddedSpatialAudio)
	{
		LateReverbComponent->bEnable			= bEnabled_DEPRECATED;
		LateReverbComponent->AuxBus				= AuxBus_DEPRECATED;
		LateReverbComponent->AuxBusName			= AuxBusName_DEPRECATED;
		LateReverbComponent->SendLevel			= SendLevel_DEPRECATED;
		LateReverbComponent->FadeRate			= FadeRate_DEPRECATED;
		LateReverbComponent->Priority			= Priority_DEPRECATED;
	}
}
