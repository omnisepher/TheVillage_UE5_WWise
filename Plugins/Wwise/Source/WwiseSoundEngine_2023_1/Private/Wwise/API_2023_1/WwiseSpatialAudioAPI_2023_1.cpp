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

#include "Wwise/API_2023_1/WwiseSpatialAudioAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

#include "CoreTypes.h"

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT
#pragma pack(push)
#pragma warning(push)
#pragma warning(disable: 4103)		// alignment changed after including header, may be due to missing #pragma pack(pop)
#endif // PLATFORM_MICROSOFT

#include "Wwise/PreSoundEngineInclude.h"
#include <AK/SpatialAudio/Common/AkReverbEstimation.h>
#include "Wwise/PostSoundEngineInclude.h"

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT
#pragma warning(pop)
#pragma pack(pop)
#endif // PLATFORM_MICROSOFT

FWwiseSpatialAudioAPI_2023_1::FWwiseSpatialAudioAPI_2023_1() :
	IWwiseSpatialAudioAPI(new FReverbEstimation)
{}

AKRESULT FWwiseSpatialAudioAPI_2023_1::Init(const AkSpatialAudioInitSettings& in_initSettings)
{
	SCOPED_WWISESOUNDENGINE_EVENT(TEXT("FWwiseSpatialAudioAPI_2023_1::Init"));
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::Init(in_initSettings);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::RegisterListener(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::RegisterListener(in_gameObjectID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::UnregisterListener(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::UnregisterListener(in_gameObjectID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetGameObjectRadius(
	AkGameObjectID in_gameObjectID,
	AkReal32 in_outerRadius,
	AkReal32 in_innerRadius
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetGameObjectRadius(in_gameObjectID, in_outerRadius, in_innerRadius);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetImageSource(
	AkImageSourceID in_srcID,
	const AkImageSourceSettings& in_info,
	const char* in_name,
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetImageSource(in_srcID, in_info, in_name, in_AuxBusID, in_gameObjectID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::RemoveImageSource(
	AkImageSourceID in_srcID,
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::RemoveImageSource(in_srcID, in_AuxBusID, in_gameObjectID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::ClearImageSources(
	AkUniqueID in_AuxBusID,
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::ClearImageSources(in_AuxBusID, in_gameObjectID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetGeometry(
	AkGeometrySetID in_GeomSetID,
	const AkGeometryParams& in_params
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetGeometry(in_GeomSetID, in_params);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::RemoveGeometry(
	AkGeometrySetID in_SetID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::RemoveGeometry(in_SetID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetGeometryInstance(
	AkGeometryInstanceID in_GeometryInstanceID,
	const AkGeometryInstanceParams& in_params
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetGeometryInstance(in_GeometryInstanceID, in_params);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::RemoveGeometryInstance(
	AkGeometryInstanceID in_GeometryInstanceID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::RemoveGeometryInstance(in_GeometryInstanceID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::QueryReflectionPaths(
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_positionIndex,
	AkVector64& out_listenerPos,
	AkVector64& out_emitterPos,
	AkReflectionPathInfo* out_aPaths,
	AkUInt32& io_uArraySize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::QueryReflectionPaths(in_gameObjectID, in_positionIndex, out_listenerPos, out_emitterPos, out_aPaths, io_uArraySize);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetRoom(
	AkRoomID in_RoomID,
	const AkRoomParams& in_Params,
	const char* in_RoomName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetRoom(in_RoomID, in_Params, in_RoomName);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::RemoveRoom(
	AkRoomID in_RoomID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::RemoveRoom(in_RoomID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetPortal(
	AkPortalID in_PortalID,
	const AkPortalParams& in_Params,
	const char* in_PortalName
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetPortal(in_PortalID, in_Params, in_PortalName);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::RemovePortal(
	AkPortalID in_PortalID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::RemovePortal(in_PortalID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetReverbZone(
	AkRoomID in_ReverbZone,
	AkRoomID in_ParentRoom,
	AkReal32 in_transitionRegionWidth
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetReverbZone(in_ReverbZone, in_ParentRoom, in_transitionRegionWidth);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::RemoveReverbZone(
	AkRoomID in_ReverbZone
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::RemoveReverbZone(in_ReverbZone);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetGameObjectInRoom(
	AkGameObjectID in_gameObjectID,
	AkRoomID in_CurrentRoomID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetGameObjectInRoom(in_gameObjectID, in_CurrentRoomID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::UnsetGameObjectInRoom(
	AkGameObjectID in_gameObjectID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::UnsetGameObjectInRoom(in_gameObjectID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetReflectionsOrder(
	AkUInt32 in_uReflectionsOrder,
	bool in_bUpdatePaths
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetReflectionsOrder(in_uReflectionsOrder, in_bUpdatePaths);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetDiffractionOrder(
	AkUInt32 in_uDiffractionOrder,
	bool in_bUpdatePaths
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetDiffractionOrder(in_uDiffractionOrder, in_bUpdatePaths);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetMaxEmitterRoomAuxSends(
	AkUInt32 in_uMaxEmitterRoomAuxSends
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetMaxEmitterRoomAuxSends(in_uMaxEmitterRoomAuxSends);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetNumberOfPrimaryRays(
	AkUInt32 in_uNbPrimaryRays
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetNumberOfPrimaryRays(in_uNbPrimaryRays);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetLoadBalancingSpread(
	AkUInt32 in_uNbFrames
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetLoadBalancingSpread(in_uNbFrames);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetEarlyReflectionsAuxSend(
	AkGameObjectID in_gameObjectID,
	AkAuxBusID in_auxBusID
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetEarlyReflectionsAuxSend(in_gameObjectID, in_auxBusID);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetEarlyReflectionsVolume(
	AkGameObjectID in_gameObjectID,
	AkReal32 in_fSendVolume
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetEarlyReflectionsVolume(in_gameObjectID, in_fSendVolume);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetPortalObstructionAndOcclusion(
	AkPortalID in_PortalID,
	AkReal32 in_fObstruction,
	AkReal32 in_fOcclusion
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetPortalObstructionAndOcclusion(in_PortalID, in_fObstruction, in_fOcclusion);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetGameObjectToPortalObstruction(
	AkGameObjectID in_gameObjectID,
	AkPortalID in_PortalID,
	AkReal32 in_fObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetGameObjectToPortalObstruction(in_gameObjectID, in_PortalID, in_fObstruction);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::SetPortalToPortalObstruction(
	AkPortalID in_PortalID0,
	AkPortalID in_PortalID1,
	AkReal32 in_fObstruction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::SetPortalToPortalObstruction(in_PortalID0, in_PortalID1, in_fObstruction);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::QueryWetDiffraction(
	AkPortalID in_portal,
	AkReal32& out_wetDiffraction
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::QueryWetDiffraction(in_portal, out_wetDiffraction);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::QueryDiffractionPaths(
	AkGameObjectID in_gameObjectID,
	AkUInt32 in_positionIndex,
	AkVector64& out_listenerPos,
	AkVector64& out_emitterPos,
	AkDiffractionPathInfo* out_aPaths,
	AkUInt32& io_uArraySize
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::QueryDiffractionPaths(in_gameObjectID, in_positionIndex, out_listenerPos, out_emitterPos, out_aPaths, io_uArraySize);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::ResetStochasticEngine()
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::ResetStochasticEngine();
}

float FWwiseSpatialAudioAPI_2023_1::FReverbEstimation::CalculateSlope(const AkAcousticTexture& texture)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::ReverbEstimation::CalculateSlope(texture);
}

void FWwiseSpatialAudioAPI_2023_1::FReverbEstimation::GetAverageAbsorptionValues(AkAcousticTexture* in_textures, float* in_surfaceAreas, int in_numTextures, AkAcousticTexture& out_average)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	AK::SpatialAudio::ReverbEstimation::GetAverageAbsorptionValues(in_textures, in_surfaceAreas, in_numTextures, out_average);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::FReverbEstimation::EstimateT60Decay(
	AkReal32 in_volumeCubicMeters,
	AkReal32 in_surfaceAreaSquaredMeters,
	AkReal32 in_environmentAverageAbsorption,
	AkReal32& out_decayEstimate
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::ReverbEstimation::EstimateT60Decay(in_volumeCubicMeters, in_surfaceAreaSquaredMeters, in_environmentAverageAbsorption, out_decayEstimate);
}

AKRESULT FWwiseSpatialAudioAPI_2023_1::FReverbEstimation::EstimateTimeToFirstReflection(
	AkVector in_environmentExtentMeters,
	AkReal32& out_timeToFirstReflectionMs,
	AkReal32 in_speedOfSound
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::ReverbEstimation::EstimateTimeToFirstReflection(in_environmentExtentMeters, out_timeToFirstReflectionMs, in_speedOfSound);
}

AkReal32 FWwiseSpatialAudioAPI_2023_1::FReverbEstimation::EstimateHFDamping(
	AkAcousticTexture* in_textures,
	float* in_surfaceAreas,
	int in_numTextures
)
{
	SCOPE_CYCLE_COUNTER(STAT_WwiseSoundEngineAPI_2023_1);
	return AK::SpatialAudio::ReverbEstimation::EstimateHFDamping(in_textures, in_surfaceAreas, in_numTextures);
}
