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

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("SoundEngine"), STATGROUP_WwiseSoundEngine, STATCAT_Wwise);

WWISESOUNDENGINE_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseSoundEngine, Log, All);

#define SCOPED_WWISESOUNDENGINE_EVENT(Text) SCOPED_WWISE_NAMED_EVENT(TEXT("WwiseSoundEngine"), Text)
#define SCOPED_WWISESOUNDENGINE_EVENT_2(Text) SCOPED_WWISE_NAMED_EVENT_2(TEXT("WwiseSoundEngine"), Text)
#define SCOPED_WWISESOUNDENGINE_EVENT_3(Text) SCOPED_WWISE_NAMED_EVENT_3(TEXT("WwiseSoundEngine"), Text)
#define SCOPED_WWISESOUNDENGINE_EVENT_4(Text) SCOPED_WWISE_NAMED_EVENT_4(TEXT("WwiseSoundEngine"), Text)
#define SCOPED_WWISESOUNDENGINE_EVENT_F(Format, ...) SCOPED_WWISE_NAMED_EVENT_F(TEXT("WwiseSoundEngine"), Format, __VA_ARGS__)
#define SCOPED_WWISESOUNDENGINE_EVENT_F_2(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_2(TEXT("WwiseSoundEngine"), Format, __VA_ARGS__)
#define SCOPED_WWISESOUNDENGINE_EVENT_F_3(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_3(TEXT("WwiseSoundEngine"), Format, __VA_ARGS__)
#define SCOPED_WWISESOUNDENGINE_EVENT_F_4(Format, ...) SCOPED_WWISE_NAMED_EVENT_F_4(TEXT("WwiseSoundEngine"), Format, __VA_ARGS__)
