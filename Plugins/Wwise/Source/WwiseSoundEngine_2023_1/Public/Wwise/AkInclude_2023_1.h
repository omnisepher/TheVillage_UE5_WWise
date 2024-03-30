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

#include "CoreTypes.h"

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT
#pragma pack(push)
#pragma warning(push)
#pragma warning(disable: 4103)		// alignment changed after including header, may be due to missing #pragma pack(pop)
#endif // PLATFORM_MICROSOFT

#include "Wwise/PreSoundEngineInclude.h"

#include <AK/AkWwiseSDKVersion.h>
#include <AK/SoundEngine/Common/AkCommonDefs.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>
#include <AK/IBytes.h>
#include <AK/SoundEngine/Common/AkVirtualAcoustics.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkMemoryMgrFuncs.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/AkNumeralTypes.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/Tools/Common/AkArray.h>
#include <AK/Tools/Common/AkInstrument.h>
#include <AK/Tools/Common/AkMonitorError.h>
#include <AK/Tools/Common/AkObject.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AK/Tools/Common/AkProfilingID.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/Comm/AkCommunication.h>
#include <AK/Plugin/AkAudioInputPlugin.h>
#include <AK/SoundEngine/Common/AkQueryParameters.h>

#if AK_SUPPORT_WAAPI
#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
#include <AK/WwiseAuthoringAPI/waapi.h>

#if WITH_EDITORONLY_DATA
#include <AK/WwiseAuthoringAPI/AkAutobahn/AkWAAPIErrorMessageTranslator.h>
#else
class AkErrorMessageTranslator;
#endif

#else // No WAAPI
#if WITH_EDITORONLY_DATA
#include <AK/SoundEngine/Common/AkErrorMessageTranslator.h>
#else
class AkErrorMessageTranslator;
#endif
#endif

#include "Wwise/PostSoundEngineInclude.h"

#if defined(PLATFORM_MICROSOFT) && PLATFORM_MICROSOFT
#pragma warning(pop)
#pragma pack(pop)
#endif // PLATFORM_MICROSOFT
