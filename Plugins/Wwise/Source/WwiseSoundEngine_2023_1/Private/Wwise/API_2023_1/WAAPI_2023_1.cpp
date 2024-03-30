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

#include "Wwise/API_2023_1/WAAPI_2023_1.h"
#include "Wwise/Stats/SoundEngine_2023_1.h"

#if AK_SUPPORT_WAAPI

bool FWAAPI_2023_1::Client_2023_1::Connect(const char* in_uri, unsigned in_port,
	AK::WwiseAuthoringAPI::disconnectHandler_t disconnectHandler, int in_timeoutMs)
{
	return Client.Connect(in_uri, in_port, disconnectHandler, in_timeoutMs);
}

bool FWAAPI_2023_1::Client_2023_1::IsConnected() const
{
	return Client.IsConnected();
}

void FWAAPI_2023_1::Client_2023_1::Disconnect()
{
	Client.Disconnect();
}

bool FWAAPI_2023_1::Client_2023_1::Subscribe(const char* in_uri, const char* in_options,
	AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback, uint64_t& out_subscriptionId, std::string& out_result,
	int in_timeoutMs)
{
	return Client.Subscribe(in_uri, in_options, in_callback, out_subscriptionId, out_result, in_timeoutMs);
}

bool FWAAPI_2023_1::Client_2023_1::Subscribe(const char* in_uri, const AK::WwiseAuthoringAPI::AkJson& in_options,
	AK::WwiseAuthoringAPI::Client::WampEventCallback in_callback, uint64_t& out_subscriptionId,
	AK::WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs)
{
	return Client.Subscribe(in_uri, in_options, in_callback, out_subscriptionId, out_result, in_timeoutMs);
}

bool FWAAPI_2023_1::Client_2023_1::Unsubscribe(const uint64_t& in_subscriptionId, std::string& out_result,
	int in_timeoutMs)
{
	return Client.Unsubscribe(in_subscriptionId, out_result, in_timeoutMs);
}

bool FWAAPI_2023_1::Client_2023_1::Unsubscribe(const uint64_t& in_subscriptionId,
	AK::WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs)
{
	return Client.Unsubscribe(in_subscriptionId, out_result, in_timeoutMs);
}

bool FWAAPI_2023_1::Client_2023_1::Call(const char* in_uri, const char* in_args, const char* in_options,
	std::string& out_result, int in_timeoutMs)
{
	return Client.Call(in_uri, in_args, in_options, out_result, in_timeoutMs);
}

bool FWAAPI_2023_1::Client_2023_1::Call(const char* in_uri, const AK::WwiseAuthoringAPI::AkJson& in_args,
	const AK::WwiseAuthoringAPI::AkJson& in_options, AK::WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs)
{
	return Client.Call(in_uri, in_args, in_options, out_result, in_timeoutMs);
}

IWAAPI::Client* FWAAPI_2023_1::NewClient()
{
	return new Client_2023_1;
}

std::string FWAAPI_2023_1::GetJsonString(const AK::WwiseAuthoringAPI::JsonProvider& InJsonProvider)
{
	return InJsonProvider.GetJsonString();
}

#endif
