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

#include "CoreMinimal.h"
#include "AkInclude.h"

#include "Wwise/WwiseSoundEngineModule.h"

class WWISESOUNDENGINE_API IWAAPI
{
public:
	inline static IWAAPI* Get()
	{
		IWwiseSoundEngineModule::ForceLoadModule();
		return IWwiseSoundEngineModule::WAAPI;
	}

	UE_NONCOPYABLE(IWAAPI);
protected:
	IWAAPI() = default;
public:
	virtual ~IWAAPI() {}

#if AK_SUPPORT_WAAPI
	class WWISESOUNDENGINE_API Client
	{
	public:
		UE_NONCOPYABLE(Client);
		Client() {}
		virtual ~Client() {}

		virtual bool Connect(const char* in_uri, unsigned int in_port, AK::WwiseAuthoringAPI::disconnectHandler_t disconnectHandler = nullptr, int in_timeoutMs = -1) = 0;
		virtual bool IsConnected() const = 0;
		virtual void Disconnect() = 0;

		virtual bool Subscribe(const char* in_uri, const char* in_options, AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback, uint64_t& out_subscriptionId, std::string& out_result, int in_timeoutMs = -1) = 0;
		virtual bool Subscribe(const char* in_uri, const AK::WwiseAuthoringAPI::AkJson& in_options, AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback, uint64_t& out_subscriptionId, AK::WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs = -1) = 0;
		
		virtual bool Unsubscribe(const uint64_t& in_subscriptionId, std::string& out_result, int in_timeoutMs = -1) = 0;
		virtual bool Unsubscribe(const uint64_t& in_subscriptionId, AK::WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs = -1) = 0;

		virtual bool Call(const char* in_uri, const char* in_args, const char* in_options, std::string& out_result, int in_timeoutMs = -1) = 0;
		virtual bool Call(const char* in_uri, const AK::WwiseAuthoringAPI::AkJson& in_args, const AK::WwiseAuthoringAPI::AkJson& in_options, AK::WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs = -1) = 0;
	};
	virtual Client* NewClient() = 0;

	virtual std::string GetJsonString(const AK::WwiseAuthoringAPI::JsonProvider&) = 0;
#endif		// AK_SUPPORT_WAAPI
};
