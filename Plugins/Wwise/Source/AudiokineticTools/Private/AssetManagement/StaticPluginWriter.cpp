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

#include "StaticPluginWriter.h"

#include "AkAudioBankGenerationHelpers.h"
#include "IAudiokineticTools.h"
#include "Platforms/AkPlatformInfo.h"
#include "Platforms/AkUEPlatform.h"

#include "Containers/Map.h"
#include "Misc/FileHelper.h"

namespace StaticPluginWriter_Helper
{
	// known packaged plug-in IDs
	enum class PluginID
	{
		Invalid = 0x0,

		Ak3DAudioBedMixer = 0x00BE0003,
		AkCompressor = 0x006C0003,
		AkDelay = 0x006A0003,
		AkExpander = 0x006D0003,
		AkGain = 0x008B0003,
		AkMatrixReverb = 0x00730003,
		AkMeter = 0x00810003,
		AkParametricEQ = 0x00690003,
		AkPeakLimiter = 0x006E0003,
		AkRoomVerb = 0x00760003,
		SineGenerator = 0x00640002,
		SinkAuxiliary = 0xB40007, // *
		SinkCommunication = 0xB00007, // *
		SinkControllerHeadphones = 0xB10007, // *
		SinkControllerSpeaker = 0xB30007, // *
		SinkDVRByPass = 0xAF0007, // *
		SinkNoOutput = 0xB50007, // *
		SinkSystem = 0xAE0007, // *
		ToneGenerator = 0x00660002,
		WwiseSilence = 0x00650002,

		AkAudioInput = 0xC80002,
		AkConvolutionReverb = 0x7F0003,
		AkFlanger = 0x7D0003,
		AkGuitarDistortion = 0x7E0003,
		AkHarmonizer = 0x8A0003,
		AkMotionSink = 0x1FB0007,
		AkMotionSource = 0x1990002,
		AkPitchShifter = 0x880003,
		AkRecorder = 0x840003,
		AkReflect = 0xAB0003,
		AkSoundSeedGrain = 0xB70002,
		AkSoundSeedWind = 0x770002,
		AkSoundSeedWoosh = 0x780002,
		AkStereoDelay = 0x870003,
		AkSynthOne = 0x940002,
		AkTimeStretch = 0x820003,
		AkTremolo = 0x830003,
		AuroHeadphone = 0x44C1073,
		CrankcaseAudioREVModelPlayer = 0x1A01052,
		iZHybridReverb = 0x21033,
		iZTrashBoxModeler = 0x71033,
		iZTrashDelay = 0x41033,
		iZTrashDistortion = 0x31033,
		iZTrashDynamics = 0x51033,
		iZTrashFilters = 0x61033,
		iZTrashMultibandDistortion = 0x91033,
		McDSPFutzBox = 0x6E1003,
		McDSPLimiter = 0x671003,
		ResonanceAudio = 0x641103,
	};

	const TMap<PluginID, const char*> PluginIDToStaticLibraryName =
	{
		{ PluginID::Ak3DAudioBedMixer, "Ak3DAudioBedMixer" },
		{ PluginID::AkAudioInput, "AkAudioInputSource" },
		{ PluginID::AkCompressor, "AkCompressorFX" },
		{ PluginID::AkConvolutionReverb, "AkConvolutionReverbFX" },
		{ PluginID::AkDelay, "AkDelayFX" },
		{ PluginID::AkExpander, "AkExpanderFX" },
		{ PluginID::AkFlanger, "AkFlangerFX" },
		{ PluginID::AkGain, "AkGainFX" },
		{ PluginID::AkGuitarDistortion, "AkGuitarDistortionFX" },
		{ PluginID::AkHarmonizer, "AkHarmonizerFX" },
		{ PluginID::AkMatrixReverb, "AkMatrixReverbFX" },
		{ PluginID::AkMeter, "AkMeterFX" },
		{ PluginID::AkMotionSink, "AkMotionSink" },
		{ PluginID::AkMotionSource, "AkMotionSourceSource" },
		{ PluginID::AkParametricEQ, "AkParametricEQFX" },
		{ PluginID::AkPeakLimiter, "AkPeakLimiterFX" },
		{ PluginID::AkPitchShifter, "AkPitchShifterFX" },
		{ PluginID::AkRecorder, "AkRecorderFX" },
		{ PluginID::AkReflect, "AkReflectFX" },
		{ PluginID::AkRoomVerb, "AkRoomVerbFX" },
		{ PluginID::WwiseSilence, "AkSilenceSource" },
		{ PluginID::SineGenerator, "AkSineSource" },
		{ PluginID::AkSoundSeedGrain, "AkSoundSeedGrainSource" },
		{ PluginID::AkSoundSeedWind, "AkSoundSeedWindSource" },
		{ PluginID::AkSoundSeedWoosh, "AkSoundSeedWooshSource" },
		{ PluginID::AkStereoDelay, "AkStereoDelayFX" },
		{ PluginID::AkSynthOne, "AkSynthOneSource" },
		{ PluginID::ToneGenerator, "AkToneSource" },
		{ PluginID::AkTimeStretch, "AkTimeStretchFX" },
		{ PluginID::AkTremolo, "AkTremoloFX" },
		{ PluginID::AuroHeadphone, "AuroHeadphoneFX" },
		{ PluginID::CrankcaseAudioREVModelPlayer, "CrankcaseAudioREVModelPlayerFX" },
		{ PluginID::iZHybridReverb, "iZHybridReverbFX" },
		{ PluginID::iZTrashBoxModeler, "iZTrashBoxModelerFX" },
		{ PluginID::iZTrashDelay, "iZTrashDelayFX" },
		{ PluginID::iZTrashDistortion, "iZTrashDistortionFX" },
		{ PluginID::iZTrashDynamics, "iZTrashDynamicsFX" },
		{ PluginID::iZTrashFilters, "iZTrashFiltersFX" },
		{ PluginID::iZTrashMultibandDistortion, "iZTrashMultibandDistortionFX" },
		{ PluginID::McDSPFutzBox, "McDSPFutzBoxFX" },
		{ PluginID::McDSPLimiter, "McDSPLimiterFX" },
		{ PluginID::ResonanceAudio, "ResonanceAudioFX" },
	};

	FString GetLibraryName(const FAkPluginInfo& PluginInfo)
	{
		auto pluginID = static_cast<PluginID>(PluginInfo.PluginID);

		if (pluginID != PluginID::Invalid)
		{
			if (auto StaticLibraryName = PluginIDToStaticLibraryName.FindRef(pluginID))
			{
				return StaticLibraryName;
			}
		}

		return PluginInfo.DLL;
	}

	TArray<FString> GetLibraryFileNames(const FString& Platform)
	{
		TArray<FString> Result;
		FWwiseProjectDatabase* ProjectDatabase = FWwiseProjectDatabase::Get();
		
		if(UNLIKELY(!ProjectDatabase))
		{
			return {};
		}

		const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);

		WwisePluginLibGlobalIdsMap PluginLibs = DataStructure.GetPluginLibs();
		for (const auto& PluginLib: PluginLibs)
		{
			if(auto PluginLibName = PluginIDToStaticLibraryName.Find((PluginID)PluginLib.Value.PluginLibId()))
			{
				Result.Add(*PluginLibName);				
			}
		}

		return Result;
	}
	
	TArray<FString> GetLibraryNames(const FString& Platform)
	{
		TArray<FString> Result;
		FWwiseProjectDatabase* ProjectDatabase = FWwiseProjectDatabase::Get();
		const FWwiseDataStructureScopeLock DataStructure(*ProjectDatabase);

		WwisePluginLibGlobalIdsMap PluginLibs = DataStructure.GetPluginLibs();
		for (const auto& PluginLib: PluginLibs)
		{
			FString PluginLibName = PluginLib.Value.PluginLibName().ToString();
			Result.Add(PluginLibName);
		}

		return Result;
	};

	void ModifySourceCode(const TCHAR* FilePath, const FString& PlatformName, const TArray<FString>& Plugins)
	{
		constexpr auto AkFactoryHeaderFormatString = TEXT("#include <AK/Plugin/{0}Factory.h>");

		constexpr auto StaticFactoryHeaderFileTemplate = TEXT("#if PLATFORM_{0}\n{1}\n#endif\n");

		TArray<FString> PluginLines;

		for (auto& PluginName : Plugins)
		{
			//Don't include AkAudioInputSourceFactory.h in file because it is already linked
			if (PluginName.Equals(TEXT("AkAudioInputSource")))
			{
				continue;
			}
			PluginLines.Add(FString::Format(AkFactoryHeaderFormatString, { PluginName }));
		}


		TArray<FString> Lines;
		FFileHelper::LoadFileToStringArray(Lines, FilePath);

		TArray<FString> Header;
		for(auto& Line : Lines)
		{
			if(Line.Len() > 0)
			{
				Header.Add(Line);
			}
			else
			{
				break;
			}
		}

		FString HeaderString = FString::Join(Header, TEXT("\n"));
		HeaderString.Append(TEXT("\n"));
		HeaderString.Append(TEXT("\n"));
		
		FString StaticFactoryHeaderContent = FString::Format(StaticFactoryHeaderFileTemplate, { PlatformName.ToUpper(), FString::Join(PluginLines, TEXT("\n")) });

		HeaderString.Append(StaticFactoryHeaderContent);
		if (FFileHelper::SaveStringToFile(HeaderString, FilePath))
		{
			UE_LOG(LogAudiokineticTools, Display, TEXT("Modified <%s> for <%s> platform."), FilePath, *PlatformName);
		}
		else
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("Could not modify <%s> for <%s> platform."), FilePath, *PlatformName);
		}
	}
}

namespace StaticPluginWriter
{
	void OutputPluginInformation(const FString& Platform)
	{
		auto* PlatformInfo = UAkPlatformInfo::GetAkPlatformInfo(Platform);
		if (!PlatformInfo)
		{
			UE_LOG(LogAudiokineticTools, Warning, TEXT("AkPlatformInfo class not found for <%s> platform."), *Platform);
			return;
		}

		if (PlatformInfo->bUsesStaticLibraries)
		{
			const auto PluginArray = StaticPluginWriter_Helper::GetLibraryFileNames(Platform);

			const FString AkPluginIncludeFileName = FString::Format(TEXT("Ak{0}Plugins.h"), { PlatformInfo->WwisePlatform });
			const FString AkPluginIncludeFilePath = FPaths::Combine(FAkPlatform::GetWwisePluginDirectory(), TEXT("Source"), TEXT("WwiseSoundEngine"), TEXT("Public"), TEXT("Generated"), AkPluginIncludeFileName);

			StaticPluginWriter_Helper::ModifySourceCode(*AkPluginIncludeFilePath, PlatformInfo->WwisePlatform, PluginArray);
		}
	}
}
