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

#pragma once

#include "Wwise/AudioLink/WwiseAudioLinkSettings.h"
#include "AkComponent.h"
#include "IAudioLink.h"
#include "IAudioLinkBlueprintInterface.h"

class UAudioComponent;

#include "WwiseAudioLinkComponent.generated.h"

UCLASS(ClassGroup = (Audio, Common), HideCategories = (Object, ActorComponent, Physics, Rendering, Mobility, LOD), ShowCategories = Trigger, meta = (BlueprintSpawnableComponent))
class UWwiseAudioLinkComponent : public UAkComponent, public IAudioLinkBlueprintInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioLink")
	TObjectPtr<UWwiseAudioLinkSettings> Settings;

	/** The sound to be played */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	TObjectPtr<class USoundBase> Sound;

	/** Whether or not to play the Link on start*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioLink")
	bool bAutoPlay = false;
	
protected:	
	//~ Begin IAudioLinkInterface
	virtual void SetLinkSound(USoundBase* NewSound) override;
	virtual void PlayLink(float StartTime = 0.0f) override;
	virtual void StopLink() override;
	virtual bool IsLinkPlaying() const override;
	//~ End IAudioLinkInterface

	//~ Begin ActorComponent Interface.
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//~ End ActorComponent Interface.

	/** Stop sound when owner is destroyed */
	void CreateAudioComponent();

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AudioComponent;
	
	void CreateLink();
	TUniquePtr<IAudioLink> LinkInstance;
};