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
	AkReverbZone.h:
=============================================================================*/
#pragma once

#include "AkSpatialAudioVolume.h"
#include "AkReverbZone.generated.h"


/*------------------------------------------------------------------------------------
	AAkSpatialAudioVolume
------------------------------------------------------------------------------------*/
UCLASS(ClassGroup = Audiokinetic, BlueprintType, hidecategories = (Advanced, Attachment, Volume))
class AKAUDIO_API AAkReverbZone : public AAkSpatialAudioVolume
{
	GENERATED_BODY()

public:
	AAkReverbZone(const class FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void PostRegisterAllComponents() override;

private:
#if WITH_EDITORONLY_DATA
	/**
	* In Wwise 2023.1.1, the Reverb Zone properties were moved to AkRoomComponent.
	* AAkReverbZone.ParentSpatialAudioVolume is now called AkRoomComponent.ParentRoomActor
	* The type has also been changed to Actor to allow any actor with Room components to be a parent Room.
	* @warning Deprecated as of Wwise 2023.1.1.
	*/
	UPROPERTY()
	AAkSpatialAudioVolume* ParentSpatialAudioVolume;

	/**
	* In Wwise 2023.1.1, the Reverb Zone properties were moved to AkRoomComponent.
	* AAkReverbZone.TransitionRegionWidth is now AkRoomComponent.TransitionRegionWidth
	* @warning Deprecated as of Wwise 2023.1.1.
	*/
	UPROPERTY()
	float TransitionRegionWidth;

	UPROPERTY(Transient)
	bool bRequiresMigration = false;
#endif
};
