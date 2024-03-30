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
using System;
using System.IO;
using System.Collections.Generic;

#if UE_5_3_OR_LATER
using Microsoft.Extensions.Logging;
#elif UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

// Platform-specific files implement this interface, returning their particular dependencies, defines, etc.
public abstract class WwiseUEPlatform
{
	protected ReadOnlyTargetRules Target;
	protected string ThirdPartyFolder;
#if UE_5_3_OR_LATER
	private ILogger Logger => Target.Logger;
#endif
	
	public WwiseUEPlatform(ReadOnlyTargetRules in_Target, string in_ThirdPartyFolder)
	{
		Target = in_Target;
		ThirdPartyFolder = in_ThirdPartyFolder;
	}

    public bool IsWwiseTargetSupported()
    {
        var platformPath = Path.Combine(ThirdPartyFolder, AkPlatformLibDir);
        var hasPlatform = Directory.Exists(platformPath);
        var supportedTargetType = Target.Type != TargetRules.TargetType.Server && Target.Type != TargetRules.TargetType.Program;
#if UE_5_3_OR_LATER
        if (!supportedTargetType)
        {
	        Logger.LogInformation("Wwise SoundEngine is disabled: Using the null SoundEngine instead. Unsupported {Target} target type for Wwise SoundEngine", Target.Type.ToString());
        }
        else if (!hasPlatform)
        {
	        Logger.LogInformation("Wwise SoundEngine is disabled: Using the null SoundEngine instead. Could not find the {Platform} platform folder in ThirdParty", AkPlatformLibDir.ToString());
        }
#else
        if (!supportedTargetType)
        {
	        Log.TraceInformation("Wwise SoundEngine is disabled: Using the null SoundEngine instead. Unsupported {0} target type for Wwise SoundEngine", Target.Type.ToString());
        }
        else if (!hasPlatform)
        {
	        Log.TraceInformation("Wwise SoundEngine is disabled: Using the null SoundEngine instead. Could not find the {0} platform folder in ThirdParty", AkPlatformLibDir.ToString());
        }
#endif
        return hasPlatform && supportedTargetType;
    }

	public static WwiseUEPlatform GetWwiseUEPlatformInstance(ReadOnlyTargetRules Target, string VersionNumber, string ThirdPartyFolder)
	{
		var WwiseUEPlatformType = System.Type.GetType(
            VersionNumber == "Null" ? "WwiseUEPlatform_Null" :
            "WwiseUEPlatform_" + VersionNumber + "_" + Target.Platform.ToString());
		if (WwiseUEPlatformType == null)
		{
			throw new BuildException("Wwise does not support platform " + Target.Platform.ToString() + " on " + VersionNumber);
		}

		var PlatformInstance = Activator.CreateInstance(WwiseUEPlatformType, Target, ThirdPartyFolder) as WwiseUEPlatform;
		if (PlatformInstance == null)
		{
			throw new BuildException("Wwise could not instantiate platform " + Target.Platform.ToString() + " on " + VersionNumber);
		}

		return PlatformInstance;
	}

	protected static List<string> GetAllLibrariesInFolder(string LibFolder, string Extension, bool RemoveLibPrefix = true, bool GetFullPath = false)
	{
		List<string> ret = null;
		var FoundLibs = Directory.GetFiles(LibFolder, "*."+Extension);

		if (GetFullPath)
		{
			ret = new List<string>(FoundLibs);
		}
		else
		{
			ret = new List<string>();
			foreach (var Library in FoundLibs)
			{
				var LibName = Path.GetFileNameWithoutExtension(Library);
				if (RemoveLibPrefix && LibName.StartsWith("lib"))
				{
					LibName = LibName.Remove(0, 3);
				}
				ret.Add(LibName);
			}

		}

		ret.Sort();
		return ret;
	}

	/// <summary>
	/// Wwise Target Configuration based on Unreal Target Configuration. 
	/// </summary>
	public string WwiseConfiguration
	{
		get
		{
			switch (Target.Configuration)
			{
				case UnrealTargetConfiguration.Debug:
				case UnrealTargetConfiguration.DebugGame:
					return "Debug";

				case UnrealTargetConfiguration.Development:
				default:
					return "Profile";

				case UnrealTargetConfiguration.Test:
				case UnrealTargetConfiguration.Shipping:
					return "Release";
			}
		}
	}

	/// <summary>
	/// Wwise Library's Configuration folder (e.g.: ThirdParty/Platform/ConfigurationDir/lib).
	/// </summary>
	public virtual string WwiseConfigurationDir
	{
		get
		{
			return WwiseConfiguration;
		}
	}

	/// <summary>
	/// Wwise Library's DSP folder (e.g.: ThirdParty/Platform/DspDir/bin). 
	/// </summary>
	public virtual string WwiseDspDir
	{
		get
		{
			return WwiseConfiguration;
		}
	}
	
	public abstract string GetLibraryFullPath(string LibName, string LibPath);
	public abstract bool SupportsAkAutobahn { get; }
	public abstract bool SupportsCommunication { get; }
	public abstract bool SupportsDeviceMemory { get; }
	public abstract string AkPlatformLibDir { get; }
	public abstract string DynamicLibExtension { get; }
	public virtual bool SupportsOpus { get { return true; } }

	public virtual List<string> GetPublicLibraryPaths()
	{
		return new List<string>
		{
			Path.Combine(ThirdPartyFolder, AkPlatformLibDir, WwiseConfigurationDir, "lib")
		};
	}

	public virtual List<string> GetRuntimeDependencies()
	{
		string PluginsDir = WwiseDspDir;
		List<string> Result = GetAllLibrariesInFolder(Path.Combine(ThirdPartyFolder, AkPlatformLibDir, WwiseDspDir, "bin"), DynamicLibExtension, false, true);
		return Result;
	}

	public abstract List<string> GetAdditionalWwiseLibs();
	public abstract List<string> GetPublicSystemLibraries();
	public abstract List<string> GetPublicDelayLoadDLLs();
	public abstract List<string> GetPublicDefinitions();
	public abstract Tuple<string, string> GetAdditionalPropertyForReceipt(string ModuleDirectory);
	public abstract List<string> GetPublicFrameworks();
	
	public virtual List<string> GetSanitizedAkLibList(List<string> AkLibs)
	{
		List<string> SanitizedLibs = new List<string>();
		foreach(var lib in AkLibs)
		{
			foreach(var libPath in GetPublicLibraryPaths())
			{
				SanitizedLibs.Add(GetLibraryFullPath(lib, libPath));
			}
		}
		
		return SanitizedLibs;
	}
}
