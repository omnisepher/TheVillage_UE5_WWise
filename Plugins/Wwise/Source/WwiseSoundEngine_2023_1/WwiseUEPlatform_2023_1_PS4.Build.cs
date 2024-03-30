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

public class WwiseUEPlatform_2023_1_PS4 : WwiseUEPlatform
{
	public WwiseUEPlatform_2023_1_PS4(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public override string GetLibraryFullPath(string LibName, string LibPath)
	{		
		return Path.Combine(LibPath, "lib" + LibName + ".a");
	}

	public override bool SupportsAkAutobahn { get { return false; } }

	public override bool SupportsCommunication { get { return true; } }

	public override bool SupportsDeviceMemory { get { return false; } }

	public override string AkPlatformLibDir 
	{ 
		get 
		{
			string ExpectedSdkVersion = Environment.ExpandEnvironmentVariables("%SCE_ORBIS_SDK_DIR%");
			return "PS4_SDK" + System.IO.Path.GetFileName(ExpectedSdkVersion); 
		}
	}
	
	public override string DynamicLibExtension { get { return "prx"; } }

	public override List<string> GetAdditionalWwiseLibs()
	{
		return new List<string>();
	}

	public override List<string> GetPublicSystemLibraries()
	{
		return new List<string>
		{
			"SceAjm_stub_weak",
			"SceAudio3d_stub_weak",
			"SceMove_stub_weak"
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
			"__ORBIS__"
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
}
