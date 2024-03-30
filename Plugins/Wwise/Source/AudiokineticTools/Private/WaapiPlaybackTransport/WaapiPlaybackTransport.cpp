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

#include "WaapiPlaybackTransport.h"

#include "AkWaapiUtils.h"
#include "IAudiokineticTools.h"
#include "Async/Async.h"


WaapiPlaybackTransport::~WaapiPlaybackTransport()
{
	StopAndDestroyAll();
}

int32 WaapiPlaybackTransport::FindOrAdd(const FGuid& InItemID)
{
	const FString itemIdStringField = InItemID.ToString(EGuidFormats::DigitsWithHyphensInBraces);
	TSharedPtr<FJsonObject> Result;
	int32 TransportID = -1;

	{
		FScopeLock AutoLock(&TransportItemsLock);
		if (TransportItems.Contains(InItemID))
		{
			return TransportItems[InItemID].TransportID;
		}
	}

#if AK_SUPPORT_WAAPI
	auto WaapiClient = FAkWaapiClient::Get();
	if(!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Unable to connect to Waapi"));
		return TransportID;
	}
	if (WaapiClient->Call(ak::wwise::core::transport::create, { { WwiseWaapiHelper::OBJECT, itemIdStringField } }, Result))
	{
		TransportID = Result->GetIntegerField(WwiseWaapiHelper::TRANSPORT);
		uint64 SubscriptionID = SubscribeToStateChanged(TransportID);

		{
			FScopeLock AutoLock(&TransportItemsLock);
			TransportItems.Add(InItemID, TransportInfo(TransportID, SubscriptionID));
		}
	}
#endif

	return TransportID;
}

void WaapiPlaybackTransport::Remove(const FGuid& InItemID)
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
		return;

	uint64 SubscriptionID;
	TransportInfo Item(0, 0);

	{
		FScopeLock AutoLock(&TransportItemsLock);
		if (!TransportItems.Contains(InItemID))
		{
			return;
		}

		Item = TransportItems[InItemID];

	}

	SubscriptionID = Item.SubscriptionID;

	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	Args->SetNumberField(WwiseWaapiHelper::TRANSPORT, Item.TransportID);

	TSharedPtr<FJsonObject> Result;
	if (SubscriptionID != 0)
	{
		WaapiClient->Unsubscribe(SubscriptionID, Result);
	}

#if AK_SUPPORT_WAAPI
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	if (WaapiClient->Call(ak::wwise::core::transport::destroy, Args, Options, Result))
	{
		{
			FScopeLock AutoLock(&TransportItemsLock);
			TransportItems.Remove(InItemID);
		}
	}
#endif
}

void WaapiPlaybackTransport::TogglePlay(int32 InTransportID)
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Unable to connect to localhost"));
		return;
	}

	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	Args->SetStringField(WwiseWaapiHelper::ACTION, WwiseWaapiHelper::PLAYSTOP);
	Args->SetNumberField(WwiseWaapiHelper::TRANSPORT, InTransportID);

#if AK_SUPPORT_WAAPI
	
	TSharedPtr<FJsonObject> Result;
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	if (!WaapiClient->Call(ak::wwise::core::transport::executeAction, Args, Options, Result))
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to trigger playback"));
	}
#endif
}

void WaapiPlaybackTransport::Stop(int32 InTransportID)
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
		return;

	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	Args->SetStringField(WwiseWaapiHelper::ACTION, WwiseWaapiHelper::STOP);
	Args->SetNumberField(WwiseWaapiHelper::TRANSPORT, InTransportID);

#if AK_SUPPORT_WAAPI
	TSharedPtr<FJsonObject> Result;
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	if (!WaapiClient->Call(ak::wwise::core::transport::executeAction, Args, Options, Result))
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Cannot stop event."));
	}
#endif
}

void WaapiPlaybackTransport::StopAndDestroyAll()
{
	FScopeLock AutoLock(&TransportItemsLock);

	for (auto Iter = TransportItems.CreateIterator(); Iter; ++Iter)
	{
		Stop(Iter.Value().TransportID);
		Remove(Iter.Key());
	}

	TransportItems.Empty();
}

bool WaapiPlaybackTransport::IsPlaying(const FGuid& InItemID)
{
	if (!FAkWaapiClient::Get())
	{
		return false;
	}
	
	return TransportItems.Contains(InItemID);
}

uint64 WaapiPlaybackTransport::SubscribeToStateChanged(int32 TransportID)
{
	auto WaapiClient = FAkWaapiClient::Get();

	if (!WaapiClient)
		return 0;

	auto WampEventCallback = WampEventCallback::CreateLambda( 
		[this](uint64_t ID, TSharedPtr<FJsonObject> UEJsonObject)
		{
			AsyncTask(ENamedThreads::AnyThread, [this, UEJsonObject]
			{
				this->OnStateChanged(UEJsonObject);
			});
		});

	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	Options->SetNumberField(WwiseWaapiHelper::TRANSPORT, TransportID);

	TSharedPtr<FJsonObject> OutJsonResult;
	uint64 SubscriptionID = 0;
#if AK_SUPPORT_WAAPI
	WaapiClient->Subscribe(ak::wwise::core::transport::stateChanged, Options, WampEventCallback, SubscriptionID, OutJsonResult);
#endif
	return SubscriptionID;
}

void WaapiPlaybackTransport::OnStateChanged(TSharedPtr<FJsonObject> InUEJsonObject)
{
	const FString NewState = InUEJsonObject->GetStringField(WwiseWaapiHelper::STATE);
	FGuid ItemID;
	FGuid::Parse(InUEJsonObject->GetStringField(WwiseWaapiHelper::OBJECT), ItemID);
	const int32 TransportID = InUEJsonObject->GetNumberField(WwiseWaapiHelper::TRANSPORT);

	if (NewState == WwiseWaapiHelper::STOPPED)
	{
		Remove(ItemID);
	}

	else if (NewState == WwiseWaapiHelper::PLAYING)
	{
		{
			FScopeLock AutoLock(&TransportItemsLock);
			if (!TransportItems.Contains(ItemID))
			{
				TransportItems.Add(ItemID, TransportInfo(TransportID, 0));
			}
		}
	}
}
