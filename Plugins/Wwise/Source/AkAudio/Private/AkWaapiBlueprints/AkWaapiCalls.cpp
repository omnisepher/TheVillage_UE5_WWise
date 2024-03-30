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
	AkWaapiCalls.cpp:
=============================================================================*/

#include "AkWaapiBlueprints/AkWaapiCalls.h"
#include "AkAudioDevice.h"
#include "AkWaapiBlueprints/AkWaapiUriCustomization.h"
#include "AkWaapiBlueprints/AkWaapiFieldNamesCustomization.h"
#include "Async/Async.h"
#include "Modules/ModuleManager.h"

/*-----------------------------------------------------------------------------
	AkWaapiCalls.
-----------------------------------------------------------------------------*/


UAkWaapiCalls::UAkWaapiCalls(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
#if WITH_EDITOR
	// Property initialization
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("AkWaapiUri", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAkWaapiUriCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("AkWaapiFieldNames", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAkWaapiFieldNamesCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
#endif
}

void UAkWaapiCalls::SetSubscriptionID(const FAkWaapiSubscriptionId& Subscription, int id)
{
    Subscription.SubscriptionId = (uint64_t)id;
}

int UAkWaapiCalls::GetSubscriptionID(const FAkWaapiSubscriptionId& Subscription)
{
    return Subscription.SubscriptionId;
}

bool UAkWaapiCalls::RegisterWaapiProjectLoadedCallback(const FOnWaapiProjectLoaded& Callback)
{
    if (auto waapiClient = FAkWaapiClient::Get())
    {
        waapiClient->OnProjectLoaded.AddLambda([Callback]() { Callback.ExecuteIfBound(); });
        return true;
    }
    return false;
}

bool UAkWaapiCalls::RegisterWaapiConnectionLostCallback(const FOnWaapiConnectionLost& Callback)
{
    if (auto waapiClient = FAkWaapiClient::Get())
    {
        waapiClient->OnConnectionLost.AddLambda([Callback]() 
        { 
            Callback.ExecuteIfBound(); 
        });
        return true;
    }
    return false;
}

FAKWaapiJsonObject UAkWaapiCalls::CallWaapi(const FAkWaapiUri& WaapiUri, const FAKWaapiJsonObject& WaapiArgs, const FAKWaapiJsonObject& WaapiOptions)
{
	FAKWaapiJsonObject outJsonResult = FAKWaapiJsonObject();
	// Connect to Wwise Authoring on localhost.
    if (auto waapiClient = FAkWaapiClient::Get())
	{	
		// Request data from Wwise using WAAPI
		if (!waapiClient->Call(TCHAR_TO_ANSI(*WaapiUri.Uri), WaapiArgs.WaapiJsonObj.ToSharedRef(), WaapiOptions.WaapiJsonObj.ToSharedRef(), outJsonResult.WaapiJsonObj))
		{
			UE_LOG(LogAkAudio, Log, TEXT("Call Failed"));
		}
	}
	else
	{
		UE_LOG(LogAkAudio, Log, TEXT("Unable to connect to localhost"));
	}
	return outJsonResult;
}

FAKWaapiJsonObject UAkWaapiCalls::SubscribeToWaapi(const FAkWaapiUri& WaapiUri, const FAKWaapiJsonObject& WaapiOptions, const FOnEventCallback& CallBack, FAkWaapiSubscriptionId& SubscriptionId, bool& SubscriptionDone)
{
	FAKWaapiJsonObject outJsonResult = FAKWaapiJsonObject();

	// Connect to Wwise Authoring on localhost.
    if (auto waapiClient = FAkWaapiClient::Get())
	{
		auto wampEventCallback = WampEventCallback::CreateLambda([CallBack](uint64_t id, TSharedPtr<FJsonObject> in_UEJsonObject)
		{
			AsyncTask(ENamedThreads::GameThread,[CallBack,id,in_UEJsonObject] 
			{
				FAKWaapiJsonObject outWaapiObj = FAKWaapiJsonObject();
				outWaapiObj.WaapiJsonObj = in_UEJsonObject;
				CallBack.ExecuteIfBound(id, outWaapiObj);
			});
		});

		// Subscribe to action notifications.
		SubscriptionDone = waapiClient->Subscribe(TCHAR_TO_ANSI(*WaapiUri.Uri), WaapiOptions.WaapiJsonObj.ToSharedRef(), wampEventCallback, SubscriptionId.SubscriptionId, outJsonResult.WaapiJsonObj);
	}
	return outJsonResult;
}

FAKWaapiJsonObject UAkWaapiCalls::Unsubscribe(const FAkWaapiSubscriptionId& SubscriptionId, bool& UnsubscriptionDone)
{
	FAKWaapiJsonObject outJsonResult = FAKWaapiJsonObject();
	// Connect to Wwise Authoring on localhost.
    if (auto waapiClient = FAkWaapiClient::Get())
	{
		// Subscribe to action notifications.
		UnsubscriptionDone = waapiClient->Unsubscribe(SubscriptionId.SubscriptionId, outJsonResult.WaapiJsonObj);
	}
	return outJsonResult;
}

FString UAkWaapiCalls::Conv_FAkWaapiSubscriptionIdToString(const FAkWaapiSubscriptionId& INAkWaapiSubscriptionId)
{
	return FString::Printf(TEXT("%u"), INAkWaapiSubscriptionId.SubscriptionId);
}

FText UAkWaapiCalls::Conv_FAkWaapiSubscriptionIdToText(const FAkWaapiSubscriptionId& INAkWaapiSubscriptionId)
{
	return FText::FromString(*Conv_FAkWaapiSubscriptionIdToString(INAkWaapiSubscriptionId));
}