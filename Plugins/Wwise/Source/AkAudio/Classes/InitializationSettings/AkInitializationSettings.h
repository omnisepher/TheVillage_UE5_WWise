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

#include "Engine/EngineTypes.h"
#include "AkInclude.h"
#include "AkInitializationSettings.generated.h"

UENUM()
enum class EAkPanningRule
{
	Speakers = AkPanningRule::AkPanningRule_Speakers,
	Headphones = AkPanningRule::AkPanningRule_Headphones,
};

UENUM()
enum class EAkChannelConfigType
{
	Anonymous = AkChannelConfigType::AK_ChannelConfigType_Anonymous,
	Standard = AkChannelConfigType::AK_ChannelConfigType_Standard,
	Ambisonic = AkChannelConfigType::AK_ChannelConfigType_Ambisonic,
};

UENUM(Meta = (Bitmask))
enum class EAkChannelMask : uint32
{
	FrontLeft,
	FrontRight,
	FrontCenter,
	LowFrequency,
	BackLeft,
	BackRight,
	BackCenter = BackRight + 3,
	SideLeft,
	SideRight,

	Top,
	HeightFrontLeft,
	HeightFrontCenter,
	HeightFrontRight,
	HeightBackLeft,
	HeightBackCenter,
	HeightBackRight,
};

static_assert((1 << (uint32)EAkChannelMask::FrontLeft) == AK_SPEAKER_FRONT_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::FrontRight) == AK_SPEAKER_FRONT_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::FrontCenter) == AK_SPEAKER_FRONT_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::LowFrequency) == AK_SPEAKER_LOW_FREQUENCY, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::BackLeft) == AK_SPEAKER_BACK_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::BackRight) == AK_SPEAKER_BACK_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::BackCenter) == AK_SPEAKER_BACK_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::SideLeft) == AK_SPEAKER_SIDE_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::SideRight) == AK_SPEAKER_SIDE_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::Top) == AK_SPEAKER_TOP, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightFrontLeft) == AK_SPEAKER_HEIGHT_FRONT_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightFrontCenter) == AK_SPEAKER_HEIGHT_FRONT_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightFrontRight) == AK_SPEAKER_HEIGHT_FRONT_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightBackLeft) == AK_SPEAKER_HEIGHT_BACK_LEFT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightBackCenter) == AK_SPEAKER_HEIGHT_BACK_CENTER, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");
static_assert((1 << (uint32)EAkChannelMask::HeightBackRight) == AK_SPEAKER_HEIGHT_BACK_RIGHT, "Review constants defined in \"include\\AK\\SoundEngine\\Common\\AkSpeakerConfig.h\"");


UENUM()
enum class EAkCommSystem
{
	Socket = 0, // AkCommSettings::AkCommSystem_Socket,
	HTCS = 1 // AkCommSettings::AkCommSystem_HTCS,
};

#if AK_ENABLE_COMMUNICATION
static_assert((uint32)EAkCommSystem::Socket == (uint32)AkCommSettings::AkCommSystem_Socket, "Review constants defined in \"include\\AK\\Comm\\AkCommunication.h\"");
static_assert((uint32)EAkCommSystem::HTCS == (uint32)AkCommSettings::AkCommSystem_HTCS, "Review constants defined in \"include\\AK\\Comm\\AkCommunication.h\"");
#endif

struct FAkInitializationStructure
{
	FAkInitializationStructure();
	~FAkInitializationStructure();

	AkMemSettings MemSettings;
	AkStreamMgrSettings StreamManagerSettings;
	AkDeviceSettings DeviceSettings;
	AkInitSettings InitSettings;
	AkPlatformInitSettings PlatformInitSettings;
	AkMusicSettings MusicSettings;
	AkSpatialAudioInitSettings SpatialAudioInitSettings;
#if AK_ENABLE_COMMUNICATION
	AkCommSettings CommSettings;
#endif

	void SetPluginDllPath(const FString& PlatformArchitecture);
};


USTRUCT()
struct FAkMainOutputSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "The name of a custom audio device to use. Custom audio devices are defined in the Audio Device ShareSet section of the Wwise project. Leave this empty to output normally through the default audio device."))
	FString AudioDeviceShareSet;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "Device-specific identifier when you are using multiple devices of the same type. Leave the setting at 0 (default) if you are using only one device."))
	uint32 DeviceID = AK_INVALID_UNIQUE_ID;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "Rule for 3D panning of signals routed to a stereo bus. In \"Speakers\" mode, the angle of the front loudspeakers is used. In \"Headphones\" mode, the speaker angles are superseded by constant power panning between two virtual microphones spaced 180 degrees apart."))
	EAkPanningRule PanningRule = EAkPanningRule::Speakers;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "A code that completes the identification of channels by uChannelMask. Anonymous: Channel mask == 0 and channels. Standard: Channels must be identified with standard defines in AkSpeakerConfigs. Ambisonic: Channel mask == 0 and channels follow standard ambisonic order."))
	EAkChannelConfigType ChannelConfigType = EAkChannelConfigType::Anonymous;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkChannelMask", ToolTip = "A bit field, whose channel identifiers depend on AkChannelConfigType (up to 20)."))
	uint32 ChannelMask = 0;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Main Output Settings", meta = (ToolTip = "The number of channels, identified (deduced from channel mask) or anonymous (set directly)."))
	uint32 NumberOfChannels = 0;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkSpatialAudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Maximum number of portals that sound can propagate through. The default value is 8.", ClampMin = "0", ClampMax = "8"))
	uint32 MaxSoundPropagationDepth = AK_MAX_SOUND_PROPAGATION_DEPTH;
	static_assert(AK_MAX_SOUND_PROPAGATION_DEPTH == 8, "AK_MAX_SOUND_PROPAGATION_DEPTH has changed values. Ensure that the limits of MaxSoundPropagationDepth are equally reflected.");

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Distance (in game units) that an emitter or listener has to move to trigger a recalculation of reflections and diffraction. A high distance value has a lower CPU load than a low distance value, but the accuracy is also lower. Note that this value does not affect the ray tracing itself. Rays are cast each time a Spatial Audio update is executed. The default value is 25.", ClampMin = "0"))
	float MovementThreshold = 25.f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "The number of primary rays used in the ray tracing engine. A larger value increases the chances of finding reflection and diffraction paths but results in higher CPU usage. When the CPU limit is active (see the CPU Limit Percentage Spatial Audio Setting), this setting represents the maximum allowed number of primary rays. The default value is 35.", ClampMin = "0"))
	uint32 NumberOfPrimaryRays = 35;
	
	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "The maximum reflection order: the number of \"bounces\" in a reflection path.A higher reflection order renders more detail at the expense of higher CPU usage.The default value is 2.", ClampMin = "0", ClampMax = "4"))
	uint32 ReflectionOrder = 2;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Maximum diffraction order: the number of \"bends\" in a diffraction path. A high diffraction order accommodates more complex geometry at the expense of higher CPU usage. Diffraction must be enabled on the geometry to find diffraction paths. Set to 0 to disable diffraction on all geometry. This parameter limits the recursion depth of diffraction rays cast from the listener to scan the environment and also the depth of the diffraction search to find paths between emitter and listener. To optimize CPU usage, set it to the maximum number of edges you expect the obstructing geometry to traverse. The default value is 4.", ClampMin = "0", ClampMax = "8"))
	uint32 DiffractionOrder = 4;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "The maximum number of game-defined auxiliary sends that can originate from a single emitter. An emitter can send to its own Room and to all adjacent Rooms if the emitter and listener are in the same Room. If a limit is set, the most prominent sends are kept, based on spread to the adjacent portal from the emitter's perspective. Set to 1 to only allow emitters to send directly to their current Room, and to the Room a listener is transitioning to if inside a portal. Set to 0 to disable the limit. The default value is 3.", ClampMin = "0"))
	uint32 MaxEmitterRoomAuxSends = 3;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "The maximum possible number of diffraction points at each end of a reflection path. Diffraction on reflection allows reflections to fade in and out smoothly as the listener or emitter moves in and out of the reflection's shadow zone. When greater than zero, diffraction rays are sent from the listener to search for reflections around one or more corners from the listener. Diffraction must be enabled on the geometry to find diffracted reflections. Set to 0 to disable diffraction on reflections. Set to 2 or greater to allow Reflection paths to travel through Portals. The default value is 2.", ClampMin = "0", ClampMax = "4"))
	uint32 DiffractionOnReflectionsOrder = 2;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Length of the rays that are cast inside Spatial Audio. Effectively caps the maximum length of an individual segment in a reflection or diffraction path. The default value is 100000.", ClampMin = "0"))
	float MaximumPathLength = 100000.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (DisplayName = "CPU Limit Percentage", ToolTip = "Defines the targeted computation time allocated for the ray tracing engine as a percentage [0, 100] of the current audio frame. The ray tracing engine dynamically adapts the number of primary rays to target the specified computation time. The computed number of primary rays cannot exceed the value specified by the Number Of Primary Rays Spatial Audio Setting. A value of 0 indicates no target has been set. In this case, the number of primary rays is fixed and is set by the Number Of Primary Rays Spatial Audio Setting. The default value is 0.", ClampMin = "0", ClampMax = "100"))
	float CPULimitPercentage = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (DisplayName = "Load Balancing Spread", ToolTip = "The computation of spatial audio paths is spread on LoadBalancingSpread frames. Spreading the computation of paths over several frames can prevent CPU peaks. The spread introduces a delay in path computation. The default value is 1.", ClampMin = "1"))
	uint32 LoadBalancingSpread = 1;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "Enable computation of geometric diffraction and transmission paths for all sources that have the \"Diffraction and Transmission\" option selected in the Positioning tab of the Wwise Property Editor. This flag enables sound paths around (diffraction) and through (transmission) geometry. Setting EnableGeometricDiffractionAndTransmission to false implies that geometry is only to be used for reflection calculation. Diffraction edges must be enabled on geometry for diffraction calculation. If EnableGeometricDiffractionAndTransmission is false but a sound has \"Diffraction and Transmission\" selected in the Positioning tab of Wwise Authoring, the sound will diffract through portals but pass through geometry as if it isn't there. Typically, we recommend you disable this setting if the game will perform obstruction calculations, but geometry is still passed to Spatial Audio for reflection calculations. The default value is true."))
	bool EnableGeometricDiffractionAndTransmission = true;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Spatial Audio Settings", meta = (ToolTip = "An emitter that is diffracted through a portal or around geometry will have its apparent or virtual position calculated by Wwise Spatial Audio and passed on to the sound engine. The default value is true."))
	bool CalcEmitterVirtualPosition = true;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkCommunicationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "Size of the communication pool."))
	uint32 PoolSize = 256 * 1024;

	enum { DefaultDiscoveryBroadcastPort = 24024, };
	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "The port where the authoring application broadcasts \"Game Discovery\" requests to discover games running on the network. Default value: 24024. (Cannot be set to 0)."))
	uint16 DiscoveryBroadcastPort = DefaultDiscoveryBroadcastPort;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "The \"command\" channel port. Set to 0 to request a dynamic/ephemeral port."))
	uint16 CommandPort = 0;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings|Communication Settings", meta = (ToolTip = "The name used to identify this game within the authoring application. Leave empty to use FApp::GetProjectName()."))
	FString NetworkName;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;

protected:
	FString GetCommsNetworkName() const;
};


USTRUCT()
struct FAkCommunicationSettingsWithSystemInitialization : public FAkCommunicationSettings
{
	GENERATED_BODY()

	UPROPERTY(Config, Category = "Ak Initialization Settings|Communication Settings", EditAnywhere, meta = (ToolTip = "Indicates whether or not to initialize the communication system. Some consoles have critical requirements for initialization of their communications systems. Set to false only if your game already uses sockets before sound engine initialization."))
	bool InitializeSystemComms = true;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};

USTRUCT()
struct FAkCommunicationSettingsWithCommSelection : public FAkCommunicationSettings
{
	GENERATED_BODY()

	UPROPERTY(Config, Category = "Ak Initialization Settings|Communication Settings", EditAnywhere, meta = (ToolTip = "Select between Socket and HTCS communication protocol. Socket is the Default option."))
	EAkCommSystem CommunicationSystem = EAkCommSystem::Socket;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkCommonInitializationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Maximum number of memory pools. A memory pool is required for each loaded bank."))
	uint32 MaximumNumberOfMemoryPools = 256;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Maximum number of automation paths for positioning sounds."))
	uint32 MaximumNumberOfPositioningPaths = 255;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of the command queue."))
	uint32 CommandQueueSize = 256 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Number of samples per audio frame (256, 512, 1024, or 2048)."))
	uint32 SamplesPerFrame = 512;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Platform-independent initialization settings of output devices."))
	FAkMainOutputSettings MainOutputSettings;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Multiplication factor for all streaming look-ahead heuristic values.", ClampMin = "0.0", ClampMax = "1.0"))
	float StreamingLookAheadRatio = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Number of refill buffers in voice buffer. Set to 2 for double-buffered. The default value is 4."))
	uint16 NumberOfRefillsInVoice = 4;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings")
	FAkSpatialAudioSettings SpatialAudioSettings;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};


USTRUCT()
struct FAkCommonInitializationSettingsWithSampleRate : public FAkCommonInitializationSettings
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, Category = "Common Settings", meta = (ToolTip = "Sampling Rate. Default is 48000 Hz. Use 24000hz for low quality. Any positive reasonable sample rate is supported; however, be careful setting a custom value. Using an odd or really low sample rate may cause the sound engine to malfunction."))
	uint32 SampleRate = 48000;
};


USTRUCT()
struct FAkAdvancedInitializationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of memory pool for I/O (for automatic streams). It is rounded down to a multiple of uGranularity and then passed directly to AK::MemoryMgr::CreatePool()."))
	uint32 IO_MemorySize = 2 * 1024 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "I/O requests granularity (typical bytes/request)."))
	uint32 IO_Granularity = 32 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Targeted automatic stream buffer length (ms). When a stream reaches that buffering, it stops being scheduled for I/O except if the scheduler is idle."))
	float TargetAutoStreamBufferLength = 380.0f;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "If true, the device attempts to reuse I/O buffers that have already been streamed from disk. This is particularly useful when streaming small looping sounds. However, there is a small increase in CPU usage when allocating memory, and a slightly larger memory footprint in the StreamManager pool."))
	bool UseStreamCache = false;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Maximum number of bytes that can be \"pinned\" using AK::SoundEngine::PinEventInStreamCache() or AK::IAkStreamMgr::PinFileInCache()."))
	uint32 MaximumPinnedBytesInCache = (uint32)-1;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Set to true to enable AK::SoundEngine::PrepareGameSync usage."))
	bool EnableGameSyncPreparation = false;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Number of quanta ahead when continuous containers instantiate a new voice before the following sounds start playing. This look-ahead time allows I/O to occur, and is especially useful to reduce the latency of continuous containers with trigger rate or sample-accurate transitions."))
	uint32 ContinuousPlaybackLookAhead = 1;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Size of the monitoring queue pool. This parameter is ignored in Release builds."))
	uint32 MonitorQueuePoolSize = 1024 * 1024;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Time (in milliseconds) to wait for hardware devices to trigger an audio interrupt. If there is no interrupt after that time, the sound engine reverts to silent mode and continues operating until the hardware responds."))
	uint32 MaximumHardwareTimeoutMs = 1000;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Debug setting: Enable checks for out-of-range (and NAN) floats in the processing code. Do not enable in any normal usage because this setting uses a lot of CPU. It prints error messages in the log if invalid values are found at various points in the pipeline. Contact AK Support with the new error messages for more information."))
	bool DebugOutOfRangeCheckEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (EditCondition = "DebugOutOfRangeCheckEnabled", ToolTip = "Debug setting: Only used when Debug Out Of Range Check Enabled is true. This defines the maximum values samples can have. Normal audio must be contained within +1/-1. Set this limit to a value greater than 1 to allow temporary or short excursions out of range. The default value is 16."))
	float DebugOutOfRangeLimit = 16.f;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};

USTRUCT()
struct FAkAdvancedInitializationSettingsWithMultiCoreRendering : public FAkAdvancedInitializationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (ToolTip = "Enable to run SoundEngine processing tasks on the Unreal Engine worker threads. Requires Editor restart."))
	bool EnableMultiCoreRendering = true;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (EditCondition = "EnableMultiCoreRendering", ToolTip = "Configure the maximum number of workers that the Sound Engine will request at any given time. Requires Editor restart."))
	uint32 MaxNumJobWorkers = 1;

	UPROPERTY(EditAnywhere, Category = "Ak Initialization Settings", meta = (EditCondition = "EnableMultiCoreRendering", ToolTip = "Maximum time allotted for each Sound Engine job in microseconds (0 is unlimited). Requires Editor restart."))
	uint32 JobWorkerMaxExecutionTimeUSec = 0;

	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};

class FWwiseIOHook;

namespace FAkSoundEngineInitialization
{
	bool Initialize(FWwiseIOHook* IOHook);
	void Finalize(FWwiseIOHook* IOHook);
}
