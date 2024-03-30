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

public class WwiseUEPlatform_2023_1_XSX : WwiseUEPlatform
{
	public WwiseUEPlatform_2023_1_XSX(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public override string GetLibraryFullPath(string LibName, string LibPath)
	{
		return Path.Combine(LibPath, LibName + ".lib");
	}

	public override bool SupportsAkAutobahn { get { return false; } }

	public override bool SupportsCommunication { get { return true; } }

	public override bool SupportsDeviceMemory { get { return true; } }

	public override string AkPlatformLibDir { get { return "XboxSeriesX_" + GetVisualStudioVersion(); } }

	public override string DynamicLibExtension { get { return "dll"; } }

	public override List<string> GetAdditionalWwiseLibs()
	{
		return new List<string>();
	}
	
	public override List<string> GetPublicSystemLibraries()
	{
		return new List<string>
		{
			"AcpHal.lib",
			"xapu.lib",
			"MMDevApi.lib"
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
			"_GAMING_XBOX_SCARLETT",
			"AK_XBOXSERIESX_VS_VERSION=\"" + GetVisualStudioVersion() + "\"",
			"AK_XBOXONE_COMMON"
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
}

public class WwiseUEPlatform_2023_1_MPX : WwiseUEPlatform_2023_1_XSX
{
	public WwiseUEPlatform_2023_1_MPX(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}
}