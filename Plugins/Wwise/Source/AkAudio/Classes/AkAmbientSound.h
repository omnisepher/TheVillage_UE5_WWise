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
	AkAmbientSound.h:
=============================================================================*/
#pragma once

#include "GameFramework/Actor.h"
#include "AkAmbientSound.generated.h"

/*------------------------------------------------------------------------------------
	AAkAmbientSound
------------------------------------------------------------------------------------*/
UCLASS(config=Engine, hidecategories=Audio, AutoExpandCategories=AkAmbientSound, BlueprintType)
class AKAUDIO_API AAkAmbientSound : public AActor
{
	GENERATED_BODY()

public:
	AAkAmbientSound(const class FObjectInitializer& ObjectInitializer);

	/** AkAudioEvent to play. Deprecated as UE4.7 integration: Use AkComponent->AkAudioEvent instead */
	UPROPERTY()
	class UAkAudioEvent * AkAudioEvent_DEPRECATED = nullptr;

	/** AkComponent to handle playback */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=AkAmbientSound, meta=(ShowOnlyInnerProperties) )
	class UAkComponent* AkComponent = nullptr;
	
	/** Stop playback if the owner is destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkAmbientSound, SimpleDisplay)
	bool StopWhenOwnerIsDestroyed = false;

	/** Automatically post the associated AkAudioEvent on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AkAmbientSound, SimpleDisplay)
	bool AutoPost = false;

	/*
	 * Start an Ak ambient sound.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkAmbientSound")
	void StartAmbientSound();

	/*
	 * Stop an Ak ambient sound.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Audiokinetic|AkAmbientSound")
	void StopAmbientSound();


#if CPP
public:

	/**
	 * Start the ambience playback
	 */
	void StartPlaying();

	/**
	 * Stop the ambience playback
	 */
	void StopPlaying();

	/**
	 * Is whether this ambient sound currently playing
	 *
	 * @return		True if ambient sound is currently playing, false if not.
	 */
	bool IsCurrentlyPlaying();


protected:
	/*------------------------------------------------------------------------------------
		AActor interface.
	------------------------------------------------------------------------------------*/

	virtual void BeginPlay() override;

#if WITH_EDITOR
	/**
	 * Check for errors
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitializeComponents() override;
	virtual void PostLoad() override;

#endif

private:
	/** used to update status of toggleable level placed ambient sounds on clients */
	bool CurrentlyPlaying;

	FCriticalSection PlayingCriticalSection;
};
