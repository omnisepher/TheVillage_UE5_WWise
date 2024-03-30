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

#include "InitializationSettings/AkInitializationSettings.h"

#include "Platforms/AkUEPlatform.h"
#include "HAL/PlatformMemory.h"
#include "Misc/App.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "ProfilingDebugging/MiscTrace.h"

#include "AkAudioDevice.h"
#include "Wwise/WwiseAssertHook.h"
#include "Wwise/WwiseIOHook.h"
#include "Wwise/API/WwiseCommAPI.h"
#include "Wwise/API/WwiseMemoryMgrAPI.h"
#include "Wwise/API/WwiseMonitorAPI.h"
#include "Wwise/API/WwiseMusicEngineAPI.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/API/WwiseSpatialAudioAPI.h"
#include "Wwise/API/WwiseStreamMgrAPI.h"
#include "Wwise/Stats/AkAudioMemory.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/WwiseGlobalCallbacks.h"
#include "WwiseDefines.h"

namespace AkInitializationSettings_Helpers
{
	enum { IsLoggingInitialization = true };

	// expected page size and alignment requirement for all general purpose commits
	const size_t kAkVmPageSize = 64 * 1024;


	void* AkMemAllocVM(size_t size, size_t* /*extra*/)
	{
		ASYNC_INC_MEMORY_STAT_BY(STAT_WwiseMemorySoundEngineVM, size);
		LLM_SCOPE_BYTAG(Wwise_SoundEngineMalloc);
		return FMemory::Malloc(size, kAkVmPageSize);
	}

	void AkMemFreeVM(void* address, size_t /*size*/, size_t /*extra*/, size_t release)
	{
		if (release)
		{
			FMemory::Free(address);
			ASYNC_DEC_MEMORY_STAT_BY(STAT_WwiseMemorySoundEngineVM, release);
		}
	}

	void AkProfilerPushTimer(AkPluginID in_uPluginID, const char* in_pszZoneName)
	{
		if (!in_pszZoneName)
		{
			in_pszZoneName = "(Unknown)";
		}

#if CPUPROFILERTRACE_ENABLED
		FCpuProfilerTrace::OutputBeginDynamicEvent(in_pszZoneName);
#endif
#if PLATFORM_IMPLEMENTS_BeginNamedEventStatic
		FPlatformMisc::BeginNamedEventStatic(WwiseNamedEvents::Color1, in_pszZoneName);
#else
		FPlatformMisc::BeginNamedEvent(WwiseNamedEvents::Color1, in_pszZoneName);
#endif
	}

	void AkProfilerPopTimer()
	{
		FPlatformMisc::EndNamedEvent();
#if CPUPROFILERTRACE_ENABLED
		FCpuProfilerTrace::OutputEndEvent();
#endif
	}

	void AkProfilerPostMarker(AkPluginID in_uPluginID, const char* in_pszMarkerName)
	{
		// Filter out audioFrameBoundary bookmarks, because those occur too frequently
		if (in_uPluginID != AKMAKECLASSID(AkPluginTypeNone, AKCOMPANYID_AUDIOKINETIC, AK::ProfilingID::AudioFrameBoundary))
		{
			TRACE_BOOKMARK(TEXT("AK Marker: %s"), in_pszMarkerName);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// FAkInitializationStructure

FAkInitializationStructure::FAkInitializationStructure()
{
	auto* Comm = IWwiseCommAPI::Get();
	auto* MemoryMgr = IWwiseMemoryMgrAPI::Get();
	auto* MusicEngine = IWwiseMusicEngineAPI::Get();
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	auto* StreamMgr = IWwiseStreamMgrAPI::Get();
	if (UNLIKELY(!Comm || !MemoryMgr || !MusicEngine || !SoundEngine || !StreamMgr)) return;

	MemoryMgr->GetDefaultSettings(MemSettings);
	MemSettings.pfAllocVM = AkInitializationSettings_Helpers::AkMemAllocVM;
	MemSettings.pfFreeVM = AkInitializationSettings_Helpers::AkMemFreeVM;
	MemSettings.uVMPageSize = AkInitializationSettings_Helpers::kAkVmPageSize;

	// AkSpanCount setting is available in 2022.1.10+, 2023.1.1+, and future versions.
	// Locking to spanCount_Small greatly reduces the amount of memory reserved by Wwise.
#if (WWISE_2022_1_OR_LATER && AK_WWISE_SOUNDENGINE_SUBMINOR_VERSION >= 10) || (WWISE_2023_1_OR_LATER && AK_WWISE_SOUNDENGINE_SUBMINOR_VERSION >= 1) || WWISE_2024_1_OR_LATER
	MemSettings.uVMSpanCount = AkSpanCount_Small;
	MemSettings.uDeviceSpanCount = AkSpanCount_Small;
#endif

	StreamMgr->GetDefaultSettings(StreamManagerSettings);

	StreamMgr->GetDefaultDeviceSettings(DeviceSettings);
#if !WWISE_2023_1_OR_LATER
	DeviceSettings.uSchedulerTypeFlags = AK_SCHEDULER_DEFERRED_LINED_UP;
#endif
	DeviceSettings.uMaxConcurrentIO = AK_UNREAL_MAX_CONCURRENT_IO;

	SoundEngine->GetDefaultInitSettings(InitSettings);
#ifdef AK_ENABLE_ASSERTS
	InitSettings.pfnAssertHook = WwiseAssertHook;
#endif
	InitSettings.eFloorPlane = AkFloorPlane_XY;
	InitSettings.fGameUnitsToMeters = 100.f;
	InitSettings.fnProfilerPushTimer = AkInitializationSettings_Helpers::AkProfilerPushTimer;
	InitSettings.fnProfilerPopTimer = AkInitializationSettings_Helpers::AkProfilerPopTimer;
	InitSettings.fnProfilerPostMarker = AkInitializationSettings_Helpers::AkProfilerPostMarker;
	
	SoundEngine->GetDefaultPlatformInitSettings(PlatformInitSettings);

	MusicEngine->GetDefaultInitSettings(MusicSettings);

#if AK_ENABLE_COMMUNICATION
	Comm->GetDefaultInitSettings(CommSettings);
#endif
}

FAkInitializationStructure::~FAkInitializationStructure()
{
	delete[] InitSettings.szPluginDLLPath;
}

void FAkInitializationStructure::SetPluginDllPath(const FString& PlatformArchitecture)
{
	if (PlatformArchitecture.IsEmpty())
	{
		InitSettings.szPluginDLLPath = nullptr;
		return;
	}

	auto Path = FAkPlatform::GetDSPPluginsDirectory(PlatformArchitecture);
	auto Length = Path.Len() + 1;
	AkOSChar* PluginDllPath = new AkOSChar[Length];
	AKPLATFORM::SafeStrCpy(PluginDllPath, TCHAR_TO_AK(*Path), Length);
	InitSettings.szPluginDLLPath = PluginDllPath;
}

//////////////////////////////////////////////////////////////////////////
// FAkMainOutputSettings

void FAkMainOutputSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine)) return;

	auto& OutputSettings = InitializationStructure.InitSettings.settingsMainOutput;

	auto ShareSetID = !AudioDeviceShareSet.IsEmpty() ? SoundEngine->GetIDFromString(TCHAR_TO_ANSI(*AudioDeviceShareSet)) : AK_INVALID_UNIQUE_ID;
	OutputSettings.audioDeviceShareset = ShareSetID;

	switch (ChannelConfigType)
	{
	case EAkChannelConfigType::Anonymous:
		OutputSettings.channelConfig.SetAnonymous(NumberOfChannels);
		break;

	case EAkChannelConfigType::Standard:
		OutputSettings.channelConfig.SetStandard(ChannelMask);
		break;

	case EAkChannelConfigType::Ambisonic:
		OutputSettings.channelConfig.SetAmbisonic(NumberOfChannels);
		break;
	}

	OutputSettings.ePanningRule = (AkPanningRule)PanningRule;
	OutputSettings.idDevice = DeviceID;
}


//////////////////////////////////////////////////////////////////////////
// FAkSpatialAudioSettings

void FAkSpatialAudioSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& SpatialAudioInitSettings = InitializationStructure.SpatialAudioInitSettings;
	SpatialAudioInitSettings.uMaxSoundPropagationDepth = MaxSoundPropagationDepth;
	SpatialAudioInitSettings.fMovementThreshold = MovementThreshold;
	SpatialAudioInitSettings.uNumberOfPrimaryRays = NumberOfPrimaryRays;
	SpatialAudioInitSettings.uMaxReflectionOrder = ReflectionOrder;
	SpatialAudioInitSettings.uMaxDiffractionOrder = DiffractionOrder;
#if WWISE_2023_1_OR_LATER
	SpatialAudioInitSettings.uMaxEmitterRoomAuxSends = MaxEmitterRoomAuxSends;
#endif
	SpatialAudioInitSettings.uDiffractionOnReflectionsOrder = DiffractionOnReflectionsOrder;
	SpatialAudioInitSettings.fMaxPathLength = MaximumPathLength;
	SpatialAudioInitSettings.fCPULimitPercentage = CPULimitPercentage;
	SpatialAudioInitSettings.uLoadBalancingSpread = LoadBalancingSpread;
	SpatialAudioInitSettings.bEnableGeometricDiffractionAndTransmission = EnableGeometricDiffractionAndTransmission;
	SpatialAudioInitSettings.bCalcEmitterVirtualPosition = CalcEmitterVirtualPosition;
}


//////////////////////////////////////////////////////////////////////////
// FAkCommunicationSettings

void FAkCommunicationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
#if AK_ENABLE_COMMUNICATION
	auto& CommSettings = InitializationStructure.CommSettings;
	CommSettings.ports.uDiscoveryBroadcast = DiscoveryBroadcastPort;
	CommSettings.ports.uCommand = CommandPort;

	const FString GameName = GetCommsNetworkName();
	FCStringAnsi::Strcpy(CommSettings.szAppNetworkName, AK_COMM_SETTINGS_MAX_STRING_SIZE, TCHAR_TO_ANSI(*GameName));
#endif
}

FString FAkCommunicationSettings::GetCommsNetworkName() const
{
	FString CommsNetworkName = NetworkName;

	if (CommsNetworkName.IsEmpty() && FApp::HasProjectName())
	{
		CommsNetworkName = FApp::GetProjectName();
	}

#if WITH_EDITORONLY_DATA
	if (!CommsNetworkName.IsEmpty() && !IsRunningGame())
	{
		CommsNetworkName += TEXT(" (Editor)");
	}
#endif

	return CommsNetworkName;
}


//////////////////////////////////////////////////////////////////////////
// FAkCommunicationSettingsWithSystemInitialization

void FAkCommunicationSettingsWithSystemInitialization::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if AK_ENABLE_COMMUNICATION
	InitializationStructure.CommSettings.bInitSystemLib = InitializeSystemComms;
#endif
}

void FAkCommunicationSettingsWithCommSelection::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if AK_ENABLE_COMMUNICATION
	InitializationStructure.CommSettings.commSystem = (AkCommSettings::AkCommSystem)CommunicationSystem;
#endif
}

//////////////////////////////////////////////////////////////////////////
// FAkCommonInitializationSettings

void FAkCommonInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& InitSettings = InitializationStructure.InitSettings;
	InitSettings.uMaxNumPaths = MaximumNumberOfPositioningPaths;
	InitSettings.uCommandQueueSize = CommandQueueSize;
	InitSettings.uNumSamplesPerFrame = SamplesPerFrame;

	MainOutputSettings.FillInitializationStructure(InitializationStructure);

	auto& PlatformInitSettings = InitializationStructure.PlatformInitSettings;
	PlatformInitSettings.uNumRefillsInVoice = NumberOfRefillsInVoice;

	SpatialAudioSettings.FillInitializationStructure(InitializationStructure);

	InitializationStructure.MusicSettings.fStreamingLookAheadRatio = StreamingLookAheadRatio;
}


//////////////////////////////////////////////////////////////////////////
// FAkAdvancedInitializationSettings

void FAkAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	auto& DeviceSettings = InitializationStructure.DeviceSettings;
	DeviceSettings.uIOMemorySize = IO_MemorySize;
	DeviceSettings.uGranularity = IO_Granularity == 0 ? (32 * 1024) : IO_Granularity;
	DeviceSettings.fTargetAutoStmBufferLength = TargetAutoStreamBufferLength;
	DeviceSettings.bUseStreamCache = UseStreamCache;
	DeviceSettings.uMaxCachePinnedBytes = MaximumPinnedBytesInCache;

	auto& InitSettings = InitializationStructure.InitSettings;
	InitSettings.bEnableGameSyncPreparation = EnableGameSyncPreparation;
	InitSettings.uContinuousPlaybackLookAhead = ContinuousPlaybackLookAhead;
	InitSettings.uMonitorQueuePoolSize = MonitorQueuePoolSize;
	InitSettings.uMaxHardwareTimeoutMs = MaximumHardwareTimeoutMs;
	InitSettings.bDebugOutOfRangeCheckEnabled = DebugOutOfRangeCheckEnabled;
	InitSettings.fDebugOutOfRangeLimit = DebugOutOfRangeLimit;
}


//////////////////////////////////////////////////////////////////////////
// FAkAdvancedInitializationSettingsWithMultiCoreRendering

void FAkAdvancedInitializationSettingsWithMultiCoreRendering::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

	if (EnableMultiCoreRendering)
	{
		FAkAudioDevice* pDevice = FAkAudioDevice::Get();
		check(pDevice != nullptr);

		FAkJobWorkerScheduler* pScheduler = pDevice->GetAkJobWorkerScheduler();
		check(pScheduler != nullptr);

		auto& InitSettings = InitializationStructure.InitSettings;
		pScheduler->InstallJobWorkerScheduler(JobWorkerMaxExecutionTimeUSec, MaxNumJobWorkers, InitSettings.settingsJobManager);
	}
}

static void UELocalOutputFunc(
	AK::Monitor::ErrorCode in_eErrorCode,
	const AkOSChar* in_pszError,
	AK::Monitor::ErrorLevel in_eErrorLevel,
	AkPlayingID in_playingID,
	AkGameObjectID in_gameObjID)
{
	if (!IsRunningCommandlet())
	{
		FString AkError(in_pszError);

		if (in_eErrorLevel == AK::Monitor::ErrorLevel_Message)
		{
			UE_LOG(LogWwiseMonitor, Log, TEXT("%s"), *AkError);
		}

#if !UE_BUILD_SHIPPING
		else if ((FPlatformMisc::IsDebuggerPresent() || GIsAutomationTesting) && UNLIKELY(AkError.Contains(TEXT("Starvation"))))
		{
			UE_LOG(LogWwiseMonitor, Log, TEXT("%s [%s])"), *AkError, FPlatformMisc::IsDebuggerPresent() ? TEXT("Debugger") : TEXT("Automation"));
		}
#endif

		else
		{
#if UE_EDITOR
			UE_LOG(LogWwiseMonitor, Warning, TEXT("%s"), *AkError);
#else
			UE_LOG(LogWwiseMonitor, Error, TEXT("%s"), *AkError);
#endif
		}
	}
}

namespace FAkSoundEngineInitialization
{
	bool Initialize(FWwiseIOHook* IOHook)
	{
		if (!IOHook)
		{
			UE_LOG(LogAkAudio, Error, TEXT("IOHook is null."));
			return false;
		}

		const UAkInitializationSettings* InitializationSettings = FAkPlatform::GetInitializationSettings();
		if (InitializationSettings == nullptr)
		{
			UE_LOG(LogAkAudio, Error, TEXT("InitializationSettings could not be found."));
			return false;
		}

		FAkInitializationStructure InitializationStructure;
		InitializationSettings->FillInitializationStructure(InitializationStructure);

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Platform"));
		FAkPlatform::PreInitialize(InitializationStructure);

		auto* Comm = IWwiseCommAPI::Get();
		auto* MemoryMgr = IWwiseMemoryMgrAPI::Get();
		auto* Monitor = IWwiseMonitorAPI::Get();
		auto* MusicEngine = IWwiseMusicEngineAPI::Get();
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		auto* SpatialAudio = IWwiseSpatialAudioAPI::Get();
		auto* StreamMgr = IWwiseStreamMgrAPI::Get();

		// Enable AK error redirection to UE log.
		if (LIKELY(Monitor))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Monitor's Output"));
			Monitor->SetLocalOutput(AK::Monitor::ErrorLevel_All, UELocalOutputFunc);
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Memory Manager"));
		if (UNLIKELY(!MemoryMgr) || MemoryMgr->Init(&InitializationStructure.MemSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::MemoryMgr."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Global Callbacks"));
		auto* GlobalCallbacks = FWwiseGlobalCallbacks::Get();
		if (UNLIKELY(!GlobalCallbacks) || !GlobalCallbacks->Initialize())
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize Global Callbacks."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Stream Manager"));
		if (UNLIKELY(!StreamMgr) || !StreamMgr->Create(InitializationStructure.StreamManagerSettings))
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::StreamMgr."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing IOHook"));
		if (!IOHook->Init(InitializationStructure.DeviceSettings))
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize IOHook."));
			return false;
		}

		if (AkInitializationSettings_Helpers::IsLoggingInitialization && InitializationStructure.InitSettings.szPluginDLLPath)
		{
			FString DllPath(InitializationStructure.InitSettings.szPluginDLLPath);
			UE_LOG(LogAkAudio, Log, TEXT("Wwise plug-in DLL path: %s"), *DllPath);
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Sound Engine"));
		if (UNLIKELY(!SoundEngine) || SoundEngine->Init(&InitializationStructure.InitSettings, &InitializationStructure.PlatformInitSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::SoundEngine."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Music Engine"));
		if (UNLIKELY(!MusicEngine) || MusicEngine->Init(&InitializationStructure.MusicSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::MusicEngine."));
			return false;
		}

		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Spatial Audio"));
		if (UNLIKELY(!SpatialAudio) || SpatialAudio->Init(InitializationStructure.SpatialAudioInitSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Error, TEXT("Failed to initialize AK::SpatialAudio."));
			return false;
		}

#if AK_ENABLE_COMMUNICATION
		UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Initializing Communication"));
		if (UNLIKELY(!Comm) || Comm->Init(InitializationStructure.CommSettings) != AK_Success)
		{
			UE_LOG(LogAkAudio, Warning, TEXT("Could not initialize Wwise communication."));
		}
		else
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Log, TEXT("Wwise remote connection application name: %s"), ANSI_TO_TCHAR(InitializationStructure.CommSettings.szAppNetworkName));
		}
#endif

		return true;
	}

	void Finalize(FWwiseIOHook* IOHook)
	{
		auto* Comm = IWwiseCommAPI::Get();
		auto* MemoryMgr = IWwiseMemoryMgrAPI::Get();
		auto* Monitor = IWwiseMonitorAPI::Get();
		auto* MusicEngine = IWwiseMusicEngineAPI::Get();
		auto* SoundEngine = IWwiseSoundEngineAPI::Get();
		auto* StreamMgr = IWwiseStreamMgrAPI::GetAkStreamMgr();

#if AK_ENABLE_COMMUNICATION
		if (LIKELY(Comm))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Communication"));
			Comm->Term();
		}
#endif

		// Note: No Spatial Audio Term

		if (LIKELY(MusicEngine))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Music Engine"));
			MusicEngine->Term();
		}

		if (LIKELY(SoundEngine && SoundEngine->IsInitialized()))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Sound Engine"));
			SoundEngine->Term();
		}

		if (LIKELY(IOHook))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating IOHook"));
			IOHook->Term();
		}

		if (LIKELY(StreamMgr))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Stream Manager"));
			StreamMgr->Destroy();
		}

		if (LIKELY(MemoryMgr && MemoryMgr->IsInitialized()))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Terminating Memory Manager"));
			MemoryMgr->Term();
		}

		if (LIKELY(Monitor))
		{
			UE_CLOG(AkInitializationSettings_Helpers::IsLoggingInitialization, LogAkAudio, Verbose, TEXT("Resetting Monitor's Output"));
			Monitor->SetLocalOutput(0, nullptr);
		}
	}
}
