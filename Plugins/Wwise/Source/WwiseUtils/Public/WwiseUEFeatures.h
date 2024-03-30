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

// Defines which features of the Wwise-Unreal integration are supported in which version of UE.

#pragma once

#include "WwiseUnrealDefines.h"
#include "Containers/Ticker.h"

// Styling naming changed between UE4 and UE5.
#if WITH_EDITOR
#if UE_5_0_OR_LATER
#include "Styling/AppStyle.h"
using FAkAppStyle = FAppStyle;
#else
#include "EditorStyleSet.h"
using FAkAppStyle = FEditorStyle;
#endif
#endif

// UE 5.0 typedefs
#if UE_5_0_OR_LATER
using FUnrealFloatVector = FVector3f;
using FUnrealFloatVector2D = FVector2f;
using FUnrealFloatPlane = FPlane4f;
using FTickerDelegateHandle = FTSTicker::FDelegateHandle;
using FCoreTickerType = FTSTicker;
#else
using FUnrealFloatVector = FVector;
using FUnrealFloatVector2D = FVector2D;
using FCoreTickerType = FTicker;
using FUnrealFloatPlane = FPlane;
using FTickerDelegateHandle = FDelegateHandle;
#endif


