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

#include "Wwise/WwiseSoundEngineModule.h"
#include "Wwise/API/WwiseCommAPI.h"
#include "Wwise/API/WwiseMemoryMgrAPI.h"
#include "Wwise/API/WwiseMonitorAPI.h"
#include "Wwise/API/WwiseMusicEngineAPI.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/API/WwiseSpatialAudioAPI.h"
#include "Wwise/API/WwiseStreamMgrAPI.h"
#include "Wwise/API/WwisePlatformAPI.h"
#include "Wwise/API/WAAPI.h"
#include "Wwise/Stats/SoundEngine.h"

#include WWISE_SOUNDENGINE_VERSION_HEADER_PATH

#include "Misc/ScopeLock.h"
#include "Wwise/WwiseAssertHook.h"

IMPLEMENT_MODULE(FWwiseSoundEngineModule, WwiseSoundEngine)

IWwiseCommAPI* IWwiseSoundEngineModule::Comm = nullptr;
IWwiseMemoryMgrAPI* IWwiseSoundEngineModule::MemoryMgr = nullptr;
IWwiseMonitorAPI* IWwiseSoundEngineModule::Monitor = nullptr;
IWwiseMusicEngineAPI* IWwiseSoundEngineModule::MusicEngine = nullptr;
IWwiseSoundEngineAPI* IWwiseSoundEngineModule::SoundEngine = nullptr;
IWwiseSpatialAudioAPI* IWwiseSoundEngineModule::SpatialAudio = nullptr;
IWwiseStreamMgrAPI* IWwiseSoundEngineModule::StreamMgr = nullptr;

IWwisePlatformAPI* IWwiseSoundEngineModule::Platform = nullptr;
IWAAPI* IWwiseSoundEngineModule::WAAPI = nullptr;

IWwiseSoundEngineVersionModule* IWwiseSoundEngineModule::VersionInterface = nullptr;

void FWwiseSoundEngineModule::StartupModule()
{
	UE_LOG(LogWwiseSoundEngine, Display, TEXT("Loading Wwise Sound Engine version %s"), TEXT(AK_WWISE_SOUNDENGINE_VERSION));

	VersionInterface = new WWISE_SOUNDENGINE_VERSION_CLASS;

	Comm = VersionInterface->GetComm();
	MemoryMgr = VersionInterface->GetMemoryMgr();
	Monitor = VersionInterface->GetMonitor();
	MusicEngine = VersionInterface->GetMusicEngine();
	SoundEngine = VersionInterface->GetSoundEngine();
	SpatialAudio = VersionInterface->GetSpatialAudio();
	StreamMgr = VersionInterface->GetStreamMgr();

	Platform = VersionInterface->GetPlatform();
	WAAPI = VersionInterface->GetWAAPI();

#if defined(AK_ENABLE_ASSERTS)
	g_pAssertHook = WwiseAssertHook;
#endif
}

void FWwiseSoundEngineModule::ShutdownModule()
{
	DeleteInterface();
}

void FWwiseSoundEngineModule::DeleteInterface()
{
	static FCriticalSection CriticalSection;
	FScopeLock Lock(&CriticalSection);

	if (!VersionInterface)
	{
		return;
	}
	delete VersionInterface; VersionInterface = nullptr;

	if (LIKELY(SoundEngine) && UNLIKELY(SoundEngine->IsInitialized()))
	{
		UE_LOG(LogWwiseSoundEngine, Error, TEXT("Shutting down Sound Engine module without deinitializing Low-Level Sound Engine"));
		SoundEngine->Term();
	}

	UE_LOG(LogWwiseSoundEngine, Log, TEXT("Unloading Wwise Sound Engine"));
	delete Comm; Comm = nullptr;
	delete MemoryMgr; MemoryMgr = nullptr;
	delete Monitor; Monitor = nullptr;
	delete MusicEngine; MusicEngine = nullptr;
	delete SoundEngine; SoundEngine = nullptr;
	delete SpatialAudio; SpatialAudio = nullptr;
	delete StreamMgr; StreamMgr = nullptr;

	delete Platform; Platform = nullptr;
	delete WAAPI; WAAPI = nullptr;
}
