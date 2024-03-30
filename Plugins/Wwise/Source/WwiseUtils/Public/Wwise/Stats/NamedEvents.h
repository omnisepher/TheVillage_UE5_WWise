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

#include "HAL/PlatformMisc.h"
#include "Logging/LogMacros.h"

namespace WwiseNamedEvents
{
	/**
	 * @brief Highest level color. Used for user-executed operations, and high-level operations. Also used for SoundEngine operations. The Wwise Color.
	*/
	extern WWISEUTILS_API const FColor Color1;

	/**
	 * @brief Lower level color. Used for inner algorithms as defined by high-level operations, recognizable by users.
	*/
	extern WWISEUTILS_API const FColor Color2;

	/**
	 * @brief Accessory, dulled-down color. Used for main asynchronous operations, such as loading and unloading.
	*/
	extern WWISEUTILS_API const FColor Color3;

	/**
	 * @brief Accessory, off color. Used for containers of other operations, lowest-level calls, and Synchronous waiting spots (Futures, Getters, EventRef Wait).
	*/
	extern WWISEUTILS_API const FColor Color4;
}

#define SCOPED_WWISE_NAMED_EVENT_F(Prefix, Format, ...) SCOPED_NAMED_EVENT_F(TEXT("%s ") Format, WwiseNamedEvents::Color1, Prefix, __VA_ARGS__)
#define SCOPED_WWISE_NAMED_EVENT_F_2(Prefix, Format, ...) SCOPED_NAMED_EVENT_F(TEXT("%s ") Format, WwiseNamedEvents::Color2, Prefix, __VA_ARGS__)
#define SCOPED_WWISE_NAMED_EVENT_F_3(Prefix, Format, ...) SCOPED_NAMED_EVENT_F(TEXT("%s ") Format, WwiseNamedEvents::Color3, Prefix, __VA_ARGS__)
#define SCOPED_WWISE_NAMED_EVENT_F_4(Prefix, Format, ...) SCOPED_NAMED_EVENT_F(TEXT("%s ") Format, WwiseNamedEvents::Color4, Prefix, __VA_ARGS__)

#define SCOPED_WWISE_NAMED_EVENT(Prefix, Text) SCOPED_WWISE_NAMED_EVENT_F(Prefix, TEXT("%s"), Text)
#define SCOPED_WWISE_NAMED_EVENT_2(Prefix, Text) SCOPED_WWISE_NAMED_EVENT_F_2(Prefix, TEXT("%s"), Text)
#define SCOPED_WWISE_NAMED_EVENT_3(Prefix, Text) SCOPED_WWISE_NAMED_EVENT_F_3(Prefix, TEXT("%s"), Text)
#define SCOPED_WWISE_NAMED_EVENT_4(Prefix, Text) SCOPED_WWISE_NAMED_EVENT_F_4(Prefix, TEXT("%s"), Text)
