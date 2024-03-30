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

using UnrealBuildTool;
using System.IO;
using System.Linq;
using System.Collections.Generic;

#if UE_5_3_OR_LATER
using Microsoft.Extensions.Logging;
#elif UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public struct WwiseSoundEngine_2022_1
{
	private static List<string> AkLibs = new List<string> 
	{
		"AkSoundEngine",
		"AkMemoryMgr",
		"AkStreamMgr",
		"AkMusicEngine",
		"AkSpatialAudio",
		"AkAudioInputSource",
		"AkVorbisDecoder",
		"AkMeterFX", // AkMeter does not have a dedicated DLL
	};
	
	public static void Apply(WwiseSoundEngine SE, ReadOnlyTargetRules Target, bool Latest = false)
	{
#if UE_5_3_OR_LATER
		ILogger Logger = Target.Logger;
#endif
		var TargetAkLibs = new List<string>(AkLibs);
		var VersionNumber = "2022_1";
		var ModuleName = "WwiseSoundEngine_" + VersionNumber;
		var ModuleDirectory = Path.Combine(SE.ModuleDirectory, "../" + ModuleName);

		if (!WwiseSoundEngineVersion.IsSoundEngineVersionSupported(SE.PluginDirectory, ModuleName))
		{
			// We are skipping this version since this Wwise Sound Engine is for a particular version only.
			return;
		}

		SE.PublicDefinitions.AddRange(WwiseSoundEngineVersion.GetVersionDefinesFromClassName(ModuleName));

		// If packaging as an Engine plugin, the UBT expects to already have a precompiled plugin available
		// This can be set to true so long as plugin was already precompiled
		SE.bUsePrecompiled = false;
		SE.bPrecompile = false;

		string ThirdPartyFolder = Path.Combine(SE.ModuleDirectory, "../../ThirdParty");
		var WwiseUEPlatformInstance = WwiseUEPlatform.GetWwiseUEPlatformInstance(Target, VersionNumber, ThirdPartyFolder);
		SE.PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		SE.bAllowConfidentialPlatformDefines = true;

		if (!Latest)
		{
#if UE_5_3_OR_LATER
			Logger.LogInformation("Using Wwise SoundEngine {VersionNumber} [{ConfigurationDir}]", VersionNumber, WwiseUEPlatformInstance.WwiseConfigurationDir);
#else
			Log.TraceInformation("Using Wwise SoundEngine {0} [{1}]", VersionNumber, WwiseUEPlatformInstance.WwiseConfigurationDir);
#endif
		}
		
		var IsWwiseTargetSupported = WwiseUEPlatformInstance.IsWwiseTargetSupported();
		SE.AddSoundEngineDirectory("WwiseSoundEngine_" + VersionNumber, IsWwiseTargetSupported);
		SE.AddVersionHeaders("WwiseSoundEngine_" + VersionNumber, IsWwiseTargetSupported);

		foreach (var Platform in GetAvailablePlatforms(ModuleDirectory))
		{
			SE.ExternalDependencies.Add(string.Format("{0}/WwiseUEPlatform_{1}_{2}.Build.cs", ModuleDirectory, VersionNumber, Platform));
		}
		
		if (Target.bBuildEditor)
		{
			foreach (var Platform in GetAvailablePlatforms(ModuleDirectory))
			{
				SE.PublicDefinitions.Add("AK_PLATFORM_" + Platform.ToUpper());
			}
		}

		SE.PublicIncludePaths.Add(Path.Combine(ThirdPartyFolder, "include"));

		SE.PublicDefinitions.Add("AK_UNREAL_MAX_CONCURRENT_IO=32");
		SE.PublicDefinitions.Add("AK_UNREAL_IO_GRANULARITY=32768");
		if (Target.Configuration == UnrealTargetConfiguration.Shipping || Target.Configuration == UnrealTargetConfiguration.Test || !IsWwiseTargetSupported)
		{
			SE.PublicDefinitions.Add("AK_OPTIMIZED");
		}
		else
		{
			SE.PublicDefinitions.Add("AK_ENABLE_ASSERTS");
		}

		if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test && WwiseUEPlatformInstance.SupportsCommunication)
		{
			TargetAkLibs.Add("CommunicationCentral");
			SE.PublicDefinitions.Add("AK_ENABLE_COMMUNICATION=1");
		}
		else
		{
			SE.PublicDefinitions.Add("AK_ENABLE_COMMUNICATION=0");
		}

		if (WwiseUEPlatformInstance.SupportsAkAutobahn)
		{
			TargetAkLibs.Add("AkAutobahn");
			SE.PublicDefinitions.Add("AK_SUPPORT_WAAPI=1");
		}
		else
		{
			SE.PublicDefinitions.Add("AK_SUPPORT_WAAPI=0");
		}
		
		if (WwiseUEPlatformInstance.SupportsOpus)
		{
			TargetAkLibs.Add("AkOpusDecoder");
			SE.PublicDefinitions.Add("AK_SUPPORT_OPUS=1");
		}
		else
		{
			SE.PublicDefinitions.Add("AK_SUPPORT_OPUS=0");
		}

		if (WwiseUEPlatformInstance.SupportsDeviceMemory)
		{
			SE.PublicDefinitions.Add("AK_SUPPORT_DEVICE_MEMORY=1");
		}
		else
		{
			SE.PublicDefinitions.Add("AK_SUPPORT_DEVICE_MEMORY=0");
		}

		// Platform-specific dependencies
		SE.PublicDefinitions.AddRange(WwiseUEPlatformInstance.GetPublicDefinitions());
		SE.PublicDefinitions.Add(string.Format("WWISE_CONFIGURATION_DIR=\"{0}\"", WwiseUEPlatformInstance.WwiseConfigurationDir));
		SE.PublicDefinitions.Add(string.Format("WWISE_DSP_DIR=\"{0}\"", WwiseUEPlatformInstance.WwiseDspDir));

        if (IsWwiseTargetSupported)
        {
            SE.PublicSystemLibraries.AddRange(WwiseUEPlatformInstance.GetPublicSystemLibraries());
            TargetAkLibs.AddRange(WwiseUEPlatformInstance.GetAdditionalWwiseLibs());
            var AdditionalProperty = WwiseUEPlatformInstance.GetAdditionalPropertyForReceipt(ModuleDirectory);
            if (AdditionalProperty != null)
            {
                SE.AdditionalPropertiesForReceipt.Add(AdditionalProperty.Item1, AdditionalProperty.Item2);
            }

            SE.PublicFrameworks.AddRange(WwiseUEPlatformInstance.GetPublicFrameworks());

            SE.PublicDelayLoadDLLs.AddRange(WwiseUEPlatformInstance.GetPublicDelayLoadDLLs());
            foreach (var RuntimeDependency in WwiseUEPlatformInstance.GetRuntimeDependencies())
            {
                SE.RuntimeDependencies.Add(RuntimeDependency);
            }

            SE.PublicAdditionalLibraries.AddRange(WwiseUEPlatformInstance.GetSanitizedAkLibList(TargetAkLibs));
        }
    }

	private static List<string> GetAvailablePlatforms(string ModuleDir)
	{
		var FoundPlatforms = new List<string>();
		const string StartPattern = "WwiseUEPlatform_";
		const string EndPattern = ".Build.cs";
		foreach (var BuildCsFile in System.IO.Directory.GetFiles(ModuleDir, "*" + EndPattern))
		{
			if (BuildCsFile.Contains(StartPattern) && BuildCsFile.EndsWith(EndPattern))
			{
				var Platform = BuildCsFile.Remove(BuildCsFile.Length - EndPattern.Length).Split('_').Last();
				FoundPlatforms.Add(Platform);
			}
		}

		return FoundPlatforms;
	}
}
