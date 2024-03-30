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
#include "Wwise/Stats/NamedEvents.h"

DECLARE_STATS_GROUP(TEXT("Processing"), STATGROUP_WwiseProcessing, STATCAT_Wwise);

WWISEPROCESSING_API DECLARE_LOG_CATEGORY_EXTERN(LogWwiseProcessing, Log, All);

#define SCOPED_WWISEPROCESSING_EVENT_4(Text) SCOPED_WWISE_NAMED_EVENT_4(TEXT("WwiseProcessing"), Text)
