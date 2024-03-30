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
	AkReverbVolume.h:
=============================================================================*/
#pragma once

#include "GameFramework/Volume.h"
#include "AkLateReverbComponent.h"
#include "AkReverbVolume.generated.h"

/*------------------------------------------------------------------------------------
	AAkReverbVolume
------------------------------------------------------------------------------------*/
UCLASS(hidecategories=(Advanced, Attachment, Volume), BlueprintType)
class AKAUDIO_API AAkReverbVolume : public AVolume
{
	GENERATED_BODY()

public:
	AAkReverbVolume(const class FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual FName GetCustomIconName() const override
	{
		static const FName IconName("ClassIcon.AkReverbVolume");
		return IconName;
	}
#endif

	/** Whether this volume is currently enabled and able to affect sounds */
	UPROPERTY()
	bool bEnabled_DEPRECATED = false;

	UPROPERTY()
	class UAkAuxBus * AuxBus_DEPRECATED = nullptr;

	/** Wwise Auxiliary Bus associated to this AkReverbVolume */
	UPROPERTY()
	FString AuxBusName_DEPRECATED;

	/** Maximum send level to the Wwise Auxiliary Bus associated to this AkReverbVolume */
	UPROPERTY()
	float SendLevel_DEPRECATED = .0f;

	/** Rate at which to fade in/out the SendLevel of the current Reverb Volume when entering/exiting it, in percentage per second (0.2 will make the fade time 5 seconds) */
	UPROPERTY()
	float FadeRate_DEPRECATED = .0f;

	/**
	 * The precedence in which the AkReverbVolumes will be applied. In the case of overlapping volumes, only the ones 
	 * with the highest priority are chosen (the number of simultaneous AkReverbVolumes is configurable in the Unreal 
	 * Editor Project Settings under Plugins > Wwise). If two or more overlapping AkReverbVolumes have the same 
	 * priority, the chosen AkReverbVolume is unpredictable.
	 */
	UPROPERTY()
	float Priority_DEPRECATED = .0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ReverbVolume", meta = (ShowOnlyInnerProperties))
	UAkLateReverbComponent* LateReverbComponent = nullptr;

	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
};
