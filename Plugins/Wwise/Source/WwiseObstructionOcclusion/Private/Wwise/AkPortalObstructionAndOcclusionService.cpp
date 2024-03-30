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
AkObstructionAndOcclusionService.cpp:
=============================================================================*/

#include "Wwise/AkPortalObstructionAndOcclusionService.h"
#include "Wwise/API/WwiseSpatialAudioAPI.h"
#include "Engine/World.h"

void AkPortalObstructionAndOcclusionService::Init(const AkPortalID InPortalID, UWorld* InWorld, float InRefreshInterval)
{
	_Init(InWorld, InRefreshInterval);
	AssociatedPortalID = InPortalID;
}

void AkPortalObstructionAndOcclusionService::SetPortalObstruction(const AkPortalID InPortalID, const float InValue)
{
	auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
	if (UNLIKELY(!SpatialAudio)) return;

	SpatialAudio->SetPortalToPortalObstruction(AssociatedPortalID, InPortalID, InValue);
}
