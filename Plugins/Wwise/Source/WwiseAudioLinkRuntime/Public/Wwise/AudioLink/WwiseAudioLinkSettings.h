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

#include "AudioLinkSettingsAbstract.h"
#include "Engine/StreamableManager.h"
#include "WwiseAudioLinkSettings.generated.h"

class UWwiseAudioLinkSettings;
class UAkAudioEvent;

class FWwiseAudioLinkSettingsProxy : public IAudioLinkSettingsProxy
{
public:
	FWwiseAudioLinkSettingsProxy(const UWwiseAudioLinkSettings&);
	virtual ~FWwiseAudioLinkSettingsProxy() override = default;

	const TSoftObjectPtr<UAkAudioEvent>& GetStartEvent() const { return StartEvent; }
	int32 GetReceivingBufferSizeInFrames() const { return ReceivingBufferSizeInFrames; }
	bool ShouldClearBufferOnReceipt() const { return bShouldZeroBuffer; }
	float GetProducerConsumerBufferRatio() const { return ProducerToConsumerBufferRatio; }
	float GetInitialSilenceFillRatio() const { return InitialSilenceFillRatio; }

	void Update(const UWwiseAudioLinkSettings&);

	FDelegateHandle CallOnEventLoaded(TFunction<void()>&& InCallback);
	void UnregisterEventLoadedDelegate(const FDelegateHandle& InDelegate);

protected:
	void NotifyEventDataLoaded();

private:
#if WITH_EDITOR
	void RefreshFromSettings(UAudioLinkSettingsAbstract* InSettings, FPropertyChangedEvent& InPropertyChangedEvent) override;
#endif //WITH_EDITOR
	
	FSimpleMulticastDelegate OnEventLoadedDelegate;
	FCriticalSection CriticalSection;
	friend class UWwiseAudioLinkSettings;

	TSoftObjectPtr<UAkAudioEvent> StartEvent;

	int32 ReceivingBufferSizeInFrames;
	bool bShouldZeroBuffer = false;
	bool bIsEventDataLoaded = false;
	float ProducerToConsumerBufferRatio = 2.0f;
	float InitialSilenceFillRatio = 1.0f;
};

using FSharedAudioLinkSettingProxyWwisePtr = TSharedPtr<FWwiseAudioLinkSettingsProxy, ESPMode::ThreadSafe>;

UCLASS(config = Game, defaultconfig)
class WWISE_API UWwiseAudioLinkSettings : public UAudioLinkSettingsAbstract
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Wwise|AudioLink")
	TSoftObjectPtr<UAkAudioEvent> StartEvent;

	/** When enabled, the receiving code clears the buffer after it is read, so it is not rendered by Unreal. Only applies if running both renderers simultaneously.  */
	UPROPERTY(Config, EditAnywhere, Category = "Wwise|AudioLink")
	bool bShouldClearBufferOnReceipt = true;

	/** The ratio of producer to consumer buffer size. A value of 2.0 means it is twice as big as the consumer buffer.  */
	UPROPERTY(Config, EditAnywhere, Category = "Wwise|AudioLink")
	float ProducerToConsumerBufferRatio = 2.0f;

	/** Ratio of the initial buffer to fill with silence before consumption. This can prevent starvation at the cost of additional latency. */
	UPROPERTY(Config, EditAnywhere, Category = "Wwise|AudioLink")
	float InitialSilenceFillRatio = 1.0f;

	void RequestLoad() const;

protected:
	mutable bool bLoadRequested = false;
	TSharedPtr<FStreamableHandle> LoadingHandle;

	/** Once the SoftObjectReference has been resolved, attach the reference here so it's owned. */
	UPROPERTY(Transient)
	TObjectPtr<UAkAudioEvent> StartEventResolved;

	void PostLoad() override;
	void OnLoadCompleteCallback();
	void FinishDestroy() override;

	friend class FWwiseAudioLinkSettingsProxy;

	int32 GetReceivingBufferSizeInFrames() const;
	
	UAudioLinkSettingsAbstract::FSharedSettingsProxyPtr MakeProxy() const override
	{
		return UAudioLinkSettingsAbstract::FSharedSettingsProxyPtr(new FWwiseAudioLinkSettingsProxy{ *this });
	}

	FName GetFactoryName() const override;

	struct FWwiseSettings
	{
		int32 NumSamplesPerBuffer = 0;
		int32 NumSamplesPerSecond = 0;
	};

	bool GetSettingsFromWwise(FWwiseSettings& OutSettings) const;
};

