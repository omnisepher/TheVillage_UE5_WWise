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

public abstract class WwiseUEPlatform_2023_1_Windows : WwiseUEPlatform
{
	public WwiseUEPlatform_2023_1_Windows(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public abstract string PlatformPrefix { get; }

	public override string GetLibraryFullPath(string LibName, string LibPath)
	{
		// Fix: AkAutobahn Debug requires different RuntimeLibrary
		if (LibName == "AkAutobahn")
		{
			LibPath = LibPath.Replace("Debug", "Profile");
			LibPath = LibPath.Replace("(StaticCRT)", "");
		}
		
		return Path.Combine(LibPath, LibName + ".lib");
	}

	public override bool SupportsAkAutobahn { get { return Target.Configuration != UnrealTargetConfiguration.Shipping; } }

	public override bool SupportsCommunication { get { return true; } }

	public override bool SupportsDeviceMemory { get { return false; } }

	public override bool SupportsOpus { get { return true; } }

	public override string AkPlatformLibDir { get { return PlatformPrefix + "_" + GetVisualStudioVersion(); } }

	public override string DynamicLibExtension { get { return "dll"; } }

	public override List<string> GetAdditionalWwiseLibs()
	{
		return new List<string>();
	}
	
	public override List<string> GetPublicSystemLibraries()
	{
		return new List<string> 
		{
			"dsound.lib",
			"dxguid.lib",
			"Msacm32.lib",
			"XInput.lib",
			"dinput8.lib"
		};
	}

	public override List<string> GetPublicDelayLoadDLLs()
	{
		return new List<string>();
	}

	public override List<string> GetPublicDefinitions()
	{
		return new List<string>
		{
			"AK_WINDOWS_VS_VERSION=\"" + GetVisualStudioVersion() + "\""
		};
	}

	public override Tuple<string, string> GetAdditionalPropertyForReceipt(string ModuleDirectory)
	{
		return null;
	}

	public override List<string> GetPublicFrameworks()
	{
		return new List<string>();
	}

	private static void CheckCompilerVersion(ref string Version, WindowsCompiler Compiler, string LongVersionName, string ShortVersionName)
	{
		try
		{
			if (Compiler == (WindowsCompiler)Enum.Parse(typeof(WindowsCompiler), LongVersionName))
				Version = ShortVersionName;
		}
		catch
		{
		}
	}

	private string GetVisualStudioVersion()
	{
		string VSVersion = "vc160";
		var Compiler = Target.WindowsPlatform.Compiler;
		CheckCompilerVersion(ref VSVersion, Compiler, "VisualStudio2022", "vc170");
		CheckCompilerVersion(ref VSVersion, Compiler, "VisualStudio2019", "vc160");
		CheckCompilerVersion(ref VSVersion, Compiler, "VisualStudio2017", "vc150");
		CheckCompilerVersion(ref VSVersion, Compiler, "VisualStudio2015", "vc140");
		CheckCompilerVersion(ref VSVersion, Compiler, "VisualStudio2013", "vc120");
		return VSVersion;
	}

	public override string WwiseConfigurationDir
	{
		get
		{
			var Configuration = base.WwiseConfigurationDir;
			var UpdatedConfiguration = Configuration + "(StaticCRT)";
			var StaticFolder = Path.Combine(ThirdPartyFolder, AkPlatformLibDir, UpdatedConfiguration);
			if (System.IO.Directory.Exists(StaticFolder))
			{
				return UpdatedConfiguration;
			}
			else if (Configuration == "Debug" && !Target.bDebugBuildsActuallyUseDebugCRT)
			{
				return "Profile";
			}
			else
			{
				return Configuration;
			}
		}
	}
}

public class WwiseUEPlatform_2023_1_Win32 : WwiseUEPlatform_2023_1_Windows
{
	public WwiseUEPlatform_2023_1_Win32(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}
	public override string PlatformPrefix { get { return "Win32"; } }
}

public class WwiseUEPlatform_2023_1_Win64 : WwiseUEPlatform_2023_1_Windows
{
	public WwiseUEPlatform_2023_1_Win64(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}
	public override string PlatformPrefix { get { return "x64"; } }
}
