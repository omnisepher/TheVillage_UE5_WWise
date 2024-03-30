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

#pragma once

#include "AkAudioDevice.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PostEventAtLocationAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPostEventAtLocationAsyncOutputPin, int32, PlayingID);

UCLASS()
class AKAUDIO_API UPostEventAtLocationAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPostEventAtLocationAsyncOutputPin Completed;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UPostEventAtLocationAsync* PostEventAtLocationAsync(const UObject* WorldContextObject, class UAkAudioEvent* AkEvent, FVector Location, FRotator Orientation);

public:
	void Activate() override;

private:
	UFUNCTION()
	void PollPostEventFuture();

private:
	const UObject* WorldContextObject = nullptr;
	class UAkAudioEvent* AkEvent = nullptr;
	FVector Location;
	FRotator Orientation;
	TFuture<AkPlayingID> playingIDFuture;
	FTimerHandle Timer;
};
