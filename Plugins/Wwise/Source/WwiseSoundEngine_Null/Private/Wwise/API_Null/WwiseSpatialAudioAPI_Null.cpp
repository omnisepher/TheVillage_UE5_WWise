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

#include "Wwise/API_Null/WwiseSpatialAudioAPI_Null.h"
#include "Wwise/Stats/SoundEngine_Null.h"

FWwiseSpatialAudioAPI_Null::FWwiseSpatialAudioAPI_Null() :
	IWwiseSpatialAudioAPI(new FReverbEstimation)
{}

AKRESULT FWwiseSpatialAudioAPI_Null::Init(const AkSpatialAudioInitSettings& in_initSettings)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseSpatialAudioAPI_Null::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::RegisterListener(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::UnregisterListener(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetGameObjectRadius(
	AkGameObjectID in_gameObjectID,
	AkReal32 in_outerRadius,
	AkReal32 in_innerRadius
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetImageSource(
	AkImageSourceID in_srcID,
	const AkImageSourceSettings& in_info,
	const char* in_name,
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::RemoveImageSource(
	AkImageSourceID in_srcID,
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::ClearImageSources(
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetGeometry(
	AkGeometrySetID in_GeomSetID,
	const AkGeometryParams& in_params
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::RemoveGeometry(
	AkGeometrySetID in_SetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetGeometryInstance(
	AkGeometryInstanceID in_GeometryInstanceID,
	const AkGeometryInstanceParams& in_params
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::RemoveGeometryInstance(
	AkGeometryInstanceID in_GeometryInstanceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::QueryReflectionPaths(
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_positionIndex,
	AkVector64& out_listenerPos,
	AkVector64& out_emitterPos,
	AkReflectionPathInfo* out_aPaths,
	AkUInt32& io_uArraySize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetRoom(
	AkRoomID in_RoomID,
	const AkRoomParams& in_Params,
	const char* in_RoomName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::RemoveRoom(
	AkRoomID in_RoomID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetPortal(
	AkPortalID in_PortalID,
	const AkPortalParams& in_Params,
	const char* in_PortalName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::RemovePortal(
	AkPortalID in_PortalID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetReverbZone(
	AkRoomID in_ReverbZone,
	AkRoomID in_ParentRoom,
	AkReal32 in_transitionRegionWidth
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::RemoveReverbZone(
	AkRoomID in_ReverbZone
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetGameObjectInRoom(
	AkGameObjectID in_gameObjectID,
	AkRoomID in_CurrentRoomID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::UnsetGameObjectInRoom(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetReflectionsOrder(
	AkUInt32 in_uReflectionsOrder,
	bool in_bUpdatePaths
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetDiffractionOrder(
	AkUInt32 in_uDiffractionOrder,
	bool in_bUpdatePaths
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetMaxEmitterRoomAuxSends(
	AkUInt32 in_uMaxEmitterRoomAuxSends
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetNumberOfPrimaryRays(
	AkUInt32 in_uNbPrimaryRays
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetLoadBalancingSpread(
	AkUInt32 in_uNbFrames
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetEarlyReflectionsAuxSend(
	AkGameObjectID in_gameObjectID,
	AkAuxBusID in_auxBusID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetEarlyReflectionsVolume(
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fSendVolume
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetPortalObstructionAndOcclusion(
	AkPortalID in_PortalID,
	AkReal32 in_fObstruction,
	AkReal32 in_fOcclusion
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetGameObjectToPortalObstruction(
	AkGameObjectID in_gameObjectID,
	AkPortalID in_PortalID,
	AkReal32 in_fObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::SetPortalToPortalObstruction(
	AkPortalID in_PortalID0,
	AkPortalID in_PortalID1,
	AkReal32 in_fObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::QueryWetDiffraction(
	AkPortalID in_portal,
	AkReal32& out_wetDiffraction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::QueryDiffractionPaths(
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_positionIndex,
	AkVector64& out_listenerPos,
	AkVector64& out_emitterPos,
	AkDiffractionPathInfo* out_aPaths,
	AkUInt32& io_uArraySize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::ResetStochasticEngine()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

float FWwiseSpatialAudioAPI_Null::FReverbEstimation::CalculateSlope(const AkAcousticTexture& texture)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return .0f;
}

void FWwiseSpatialAudioAPI_Null::FReverbEstimation::GetAverageAbsorptionValues(AkAcousticTexture* in_textures, float* in_surfaceAreas, int in_numTextures, AkAcousticTexture& out_average)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
}

AKRESULT FWwiseSpatialAudioAPI_Null::FReverbEstimation::EstimateT60Decay(
	AkReal32 in_volumeCubicMeters,
	AkReal32 in_surfaceAreaSquaredMeters,
	AkReal32 in_environmentAverageAbsorption,
	AkReal32& out_decayEstimate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AKRESULT FWwiseSpatialAudioAPI_Null::FReverbEstimation::EstimateTimeToFirstReflection(
	AkVector in_environmentExtentMeters,
	AkReal32& out_timeToFirstReflectionMs,
	AkReal32 in_speedOfSound
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return AK_NotImplemented;
}

AkReal32 FWwiseSpatialAudioAPI_Null::FReverbEstimation::EstimateHFDamping(
	AkAcousticTexture* in_textures,
	float* in_surfaceAreas,
	int in_numTextures
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_Null);
	return .0f;
}
