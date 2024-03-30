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

#include "Wwise/Ref/WwiseRefForwardDeclarations.h"
#include "Wwise/WwiseDatabaseIdentifiers.h"

#include "CoreMinimal.h"

using WwiseLanguageNamesMap = TMap<FName, FWwiseRefLanguage>;
using WwiseLanguageIdsMap = TMap<uint32, FWwiseRefLanguage>;
using WwisePlatformNamesMap = TMap<FName, FWwiseRefPlatform>;
using WwisePlatformGuidsMap = TMap<FGuid, FWwiseRefPlatform>;
using WwiseMediaIdsMap = TMap<uint32, FWwiseRefMedia>;
using WwisePluginLibNamesMap = TMap<FName, FWwiseRefPluginLib>;
using WwisePluginLibIdsMap = TMap<uint32, FWwiseRefPluginLib>;
using WwiseSoundBankIdsMap = TMap<uint32, FWwiseRefSoundBank>;
using WwiseDialogueEventIdsMap = TMap<uint32, FWwiseRefDialogueEvent>;
using WwiseDialogueArgumentIdsMap = TMap<uint32, FWwiseRefDialogueArgument>;
using WwiseBusIdsMap = TMap<uint32, FWwiseRefBus>;
using WwiseAuxBusIdsMap = TMap<uint32, FWwiseRefAuxBus>;
using WwiseCustomPluginIdsMap = TMap<uint32, FWwiseRefCustomPlugin>;
using WwisePluginShareSetIdsMap = TMap<uint32, FWwiseRefPluginShareSet>;
using WwiseAudioDeviceIdsMap = TMap<uint32, FWwiseRefAudioDevice>;
using WwiseEventIdsMap = TMap<uint32, FWwiseRefEvent>;
using WwiseExternalSourceIdsMap = TMap<uint32, FWwiseRefExternalSource>;
using WwiseAcousticTextureIdsMap = TMap<uint32, FWwiseRefAcousticTexture>;
using WwiseGameParameterIdsMap = TMap<uint32, FWwiseRefGameParameter>;
using WwiseStateGroupIdsMap = TMap<uint32, FWwiseRefStateGroup>;
using WwiseSwitchGroupIdsMap = TMap<uint32, FWwiseRefSwitchGroup>;
using WwiseTriggerIdsMap = TMap<uint32, FWwiseRefTrigger>;
using WwiseStateIdsMap = TMap<FWwiseDatabaseGroupValueKey, FWwiseRefState>;
using WwiseSwitchIdsMap = TMap<FWwiseDatabaseGroupValueKey, FWwiseRefSwitch>;
using WwiseSwitchContainerArray = TArray<FWwiseRefSwitchContainer>;

using WwiseMediaGlobalIdsMap = TMap<FWwiseDatabaseMediaIdKey, FWwiseRefMedia>;
using WwiseSoundBankGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefSoundBank>;
using WwiseDialogueEventGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefDialogueEvent>;
using WwiseDialogueArgumentGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefDialogueArgument>;
using WwiseBusGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefBus>;
using WwiseAuxBusGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefAuxBus>;
using WwiseCustomPluginGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefCustomPlugin>;
using WwisePluginShareSetGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefPluginShareSet>;
using WwisePluginLibGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefPluginLib>;
using WwiseAudioDeviceGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefAudioDevice>;
using WwiseEventGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefEvent>;
using WwiseExternalSourceGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefExternalSource>;
using WwiseAcousticTextureGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefAcousticTexture>;
using WwiseGameParameterGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefGameParameter>;
using WwiseStateGroupGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefStateGroup>;
using WwiseSwitchGroupGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefSwitchGroup>;
using WwiseTriggerGlobalIdsMap = TMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefTrigger>;
using WwiseStateGlobalIdsMap = TMap<FWwiseDatabaseLocalizableGroupValueKey, FWwiseRefState>;
using WwiseSwitchGlobalIdsMap = TMap<FWwiseDatabaseLocalizableGroupValueKey, FWwiseRefSwitch>;
using WwiseSwitchContainersByEvent = TMultiMap<FWwiseDatabaseLocalizableIdKey, FWwiseRefSwitchContainer>;

using WwiseGuidMap = TMultiMap<FWwiseDatabaseLocalizableGuidKey, FWwiseAnyRef>;
using WwiseNameMap = TMultiMap<FWwiseDatabaseLocalizableNameKey, FWwiseAnyRef>;
