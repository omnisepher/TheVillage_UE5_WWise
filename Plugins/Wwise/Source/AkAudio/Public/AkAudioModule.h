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

#include "AkAudioDevice.h"
#include "WwiseUEFeatures.h"
#include "Modules/ModuleManager.h"
#include "Containers/Ticker.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class IAkAudioModule : public IModuleInterface
{

public:
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IAkAudioModule& Get()
	{
		return FModuleManager::LoadModuleChecked< IAkAudioModule >(TEXT("AkAudio"));
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(TEXT("AkAudio"));
	}
};

class AKAUDIO_API FAkAudioModule : public IAkAudioModule
{
public:
	static FAkAudioModule* AkAudioModuleInstance;
	static FSimpleMulticastDelegate OnModuleInitialized;
	static FSimpleMulticastDelegate OnWwiseAssetDataReloaded;
	bool bModuleInitialized;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FAkAudioDevice* GetAkAudioDevice() const;
	void ReloadWwiseAssetData() const;
	static void UpdateWwiseResourceLoaderSettings();
#if WITH_EDITORONLY_DATA
	static void ParseGeneratedSoundBankData();
#endif
	FAkAudioDevice* AkAudioDevice;

	/** Call to update AkAudioDevice. */
	FTickerDelegate OnTick;

	/** Handle for OnTick. */
	FTickerDelegateHandle TickDelegateHandle;
};

