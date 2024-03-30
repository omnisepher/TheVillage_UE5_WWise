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
	ActorFactoryAkAmbientSound.cpp: 
=============================================================================*/
#include "Factories/ActorFactoryAkAmbientSound.h"

#include "AkAmbientSound.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "AssetRegistry/AssetData.h"
#include "Editor/EditorEngine.h"

#define LOCTEXT_NAMESPACE "ActorFactoryAkAmbientSound"

/*-----------------------------------------------------------------------------
	UActorFactoryAkAmbientSound
-----------------------------------------------------------------------------*/

UActorFactoryAkAmbientSound::UActorFactoryAkAmbientSound(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Property initialization
	DisplayName = LOCTEXT("AkAmbientSoundName", "AkAmbientSound");
	NewActorClass = AAkAmbientSound::StaticClass();
	bShowInEditorQuickMenu = true;
}

bool UActorFactoryAkAmbientSound::CanCreateActorFrom( const FAssetData& AssetData, FText& OutErrorMsg )
{
	//We allow creating AAmbientSounds without an existing sound asset
	if ( UActorFactory::CanCreateActorFrom( AssetData, OutErrorMsg ) )
	{
		return true;
	}

	if ( AssetData.IsValid() && !AssetData.GetClass()->IsChildOf( UAkAudioEvent::StaticClass() ) )
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoSoundAsset", "A valid sound asset must be specified.");
		return false;
	}

	return true;
}

void UActorFactoryAkAmbientSound::PostSpawnActor( UObject* Asset, AActor* NewActor )
{
	UAkAudioEvent* AmbientSound = Cast<UAkAudioEvent>( Asset );

	if ( AmbientSound != NULL )
	{
		AAkAmbientSound* NewSound = CastChecked<AAkAmbientSound>( NewActor );
		FActorLabelUtilities::SetActorLabelUnique(NewSound, AmbientSound->EventInfo.WwiseName.ToString());
		NewSound->AkComponent->AkAudioEvent = AmbientSound;
		if (AmbientSound->bAutoLoad)
		{
			AmbientSound->LoadData();
		}
	}
}

UObject* UActorFactoryAkAmbientSound::GetAssetFromActorInstance(AActor* Instance)
{
	check(Instance->IsA(NewActorClass));
	AAkAmbientSound* SoundActor = CastChecked<AAkAmbientSound>(Instance);

	check(SoundActor->AkComponent->AkAudioEvent);
	return SoundActor->AkComponent->AkAudioEvent;
}

void UActorFactoryAkAmbientSound::PostCreateBlueprint( UObject* Asset, AActor* CDO )
{
	UAkAudioEvent* AmbientSound = Cast<UAkAudioEvent>( Asset );

	if ( AmbientSound != NULL )
	{
		AAkAmbientSound* NewSound = CastChecked<AAkAmbientSound>( CDO );
		NewSound->AkComponent->AkAudioEvent = AmbientSound;
		AmbientSound->LoadData();
	}
}

#undef LOCTEXT_NAMESPACE
