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

#include "Wwise/AkObstructionAndOcclusionService.h"
#include "Wwise/Stats/ObstructionOcclusion.h"
#include "WwiseUnrealObjectHelper.h"
#include "WwiseUnrealEngineHelper.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Async/Async.h"
#include "GameFramework/PlayerController.h"


#define AK_DEBUG_OCCLUSION_PRINT 0
#if AK_DEBUG_OCCLUSION_PRINT
static int framecounter = 0;
#endif

#define AK_DEBUG_OCCLUSION 0
#if AK_DEBUG_OCCLUSION
#include "DrawDebugHelpers.h"
#endif



FAkObstructionAndOcclusion::FAkObstructionAndOcclusion(float InTargetValue, float InCurrentValue)
	: CurrentValue(InCurrentValue)
	, TargetValue(InTargetValue)
	, Rate(0.0f)
{}

void FAkObstructionAndOcclusion::SetTarget(float InTargetValue)
{
	TargetValue = FMath::Clamp(InTargetValue, 0.0f, 1.0f);

	const float UAkComponent_OCCLUSION_FADE_RATE = 2.0f; // from 0.0 to 1.0 in 0.5 seconds
	Rate = FMath::Sign(TargetValue - CurrentValue) * UAkComponent_OCCLUSION_FADE_RATE;
}

bool FAkObstructionAndOcclusion::Update(float InDeltaTime)
{
	auto OldValue = CurrentValue;
	if (OldValue != TargetValue)
	{
		const auto NewValue = OldValue + Rate * InDeltaTime;
		if (OldValue > TargetValue)
			CurrentValue = FMath::Clamp(NewValue, TargetValue, OldValue);
		else
			CurrentValue = FMath::Clamp(NewValue, OldValue, TargetValue);

		checkf(CurrentValue >= 0.f && CurrentValue <= 1.f, TEXT("FAkObstructionAndOcclusion::Update: CurrentValue (%f) not normalized."), CurrentValue);
		return true;
	}

	return false;
}

bool FAkObstructionAndOcclusion::ReachedTarget()
{
	return CurrentValue == TargetValue;
}

//=====================================================================================
// FAkListenerObstructionAndOcclusionPair
//=====================================================================================

FAkObstructionAndOcclusionPair::FAkObstructionAndOcclusionPair()
{
	SourceRayCollisions.AddZeroed(NUM_BOUNDING_BOX_TRACE_POINTS);
	ListenerRayCollisions.AddZeroed(NUM_BOUNDING_BOX_TRACE_POINTS);

	SourceTraceHandles.AddDefaulted(NUM_BOUNDING_BOX_TRACE_POINTS);
	ListenerTraceHandles.AddDefaulted(NUM_BOUNDING_BOX_TRACE_POINTS);
}

bool FAkObstructionAndOcclusionPair::Update(float InDeltaTime)
{
	if (CurrentCollisionCount != GetCollisionCount())
	{
		CurrentCollisionCount = GetCollisionCount();
		const float ratio = (float)CurrentCollisionCount / NUM_BOUNDING_BOX_TRACE_POINTS;
		Occ.SetTarget(ratio);
		Obs.SetTarget(ratio);
	}
	const bool bObsChanged = Obs.Update(InDeltaTime);
	const bool bOccChanged = Occ.Update(InDeltaTime);
	return bObsChanged || bOccChanged;
}

void FAkObstructionAndOcclusionPair::Reset()
{
	for (int i = 0; i < NUM_BOUNDING_BOX_TRACE_POINTS; ++i)
	{
		SourceRayCollisions[i] = ListenerRayCollisions[i] = false;
	}
}

bool FAkObstructionAndOcclusionPair::ReachedTarget()
{
	return Obs.ReachedTarget() && Occ.ReachedTarget();
}

void FAkObstructionAndOcclusionPair::AsyncTraceFromSource(const FVector& InSourcePosition, const FVector& InEndPosition, int InBoundingBoxPointIndex, ECollisionChannel InCollisionChannel, UWorld* InWorld, const FCollisionQueryParams& InCollisionParams)
{
	ensure(InBoundingBoxPointIndex < NUM_BOUNDING_BOX_TRACE_POINTS);
	// Check that we're not stacking another async trace on top of one that hasn't completed yet.
	if (!InWorld->IsTraceHandleValid(SourceTraceHandles[InBoundingBoxPointIndex], false))
	{
		SourceTraceHandles[InBoundingBoxPointIndex] = InWorld->AsyncLineTraceByChannel(EAsyncTraceType::Single, InSourcePosition, InEndPosition, InCollisionChannel, InCollisionParams);
	}
}
void FAkObstructionAndOcclusionPair::AsyncTraceFromListener(const FVector& InListenerPosition, const FVector& InEndPosition, int InBoundingBoxPointIndex, ECollisionChannel InCollisionChannel, UWorld* InWorld, const FCollisionQueryParams& InCollisionParams)
{
	ensure(InBoundingBoxPointIndex < NUM_BOUNDING_BOX_TRACE_POINTS);
	// Check that we're not stacking another async trace on top of one that hasn't completed yet.
	if (!InWorld->IsTraceHandleValid(ListenerTraceHandles[InBoundingBoxPointIndex], false))
	{
		ListenerTraceHandles[InBoundingBoxPointIndex] = InWorld->AsyncLineTraceByChannel(EAsyncTraceType::Single, InListenerPosition, InEndPosition, InCollisionChannel, InCollisionParams);
	}
}

int FAkObstructionAndOcclusionPair::GetCollisionCount()
{
	int CollisionCount = 0;
	for (int i = 0; i < NUM_BOUNDING_BOX_TRACE_POINTS; ++i)
	{
		CollisionCount += (SourceRayCollisions[i] || ListenerRayCollisions[i]) ? 1 : 0;
	}
	return CollisionCount;
}

void FAkObstructionAndOcclusionPair::CheckTraceResults(UWorld* InWorld)
{
	CheckListenerTraceHandles(InWorld);
	CheckSourceTraceHandles(InWorld);
}

void FAkObstructionAndOcclusionPair::CheckListenerTraceHandles(UWorld* InWorld)
{
	for (int BoundingBoxPointIndex = 0; BoundingBoxPointIndex < NUM_BOUNDING_BOX_TRACE_POINTS; ++BoundingBoxPointIndex)
	{
		if (ListenerTraceHandles[BoundingBoxPointIndex]._Data.FrameNumber != 0)
		{
			FTraceDatum OutData;
			if (InWorld->QueryTraceData(ListenerTraceHandles[BoundingBoxPointIndex], OutData))
			{
				ListenerTraceHandles[BoundingBoxPointIndex]._Data.FrameNumber = 0;
				ListenerRayCollisions[BoundingBoxPointIndex] = OutData.OutHits.Num() > 0;
			}
		}
	}
}

void FAkObstructionAndOcclusionPair::CheckSourceTraceHandles(UWorld* InWorld)
{
	for (int BoundingBoxPointIndex = 0; BoundingBoxPointIndex < NUM_BOUNDING_BOX_TRACE_POINTS; ++BoundingBoxPointIndex)
	{
		if (SourceTraceHandles[BoundingBoxPointIndex]._Data.FrameNumber != 0)
		{
			FTraceDatum OutData;
			if (InWorld->QueryTraceData(SourceTraceHandles[BoundingBoxPointIndex], OutData))
			{
				SourceTraceHandles[BoundingBoxPointIndex]._Data.FrameNumber = 0;
				SourceRayCollisions[BoundingBoxPointIndex] = OutData.OutHits.Num() > 0;
			}
		}
	}
}

//=====================================================================================
// AkObstructionAndOcclusionService
//=====================================================================================

void AkObstructionAndOcclusionService::_Init(UWorld* InWorld, float InRefreshInterval)
{
	if (InRefreshInterval > 0 && InWorld != nullptr)
		LastObstructionAndOcclusionRefresh = InWorld->GetTimeSeconds() + FMath::RandRange(0.0f, InRefreshInterval);
	else
		LastObstructionAndOcclusionRefresh = -1;
}

void AkObstructionAndOcclusionService::RefreshObstructionAndOcclusion(
	const ListenerMap& InListeners,
	const PortalMap& InPortals,
	const FVector& InSourcePosition,
	const AActor* InActor,
	const AkRoomID InRoomID,
	ECollisionChannel InCollisionChannel,
	const float InDeltaTime,
	float InOcclusionRefreshInterval)
{
	UWorld* CurrentWorld = InActor ? InActor->GetWorld() : nullptr;
	if (CurrentWorld == nullptr)
	{
		return;
	}

	// Fade the active occlusions
	bool StillClearingObsOcc = false;

	for (auto It = ListenerObsOccMap.CreateIterator(); It; ++It)
	{
		AkGameObjectID ListenerID = It->Key;

		if (InListeners.Find(ListenerID) == nullptr)
		{
			It.RemoveCurrent();
			continue;
		}

		FAkObstructionAndOcclusionPair& ObsOccPair = It->Value;
		ObsOccPair.CheckTraceResults(CurrentWorld);
		if (ObsOccPair.Update(InDeltaTime))
		{
			SetListenerObstructionAndOcclusion(ObsOccPair, ListenerID);
		}

		if (bClearingObstructionAndOcclusion)
		{
			StillClearingObsOcc |= !ObsOccPair.ReachedTarget();
		}
	}

	auto PortalObsOccMap = PortalObsOccMapPerRoom.Find(InRoomID);
	if (PortalObsOccMap)
	{
		for (auto It = PortalObsOccMap->CreateIterator(); It; ++It)
		{
			AkPortalID PortalID = It->Key;
			auto Portal = InPortals.Find(PortalID);

			if (Portal == nullptr)
			{
				It.RemoveCurrent();
				continue;
			}

			FAkObstructionAndOcclusionPair& ObsOccPair = It->Value;
			ObsOccPair.CheckTraceResults(CurrentWorld);
			if (ObsOccPair.Update(InDeltaTime))
			{
				SetPortalObstruction(PortalID, ObsOccPair.Obs.CurrentValue);
			}

			if (bClearingObstructionAndOcclusion)
			{
				StillClearingObsOcc |= !ObsOccPair.ReachedTarget();
			}
		}
	}

	if (bClearingObstructionAndOcclusion)
	{
		bClearingObstructionAndOcclusion = StillClearingObsOcc;
		return;
	}

	float CurrentTime = CurrentWorld->GetTimeSeconds();
	if (CurrentTime < LastObstructionAndOcclusionRefresh && LastObstructionAndOcclusionRefresh - CurrentTime > InOcclusionRefreshInterval)
	{
		// Occlusion refresh interval was made shorter since the last refresh, we need to re-distribute the next random calculation
		LastObstructionAndOcclusionRefresh = CurrentTime + FMath::RandRange(0.0f, InOcclusionRefreshInterval);
	}

	if (LastObstructionAndOcclusionRefresh == -1 || (CurrentTime - LastObstructionAndOcclusionRefresh) >= InOcclusionRefreshInterval)
	{
		LastObstructionAndOcclusionRefresh = CurrentTime;

		for (auto& Listener : InListeners)
		{
			auto& MapEntry = ListenerObsOccMap.FindOrAdd(Listener.Key);
			MapEntry.Position = Listener.Value.Position;
		}
		CalculateObstructionAndOcclusionValuesToListeners(InListeners, InSourcePosition, InActor, InCollisionChannel);

		for (auto& Portal : InPortals)
		{
			auto& MapEntry = PortalObsOccMapPerRoom.FindOrAdd(InRoomID).FindOrAdd(Portal.Key);
			MapEntry.Position = Portal.Value.Position;
		}
		CalculateObstructionValuesToPortals(InPortals, InSourcePosition, InActor, InRoomID, InCollisionChannel);
	}
}

void _CalculateObstructionAndOcclusionValues(FAkObstructionAndOcclusionPair* InObsOccPair, UWorld* InCurrentWorld, const FVector& InSourcePosition, const FVector& InDestinationPosition, ECollisionChannel InCollisionChannel, FCollisionQueryParams InCollisionParams, bool bInAsync)
{
	FHitResult OutHit;
	const bool bNowOccluded = InCurrentWorld->LineTraceSingleByChannel(OutHit, InSourcePosition, InDestinationPosition, InCollisionChannel, InCollisionParams);

	if (bNowOccluded)
	{
		FBox BoundingBox;
		AActor* HitActor = WwiseUnrealHelper::GetActorFromHitResult(OutHit);
		if (HitActor)
		{
			BoundingBox = HitActor->GetComponentsBoundingBox();
		}
		else if (OutHit.Component.IsValid())
		{
			BoundingBox = OutHit.Component->Bounds.GetBox();
		}
		// Translate the impact point to the bounding box of the obstacle
		const FVector Points[] =
		{
			FVector(OutHit.ImpactPoint.X, BoundingBox.Min.Y, BoundingBox.Min.Z),
			FVector(OutHit.ImpactPoint.X, BoundingBox.Min.Y, BoundingBox.Max.Z),
			FVector(OutHit.ImpactPoint.X, BoundingBox.Max.Y, BoundingBox.Min.Z),
			FVector(OutHit.ImpactPoint.X, BoundingBox.Max.Y, BoundingBox.Max.Z),

			FVector(BoundingBox.Min.X, OutHit.ImpactPoint.Y, BoundingBox.Min.Z),
			FVector(BoundingBox.Min.X, OutHit.ImpactPoint.Y, BoundingBox.Max.Z),
			FVector(BoundingBox.Max.X, OutHit.ImpactPoint.Y, BoundingBox.Min.Z),
			FVector(BoundingBox.Max.X, OutHit.ImpactPoint.Y, BoundingBox.Max.Z),

			FVector(BoundingBox.Min.X, BoundingBox.Min.Y, OutHit.ImpactPoint.Z),
			FVector(BoundingBox.Min.X, BoundingBox.Max.Y, OutHit.ImpactPoint.Z),
			FVector(BoundingBox.Max.X, BoundingBox.Min.Y, OutHit.ImpactPoint.Z),
			FVector(BoundingBox.Max.X, BoundingBox.Max.Y, OutHit.ImpactPoint.Z)
		};

		if (bInAsync)
		{
			for (int PointIndex = 0; PointIndex < NUM_BOUNDING_BOX_TRACE_POINTS; ++PointIndex)
			{
				auto Point = Points[PointIndex];
				InObsOccPair->AsyncTraceFromListener(InDestinationPosition, Point, PointIndex, InCollisionChannel, InCurrentWorld, InCollisionParams);
				InObsOccPair->AsyncTraceFromSource(InSourcePosition, Point, PointIndex, InCollisionChannel, InCurrentWorld, InCollisionParams);
			}
		}
		else
		{
			// Compute the number of "second order paths" that are also obstructed. This will allow us to approximate
			// "how obstructed" the source is.
			int32 NumObstructedPaths = 0;
			for (const auto& Point : Points)
			{
				if (InCurrentWorld->LineTraceSingleByChannel(OutHit, InDestinationPosition, Point, InCollisionChannel, InCollisionParams) ||
					InCurrentWorld->LineTraceSingleByChannel(OutHit, InSourcePosition, Point, InCollisionChannel, InCollisionParams))
					++NumObstructedPaths;
			}
			// Modulate occlusion by blocked secondary paths. 
			const float ratio = (float)NumObstructedPaths / NUM_BOUNDING_BOX_TRACE_POINTS;
			InObsOccPair->Occ.SetTarget(ratio);
			InObsOccPair->Obs.SetTarget(ratio);
		}

#if AK_DEBUG_OCCLUSION
		check(IsInGameThread());
		// Draw bounding box and "second order paths"
		//UE_LOG(LogAkAudio, Log, TEXT("Target Occlusion level: %f"), ListenerOcclusionInfo[ListenerIdx].TargetValue);
		FlushPersistentDebugLines(InCurrentWorld);
		FlushDebugStrings(InCurrentWorld);
		DrawDebugBox(InCurrentWorld, BoundingBox.GetCenter(), BoundingBox.GetExtent(), FColor::White, false, 4);
		DrawDebugPoint(InCurrentWorld, InDestinationPosition, 10.0f, FColor(0, 255, 0), false, 4);
		DrawDebugPoint(InCurrentWorld, InSourcePosition, 10.0f, FColor(0, 255, 0), false, 4);
		DrawDebugPoint(InCurrentWorld, OutHit.ImpactPoint, 10.0f, FColor(0, 255, 0), false, 4);

		for (int32 i = 0; i < NUM_BOUNDING_BOX_TRACE_POINTS; i++)
		{
			DrawDebugPoint(InCurrentWorld, Points[i], 10.0f, FColor(255, 255, 0), false, 4);
			DrawDebugString(InCurrentWorld, Points[i], FString::Printf(TEXT("%d"), i), nullptr, FColor::White, 4);
			DrawDebugLine(InCurrentWorld, Points[i], InDestinationPosition, FColor::Cyan, false, 4);
			DrawDebugLine(InCurrentWorld, Points[i], InSourcePosition, FColor::Cyan, false, 4);
		}
		FColor LineColor = FColor::MakeRedToGreenColorFromScalar(1.0f - InObsOccPair->Occ.TargetValue);
		DrawDebugLine(InCurrentWorld, InDestinationPosition, InSourcePosition, LineColor, false, 4);
#endif // AK_DEBUG_OCCLUSION
	}
	else
	{
		InObsOccPair->Occ.SetTarget(0.0f);
		InObsOccPair->Obs.SetTarget(0.0f);
		InObsOccPair->Reset();
	}
}

void AkObstructionAndOcclusionService::CalculateObstructionAndOcclusionValuesToListeners(const ListenerMap& InListeners, const FVector& InSourcePosition, const AActor* InActor, ECollisionChannel InCollisionChannel, bool bInAsync /* = true */)
{
	SCOPED_WWISEOBSTRUCTIONOCCLUSION_EVENT_3(TEXT("AkObstructionAndOcclusionService::CalculateObstructionAndOcclusionValuesToListeners"));
	auto CurrentWorld = InActor->GetWorld();
	if (!CurrentWorld)
	{
		return;
	}

	static const FName NAME_SoundOcclusion = TEXT("SoundOcclusion");
	FCollisionQueryParams CollisionParams(NAME_SoundOcclusion, true, InActor);
	auto PlayerController = GEngine->GetFirstLocalPlayerController(CurrentWorld);
	if (PlayerController)
	{
		CollisionParams.AddIgnoredActor(PlayerController->GetViewTarget());
	}

	for (auto& Listener : InListeners)
	{
		auto MapEntry = ListenerObsOccMap.Find(Listener.Key);
		if (MapEntry == nullptr)
		{
			continue;
		}

		const FVector ListenerPosition = MapEntry->Position;

		_CalculateObstructionAndOcclusionValues(MapEntry, CurrentWorld, InSourcePosition, ListenerPosition, InCollisionChannel, CollisionParams, bInAsync);
	}
}

void AkObstructionAndOcclusionService::CalculateObstructionValuesToPortals(const PortalMap& InPortals, const FVector& InSourcePosition, const AActor* InActor, const AkRoomID InRoomID, ECollisionChannel InCollisionChannel, bool bInAsync/* = true*/)
{
	SCOPED_WWISEOBSTRUCTIONOCCLUSION_EVENT_3(TEXT("AkObstructionAndOcclusionService::CalculateObstructionValuesToPortals"));
	auto CurrentWorld = InActor->GetWorld();
	if (!CurrentWorld)
	{
		return;
	}

	static const FName NAME_SoundOcclusion = TEXT("SoundOcclusion");
	FCollisionQueryParams CollisionParams(NAME_SoundOcclusion, true, InActor);
	auto PlayerController = GEngine->GetFirstLocalPlayerController(CurrentWorld);
	if (PlayerController)
	{
		CollisionParams.AddIgnoredActor(PlayerController->GetViewTarget());
	}

	for (auto& Portal : InPortals)
	{
		if (!Portal.Value.EnableObstruction)
		{
			continue;
		}

		auto PortalObsOccMap = PortalObsOccMapPerRoom.Find(InRoomID);
		if (PortalObsOccMap == nullptr)
		{
			continue;
		}

		auto MapEntry = PortalObsOccMap->Find(Portal.Key);
		if (MapEntry == nullptr)
		{
			continue;
		}

		const FVector PortalPosition = MapEntry->Position;

		_CalculateObstructionAndOcclusionValues(MapEntry, CurrentWorld, InSourcePosition, PortalPosition, InCollisionChannel, CollisionParams, bInAsync);
	}
}

void AkObstructionAndOcclusionService::SetListenerObstructionAndOcclusion(const ListenerMap& InListeners)
{
	SCOPED_WWISEOBSTRUCTIONOCCLUSION_EVENT_3(TEXT("AkObstructionAndOcclusionService::SetListenerObstructionAndOcclusion"));
	for (auto& Listener : InListeners)
	{
		auto ListenerID = Listener.Key;

		auto MapEntry = ListenerObsOccMap.Find(ListenerID);

		if (MapEntry == nullptr)
		{
			continue;
		}

		MapEntry->Occ.CurrentValue = MapEntry->Occ.TargetValue;

		SetListenerObstructionAndOcclusion(*MapEntry, ListenerID);
	}
}

void AkObstructionAndOcclusionService::SetListenerObstructionAndOcclusion(const FAkObstructionAndOcclusionPair& InObsOccPair, const AkGameObjectID InListenerID)
{
	SetObstructionAndOcclusion(InListenerID, InObsOccPair.Obs.CurrentValue);
}

void AkObstructionAndOcclusionService::SetPortalObstruction(const PortalMap& InPortals, const AkRoomID InRoomID)
{
	SCOPED_WWISEOBSTRUCTIONOCCLUSION_EVENT_3(TEXT("AkObstructionAndOcclusionService::SetPortalObstruction"));
	for (auto& Portal : InPortals)
	{
		AkPortalID PortalID = Portal.Key;

		auto PortalObsOccMap = PortalObsOccMapPerRoom.Find(InRoomID);
		if (PortalObsOccMap == nullptr)
		{
			continue;
		}

		auto MapEntry = PortalObsOccMap->Find(PortalID);
		if (MapEntry == nullptr)
		{
			continue;
		}

		SetPortalObstruction(PortalID, MapEntry->Obs.CurrentValue);
	}
}

void AkObstructionAndOcclusionService::ClearOcclusionValues()
{
	bClearingObstructionAndOcclusion = false;

	for (auto& ListenerPack : ListenerObsOccMap)
	{
		FAkObstructionAndOcclusionPair& Pair = ListenerPack.Value;
		Pair.Occ.SetTarget(0.0f);
		Pair.Obs.SetTarget(0.0f);
		bClearingObstructionAndOcclusion |= !Pair.ReachedTarget();
	}
}

void AkObstructionAndOcclusionService::Tick(
	const ListenerMap& InListeners,
	const PortalMap& InPortals,
	const FVector& InSourcePosition,
	const AActor* InActor,
	const AkRoomID InRoomID,
	ECollisionChannel InCollisionChannel,
	float InDeltaTime,
	float InOcclusionRefreshInterval)
{
	// Check Occlusion/Obstruction, if enabled
	if (InOcclusionRefreshInterval > 0.0f || bClearingObstructionAndOcclusion)
	{
		RefreshObstructionAndOcclusion(InListeners, InPortals, InSourcePosition, InActor, InRoomID, InCollisionChannel, InDeltaTime, InOcclusionRefreshInterval);
	}
	else if (InOcclusionRefreshInterval != PreviousRefreshInterval)
	{
		// Reset the occlusion obstruction pairs so that the occlusion is correctly recalculated.
		for (auto& ListenerPack : ListenerObsOccMap)
		{
			FAkObstructionAndOcclusionPair& Pair = ListenerPack.Value;
			Pair.Reset();
		}
		if (InOcclusionRefreshInterval <= 0.0f)
			ClearOcclusionValues();
	}
	PreviousRefreshInterval = InOcclusionRefreshInterval;
}

void AkObstructionAndOcclusionService::UpdateObstructionAndOcclusion(
	const ListenerMap& InListeners,
	const PortalMap& InPortals,
	const FVector& InSourcePosition,
	const AActor* InActor,
	const AkRoomID InRoomID,
	ECollisionChannel InCollisionChannel,
	float InOcclusionRefreshInterval)
{
	SCOPED_WWISEOBSTRUCTIONOCCLUSION_EVENT_2(TEXT("AkObstructionAndOcclusionService::UpdateObstructionAndOcclusion"));
	if ((InOcclusionRefreshInterval > 0.f || bClearingObstructionAndOcclusion) && InActor)
	{
		for (auto& Listener : InListeners)
		{
			auto& MapEntry = ListenerObsOccMap.FindOrAdd(Listener.Key);
			MapEntry.Position = Listener.Value.Position;
		}

		CalculateObstructionAndOcclusionValuesToListeners(InListeners, InSourcePosition, InActor, InCollisionChannel, false);

		for (auto& ListenerPair : ListenerObsOccMap)
		{
			ListenerPair.Value.Obs.CurrentValue = ListenerPair.Value.Obs.TargetValue;
			ListenerPair.Value.Occ.CurrentValue = ListenerPair.Value.Occ.TargetValue;
		}
		SetListenerObstructionAndOcclusion(InListeners);

		CalculateObstructionValuesToPortals(InPortals, InSourcePosition, InActor, InRoomID, InCollisionChannel, false);

		auto PortalObsOccMap = PortalObsOccMapPerRoom.Find(InRoomID);
		if (PortalObsOccMap)
		{
			for (auto& PortalPair : *PortalObsOccMap)
			{
				PortalPair.Value.Obs.CurrentValue = PortalPair.Value.Obs.TargetValue;
			}
			SetPortalObstruction(InPortals, InRoomID);
		}
	}
}

