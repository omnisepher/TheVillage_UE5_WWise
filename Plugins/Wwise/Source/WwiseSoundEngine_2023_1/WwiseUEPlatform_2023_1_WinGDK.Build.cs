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

public class WwiseUEPlatform_2023_1_WinGDK : WwiseUEPlatform
{
	public WwiseUEPlatform_2023_1_WinGDK(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public override string GetLibraryFullPath(string LibName, string LibPath)
	{
		return Path.Combine(LibPath, LibName + ".lib");
	}

	public override bool SupportsAkAutobahn { get { return false; } }

	public override bool SupportsCommunication { get { return true; } }

	public override bool SupportsDeviceMemory { get { return false; } }

	public override bool SupportsOpus { get { return true; } }

	public override string AkPlatformLibDir { get { return "WinGC_" + GetVisualStudioVersion(); } }

	public override string DynamicLibExtension { get { return "dll"; } }

	public override List<string> GetPublicLibraryPaths()
	{
		return new List<string> { Path.Combine(ThirdPartyFolder, AkPlatformLibDir, WwiseConfigurationDir, "lib") };
	}

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
			"AK_WINDOWSGC",
			"AK_WINGC_VS_VERSION=\"" + GetVisualStudioVersion() + "\""
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

	private string GetVisualStudioVersion()
	{
		// TODO: WinAnvilPlatform does not have a Compiler property
		return "vc160";
	}
}

public class WwiseUEPlatform_2023_1_WinAnvil : WwiseUEPlatform_2023_1_WinGDK
{
	public WwiseUEPlatform_2023_1_WinAnvil(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}
}