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

#include "BlueprintNodes/PostEventAtLocationAsync.h"

#include "AkAudioEvent.h"
#include "AkGameplayTypes.h"
#include "TimerManager.h"

UPostEventAtLocationAsync* UPostEventAtLocationAsync::PostEventAtLocationAsync(const UObject* WorldContextObject, UAkAudioEvent* AkEvent, FVector Location, FRotator Orientation)
{
	UPostEventAtLocationAsync* newNode = NewObject<UPostEventAtLocationAsync>();
	newNode->WorldContextObject = WorldContextObject;
	newNode->AkEvent = AkEvent;
	newNode->Location = Location;
	newNode->Orientation = Orientation;
	return newNode;
}

void UPostEventAtLocationAsync::Activate()
{
	if (AkEvent == nullptr)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("PostEventAtLocationAsync: No Event specified!"));
		Completed.Broadcast(AK_INVALID_PLAYING_ID);
		return;
	}

	AkDeviceAndWorld DeviceAndWorld(WorldContextObject);
	if (DeviceAndWorld.IsValid())
	{
		AkEvent->PostAtLocation(Location, Orientation, {}, 0, WorldContextObject);

		WorldContextObject->GetWorld()->GetTimerManager().SetTimer(Timer, this, &UPostEventAtLocationAsync::PollPostEventFuture, 1.f / 60.f, true);
	}
	else
	{
		Completed.Broadcast(AK_INVALID_PLAYING_ID);
	}
}

void UPostEventAtLocationAsync::PollPostEventFuture()
{
	if (playingIDFuture.IsReady())
	{
		WorldContextObject->GetWorld()->GetTimerManager().ClearTimer(Timer);
		Timer.Invalidate();
		Completed.Broadcast(playingIDFuture.Get());
	}
}