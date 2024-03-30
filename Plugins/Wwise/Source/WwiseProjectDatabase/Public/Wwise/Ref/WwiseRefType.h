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

UENUM()
enum class EWwiseRefType : uint32
{
	RootFile,
		
		ProjectInfo,
			Language,

		PlatformInfo,
			Platform,
		
		PluginInfo,
			PluginLib,

		SoundBanksInfo,
			SoundBank,
				Media,
				CustomPlugin,
				PluginShareSet,
				AudioDevice,
				Event,
					SwitchContainer,
				DialogueEvent,
					DialogueArgument,
				Bus,
				AuxBus,
				GameParameter,
				StateGroup,
					State,
				SwitchGroup,
					Switch,
				Trigger,
				ExternalSource,
				AcousticTexture,

	None = (uint32)-1
};

using WwiseRefIndexType = int32;
