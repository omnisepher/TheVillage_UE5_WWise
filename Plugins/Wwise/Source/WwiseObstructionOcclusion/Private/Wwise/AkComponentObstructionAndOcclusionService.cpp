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
AkComponentObstructionAndOcclusionService.h:
=============================================================================*/

#include "Wwise/AkComponentObstructionAndOcclusionService.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/API/WwiseSpatialAudioAPI.h"
#include "Engine/World.h"

void AkComponentObstructionAndOcclusionService::Init(const AkGameObjectID InAkComponentID, UWorld* InWorld, float InRefreshInterval, bool bInUsingRooms)
{
	_Init(InWorld, InRefreshInterval);
	AssociatedComponentID = InAkComponentID;
	bWorldIsUsingRooms = bInUsingRooms;
}

void AkComponentObstructionAndOcclusionService::SetObstructionAndOcclusion(const AkGameObjectID InListenerId, const float InValue)
{
	float Obstruction, Occlusion;

	if (bWorldIsUsingRooms)
	{
		Obstruction = InValue;
		Occlusion = 0.0f;
	}
	else
	{
		Obstruction = 0.0f;
		Occlusion = InValue;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return;

	SoundEngine->SetObjectObstructionAndOcclusion(AssociatedComponentID, InListenerId, Obstruction, Occlusion);
}

void AkComponentObstructionAndOcclusionService::SetPortalObstruction(const AkPortalID InPortalID, const float InValue)
{
	auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
	if (UNLIKELY(!SpatialAudio)) return;

	SpatialAudio->SetGameObjectToPortalObstruction(AssociatedComponentID, InPortalID, InValue);
}
