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

#include "AudioMixer.h"
#include "WwiseUnrealDefines.h"

class FAudioMixerInputComponent;
class UAkAudioEvent;

class FAkMixerPlatform : public Audio::IAudioMixerPlatformInterface
{
public:
	FAkMixerPlatform();
	~FAkMixerPlatform();

#if UE_5_0_OR_LATER
	virtual FString GetPlatformApi() const override { return TEXT("AkMixerPlatform"); }
#else
	virtual Audio::EAudioMixerPlatformApi::Type GetPlatformApi() const override { return Audio::EAudioMixerPlatformApi::Other; }
#endif
	virtual bool InitializeHardware() override;
	virtual bool TeardownHardware() override;
	virtual bool IsInitialized() const override;
	virtual bool GetNumOutputDevices(uint32& OutNumOutputDevices) override;
	virtual bool GetOutputDeviceInfo(const uint32 InDeviceIndex, Audio::FAudioPlatformDeviceInfo& OutInfo) override;
	virtual bool GetDefaultOutputDeviceIndex(uint32& OutDefaultDeviceIndex) const override;
	virtual bool OpenAudioStream(const Audio::FAudioMixerOpenStreamParams& Params) override;
	virtual bool CloseAudioStream() override;
	virtual bool StartAudioStream() override;
	virtual bool StopAudioStream() override;
	virtual Audio::FAudioPlatformDeviceInfo GetPlatformDeviceInfo() const override;
	virtual void SubmitBuffer(const uint8* Buffer) override;
#if UE_5_0_OR_LATER
	virtual FName GetRuntimeFormat(const USoundWave* InSoundWave) const override;
	virtual ICompressedAudioInfo* CreateCompressedAudioInfo(const FName& InRuntimeFormat) const override;
#else
	virtual FName GetRuntimeFormat(USoundWave* InSoundWave) override;
	virtual bool HasCompressedAudioInfoClass(USoundWave* InSoundWave) override;
	virtual ICompressedAudioInfo* CreateCompressedAudioInfo(USoundWave* InSoundWave) override;
#endif
	virtual bool SupportsRealtimeDecompression() const { return true; }
	virtual FString GetDefaultDeviceName() override;
	FString GetDeviceId() const;
	virtual FAudioPlatformSettings GetPlatformSettings() const override;

private:
	FAudioMixerInputComponent* AkAudioMixerInputComponent;
	bool bIsInitialized;
	bool bIsDeviceOpen;
	UAkAudioEvent* InputEvent;
	float** OutputBuffer;
	int OutputBufferByteLength;
	FCriticalSection OutputBufferMutex;
	FDelegateHandle AkAudioModuleInitHandle;

	void OnAkAudioModuleInit();
	void WriteSilence(uint32 NumChannels, uint32 NumSamples, float** OutBufferToFill);
	bool OnNextBuffer(uint32 NumChannels, uint32 NumSamples, float** OutBufferToFill);
	int32 GetAudioStreamChannelSize() { return sizeof(float); }

private:
	static FName NAME_OGG;
	static FName NAME_OPUS;
	static FName NAME_ADPCM;

};

