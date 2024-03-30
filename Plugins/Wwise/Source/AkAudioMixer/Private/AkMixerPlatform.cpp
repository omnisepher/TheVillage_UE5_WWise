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

#include "AkMixerPlatform.h"

#include "AkAudioEvent.h"
#include "AkAudioModule.h"
#include "AkSettings.h"
#include "Wwise/Stats/Global.h"

#include "AudioDevice.h"
#include "AudioMixerInputComponent.h"
#include "AudioMixerTypes.h"
#include "CoreGlobals.h"
#include "UObject/UObjectGlobals.h"

#if WITH_ENGINE
#include "AudioPluginUtilities.h"
#include "OpusAudioInfo.h"
#include "VorbisAudioInfo.h"
#include "ADPCMAudioInfo.h"
#endif // WITH_ENGINE

#if UE_5_0_OR_LATER
#include "BinkAudioInfo.h"
#endif


DECLARE_LOG_CATEGORY_EXTERN(LogAkAudioMixer, Log, All);
DEFINE_LOG_CATEGORY(LogAkAudioMixer);

FName FAkMixerPlatform::NAME_OGG(TEXT("OGG"));
FName FAkMixerPlatform::NAME_OPUS(TEXT("OPUS"));
FName FAkMixerPlatform::NAME_ADPCM(TEXT("ADPCM"));


FAkMixerPlatform::FAkMixerPlatform() :
	AkAudioMixerInputComponent(nullptr),
	bIsInitialized(false),
	bIsDeviceOpen(false),
	InputEvent(nullptr),
	OutputBuffer(nullptr),
	OutputBufferByteLength(0)
{
#if !WITH_EDITOR
	LoadVorbisLibraries();
#endif
}

FAkMixerPlatform::~FAkMixerPlatform()
{
	if (bIsInitialized)
	{
		TeardownHardware();
	}
}

void FAkMixerPlatform::OnAkAudioModuleInit()
{
	Audio::FAudioMixerOpenStreamParams CurrentStreamParams = OpenStreamParams;
	CloseAudioStream();
	OpenAudioStream(CurrentStreamParams);
	StartAudioStream();
}

void FAkMixerPlatform::WriteSilence(uint32 NumChannels, uint32 NumSamples, float** OutBufferToFill)
{
	for (uint32 Channel = 0; Channel < NumChannels; Channel++)
	{
		for (uint32 Sample = 0; Sample < NumSamples; Sample++)
		{
			OutBufferToFill[Channel][Sample] = 0;
		}
	}

}

bool FAkMixerPlatform::OnNextBuffer(uint32 NumChannels, uint32 NumSamples, float** OutBufferToFill)
{
	static auto bFailureShown = false;
	const auto RequestedOutBufferByteLength = NumChannels * NumSamples * sizeof(float);
	if (UNLIKELY(!bIsDeviceInitialized))
	{
		UE_CLOG(!bFailureShown, LogAkAudioMixer, Verbose, TEXT("FAkMixerPlatform::OnNextBuffer failed: Not initialized; State: %s. Skipping buffer. (Showing warning once)"),
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Closed ? TEXT("Closed") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Open ? TEXT("Open") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Running ? TEXT("Running") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Stopped ? TEXT("Stopped") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Stopping ? TEXT("Stopping") : TEXT("Unknown"));

		bFailureShown = true;
		OutputBuffer = nullptr;
		WriteSilence(NumChannels, NumSamples, OutBufferToFill);
		ReadNextBuffer();
		return true;
	}
	if (UNLIKELY(!(AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Open
			|| AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Running)))
	{
		UE_CLOG(!bFailureShown, LogAkAudioMixer, Verbose, TEXT("FAkMixerPlatform::OnNextBuffer cannot be called with state: %s. Stopping pump."),
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Closed ? TEXT("Closed") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Open ? TEXT("Open") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Running ? TEXT("Running") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Stopped ? TEXT("Stopped") :
			AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Stopping ? TEXT("Stopping") : TEXT("Unknown"));

		bFailureShown = true;
		OutputBuffer = nullptr;
		WriteSilence(NumChannels, NumSamples, OutBufferToFill);
		ReadNextBuffer();
		return true;
	}

	if (UNLIKELY(NumChannels != AudioStreamInfo.DeviceInfo.NumChannels))
	{
		UE_LOG(LogAkAudioMixer, Error, TEXT("FAkMixerPlatform::OnNextBuffer: Invalid number of channels: %d requested when %d were used on initialization. Failing permanently."), (int)NumChannels, (int)AudioStreamInfo.DeviceInfo.NumChannels);
		WriteSilence(NumChannels, NumSamples, OutBufferToFill);
		return false;
	}
	bFailureShown = false;

	if (UNLIKELY(NumChannels != AudioStreamInfo.DeviceInfo.NumChannels))
	{
		UE_LOG(LogAkAudioMixer, Error, TEXT("FAkMixerPlatform::OnNextBuffer: Invalid number of channels: %d requested when %d were used on initialization. Failing permanently."), (int)NumChannels, (int)AudioStreamInfo.DeviceInfo.NumChannels);
		WriteSilence(NumChannels, NumSamples, OutBufferToFill);
		return false;
	}
	if (UNLIKELY(NumSamples != AudioStreamInfo.NumOutputFrames))
	{
		UE_LOG(LogAkAudioMixer, Error, TEXT("FAkMixerPlatform::OnNextBuffer: Invalid number of frames per buffer: %d requested when %d were used on initialization. Failing permanently."), (int)NumSamples, (int)AudioStreamInfo.NumOutputFrames);
		UE_LOG(LogWwiseHints, Log, TEXT("Ensure that Wwise \"Samples per frame\" initialization setting matches the Unreal Audio \"Callback Buffer Size\" setting for the current platform."));
		UE_LOG(LogWwiseHints, Log, TEXT("This can also happen when the requested buffer size changes after initialization, for example when Wwise initialization's bRoundFrameSizeToHWSize parameter is activated."));
		WriteSilence(NumChannels, NumSamples, OutBufferToFill);
		return false;
	}
	if (UNLIKELY(OutputBufferByteLength != RequestedOutBufferByteLength))
	{
		UE_LOG(LogAkAudioMixer, Error, TEXT("FAkMixerPlatform::OnNextBuffer: Invalid output buffer byte length: %d requested when %d was calculated on initialization. Failing permanently."), (int)RequestedOutBufferByteLength, (int)OutputBufferByteLength);
		WriteSilence(NumChannels, NumSamples, OutBufferToFill);
		return false;
	}

	OutputBuffer = OutBufferToFill;
	ReadNextBuffer();
	return true;
}

bool FAkMixerPlatform::InitializeHardware()
{
	UE_LOG(LogAkAudioMixer, VeryVerbose, TEXT("FAkMixerPlatform::InitializeHardware"));
	if (!FAkAudioDevice::IsInitialized())
	{
		AkAudioModuleInitHandle = FAkAudioModule::OnModuleInitialized.AddRaw(this, &FAkMixerPlatform::OnAkAudioModuleInit);
	}

#if ENGINE_MAJOR_VERSION >= 5
	if(Audio::IAudioMixer::ShouldRecycleThreads())
	{
		// Pre-create the null render device thread, so we can simple wake it up when we need it.
		// Give it nothing to do, with a slow tick as the default, but ask it to wait for a signal to wake up.
		// Ensuring this exists prevents a crash in FMixerNullCallback::Run if the callback does not already exist.
		CreateNullDeviceThread([] {}, 1.0f, true);
	}
#endif

	bIsInitialized = true;

	// Must always return true at Editor startup at least, since a failed
	// initialization results in a failed initialized AudioDevice, which later
	// results in a crash.
	return true;
}

bool FAkMixerPlatform::TeardownHardware()
{
	UE_LOG(LogAkAudioMixer, VeryVerbose, TEXT("FAkMixerPlatform::TeardownHardware"));
	if (!bIsInitialized)
	{
		return true;
	}

	StopAudioStream();
	CloseAudioStream();

	if (AkAudioMixerInputComponent)
	{
		delete AkAudioMixerInputComponent;
		AkAudioMixerInputComponent = nullptr;
	}

	AkAudioModuleInitHandle.Reset();

	bIsInitialized = false;
	return true;
}

bool FAkMixerPlatform::IsInitialized() const
{
	return bIsInitialized;
}

bool FAkMixerPlatform::GetNumOutputDevices(uint32& OutNumOutputDevices)
{
	// TODO Define constant
	OutNumOutputDevices = 1;
	return true;
}

bool FAkMixerPlatform::GetOutputDeviceInfo(const uint32 InDeviceIndex, Audio::FAudioPlatformDeviceInfo& OutInfo)
{
	AkAudioFormat AudioFormat;
	AkAudioMixerInputComponent->GetChannelConfig(AudioFormat);

	OutInfo.Format = Audio::EAudioMixerStreamDataFormat::Float;
	OutInfo.Name = GetDefaultDeviceName();
	OutInfo.DeviceId = TEXT("WwiseDevice");
	OutInfo.NumChannels = AudioFormat.GetNumChannels();
	OutInfo.SampleRate = AudioFormat.uSampleRate;

	OutInfo.OutputChannelArray.SetNum(OutInfo.NumChannels);

	for (int32 ChannelNum = 0; ChannelNum < OutInfo.NumChannels; ++ChannelNum)
	{
		OutInfo.OutputChannelArray.Add(EAudioMixerChannel::Type(ChannelNum));
	}

	return true;
}
bool FAkMixerPlatform::GetDefaultOutputDeviceIndex(uint32& OutDefaultDeviceIndex) const
{
	OutDefaultDeviceIndex = AUDIO_MIXER_DEFAULT_DEVICE_INDEX;
	return true;
}

bool FAkMixerPlatform::OpenAudioStream(const Audio::FAudioMixerOpenStreamParams& Params)
{
	if (!bIsInitialized || AudioStreamInfo.StreamState != Audio::EAudioOutputStreamState::Closed)
	{
		return false;
	}

	OpenStreamParams = Params;

	AudioStreamInfo.Reset();
	AudioStreamInfo.OutputDeviceIndex = OpenStreamParams.OutputDeviceIndex;
	AudioStreamInfo.NumOutputFrames = OpenStreamParams.NumFrames;
	AudioStreamInfo.NumBuffers = OpenStreamParams.NumBuffers;
	AudioStreamInfo.AudioMixer = OpenStreamParams.AudioMixer;

	// Allow negotiating output format with the Sound Engine
	if (!GetOutputDeviceInfo(AudioStreamInfo.OutputDeviceIndex, AudioStreamInfo.DeviceInfo))
	{
		return false;
	}

	OutputBufferByteLength = OpenStreamParams.NumFrames * AudioStreamInfo.DeviceInfo.NumChannels * GetAudioStreamChannelSize();

	UE_LOG(LogAkAudioMixer, Verbose, TEXT("Opening Audio stream for device: %s"), *GetDeviceId())

	AudioStreamInfo.StreamState = Audio::EAudioOutputStreamState::Open;


	if (FAkAudioDevice::IsInitialized())
	{
		bool bOpenStreamError = false;

		AkAudioMixerInputComponent = new FAudioMixerInputComponent();
		AkAudioMixerInputComponent->OnNextBuffer = FAkGlobalAudioInputDelegate::CreateRaw(this, &FAkMixerPlatform::OnNextBuffer);

		UAkSettings* AkSettings = GetMutableDefault<UAkSettings>();
		if (AkSettings != nullptr)
		{
			AkSettings->GetAudioInputEvent(InputEvent);
		}

		if (!InputEvent)
		{
			UE_LOG(LogAkAudioMixer, Error, TEXT("Unable to open audio stream. Ak Audio Event is not set."));
			bOpenStreamError = true;
		}
		else
		{
			InputEvent->AddToRoot(); // Make sure the event can't be garbage collected.
			AkPlayingID PlayingID = AkAudioMixerInputComponent->PostAssociatedAudioInputEvent(InputEvent);

			if (PlayingID == AK_INVALID_PLAYING_ID)
			{
				UE_LOG(LogAkAudioMixer, Error, TEXT("Unable to open audio stream. Could not post Ak Audio Event."));
				bOpenStreamError = true;
			}
		}

		if (!bOpenStreamError)
		{
			UE_LOG(LogAkAudioMixer, Verbose, TEXT("Opened Audio stream for device: %s"), *GetDeviceId())
			bIsDeviceOpen = true;
		}
	}

	else
	{
		UE_LOG(LogAkAudioMixer, Verbose, TEXT("Audio stream deferred for device %s, AkAudioDevice is not yet initialized"), *GetDeviceId())
	}

	// Must always return true Editor startup at least, since a failed
	// initialization results in a failed initialized AudioDevice, which later
	// results in a crash.
	return true;
}

bool FAkMixerPlatform::CloseAudioStream()
{
	if (AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Closed)
	{
		return false;
	}

	UE_LOG(LogAkAudioMixer, Verbose, TEXT("Closing Audio stream for device: %s"), *GetDeviceId())

	if (!StopAudioStream())
	{
		return false;
	}

	if (bIsUsingNullDevice)
	{
		StopRunningNullDevice();
	}

	{
		FScopeLock ScopedLock(&OutputBufferMutex);
		OutputBuffer = nullptr;
		OutputBufferByteLength = 0;
	}

	if (FAkAudioDevice::IsInitialized())
	{
		if (AkAudioMixerInputComponent)
		{
			AkAudioMixerInputComponent->PostUnregisterGameObject();
			delete AkAudioMixerInputComponent;
			AkAudioMixerInputComponent = nullptr;
		}
		
		if (InputEvent)
		{
			InputEvent->RemoveFromRoot(); // Allow garbage collection on the event.
			InputEvent = nullptr;
		}
	}

	bIsDeviceOpen = false;

	AudioStreamInfo.StreamState = Audio::EAudioOutputStreamState::Closed;

	return true;
}

bool FAkMixerPlatform::StartAudioStream()
{
	if (!bIsInitialized || (AudioStreamInfo.StreamState != Audio::EAudioOutputStreamState::Open
		&& AudioStreamInfo.StreamState != Audio::EAudioOutputStreamState::Stopped))
	{
		return false;
	}

	UE_LOG(LogAkAudioMixer, Verbose, TEXT("Starting Audio stream for device: %s"), *GetDeviceId())

	BeginGeneratingAudio();

	if (!bIsDeviceOpen)
	{
		check(!bIsUsingNullDevice);
		StartRunningNullDevice();
	}

	check(AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Running);

	return true;

}

bool FAkMixerPlatform::StopAudioStream()
{
	if (AudioStreamInfo.StreamState != Audio::EAudioOutputStreamState::Stopped
		&& AudioStreamInfo.StreamState != Audio::EAudioOutputStreamState::Closed)
	{

		UE_LOG(LogAkAudioMixer, Verbose, TEXT("Stopping Audio stream for device: %s"), *GetDeviceId())

		if (AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Running)
		{
			StopGeneratingAudio();
		}

		check(AudioStreamInfo.StreamState == Audio::EAudioOutputStreamState::Stopped);
	}

	return true;
}

Audio::FAudioPlatformDeviceInfo FAkMixerPlatform::GetPlatformDeviceInfo() const
{
	return AudioStreamInfo.DeviceInfo;
}

void FAkMixerPlatform::SubmitBuffer(const uint8* Buffer)
{
	FScopeLock ScopedLock(&OutputBufferMutex);

	if (OutputBuffer)
	{
		for (int32 Channel = 0; Channel < AudioStreamInfo.DeviceInfo.NumChannels; Channel++)
		{
			for (int32 Frame = 0; Frame < AudioStreamInfo.NumOutputFrames; Frame++)
			{
				FMemory::Memcpy(&OutputBuffer[Channel][Frame],
					&Buffer[sizeof(float) * ((AudioStreamInfo.DeviceInfo.NumChannels * Frame) + Channel)],
					sizeof(float));
			}
		}
	}
}

#if UE_5_0_OR_LATER
FName FAkMixerPlatform::GetRuntimeFormat(const USoundWave* InSoundWave) const
{
	const FName RuntimeFormat = Audio::ToName(InSoundWave->GetSoundAssetCompressionType());

	if (RuntimeFormat == Audio::NAME_PLATFORM_SPECIFIC)
	{
#if defined(PLATFORM_PS5) && PLATFORM_PS5
		if (InSoundWave->IsStreaming() && InSoundWave->IsSeekable())
		{
			return Audio::NAME_ADPCM;
		}

		checkf(false, TEXT("Please set your Unreal audio sources to Streaming and Seekable in order to play them through Wwise on the PS5"));
		return NAME_ADPCM;
#else
		if (InSoundWave->IsStreaming())
		{
			if (InSoundWave->IsSeekable())
			{
				return Audio::NAME_ADPCM;
			}

			return Audio::NAME_OPUS;
		}
		return Audio::NAME_OGG;
#endif
	}
	return RuntimeFormat;
}

ICompressedAudioInfo* FAkMixerPlatform::CreateCompressedAudioInfo(const FName& InRuntimeFormat) const
{
	ICompressedAudioInfo* Decoder = nullptr;

	if (InRuntimeFormat == Audio::NAME_OGG)
	{
		Decoder = new FVorbisAudioInfo();
	}
	else if (InRuntimeFormat == Audio::NAME_OPUS)
	{
		Decoder = new FOpusAudioInfo();
	}
#if WITH_BINK_AUDIO
	else if (InRuntimeFormat == Audio::NAME_BINKA)
	{
		Decoder = new FBinkAudioInfo();
	}
#endif // WITH_BINK_AUDIO	
	else
	{
		Decoder = Audio::CreateSoundAssetDecoder(InRuntimeFormat);
	}
	ensureMsgf(Decoder != nullptr, TEXT("Failed to create a sound asset decoder for compression type: %s"), *InRuntimeFormat.ToString());
	return Decoder;
}

#else	// UE4.27

FName FAkMixerPlatform::GetRuntimeFormat(USoundWave* InSoundWave)
{
#if WITH_ENGINE
	check(InSoundWave);
#if defined(PLATFORM_PS5) && PLATFORM_PS5
	static FName NAME_ADPCM(TEXT("ADPCM"));

	if (InSoundWave->IsStreaming() && InSoundWave->IsSeekableStreaming())
	{
		return NAME_ADPCM;
	}

	checkf(false, TEXT("Please set your Unreal audio sources to Streaming and Seekable in order to play them through Wwise on the PS5"));
	return NAME_ADPCM;
#else
	if (InSoundWave->IsStreaming(nullptr))
	{
		if (InSoundWave->IsSeekableStreaming())
		{
			return NAME_ADPCM;
		}

		return NAME_OPUS;
	}
	return NAME_OGG;
#endif
#else
	checkNoEntry();
	return FName();
#endif // WITH_ENGINE
}

bool FAkMixerPlatform::HasCompressedAudioInfoClass(USoundWave* InSoundWave)
{
	return true;
}

ICompressedAudioInfo* FAkMixerPlatform::CreateCompressedAudioInfo(USoundWave* InSoundWave)
{
#if WITH_ENGINE
	check(InSoundWave);

#if defined(PLATFORM_PS5) && PLATFORM_PS5
	if (InSoundWave->IsStreaming() && InSoundWave->IsSeekableStreaming())
	{
		return new FADPCMAudioInfo();
	}

	checkf(false, TEXT("Please set your Unreal audio sources to Streaming and Seekable in order to play them through Wwise on the PS5"));
	return new FADPCMAudioInfo();
#else
	if (InSoundWave->IsStreaming())
	{
		if (InSoundWave->IsSeekableStreaming())
		{
			return new FADPCMAudioInfo();
		}

		return new FOpusAudioInfo();
	}

	if (InSoundWave->HasCompressedData(NAME_OGG))
	{
		return new FVorbisAudioInfo();
	}

	return new FADPCMAudioInfo();
#endif
#else
	checkNoEntry();
	return nullptr;
#endif // WITH_ENGINE
}
#endif

FString FAkMixerPlatform::GetDefaultDeviceName() {
	static FString DefaultName(TEXT("Wwise Audio Mixer Device."));
	return DefaultName;
}

FString FAkMixerPlatform::GetDeviceId() const
{
	return AudioStreamInfo.DeviceInfo.DeviceId;
}

FAudioPlatformSettings FAkMixerPlatform::GetPlatformSettings() const
{
	return FAudioPlatformSettings::GetPlatformSettings(FPlatformProperties::GetRuntimeSettingsClassName());
}
