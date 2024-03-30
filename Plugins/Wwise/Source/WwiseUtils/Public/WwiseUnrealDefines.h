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

#include "Runtime/Launch/Resources/Version.h"

#define UE_4_26_OR_LATER ((ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26) || (ENGINE_MAJOR_VERSION >= 5))
#define UE_4_27_OR_LATER ((ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27) || (ENGINE_MAJOR_VERSION >= 5))
#define UE_5_0_OR_LATER   (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 0)
#define UE_5_1_OR_LATER   (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
#define UE_5_2_OR_LATER   (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2)
#define UE_5_3_OR_LATER   (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)

// PhysX (deprecated in UE5.0) and Chaos (mandatory in UE5.1) selection
#if UE_5_1_OR_LATER
#define AK_USE_CHAOS 1
#define AK_USE_PHYSX 0
#else
#if defined(PHYSICS_INTERFACE_PHYSX) && PHYSICS_INTERFACE_PHYSX
#define AK_USE_PHYSX 1
#else
#define AK_USE_PHYSX 0
#endif
#if defined(WITH_CHAOS) && WITH_CHAOS
#define AK_USE_CHAOS 1
#else
#define AK_USE_CHAOS 0
#endif
#endif