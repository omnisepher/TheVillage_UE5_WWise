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

using System.IO;
using UnrealBuildTool;

public class WwiseSoundEngine : ModuleRules
{
	public WwiseSoundEngine(ReadOnlyTargetRules Target) : base(Target)
	{
		WwiseSoundEngine_2022_1.Apply(this, Target);
		WwiseSoundEngine_2023_1.Apply(this, Target, true);		// Latest version should be written with "latest" to true for logging purposes
		WwiseSoundEngine_Null.Apply(this, Target);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"WwiseUtils"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
		);

		ExternalDependencies.Add("WwiseSoundEngineVersion.Build.cs");
		ExternalDependencies.Add("WwiseUEPlatform.Build.cs");

		bAllowConfidentialPlatformDefines = true;
	}

	public void AddSoundEngineDirectory(string Module, bool TargetSupported)
	{
        if (TargetSupported)
        {
#if UE_5_0_OR_LATER
            ConditionalAddModuleDirectory(
                EpicGames.Core.DirectoryReference.Combine(new EpicGames.Core.DirectoryReference(ModuleDirectory), "..",
                    Module));
#else
		    ConditionalAddModuleDirectory(
			    Tools.DotNETCommon.DirectoryReference.Combine(new Tools.DotNETCommon.DirectoryReference(ModuleDirectory),  "..", Module));
#endif
        }

        ExternalDependencies.Add(Path.Combine(ModuleDirectory, "..", Module, Module + "_OptionalModule.Build.cs"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "..", Module, "Public"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", Module, "Private"));

#if UE_5_3_OR_LATER
		bLegacyParentIncludePaths = false;
		CppStandard = CppStandardVersion.Default;
#endif
	}

    public void AddVersionHeaders(string Module, bool TargetSupported)
    {
        if (TargetSupported)
        {
            PrivateDefinitions.Add("WWISE_SOUNDENGINE_VERSION_HEADER_PATH=\"Wwise/" + Module + ".h\"");
            PrivateDefinitions.Add("WWISE_SOUNDENGINE_VERSION_CLASS=F" + Module);
            PublicDefinitions.Add("AK_USE_NULL_SOUNDENGINE=0");
        }
        else
        {
            PrivateDefinitions.Add("WWISE_SOUNDENGINE_VERSION_HEADER_PATH=\"Wwise/WwiseSoundEngine_Null.h\"");
            PrivateDefinitions.Add("WWISE_SOUNDENGINE_VERSION_CLASS=FWwiseSoundEngine_Null");
            PublicDefinitions.Add("AK_USE_NULL_SOUNDENGINE=1");
        }
    }
}
