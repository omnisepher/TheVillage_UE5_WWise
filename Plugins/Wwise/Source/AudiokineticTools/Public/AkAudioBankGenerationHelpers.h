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

#include "Misc/EnumClassFlags.h"
#include "Containers/UnrealString.h"

class UAkAudioBank;

namespace AkAudioBankGenerationHelper
{
	/**
	 * Get path to the WwiseConsole application
	 */
	FString GetWwiseConsoleApplicationPath();

	/**
	 * Function to create the Generate SoundBanks window
	 *
	 * @param pSoundBanks				List of SoundBanks to be pre-selected
	 * @paramin_bShouldSaveWwiseProject	Whether the Wwise project should be saved or not
	 */
	void CreateGenerateSoundDataWindow(bool ProjectSave = false);

	enum class AkSoundDataClearFlags
	{
		None = 0,
		SoundBankInfoCache = 1 << 0,
		MediaCache = 1 << 1,
	};

	ENUM_CLASS_FLAGS(AkSoundDataClearFlags)

	void DoClearSoundData(AkSoundDataClearFlags ClearFlags);
}
