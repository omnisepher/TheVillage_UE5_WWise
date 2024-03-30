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
#include "AkGameplayTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PostEventAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPostEventAsyncOutputPin, int32, PlayingID);

UCLASS()
class AKAUDIO_API UPostEventAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPostEventAsyncOutputPin Completed;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Audiokinetic|Actor", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AdvancedDisplay = "3", AutoCreateRefTerm = "PostEventCallback,ExternalSources"))
	static UPostEventAsync* PostEventAsync(const UObject* WorldContextObject, 
			class UAkAudioEvent* AkEvent,
			class AActor* Actor,
			UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkCallbackType")) int32 CallbackMask,
			const FOnAkPostEventCallback& PostEventCallback,
			bool bStopWhenAttachedToDestroyed = false
		);

public:
	void Activate() override;

private:
	UFUNCTION()
	void PollPostEventFuture();

private:
	const UObject* WorldContextObject = nullptr;
	UAkAudioEvent* AkEvent = nullptr;
	AActor* Actor = nullptr;
	int32 CallbackMask = 0;
	FOnAkPostEventCallback PostEventCallback;
	bool bStopWhenAttachedToDestroyed = false;
	TFuture<AkPlayingID> PlayingIDFuture;
	FTimerHandle Timer;
};
