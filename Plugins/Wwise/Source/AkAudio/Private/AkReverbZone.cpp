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
	AkReverbZone.cpp:
=============================================================================*/

#include "AkReverbZone.h"
#include "AkCustomVersion.h"

AAkReverbZone::AAkReverbZone(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// Set default values
	Room->bEnableReverbZone = true;
	Room->WallOcclusion = 0;
	SurfaceReflectorSet->bEnableSurfaceReflectors = false;
}

void AAkReverbZone::BeginPlay()
{
	Super::BeginPlay();

	if (Room == nullptr || (Room && !Room->bEnable))
	{
		UE_LOG(LogAkAudio, Error, TEXT("AkReverbZone %s: child Room component is not enabled. A Reverb Zone needs to be a Spatial Audio Room."), *GetName());
		return;
	}
}

void AAkReverbZone::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITORONLY_DATA
	const int32 AkVersion = GetLinkerCustomVersion(FAkCustomVersion::GUID);

	if (AkVersion < FAkCustomVersion::ReverbZoneComponentisation)
	{
		bRequiresMigration = true;
	}
#endif
}

void AAkReverbZone::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

#if WITH_EDITORONLY_DATA
	if (bRequiresMigration)
	{
		if (Room != nullptr)
		{
			Room->SetEnableReverbZone(true);
			Room->UpdateParentRoomActor(ParentSpatialAudioVolume);
			Room->UpdateTransitionRegionWidth(TransitionRegionWidth);
		}
		bRequiresMigration = false;
	}
#endif
}
