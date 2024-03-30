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
AkObstructionAndOcclusionService.h:
=============================================================================*/

#pragma once

#include "AkInclude.h"
#include "WorldCollision.h"
#include "HAL/ThreadSafeBool.h"
#include "WwiseUnrealHelper.h"
#include "WwiseUnrealObjectHelper.h"
#include "Wwise/WwiseSoundEngineUtils.h"

#define NUM_BOUNDING_BOX_TRACE_POINTS 12

struct FAkObstructionAndOcclusion
{
	float CurrentValue;
	float TargetValue;
	float Rate;

	FAkObstructionAndOcclusion(float InTargetValue = 0.0f, float InCurrentValue = 0.0f);

	void SetTarget(float InTargetValue);
	bool ReachedTarget();
	bool Update(float InDeltaTime);
};

struct FAkObstructionAndOcclusionPair
{
	FAkObstructionAndOcclusionPair();

	FAkObstructionAndOcclusion Occ;
	FAkObstructionAndOcclusion Obs;
	FVector Position;

	bool Update(float InDeltaTime);

	bool ReachedTarget();

	/** Trace a ray from a source position to a bounding box point asynchronously */
	void AsyncTraceFromSource(const FVector& InSourcePosition, const FVector& InEndPosition, int InBoundingBoxPointIndex, ECollisionChannel InCollisionChannel, UWorld* InWorld, const FCollisionQueryParams& InCollisionParams);
	/** Trace a ray from a listener position to a bounding box point asynchronously */
	void AsyncTraceFromListener(const FVector& InListenerPosition, const FVector& InEndPosition, int InBoundingBoxPointIndex, ECollisionChannel InCollisionChannel, UWorld* InWorld, const FCollisionQueryParams& InCollisionParams);

	/** Get the total number of listener OR source collisions. */
	int GetCollisionCount();

	void Reset();


	/** Iterate through all trace handles and handle the results if ready */
	void CheckTraceResults(UWorld* InWorld);


private:
	/** Used to check when obstruction and occlusion targets need to be updated (when GetCollisionCount() != CurrentCollisionCount) */
	int CurrentCollisionCount = 0;
	TArray<FTraceHandle> SourceTraceHandles;
	TArray<FTraceHandle> ListenerTraceHandles;

	/** Iterate through all listener trace handles and handle the trace results if ready */
	void CheckListenerTraceHandles(UWorld* InWorld);
	/** Iterate through all source trace handles and handle the trace results if ready */
	void CheckSourceTraceHandles(UWorld* InWorld);

	TArray<FThreadSafeBool> SourceRayCollisions;
	TArray<FThreadSafeBool> ListenerRayCollisions;
};

class WWISEOBSTRUCTIONOCCLUSION_API AkObstructionAndOcclusionService
{
public:

	struct FListenerInfo
	{
		FVector Position;
		AkRoomID RoomID;

		FListenerInfo(FVector InPosition, AkRoomID InRoomID)
		{
			Position = InPosition;
			RoomID = InRoomID;
		}
	};
	typedef TMap<AkGameObjectID, FListenerInfo, FDefaultSetAllocator, WwiseUnrealHelper::AkGameObjectIdKeyFuncs<FListenerInfo, false>> ListenerMap;

	struct FPortalInfo
	{
		FVector Position;
		bool EnableObstruction;

		FPortalInfo(FVector InPosition, bool InEnableObstruction)
		{
			Position = InPosition;
			EnableObstruction = InEnableObstruction;
		}
	};
	typedef TMap<AkGameObjectID, FPortalInfo, FDefaultSetAllocator, WwiseUnrealHelper::AkGameObjectIdKeyFuncs<FPortalInfo, false>> PortalMap;

	virtual ~AkObstructionAndOcclusionService() {}

	void Tick(
		const ListenerMap& InListeners,
		const PortalMap& InPortals,
		const FVector& InSourcePosition,
		const AActor* InActor,
		const AkRoomID InRoomID,
		ECollisionChannel InCollisionChannel,
		float InDeltaTime,
		float InOcclusionRefreshInterval);

	/**
	 * Calculates updated occlusion and obstruction values synchronously and then sends them to the Wwise engine.
	 */
	void UpdateObstructionAndOcclusion(
		const ListenerMap& InListeners,
		const PortalMap& InPortals,
		const FVector& InSourcePosition,
		const AActor* InActor,
		const AkRoomID InRoomID,
		ECollisionChannel InCollisionChannel,
		float InOcclusionRefreshInterval);

	void ClearOcclusionValues();

	virtual void SetObstructionAndOcclusion(const AkGameObjectID InListenerID, const float InValue) = 0;
	virtual void SetPortalObstruction(const AkPortalID InPortalID, const float InValue) = 0;

protected:
	void _Init(UWorld* InWorld, float InRefreshInterval);

private:
	/**
	 * Fades active occlusions towards targets, sends updated values to the Wwise engine, then calculates refreshed occlusion and obstruction values asynchronously. 
	 */
	void RefreshObstructionAndOcclusion(
		const ListenerMap& InListeners,
		const PortalMap& InPortals,
		const FVector& InSourcePosition,
		const AActor* InActor,
		const AkRoomID InRoomID,
		ECollisionChannel InCollisionChannel,
		const float InDeltaTime,
		float InOcclusionRefreshInterval);
	/**
	 * Loops through in_Listeners and sends the obstruction occlusion values on each to the Wwise engine.
	 */
	void SetListenerObstructionAndOcclusion(const ListenerMap& InListeners);
	void SetListenerObstructionAndOcclusion(const FAkObstructionAndOcclusionPair& InObsOccPair, const AkGameObjectID InListenerID);
	/**
	 * Loops through the portals connected to in_pRoom and sends the obstruction occlusion values on each to the Wwise engine.
	 */
	void SetPortalObstruction(const PortalMap& InPortals, const AkRoomID InRoomID);
	/**
	* Calculates updated occlusion and obstruction values to listeners.
	*/
	void CalculateObstructionAndOcclusionValuesToListeners(const ListenerMap& InListeners, const FVector& InSourcePosition, const AActor* InActor, ECollisionChannel InCollisionChannel, bool bInAsync = true);
	/**
	* Calculates updated obstruction values to portals.
	*/
	void CalculateObstructionValuesToPortals(const PortalMap& InPortals, const FVector& InSourcePosition, const AActor* InActor, const AkRoomID InRoomID, ECollisionChannel InCollisionChannel, bool bInAsync = true);


	/** Last time occlusion was refreshed */
	float LastObstructionAndOcclusionRefresh = -1;
	float PreviousRefreshInterval = -1.0f;

	bool bClearingObstructionAndOcclusion = false;

	typedef WwiseUnrealHelper::AkGameObjectIdKeyFuncs<FAkObstructionAndOcclusionPair, false> ObsOccPairGameObjectIDKeyFuncs;
	TMap<AkGameObjectID, FAkObstructionAndOcclusionPair, FDefaultSetAllocator, ObsOccPairGameObjectIDKeyFuncs> ListenerObsOccMap;

	typedef WwiseUnrealHelper::AkSpatialAudioIDKeyFuncs<FAkObstructionAndOcclusionPair, false> ObsOccPairSpatialAudioIDKeyFuncs;
	typedef TMap<AkPortalID, FAkObstructionAndOcclusionPair, FDefaultSetAllocator, ObsOccPairSpatialAudioIDKeyFuncs> PortalObsOccMap;
	typedef WwiseUnrealHelper::AkSpatialAudioIDKeyFuncs<PortalObsOccMap, false> PortalObsOccMapSpatialAudioIDKeyFuncs;
	TMap<AkRoomID, PortalObsOccMap, FDefaultSetAllocator, PortalObsOccMapSpatialAudioIDKeyFuncs> PortalObsOccMapPerRoom;
};