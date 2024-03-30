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

#include "Wwise/AudioLink/WwiseAudioLinkSettings.h"
#include "Wwise/AudioLink/WwiseAudioLinkInputClient.h"
#include "Wwise/AudioLink/WwiseAudioLinkFactory.h"
#include "Wwise/Stats/AudioLink.h"

#include "AkAudioEvent.h"
#include "InitializationSettings/AkInitializationSettings.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Async/Async.h"

FWwiseAudioLinkSettingsProxy::FWwiseAudioLinkSettingsProxy(const UWwiseAudioLinkSettings& InSettings)
{
	Update(InSettings);
}

void FWwiseAudioLinkSettingsProxy::Update(const UWwiseAudioLinkSettings& InSettings)
{
	InSettings.RequestLoad();
	ReceivingBufferSizeInFrames = InSettings.GetReceivingBufferSizeInFrames();
	bShouldZeroBuffer = InSettings.bShouldClearBufferOnReceipt;
	ProducerToConsumerBufferRatio = InSettings.ProducerToConsumerBufferRatio;
	InitialSilenceFillRatio = InSettings.InitialSilenceFillRatio;	
	StartEvent = InSettings.StartEvent;
}

FDelegateHandle FWwiseAudioLinkSettingsProxy::CallOnEventLoaded(TFunction<void()>&& InCallback)
{
	{
		FScopeLock Lock(&CriticalSection);
		if (!bIsEventDataLoaded)
		{
			UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("FWwiseAudioLinkSettingsProxy::CallOnEventLoaded: Event not loaded. Wait for load before calling callback."));
			return OnEventLoadedDelegate.Add(FSimpleMulticastDelegate::FDelegate::CreateLambda(MoveTemp(InCallback)));
		}
	}
	
	AsyncTask(ENamedThreads::GameThread, [InCallback = MoveTemp(InCallback)]
	{
		InCallback();
	});
	return FDelegateHandle();
}

void FWwiseAudioLinkSettingsProxy::UnregisterEventLoadedDelegate(const FDelegateHandle& InDelegate)
{
	FScopeLock Lock(&CriticalSection);
	OnEventLoadedDelegate.Remove(InDelegate);
}

void FWwiseAudioLinkSettingsProxy::NotifyEventDataLoaded()
{
	{
		FScopeLock Lock(&CriticalSection);
		bIsEventDataLoaded = true;
	}
	OnEventLoadedDelegate.Broadcast();
}

#if WITH_EDITOR
void FWwiseAudioLinkSettingsProxy::RefreshFromSettings(UAudioLinkSettingsAbstract* InSettings, FPropertyChangedEvent&)
{
	Update(*CastChecked<UWwiseAudioLinkSettings>(InSettings));
}
#endif //WITH_EDITOR

FName UWwiseAudioLinkSettings::GetFactoryName() const
{
	return FWwiseAudioLinkFactory::GetFactoryNameStatic();
}

bool UWwiseAudioLinkSettings::GetSettingsFromWwise(FWwiseSettings& OutSettings) const
{
	if (const UAkInitializationSettings* InitializationSettings = FAkPlatform::GetInitializationSettings())
	{
		OutSettings.NumSamplesPerBuffer = InitializationSettings->CommonSettings.SamplesPerFrame;
#if PLATFORM_WINDOWS
		OutSettings.NumSamplesPerSecond = InitializationSettings->CommonSettings.SampleRate;
#else //PLATFORM_WINDOWS
		OutSettings.NumSamplesPerSecond = 48000;
#endif //PLATFORM_WINDOWS
		return true;
	}
	return false;
}

void UWwiseAudioLinkSettings::PostLoad()
{
	RequestLoad();
	Super::PostLoad();
}

void UWwiseAudioLinkSettings::RequestLoad() const
{
	if (bLoadRequested)
	{
		return;
	}
	bLoadRequested = true;
	AsyncTask(ENamedThreads::GameThread, [WeakThis = MakeWeakObjectPtr(const_cast<UWwiseAudioLinkSettings*>(this))]() 
	{
		if (UNLIKELY(!WeakThis.IsValid()))
		{
			UE_LOG(LogWwiseAudioLink, Warning, TEXT("UWwiseAudioLinkSettings::RequestLoad: Invalid weak pointer."));
			return;
		}

		UE_LOG(LogWwiseAudioLink, VeryVerbose, TEXT("UWwiseAudioLinkSettings::RequestLoad: Loading Settings '%s' StartEvent '%s'"), *WeakThis->GetName(), *WeakThis->StartEvent.ToSoftObjectPath().GetAssetName());
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		const FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(WeakThis.Get(), &UWwiseAudioLinkSettings::OnLoadCompleteCallback);
		WeakThis->LoadingHandle = StreamableManager.RequestAsyncLoad(
			WeakThis->StartEvent.ToSoftObjectPath(),
			Delegate,
			FStreamableManager::AsyncLoadHighPriority,
			true // Managed active handle
		);
	});
}

void UWwiseAudioLinkSettings::OnLoadCompleteCallback()
{
	TArray<UObject*> LoadedAssets;
	LoadingHandle->GetLoadedAssets(LoadedAssets);
	if (UNLIKELY(LoadedAssets.Num() == 0))
	{
		UE_LOG(LogWwiseAudioLink, Warning, TEXT("UWwiseAudioLinkSettings::OnLoadCompleteCallback: Could not load asset for Settings '%s' StartEvent '%s'"), *GetName(), *StartEvent.ToSoftObjectPath().GetAssetName());
	}
	else if (UNLIKELY(LoadedAssets.Num() > 1))
	{
		UE_LOG(LogWwiseAudioLink, Warning, TEXT("UWwiseAudioLinkSettings::OnLoadCompleteCallback: Loaded more than one (%d) asset for Settings '%s' StartEvent '%s'. Using first one."), LoadedAssets.Num(), *GetName(), *StartEvent.ToSoftObjectPath().GetAssetName());
		StartEventResolved = CastChecked<UAkAudioEvent>(LoadedAssets[0]);
	}
	else
	{
		UE_LOG(LogWwiseAudioLink, Verbose, TEXT("UWwiseAudioLinkSettings::OnLoadCompleteCallback: Asset loaded for Settings '%s' StartEvent '%s'"), *GetName(), *StartEvent.ToSoftObjectPath().GetAssetName());
		StartEventResolved = CastChecked<UAkAudioEvent>(LoadedAssets[0]);
	}

	if (IsValid(StartEventResolved))
	{
		StartEventResolved->AddToRoot();
	}

	GetCastProxy<FWwiseAudioLinkSettingsProxy>()->NotifyEventDataLoaded();
	LoadingHandle.Reset();
}

void UWwiseAudioLinkSettings::FinishDestroy()
{
	if (IsValid(StartEventResolved))
	{
		StartEventResolved->RemoveFromRoot();
	}
	
	Super::FinishDestroy();
}

int32 UWwiseAudioLinkSettings::GetReceivingBufferSizeInFrames() const
{ 
	// Ask Wwise for it's setting for buffer size, which is per platform.
	FWwiseSettings SettingsFromWwise;
	if (GetSettingsFromWwise(SettingsFromWwise))
	{
		return SettingsFromWwise.NumSamplesPerBuffer; 
	}
	
	static const int32 SensibleDefaultSize = 1024;
	UE_LOG(LogWwiseAudioLink, Warning, TEXT("UWwiseAudioLinkSettings::GetReceivingBufferSizeInFrames: Failed to get Wwise settings for buffer sizes, so using a default of '%d'"), SensibleDefaultSize);
	return SensibleDefaultSize;
}

