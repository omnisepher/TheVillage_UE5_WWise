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

public struct WwiseSoundEngine_Null
{
	private static List<string> AkLibs = new List<string> 
	{
	};
	
	public static void Apply(WwiseSoundEngine SE, ReadOnlyTargetRules Target)
	{
		var VersionNumber = "Null";
		var ModuleName = "WwiseSoundEngine_" + VersionNumber;
		var ModuleDirectory = Path.Combine(SE.ModuleDirectory, "../" + ModuleName);

		SE.AddSoundEngineDirectory("WwiseSoundEngine_" + VersionNumber, true);
		
		// If packaging as an Engine plugin, the UBT expects to already have a precompiled plugin available
		// This can be set to true so long as plugin was already precompiled
		SE.bUsePrecompiled = false;
		SE.bPrecompile = false;

		string ThirdPartyFolder = Path.Combine(SE.ModuleDirectory, "../../ThirdParty");
		var WwiseUEPlatformInstance = WwiseUEPlatform.GetWwiseUEPlatformInstance(Target, VersionNumber, ThirdPartyFolder);
		SE.PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		SE.bAllowConfidentialPlatformDefines = true;

		foreach (var Platform in GetAvailablePlatforms(ModuleDirectory))
		{
			SE.ExternalDependencies.Add(string.Format("{0}/WwiseUEPlatform_{1}.Build.cs", ModuleDirectory, VersionNumber, Platform));
		}
		
		if (Target.bBuildEditor)
		{
			foreach (var Platform in GetAvailablePlatforms(ModuleDirectory))
			{
				SE.PublicDefinitions.Add("AK_PLATFORM_" + Platform.ToUpper());
			}
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
