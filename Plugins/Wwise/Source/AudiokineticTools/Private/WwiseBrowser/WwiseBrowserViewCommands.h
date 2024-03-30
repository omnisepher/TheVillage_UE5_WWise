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

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AkAudioStyle.h"

#define LOCTEXT_NAMESPACE "WwiseBrowserViewCommands"


class FWwiseBrowserViewCommands : public TCommands<FWwiseBrowserViewCommands>
{

public:

	/** FWwiseBrowserViewCommands Constructor */
	FWwiseBrowserViewCommands() : TCommands<FWwiseBrowserViewCommands>
		(
			"WwiseBrowserViewCommand", // Context name for fast lookup
			NSLOCTEXT("Contexts", "WwiseBrowserViewCommand", "Wwise Browser Command"), // Localized context name for displaying
			NAME_None, // Parent
			FAkAudioStyle::GetStyleSetName() // Icon Style Set
			)
	{
	}

	/**
	 * Initialize the commands
	 */
	virtual void RegisterCommands() override
	{
		UI_COMMAND(RequestPlayWwiseItem, "Play/Stop", "Plays or stops the selected items.", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar));
		UI_COMMAND(RequestStopAllWwiseItem, "Stop All", "Stop all playing events", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(RequestExploreWwiseItem, "Show in Folder", "Finds this item on disk.", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(RequestFindInProjectExplorerWwiseItem, "Find in the Wwise Project Explorer", "Finds the specified object in the Project Explorer (Sync Group 1).", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::One));
		UI_COMMAND(RequestFindInContentBrowser, "Find in Content Browser ", "Locates the corresponding item inside the Content Browser", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(RequestRefreshWwiseBrowser, "Refresh All", "Populates the Wwise Browser.", EUserInterfaceActionType::Button, FInputChord(EKeys::F5));
		UI_COMMAND(RequestImportWwiseItem, "Import Selected Assets", "Imports the selected assets from the Wwise Browser.", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(RequestReconcileWwiseItem, "Reconcile Selected Assets", "Reconcile the selected assets from the Wwise Browser.", EUserInterfaceActionType::Button, FInputChord());
	}

public:
	/** Requests a play action on a Wwise items */
	TSharedPtr< FUICommandInfo > RequestPlayWwiseItem;

	/** Requests a stop playing on all Wwise items */
	TSharedPtr< FUICommandInfo > RequestStopAllWwiseItem;

	/** Requests an explore action on the Item (locates its workunit in the file browser) */
	TSharedPtr< FUICommandInfo > RequestExploreWwiseItem;
	
	/** Requests a Find in the Wwise Project Explorer action on the Item */
	TSharedPtr< FUICommandInfo > RequestFindInProjectExplorerWwiseItem;
	
	/** Requests a refresh on the Wwise Browser */
	TSharedPtr< FUICommandInfo > RequestRefreshWwiseBrowser;

	/** Requests a Find in Content Browser action on the Item */
	TSharedPtr< FUICommandInfo > RequestFindInContentBrowser;

	/** Imports the selected asset into the project's Contents */
	TSharedPtr< FUICommandInfo > RequestImportWwiseItem;

	/** Reconciles the selected asset */
	TSharedPtr< FUICommandInfo > RequestReconcileWwiseItem;

};


#undef LOCTEXT_NAMESPACE
