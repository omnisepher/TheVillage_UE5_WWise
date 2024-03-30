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
#include "InputCoreTypes.h"
#include "AkAudioStyle.h"
#include "Framework/Commands/InputChord.h"
#include "Framework/Commands/Commands.h"

#define LOCTEXT_NAMESPACE "WaapiPickerViewCommands"
 
/**
 * The set of commands supported by the WaapiPickerView
 */
class AKAUDIO_API FWaapiPickerViewCommands : public TCommands<FWaapiPickerViewCommands>
{

public:

	/** FWaapiPickerViewCommands Constructor */
	FWaapiPickerViewCommands() : TCommands<FWaapiPickerViewCommands>
	(
		"WaapiPickerViewCommand", // Context name for fast lookup
		NSLOCTEXT("Contexts", "WaapiPickerViewCommand", "Waapi Picker Command"), // Localized context name for displaying
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
		UI_COMMAND(RequestRenameWwiseItem, "Rename", "Renames the selected item.", EUserInterfaceActionType::Button, FInputChord(EKeys::F2));
		UI_COMMAND(RequestPlayWwiseItem, "Play/Stop", "Plays or stops the selected item.", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar));
		UI_COMMAND(RequestStopAllWwiseItem, "Stop All", "Stop all playing events", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(RequestDeleteWwiseItem, "Delete", "Deletes the selected item(s).", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
		UI_COMMAND(RequestExploreWwiseItem, "Show in Folder", "Finds this item on disk.", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(RequestFindInProjectExplorerWwiseItem, "Find in the Project Explorer", "Finds the specified object in the Project Explorer (Sync Group 1).", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::One));
		UI_COMMAND(RequestRefreshWaapiPicker, "Refresh All", "Populates the Waapi Picker.", EUserInterfaceActionType::Button, FInputChord(EKeys::F5));
		UI_COMMAND(RequestImportWwiseItem, "Import Selected Assets", "Imports the selected assets from the Waapi Picker.", EUserInterfaceActionType::Button, FInputChord());
	}

public:

	/** Requests a rename on the Item */
	TSharedPtr< FUICommandInfo > RequestRenameWwiseItem;
	
	/** Requests a play action on a Wwise item */
	TSharedPtr< FUICommandInfo > RequestPlayWwiseItem;

	/** Requests a stop playing on all Wwise items */
	TSharedPtr< FUICommandInfo > RequestStopAllWwiseItem;
	
	/** Requests a delete action on a Wwise item(s) */
	TSharedPtr< FUICommandInfo > RequestDeleteWwiseItem;
	
	/** Requests an explore action on the Item */
	TSharedPtr< FUICommandInfo > RequestExploreWwiseItem;
	
	/** Requests a Find in the Project Explorer action on the Item */
	TSharedPtr< FUICommandInfo > RequestFindInProjectExplorerWwiseItem;
	
	/** Requests a refresh on the Waapi Picker */
	TSharedPtr< FUICommandInfo > RequestRefreshWaapiPicker;

	/** Imports the selected asset into the project's Contents */
	TSharedPtr< FUICommandInfo > RequestImportWwiseItem;
};

#undef LOCTEXT_NAMESPACE
