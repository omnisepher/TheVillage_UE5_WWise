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

#include "Wwise/AudioLink/WwiseAudioLinkSettingsFactory.h"
#include "Wwise/AudioLink/WwiseAudioLinkSettings.h"

#include "ISettingsModule.h"

class FWwiseAudioLinkEditorModule
{
public:
	FWwiseAudioLinkEditorModule()
	{
		// Register asset types		
		IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
		{
			AssetActions = MakeShared<FAssetTypeActions_WwiseAudioLinkSettings>();
			AssetTools.RegisterAssetTypeActions(AssetActions.ToSharedRef());
			
			if (ISettingsModule* SettingsModule = FModuleManager::Get().GetModulePtr<ISettingsModule>("Settings"))
			{
				SettingsModule->RegisterSettings("Project", "Wwise", "Wwise AudioLink", NSLOCTEXT("WwiseAudioLink", "Wwise AudioLink", "Wwise AudioLink"),
					NSLOCTEXT("WwiseAudioLink", "Configure AudioLink for Wwise Settings", "Configure AudioLink for Wwise Settings"), GetMutableDefault<UWwiseAudioLinkSettings>());
			}
		}
	}
	~FWwiseAudioLinkEditorModule()
	{
		if (FAssetToolsModule::IsModuleLoaded())
		{
			IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
			AssetTools.UnregisterAssetTypeActions(AssetActions.ToSharedRef());
		}
		AssetActions.Reset();
	}
private:
	TSharedPtr<FAssetTypeActions_WwiseAudioLinkSettings> AssetActions;
};
