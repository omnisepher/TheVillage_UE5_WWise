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

/*=============================================================================
	AkAudioDevice.h: Audiokinetic audio interface object.
=============================================================================*/

#pragma once

/*------------------------------------------------------------------------------------
	AkAudioDevice system headers
------------------------------------------------------------------------------------*/

#include "AkEnvironmentIndex.h"
#include "AkGameplayTypes.h"
#include "AkGroupValue.h"
#include "AkInclude.h"
#include "WwiseUnrealDefines.h"
#include "AkJobWorkerScheduler.h"
#include "Wwise/WwiseSharedLanguageId.h"
#include "Engine/EngineTypes.h"

#include "Wwise/Stats/AkAudio.h"
#include "Wwise/AkPortalObstructionAndOcclusionService.h"
#include "Wwise/WwiseSoundEngineUtils.h"

#if WITH_EDITORONLY_DATA
#include "EditorViewportClient.h"
#endif

#define GET_AK_EVENT_NAME(AkEvent, EventName) ((AkEvent) ? ((AkEvent)->GetName()) : (EventName))

DECLARE_EVENT(FAkAudioDevice, SoundbanksLoaded);
DECLARE_EVENT(FAkAudioDevice, FOnWwiseProjectModification);
DECLARE_EVENT_OneParam(FAkAudioDevice, FOnSwitchValueLoaded, UAkGroupValue*);
DECLARE_DELEGATE_OneParam(FOnSetCurrentAudioCultureCompleted, bool);

/*------------------------------------------------------------------------------------
	Dependencies, helpers & forward declarations.
------------------------------------------------------------------------------------*/

class UAkPortalComponent;
class AkCallbackInfoPool;
class AkLegacyFileCustomParamPolicy;
class CAkDiskPackage;
class FAkComponentCallbackManager;
class FWwiseIOHook;
class UAkComponent;
class UAkGameObject;
class UAkGroupValue;
class UAkLateReverbComponent;
class UAkRoomComponent;
class UAkStateValue;
class UAkSwitchValue;
class UAkAudioType;
class UAkAudioEvent;
class UAkEffectShareSet;
class AkXMLErrorMessageTranslator;
class AkWAAPIErrorMessageTranslator;
class AkUnrealErrorTranslator;

typedef TSet<TWeakObjectPtr<UAkComponent>> UAkComponentSet;

#define DUMMY_GAMEOBJ ((AkGameObjectID)0x2)
#define SOUNDATLOCATION_GAMEOBJ ((AkGameObjectID)0x3)


struct AKAUDIO_API FAkAudioDeviceDelegates
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAkGlobalCallback, AK::IAkGlobalPluginContext*, AkGlobalCallbackLocation);
};


/*------------------------------------------------------------------------------------
	Audiokinetic audio device.
------------------------------------------------------------------------------------*/
 
class AKAUDIO_API FAkAudioDevice final
{
public:
	UE_NONCOPYABLE(FAkAudioDevice);
	FAkAudioDevice() {}

	/**
	 * Initializes the audio device and creates sources.
	 *
	 * @return true if initialization was successful, false otherwise
	 */
	bool Init( void );

	/**
	 * Update the audio device and calculates the cached inverse transform later
	 * on used for spatialization.
	 */
	bool Update( float DeltaTime );
	
	/**
	 * Tears down audio device by stopping all sounds, removing all buffers, 
	 * destroying all sources, ... Called by both Destroy and ShutdownAfterError
	 * to perform the actual tear down.
	 */
	void Teardown();

	/**
	 * Stops all game sounds (and possibly UI) sounds
	 *
	 * @param bShouldStopUISounds If true, this function will stop UI sounds as well
	 */
	void StopAllSounds( bool bShouldStopUISounds = false );

	/**
	 * Stops all game sounds (and possibly UI) sounds
	 *
	 * @param AudioContext Stop sounds associated with this Context only
	 */
	void StopAllSounds(EAkAudioContext AudioContext);

	/**
	 * Stop all audio associated with a scene
	 *
	 * @param WorldToFlush		Interface of the scene to flush
	 */
	void Flush(UWorld* WorldToFlush);

	/**
	 * Determine if any rooms or reverb volumes have been added to World during the current frame
	 */
	bool WorldSpatialAudioVolumesUpdated(UWorld* World);

	/**
	 * Clears all loaded SoundBanks and associated media
	 */
	void ClearSoundBanksAndMedia();

	/**
	 * Load a soundbank by name
	 *
	 * @param in_BankName			The name of the bank to load
	 * @param out_bankID		Returned bank ID
	 * @return Result from ak sound engine 
	 */
	AKRESULT LoadBank(
		const FString&	in_BankName,
		AkBankID &      out_bankID
		);

	/**
	 * Load a soundbank asynchronously
	 *
	 * @param in_BankName			The bank to load
	 * @param in_pfnBankCallback Callback function
	 * @param in_pCookie		Callback cookie (reserved to user, passed to the callback function)
	 * @param out_bankID		Returned bank ID
	 * @return Result from ak sound engine 
	 */
	AKRESULT LoadBank(
        const FString&      in_BankName,
		AkBankCallbackFunc  in_pfnBankCallback,
		void *              in_pCookie,
		AkBankID &          out_bankID
        );
		
	/**
	 * Load a soundbank asynchronously, flagging completion in the latent action
	 *
	 * @param in_BankName			The bank to load
	 * @param LoadBankLatentAction Blueprint Latent action to flag completion
	 * @param out_bankID			Returned bank ID
	 * @return Result from ak sound engine
	 */
	AKRESULT LoadBank(
		const FString&     in_BankName,
		FWaitEndBankAction* LoadBankLatentAction
	);

	/**
	 * Load a sound bank from a memory pointer
	 * 
	 * @param in_MemoryPtr Pointer to the bank data
	 * @param in_MemorySize Size of the bank data
	 * @param out_banKID Returned bank ID
	 */
	AKRESULT LoadBankFromMemory(
		const void* MemoryPtr,
		uint32 MemorySize,
		AkBankType BankType,
		AkBankID& OutBankID
	);

	/**
	 * Load a soundbank asynchronously, using a Blueprint Delegate for completion
	 *
	 * @param in_BankName			The bank to load
	 * @param BankLoadedCallback Blueprint Delegate called upon completion
	 * @param out_bankID			Returned bank ID
	 * @return Result from ak sound engine
	 */
	AKRESULT LoadBankAsync(
		const FString& in_BankName,
		const FOnAkBankCallback& BankLoadedCallback,
		AkBankID &          out_bankID
	);

	/**
	 * Unload a soundbank by its name
	 *
	 * @param in_BankName		The name of the bank to unload
	 * @return Result from ak sound engine 
	 */
	AKRESULT UnloadBank(
        const FString&      in_BankName
        );

	/**
	 * Unload a soundbank asynchronously
	 *
	 * @param in_BankName			The bank to unload
	 * @param in_pfnBankCallback Callback function
	 * @param in_pCookie		Callback cookie (reserved to user, passed to the callback function)
	 * @return Result from ak sound engine 
	 */
	AKRESULT UnloadBank(
        const FString&      in_BankName,
		AkBankCallbackFunc  in_pfnBankCallback,
		void *              in_pCookie
        );

	/**
	 * Unload a soundbank asynchronously, flagging completion in the latent action
	 *
	 * @param in_BankName			The bank to unload
	 * @param UnloadBankLatentAction Blueprint Latent action to flag completion
	 * @return Result from ak sound engine
	 */
	AKRESULT UnloadBank(
		const FString&    in_BankName,
		FWaitEndBankAction* UnloadBankLatentAction
	);

	/**
	 * Unload bank with bank data pointer
	 *
	 * @param in_bankID The BankID returned by the LoadBankFromMemory call.
	 * @param in_memoryPtr The same bank data pointer used with th LoadBankFromMemory call.
	 */
	AKRESULT UnloadBankFromMemory(
		AkBankID in_bankID,
		const void* in_memoryPtr
	);


	/**
	 * Unload bank with bank data pointer
	 *
	 * @param in_bankID The BankID returned by the LoadBankFromMemory call.
	 * @param in_memoryPtr The same bank data pointer used with th LoadBankFromMemory call.
	 */
	AKRESULT UnloadBankFromMemoryAsync(
		AkBankID in_bankID, 
		const void* in_memoryPtr,
		AkBankCallbackFunc in_pfnBankCallback,
		void* in_pCookie, 
		uint32 BankType
	);


	/**
	 * Unload a soundbank asynchronously, using a Blueprint Delegate for completion
	 *
	 * @param in_BankName			The bank to load
	 * @param BankUnloadedCallback Blueprint Delegate called upon completion
	 * @param out_bankID			Returned bank ID
	 * @return Result from ak sound engine
	 */
	AKRESULT UnloadBankAsync(
		const FString&     in_BankName,
		const FOnAkBankCallback& BankUnloadedCallback
	);
	
	/**
	 * FString-friendly GetIDFromString
	 */
	static AkUInt32 GetShortIDFromString(const FString& InString);

	/**
	 * NUll-check asset and if invalid return ID based on BackupName
	 */
	static AkUInt32 GetShortID(UAkAudioType* AudioAsset, const FString& BackupName);

	/**
	 * Indicates the location of a specific Media ID in memory.
	 *
	 * @param in_pSourceSettings Array of Source Settings
	 * @param in_uNumSourceSettings Number of Source Settings in the array
	 */
	AKRESULT SetMedia(AkSourceSettings* in_pSourceSettings, uint32 in_uNumSourceSettings);

	/**
	 * Removes the specified source from the list of loaded media, even if this media is already in use.
	 *
	 * @param in_pSourceSettings Array of Source Settings
	 * @param in_uNumSourceSettings Number of Source Settings in the array
	 * @param out_pUnsetResults Array of AKRESULT results per Source Settings
	 */
	 AKRESULT TryUnsetMedia(AkSourceSettings* in_pSourceSettings, uint32 in_uNumSourceSettings, AKRESULT* out_pUnsetResults = nullptr);
	 
	/**
	* Removes the specified source from the list of loaded media.
	*
  	* @param in_pSourceSettings Array of Source Settings
  	* @param un_uNumSourceSettings Number of Source Settings in the array
  	*/
	 AKRESULT UnsetMedia(AkSourceSettings* in_pSourceSettings, uint32 in_uNumSourceSettings);

	/**
	 * Get the currently selected audio culture
	 */
	FString GetCurrentAudioCulture() const;

	/**
	 * Get the default language set in the Wwise project
	 */
	FString GetDefaultLanguage();

	/**
	 * Get the list of available audio cultures
	 */
	TArray<FString> GetAvailableAudioCultures() const;

	/**
	 * Get a FWwiseSharedLanguageId from the name as set in Wwise
	 */
	FWwiseLanguageCookedData GetLanguageCookedDataFromString(const FString& WwiseLanguage);

	/**
	 * Change the audio culture
	 */
	void SetCurrentAudioCulture(const FString& AudioCulture);

	/**
	 * Change the audio culture asynchronously and signal the latent action when done
	 */
	void SetCurrentAudioCultureAsync(const FString& AudioCulture, FSetCurrentAudioCultureAction* LatentAction);
	
	/**
	 * Change the audio culture asynchronous and call the callback when done
	 */
	void SetCurrentAudioCultureAsync(const FString& AudioCulture, const FOnSetCurrentAudioCultureCompleted& CompletedCallback);

	/** Spawn an AkComponent at a location. Allows, for example, to set a switch on a fire and forget sound.
	 * @param AkEvent - Wwise Event to post.
	 * @param Location - Location from which to post the Wwise Event.
	 * @param Orientation - Orientation of the event.
	 * @param AutoPost - Automatically post the event once the AkComponent is created.
	 * @param EarlyReflectionsBusName - Use the provided auxiliary bus to process early reflections.  If empty, no early reflections will be processed.
	 * @param AutoDestroy - Automatically destroy the AkComponent once the event is finished.
	 */
	class UAkComponent* SpawnAkComponentAtLocation( class UAkAudioEvent* AkEvent, FVector Location, FRotator Orientation, bool AutoPost, const FString& EventName, bool AutoDestroy, class UWorld* in_World );

    /** Seek on an event in the ak soundengine.
    * @param EventShortID         ID of the event on which to seek.
    * @param Component            The associated Actor.
    * @param Position             Desired percent where playback should restart.
    * @param bSeekToNearestMarker If true, the final seeking position will be made equal to the nearest marker.
    *
    * @return Success or failure.
    */
    AKRESULT SeekOnEvent(
	    const AkUInt32 EventShortID,
	    AActor* Actor,
	    AkReal32 Percent,
	    bool bSeekToNearestMarker = false,
	    AkPlayingID PlayingID = AK_INVALID_PLAYING_ID
    );

    /** Seek on an event in the ak soundengine.
    * @param EventShortID         ID of the event on which to seek.
    * @param Component            The associated AkComponent.
    * @param Percent              Desired percent where playback should restart.
    * @param bSeekToNearestMarker If true, the final seeking position will be made equal to the nearest marker.
    *
    * @return Success or failure.
    */
    AKRESULT SeekOnEvent(
	    const AkUInt32 EventShortID,
	    UAkComponent* Component,
	    AkReal32 Percent,
	    bool bSeekToNearestMarker = false,
	    AkPlayingID PlayingID = AK_INVALID_PLAYING_ID
    );

	/**
	 * Post a trigger to ak soundengine
	 *
	 * @param in_pszTrigger		Name of the trigger
	 * @param in_pAkComponent	AkComponent on which to post the trigger
	 * @return Result from ak sound engine
	 */
	AKRESULT PostTrigger( 
		const TCHAR * in_pszTrigger,
		AActor * in_pActor
		);

	/**
	 * Post a trigger to ak soundengine
	 *
	 * @param in_TriggerValue	Trigger value
	 * @param in_pAkComponent	AkComponent on which to post the trigger
	 * @return Result from ak sound engine
	 */
	AKRESULT PostTrigger(
		class UAkTrigger const* in_TriggerValue,
		AActor * in_pActor
	);

	/**
	 * Set a RTPC in ak soundengine
	 *
	 * @param in_pszRtpcName	Name of the RTPC
	 * @param in_value			Value to set
	 * @param in_interpolationTimeMs - Duration during which the RTPC is interpolated towards in_value (in ms)
	 * @param in_pActor			AActor on which to set the RTPC
	 * @return Result from ak sound engine
	 */
	AKRESULT SetRTPCValue(
		const TCHAR * in_pszRtpcName,
		AkRtpcValue in_value,
		int32 in_interpolationTimeMs,
		AActor * in_pActor
		);

	/**
	 * Set a RTPC in ak soundengine
	 *
	 * @param in_Rtpc			RTPC Short ID
	 * @param in_value			Value to set
	 * @param in_interpolationTimeMs - Duration during which the RTPC is interpolated towards in_value (in ms)
	 * @param in_pActor			AActor on which to set the RTPC
	 * @return Result from ak sound engine
	 */
	AKRESULT SetRTPCValue(
		AkRtpcID in_Rtpc,
		AkRtpcValue in_value,
		int32 in_interpolationTimeMs,
		AActor * in_pActor
		);

	/**
	 * Set a RTPC in ak soundengine
	 *
	 * @param in_RtpcValue	RTPC Value
	 * @param in_value			Value to set
	 * @param in_interpolationTimeMs - Duration during which the RTPC is interpolated towards in_value (in ms)
	 * @param in_pActor			AActor on which to set the RTPC
	 * @return Result from ak sound engine
	 */
	AKRESULT SetRTPCValue(
		class UAkRtpc const* in_RtpcValue,
		AkRtpcValue in_value,
		int32 in_interpolationTimeMs,
		AActor * in_pActor
	);

	/**
	 * Set a RTPC in ak soundengine by PlayingID
	 *
	 * @param in_Rtpc			RTPC Short ID
	 * @param in_value			Value to set
	 * @param in_playingID		PlayingID on which to set the value
	 * @param in_interpolationTimeMs - Duration during which the RTPC is interpolated towards in_value (in ms)
	 * @return Result from ak sound engine
	 */
	AKRESULT SetRTPCValueByPlayingID(
		AkRtpcID in_Rtpc,
		AkRtpcValue in_value,
		AkPlayingID in_playingID,
		int32 in_interpolationTimeMs
	);

	/**
	*  Get the value of a real-time parameter control (by ID)
	*  An RTPC can have a any combination of a global value, a unique value for each game object, or a unique value for each playing ID.
	*  The value requested is determined by RTPCValue_type, in_gameObjectID and in_playingID.
	*  If a value at the requested scope (determined by RTPCValue_type) is not found, the value that is available at the the next broadest scope will be returned, and io_rValueType will be changed to indicate this.
	*  @note
	* 		When looking up RTPC values via playing ID (ie. io_rValueType is RTPC_PlayingID), in_gameObjectID can be set to a specific game object (if it is available to the caller) to use as a fall back value.
	* 		If the game object is unknown or unavailable, AK_INVALID_GAME_OBJECT can be passed in in_gameObjectID, and the game object will be looked up via in_playingID.
	* 		However in this case, it is not possible to retrieve a game object value as a fall back value if the playing id does not exist.  It is best to pass in the game object if possible.
	*
	*  @return AK_Success if succeeded, AK_IDNotFound if the game object was not registered, or AK_Fail if the RTPC value could not be obtained
	*/
	AKRESULT GetRTPCValue(
		const TCHAR * in_pszRtpcName,
		AkGameObjectID in_gameObjectID,		///< Associated game object ID, ignored if io_rValueType is RTPCValue_Global.
		AkPlayingID	in_playingID,			///< Associated playing ID, ignored if io_rValueType is not RTPC_PlayingID.
		AkRtpcValue& out_rValue, 			///< Value returned
		AK::SoundEngine::Query::RTPCValue_type&	io_rValueType		///< In/Out value, the user must specify the requested type. The function will return in this variable the type of the returned value.				);
	);

	/**
	*  Get the value of a real-time parameter control (by ID)
	*  An RTPC can have a any combination of a global value, a unique value for each game object, or a unique value for each playing ID.
	*  The value requested is determined by RTPCValue_type, in_gameObjectID and in_playingID.
	*  If a value at the requested scope (determined by RTPCValue_type) is not found, the value that is available at the the next broadest scope will be returned, and io_rValueType will be changed to indicate this.
	*  @note
	* 		When looking up RTPC values via playing ID (ie. io_rValueType is RTPC_PlayingID), in_gameObjectID can be set to a specific game object (if it is available to the caller) to use as a fall back value.
	* 		If the game object is unknown or unavailable, AK_INVALID_GAME_OBJECT can be passed in in_gameObjectID, and the game object will be looked up via in_playingID.
	* 		However in this case, it is not possible to retrieve a game object value as a fall back value if the playing id does not exist.  It is best to pass in the game object if possible.
	*
	*  @return AK_Success if succeeded, AK_IDNotFound if the game object was not registered, or AK_Fail if the RTPC value could not be obtained
	*/
	AKRESULT GetRTPCValue(
		AkRtpcID in_Rtpc,
		AkGameObjectID in_gameObjectID,		///< Associated game object ID, ignored if io_rValueType is RTPCValue_Global.
		AkPlayingID	in_playingID,			///< Associated playing ID, ignored if io_rValueType is not RTPC_PlayingID.
		AkRtpcValue& out_rValue, 			///< Value returned
		AK::SoundEngine::Query::RTPCValue_type&	io_rValueType		///< In/Out value, the user must specify the requested type. The function will return in this variable the type of the returned value.				);
	);

	/**
	*  Get the value of a real-time parameter control (by ID)
	*  An RTPC can have a any combination of a global value, a unique value for each game object, or a unique value for each playing ID.
	*  The value requested is determined by RTPCValue_type, in_gameObjectID and in_playingID.
	*  If a value at the requested scope (determined by RTPCValue_type) is not found, the value that is available at the the next broadest scope will be returned, and io_rValueType will be changed to indicate this.
	*  @note
	* 		When looking up RTPC values via playing ID (ie. io_rValueType is RTPC_PlayingID), in_gameObjectID can be set to a specific game object (if it is available to the caller) to use as a fall back value.
	* 		If the game object is unknown or unavailable, AK_INVALID_GAME_OBJECT can be passed in in_gameObjectID, and the game object will be looked up via in_playingID.
	* 		However in this case, it is not possible to retrieve a game object value as a fall back value if the playing id does not exist.  It is best to pass in the game object if possible.
	*
	*  @return AK_Success if succeeded, AK_IDNotFound if the game object was not registered, or AK_Fail if the RTPC value could not be obtained
	*/
	AKRESULT GetRTPCValue(
		class UAkRtpc const* in_RtpcValue,
		AkGameObjectID in_gameObjectID,		///< Associated game object ID, ignored if io_rValueType is RTPCValue_Global.
		AkPlayingID	in_playingID,			///< Associated playing ID, ignored if io_rValueType is not RTPC_PlayingID.
		AkRtpcValue& out_rValue, 			///< Value returned
		AK::SoundEngine::Query::RTPCValue_type&	io_rValueType		///< In/Out value, the user must specify the requested type. The function will return in this variable the type of the returned value.				);
	);

	/// Resets the value of the game parameter to its default value, as specified in the Wwise project.
	/// With this function, you may reset a game parameter to its default value with global scope or with game object scope. 
	/// Game object scope supersedes global scope. Game parameter values reset with global scope are applied to all 
	/// game objects that were not overridden with a value with game object scope.
	/// To reset a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
	/// With this function, you may also reset the value of a game parameter over time. To do so, specify a non-zero 
	/// value for in_interpolationTimeMs. At each audio frame, the game parameter value will be updated internally 
	/// according to the interpolation curve. If you call SetRTPCValue() or ResetRTPCValue() with in_interpolationTimeMs = 0 in the 
	/// middle of an interpolation, the interpolation stops and the new value is set directly. 
	AKRESULT ResetRTPCValue(
		const UAkRtpc* in_RtpcValue,
		AkGameObjectID in_gameObjectID,
		int32 in_interpolationTimeMs
	);

	/// Resets the value of the game parameter to its default value, as specified in the Wwise project.
	/// With this function, you may reset a game parameter to its default value with global scope or with game object scope. 
	/// Game object scope supersedes global scope. Game parameter values reset with global scope are applied to all 
	/// game objects that were not overridden with a value with game object scope.
	/// To reset a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
	/// With this function, you may also reset the value of a game parameter over time. To do so, specify a non-zero 
	/// value for in_interpolationTimeMs. At each audio frame, the game parameter value will be updated internally 
	/// according to the interpolation curve. If you call SetRTPCValue() or ResetRTPCValue() with in_interpolationTimeMs = 0 in the 
	/// middle of an interpolation, the interpolation stops and the new value is set directly. 
	AKRESULT ResetRTPCValue(
		AkRtpcID in_rtpcID, 			///< ID of the game parameter
		AkGameObjectID in_gameObjectID,	///< Associated game object ID
		int32 in_interpolationTimeMs	///< Duration during which the game parameter is interpolated towards its default value
	);

	/// Resets the value of the game parameter to its default value, as specified in the Wwise project.
	/// With this function, you may reset a game parameter to its default value with global scope or with game object scope. 
	/// Game object scope supersedes global scope. Game parameter values reset with global scope are applied to all 
	/// game objects that were not overridden with a value with game object scope.
	/// To reset a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
	/// With this function, you may also reset the value of a game parameter over time. To do so, specify a non-zero 
	/// value for in_interpolationTimeMs. At each audio frame, the game parameter value will be updated internally 
	/// according to the interpolation curve. If you call SetRTPCValue() or ResetRTPCValue() with in_interpolationTimeMs = 0 in the 
	/// middle of an interpolation, the interpolation stops and the new value is set directly. 
	AKRESULT ResetRTPCValue(
		const TCHAR * in_pszRtpcName,		///< Name of the game parameter
		AkGameObjectID in_gameObjectID,		///< Associated game object ID
		int32 in_interpolationTimeMs	///< Duration during which the game parameter is interpolated towards its default value
	);

	/**
	 * Set a state in ak soundengine
	 *
	 * @param in_pszStateGroup	Name of the state group
	 * @param in_pszState		Name of the state
	 * @return Result from ak sound engine
	 */
	AKRESULT SetState(
		const TCHAR* in_pszStateGroup,
		const TCHAR* in_pszState
	);

	/**
	 * Set a state in ak soundengine
	 *
	 * @param in_StateGroup	State group short ID
	 * @param in_State		State short ID
	 * @return Result from ak sound engine
	 */
	AKRESULT SetState(
		AkStateGroupID in_StateGroup,
		AkStateID in_State
	);

	/**
	 * Set a state in ak soundengine
	 *
	 * @param in_stateValue State to set
	 * @return Result from ak sound engine
	 */
	AKRESULT SetState(
		const UAkStateValue* in_stateValue
	);
		
	/**
	 * Set a switch in ak soundengine
	 *
	 * @param in_pszSwitchGroup	Name of the switch group
	 * @param in_pszSwitchState	Name of the switch
	 * @param in_pComponent		AkComponent on which to set the switch
	 * @return Result from ak sound engine
	 */
	AKRESULT SetSwitch( 
		const TCHAR * in_pszSwitchGroup,
		const TCHAR * in_pszSwitchState,
		AActor * in_pActor
		);

	/**
	 * Set a switch in ak soundengine
	 *
	 * @param in_SwitchGroup	Short ID of the switch group
	 * @param in_SwitchState	Short ID of the switch
	 * @param in_pComponent		AkComponent on which to set the switch
	 * @return Result from ak sound engine
	 */
	AKRESULT SetSwitch(
		AkSwitchGroupID in_SwitchGroup,
		AkSwitchStateID in_SwitchState,
		AActor* in_pActor
	);
		
	/**
	 * Set a switch in ak soundengine
	 *
	 * @param in_switchValue	Switch to set
	 * @param in_pActor		AkActor on which to set the switch
	 * @return Result from ak sound engine
	 */
	AKRESULT SetSwitch(
		const UAkSwitchValue* in_switchValue,
		AActor * in_pActor
	);

    /** Sets multiple positions to a single game object.
    *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
    *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
    *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
    *  @param in_pGameObjectAkComponent UAkComponent of the game object.
    *  @param in_aPositions Array of positions to apply.
    *  @param in_eMultiPositionType Position type
    *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
    */
    AKRESULT SetMultiplePositions(
        UAkComponent* in_pGameObjectAkComponent,
        TArray<FTransform> in_aPositions,
        AkMultiPositionType in_eMultiPositionType = AkMultiPositionType::MultiDirections
    );

    /** Sets multiple positions to a single game object, with flexible assignment of input channels.
    *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
    *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
    *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
    *  @param in_pGameObjectAkComponent Game Object AkComponent.
    *  @param in_aChannelConfigurations Array of channel configurations for each position.
    *  @param in_pPositions Array of positions to apply.
    *  @param in_eMultiPositionType Position type
    *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
    */
    AKRESULT SetMultiplePositions(
        UAkComponent* in_pGameObjectAkComponent,
        const TArray<AkChannelConfiguration>& in_aChannelConfigurations,
        const TArray<FTransform>& in_aPositions,
        AkMultiPositionType in_eMultiPositionType = AkMultiPositionType::MultiDirections
    );

	/** Sets multiple positions to a single game object, with flexible assignment of input channels.
	*  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
	*  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
	*  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
	*  @param in_pGameObjectAkComponent Game Object AkComponent.
	*  @param in_channelMasks Array of channel mask for each position.
	*  @param in_pPositions Array of positions to apply.
	*  @param in_eMultiPositionType Position type
	*  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
	*/
	AKRESULT SetMultiplePositions(
		UAkComponent* in_pGameObjectAkComponent,
		const TArray<FAkChannelMask>& in_channelMasks,
		const TArray<FTransform>& in_aPositions,
		AkMultiPositionType in_eMultiPositionType = AkMultiPositionType::MultiDirections
	);

    /** Sets multiple positions to a single game object.
     *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
     *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
     *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
     *  @param in_GameObjectID Game Object identifier.
     *  @param in_pPositions Array of positions to apply.
     *  @param in_NumPositions Number of positions specified in the provided array.
     *  @param in_eMultiPositionType Position type
     *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
     *  
     */
    AKRESULT SetMultiplePositions(
        AkGameObjectID in_GameObjectID,
        const AkSoundPosition * in_pPositions,
        AkUInt16 in_NumPositions,
        AK::SoundEngine::MultiPositionType in_eMultiPositionType = AK::SoundEngine::MultiPositionType_MultiDirections
        );

    /** Sets multiple positions to a single game object, with flexible assignment of input channels.
     *  Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
     *  This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
     *  Note: Calling AK::SoundEngine::SetMultiplePositions() with only one position is the same as calling AK::SoundEngine::SetPosition()
     *  @param in_GameObjectID Game Object identifier.
     *  @param in_pPositions Array of positions to apply.
     *  @param in_NumPositions Number of positions specified in the provided array.
     *  @param in_eMultiPositionType Position type
     *  @return AK_Success when successful, AK_InvalidParameter if parameters are not valid.
     */
    AKRESULT SetMultiplePositions(
        AkGameObjectID in_GameObjectID,
        const AkChannelEmitter * in_pPositions,
        AkUInt16 in_NumPositions,
        AK::SoundEngine::MultiPositionType in_eMultiPositionType = AK::SoundEngine::MultiPositionType_MultiDirections
        );

	/**
	 * Set auxiliary sends
	 *
	 * @param in_GameObjId		Wwise Game Object ID
	 * @param in_AuxSendValues	Array of AkAuxSendValue, containing all Aux Sends to set on the game object
	 * @return Result from ak sound engine
	 */
	AKRESULT SetAuxSends(
		const UAkComponent* in_akComponent,
		TArray<AkAuxSendValue>& in_AuxSendValues
		);

	/**
	* Set spatial audio room
	*
	* @param in_GameObjId		Wwise Game Object ID
	* @param in_RoomID	ID of the room that the game object is inside.
	* @return Result from ak sound engine
	*/
	AKRESULT SetInSpatialAudioRoom(
		const AkGameObjectID in_GameObjId,
		AkRoomID in_RoomID
	);

	/**
	 * Force channel configuration for the specified bus.
	 * This function has unspecified behavior when changing the configuration of a bus that 
	 * is currently playing.
	 * You cannot change the configuration of the master bus.
	 *
	 * @param in_BusName	Bus Name
	 * @param in_Config		Desired channel configuration. An invalid configuration (from default constructor) means "as parent".
	 * @return Always returns AK_Success
	 */
	AKRESULT SetBusConfig(
		const FString&	in_BusName,
		AkChannelConfig	in_Config
		);

	/**
	 *  Set the panning rule of the specified output.
	 *  This may be changed anytime once the sound engine is initialized.
	 *  \warning This function posts a message through the sound engine's internal message queue, whereas GetPanningRule() queries the current panning rule directly.
	 */
	AKRESULT SetPanningRule(
		AkPanningRule		in_ePanningRule			///< Panning rule.
		);

	/**
	 * Gets the compounded output ID from ShareSet and device id.
	 * Outputs are defined by their type (Audio Device ShareSet) and their specific system ID.
	 * A system ID could be reused for other device types on some OS or platforms, hence the compounded ID.
	 *
	 * @param in_szShareSet Audio Device ShareSet Name, as defined in the Wwise Project.  If Null, will select the Default Output ShareSet (always available)
	 * @param in_idDevice Device specific identifier, when multiple devices of the same type are possible. If only one device is possible, leave to 0.
	 * @return The id of the output
	 */
	AkOutputDeviceID GetOutputID(
		const FString& in_szShareSet,
		AkUInt32 in_idDevice = 0
	);


	/**
	 * Replaces the main output device previously created during engine initialization with a new output device.
	 * In addition to simply removing one output device and adding a new one, the new output device will also be used on all of the master busses
	 * that the old output device was associated with, and preserve all listeners that were attached to the old output device.
	 *
	 * @param MainOutputSettings	Creation parameters for this output
	 * 
	 * @return 
	 * - AK_InvalidID: The audioDeviceShareSet on in_settings was not valid.
	 * - AK_IDNotFound: The audioDeviceShareSet on in_settings doesn't exist.  Possibly, the Init bank isn't loaded yet or was not updated with latest changes.
	 * - AK_DeviceNotReady: The idDevice on in_settings doesn't match with a valid hardware device.  Either the device doesn't exist or is disabled.  Disconnected devices (headphones) are not considered "not ready" therefore won't cause this error.
	 * - AK_DeviceNotFound: The in_outputDeviceId provided does not match with any of the output devices that the sound engine is currently using.
	 * - AK_InvalidParameter: Out of range parameters or unsupported parameter combinations on in_settings
	 * - AK_Success: parameters were valid, and the remove and add will occur.
	 */
	AKRESULT ReplaceMainOutput(const AkOutputSettings& MainOutputSettings);

	/**
	 * Gets speaker angles of the specified device. Speaker angles are used for 3D positioning of sounds over standard configurations.
	 * Note that the current version of Wwise only supports positioning on the plane.
	 * The speaker angles are expressed as an array of loudspeaker pairs, in degrees, relative to azimuth ]0,180].
	 * Supported loudspeaker setups are always symmetric; the center speaker is always in the middle and thus not specified by angles.
	 * Angles must be set in ascending order.
	 * Typical usage:
	 * - AkReal32 heightAngle;
	 * - TArray<AkReal32> speakerAngles;
	 * - GetSpeakerAngles(speakerAngles, heightAngle, AkOutput_Main );
	 * \aknote
	 *  On most platforms, the angle set on the plane consists of 3 angles, to account for 7.1.
	 * - When panning to stereo (speaker mode, see <tt>AK::SoundEngine::SetPanningRule()</tt>), only angle[0] is used, and 3D sounds in the back of the listener are mirrored to the front.
	 * - When panning to 5.1, the front speakers use angle[0], and the surround speakers use (angle[2] - angle[1]) / 2.
	 * \endaknote
	 * \warning Call this function only after the sound engine has been properly initialized.
	 *
	 * @param io_pfSpeakerAngles Returned array of loudspeaker pair angles, in degrees relative to azimuth [0,180]. Pass NULL to get the required size of the array.
	 * @param out_fHeightAngle Elevation of the height layer, in degrees relative to the plane [-90,90].
	 * @param in_idOutput Output ID to set the bus on.  As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output
	 * @return AK_Success if device exists
	 *
	 */
	AKRESULT GetSpeakerAngles(
		TArray<AkReal32>& io_pfSpeakerAngles,
		AkReal32& out_fHeightAngle,
		AkOutputDeviceID in_idOutput = 0
		);

	/**
	 * Sets speaker angles of the specified device. Speaker angles are used for 3D positioning of sounds over standard configurations.
	 * Note that the current version of Wwise only supports positioning on the plane.
	 * The speaker angles are expressed as an array of loudspeaker pairs, in degrees, relative to azimuth ]0,180].
	 * Supported loudspeaker setups are always symmetric; the center speaker is always in the middle and thus not specified by angles.
	 * Angles must be set in ascending order.
	 * Typical usage:
	 * - Initialize the sound engine and/or add secondary output(s).
	 * - Get number of speaker angles and their value into an array using GetSpeakerAngles().
	 * - Modify the angles and call SetSpeakerAngles().
	 * This function posts a message to the audio thread through the command queue, so it is thread safe. However the result may not be immediately read with GetSpeakerAngles().
	 * \warning This function only applies to configurations (or subset of these configurations) that are standard and whose speakers are on the plane (2D).
	 * \sa GetSpeakerAngles()
	 *
	 * @param in_pfSpeakerAngles Array of loudspeaker pair angles, in degrees relative to azimuth [0,180]
	 * @param in_fHeightAngle Elevation of the height layer, in degrees relative to the plane [-90,90]
	 * @param in_idOutput Output ID to set the bus on. As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output
	 * @return AK_Success if successful (device exists and angles are valid), AK_NotCompatible if the channel configuration of the device is not standard (AK_ChannelConfigType_Standard), AK_Fail otherwise.
	 *
	 */
	AKRESULT SetSpeakerAngles(
		const TArray<AkReal32>& in_pfSpeakerAngles,
		AkReal32 in_fHeightAngle,
		AkOutputDeviceID in_idOutput = 0
		);

	/**
	 * Set the output bus volume (direct) to be used for the specified game object.
	 * The control value is a number ranging from 0.0f to 1.0f.
	 *
	 * @param in_GameObjId		Wwise Game Object ID
	 * @param in_fControlValue	Control value to set
	 * @return	Always returns Ak_Success
	 */
	AKRESULT SetGameObjectOutputBusVolume(
		const UAkComponent* in_pEmitter,
		const UAkComponent* in_pListener,
		float in_fControlValue
		);

	/**
	 * Registers a callback that can run within the global callback at a specific AkGlobalCallbackLocation.
	 *
	 * @param Callback		The callback that will be called.
	 * @param Location		The location in the sound engine processing loop
	 * @return	Returns the handle of the delegate that must be used to unregister the callback.
	 */
	FDelegateHandle RegisterGlobalCallback(FAkAudioDeviceDelegates::FOnAkGlobalCallback::FDelegate Callback, AkGlobalCallbackLocation Location);

	/**
	 * Unregisters a callback that can run within the global callback at a specific AkGlobalCallbackLocation.
	 *
	 * @param Handle		The handle of the registered callback
	 * @param Location		The location in the sound engine processing loop
	 */
	void UnregisterGlobalCallback(FDelegateHandle Handle, AkGlobalCallbackLocation Location);

	/**
	 * Registers a callback to be called to allow the game to access metering data from any output device.
	 * @param OutputID Output ID, as returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output
	 * @param Callback Callback function
	 * @param MeteringFlags Metering flags
	 * @param Cookie User cookie 
	 * @return AK_Success if callback has been registered
	 */
	AKRESULT RegisterOutputDeviceMeteringCallback(AkOutputDeviceID OutputID,
												  AkOutputDeviceMeteringCallbackFunc Callback,
												  AkMeteringFlags MeteringFlags,
												  void* Cookie);

	/**
	 * Unregisters the output device's metering callback
	 * @param OutputID 
	 * @return AK_Success if callback has been unregistered
	 */
	AKRESULT UnregisterOutputDeviceMeteringCallback(AkOutputDeviceID OutputID);

	/**
	 * Obtain a pointer to the singleton instance of FAkAudioDevice
	 *
	 * @return Pointer to the singleton instance of FAkAudioDevice
	 */
	static FAkAudioDevice* Get();


	/**
	 * Is the Wwise SoundEngine initialized?
	 */
	static bool IsInitialized() { return m_bSoundEngineInitialized;  }

	/**
	 * Gets the system sample rate
	 *
	 * @return Sample rate
	 */
	AkUInt32 GetSampleRate();

	/**
	 * Enables/disables offline rendering
	 *
	 * @param bEnable		Set to true to enable offline rendering
	 */
	AKRESULT SetOfflineRendering(bool bEnable);

	/**
	 * Sets the offline rendering frame time in seconds.
	 *
	 * @param FrameTimeInSeconds		Frame time in seconds used during offline rendering
	 */
	AKRESULT SetOfflineRenderingFrameTime(AkReal32 FrameTimeInSeconds);

	/**
	 * Registers a callback used for retrieving audio samples.
	 *
	 * @param Callback		Capture callback function to register
	 * @param OutputId			The audio device specific id, return by AK::SoundEngine::AddOutput or AK::SoundEngine::GetOutputID
	 * @param Cookie			Callback cookie that will be sent to the callback function along with additional information
	 */
	AKRESULT RegisterCaptureCallback(AkCaptureCallbackFunc Callback, AkOutputDeviceID OutputId = AK_INVALID_OUTPUT_DEVICE_ID, void* Cookie = nullptr);

	/**
	 * Unregisters a callback used for retrieving audio samples.
	 *
	 * @param Callback		Capture callback function to register
	 * @param OutputId			The audio device specific id, return by AK::SoundEngine::AddOutput or AK::SoundEngine::GetOutputID
	 * @param Cookie			Callback cookie that will be sent to the callback function along with additional information
	 */
	AKRESULT UnregisterCaptureCallback(AkCaptureCallbackFunc Callback, AkOutputDeviceID OutputId = AK_INVALID_OUTPUT_DEVICE_ID, void* Cookie = nullptr);

	/**
	 * Stop all audio associated with a game object
	 *
	 * @param in_pComponent		AkComponent which should be stopped
	 */
	void StopGameObject(UAkComponent * in_pComponent);

	/**
	 * Stop all audio associated with a playing ID
	 *
	 * @param in_playingID		AkPlayingID which should be stopped
	 */
	void StopPlayingID( AkPlayingID in_playingID,
                        AkTimeMs in_uTransitionDuration = 0,
                        AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear);

	/**
	 * Register an ak audio component with ak sound engine
	 *
	 * @param in_pComponent		Pointer to the component to register
	 */
	void RegisterComponent(UAkComponent * in_pComponent);

	/**
 * Register a game object with ak sound engine
 *
 * @param GameObjectID		ID of the game object to register
 */
	void RegisterComponent(AkGameObjectID GameObjectID);

	/**
	 * Unregister an ak audio component with ak sound engine
	 *
	 * @param in_pComponent		Pointer to the component to unregister
	 */
	void UnregisterComponent(UAkComponent * in_pComponent);

	/**
	 * Unregister an ak game object with ak sound engine
	 *
	 * @param GameObjectID		ID of the game object to unregister
	 */
	void UnregisterComponent(AkGameObjectID GameObjectID);
	
	/**
	* Send a set of triangles to the Spatial Audio Engine
	*/
	AKRESULT SetGeometry(AkGeometrySetID GeometrySetID, const AkGeometryParams& Params);

	/**
	* Create a geometry instance in Spatial Audio
	*/
	AKRESULT SetGeometryInstance(AkGeometryInstanceID GeometryInstanceID, const AkGeometryInstanceParams& Params);

	/**
	* Remove a set of triangles from the Spatial Audio Engine
	*/
	AKRESULT RemoveGeometrySet(AkGeometrySetID GeometrySetID);

	/**
	* Remove a geometry instance from Spatial Audio
	*/
	AKRESULT RemoveGeometryInstance(AkGeometryInstanceID GeometryInstanceID);

	/**
	* Set the early reflections aux bus for an AK Component
	*/
	AKRESULT SetEarlyReflectionsAuxBus(UAkComponent* in_pComponent, const AkUInt32 AuxBusID);

	/**
	* Set the early reflections send volume for an AK Component
	*/
	AKRESULT SetEarlyReflectionsVolume(UAkComponent* in_pComponent, float in_fSendVolume);

	/**
	* Set the reflections order for the project
	*/
	AKRESULT SetReflectionsOrder(int Order, bool RefreshPaths);

	/**
	* Sets a game object's obstruction and occlusion levels.
	*/
	AKRESULT SetObjectObstructionAndOcclusion(AkGameObjectID in_Object, AkGameObjectID in_listener, AkReal32 Obstruction, AkReal32 Occlusion);

	/**
	* Sets a game object's obstruction and occlusion levels for each position defined by SetMultiplePositions.
	*/
	AKRESULT SetMultipleObstructionAndOcclusion(AkGameObjectID in_Object, AkGameObjectID in_listener, AkObstructionOcclusionValues* ObstructionAndOcclusionValues, AkUInt32 in_uNumObstructionAndOcclusion);

	/**
	* Set obstruction and occlusion on sounds going through this portal
	*/
	AKRESULT SetPortalObstructionAndOcclusion(const UAkPortalComponent* in_pPortal, float in_fObstructionValue, float in_fOcclusionValue);

	/**
	* Set obstruction on sounds from this game object going through this portal
	*/
	AKRESULT SetGameObjectToPortalObstruction(const UAkComponent* in_pComponent, const UAkPortalComponent* in_pPortal, float in_fObstructionValue);

	/**
	* Set obstruction on sounds from a first portal going through the next portal
	*/
	AKRESULT SetPortalToPortalObstruction(const UAkPortalComponent* in_pPortal0, const UAkPortalComponent* in_pPortal1, float in_fObstructionValue);

	/** @brief Sets an effect ShareSet on an output device
	* 
	* Make sure the new effect ShareSet is included in a soundbank, and that sound bank is loaded. Otherwise you will see errors in the Capture Log.
	* This function will replace existing effects of the audio device ShareSet.
	* Audio device effects support is limited to one ShareSet per plug-in type at any time.
	* Errors are reported in the Wwise Capture Log if the effect cannot be set on the output device.
	* ShareSet must be in a loaded bank.
	*
	*  @param InDeviceID Output ID, as returned from AddOutput or GetOutputID. You can pass 0 for the main (default) output
	*  @param InFXIndex Effect slot index (0-3)
	*  @param InFXShareSetID  Effect ShareSet ID; pass AK_INVALID_UNIQUE_ID to use the effect from the Audio Device ShareSet.
	*  @return Always returns AK_Success
	*/
	AKRESULT SetOutputDeviceEffect(AkOutputDeviceID InDeviceID, AkUInt32 InFXIndex, AkUniqueID InFXShareSetID);

	/**  @brief Sets an Effect ShareSet at the specified Bus and Effect slot index.
	* 
	* The Bus can either be an Audio Bus or an Auxiliary Bus.
	* This adds a reference on the audio node to an existing ShareSet.
	* This function has unspecified behavior when adding an Effect to a currently playing
	* Bus which does not have any effects, or removing the last Effect on a currently playing bus. 
	* Make sure the new effect ShareSet is included in a soundbank, and that sound bank is loaded. Otherwise you will see errors in the Capture Log. 
	* This function will replace existing Effects on the node. If the target node is not at
	* the top of the hierarchy and is in the Actor-Mixer Hierarchy, the option "Override Parent" in
	* the Effect section in Wwise must be enabled for this node, otherwise the parent's Effect will
	* still be the one in use and the call to SetBusEffect will have no impact.
	* ShareSet must be in a loaded bank.
	*
	*  @param InBusName Bus name
	*  @param InFXIndex Effect slot index (0-3)
	*  @param InFXShareSetID  Effect ShareSet ID; pass AK_INVALID_UNIQUE_ID to clear the Effect slot
	*  @return - AK_Success when successfully posted,  AK_IDNotFound if the Bus name doesn't point to a valid bus, AK_InvalidParameter if in_uFXIndex isn't in range.
	*/
	AKRESULT SetBusEffect(const FString& InBusName, AkUInt32 InFXIndex, AkUniqueID InFXShareSetID);

	/** @briefSets an Effect ShareSet at the specified Bus and Effect slot index.
	* 
	* The Bus can either be an Audio Bus or an Auxiliary Bus.
	* This adds a reference on the audio node to an existing ShareSet.
	* This function has unspecified behavior when adding an Effect to a currently playing
	* Bus which does not have any effects, or removing the last Effect on a currently playing bus. 
	* Make sure the new effect ShareSet is included in a soundbank, and that sound bank is loaded. Otherwise you will see errors in the Capture Log. 
	* This function will replace existing Effects on the node. If the target node is not at 
	* the top of the hierarchy and is in the Actor-Mixer Hierarchy, the option "Override Parent" in
	* the Effect section in Wwise must be enabled for this node, otherwise the parent's Effect will
	* still be the one in use and the call to SetBusEffect will have no impact.
	* ShareSet must be in a loaded bank.
	*
	*  @param InBusID Bus Short ID.
	*  @param InFXIndex Effect slot index (0-3)
	*  @param InFXShareSetID  Effect ShareSet ID; pass AK_INVALID_UNIQUE_ID to clear the Effect slot
	*  @return AK_Success when successfully posted, AK_IDNotFound if the Bus name doesn't point to a valid bus, AK_InvalidParameter if in_uFXIndex isn't in range.
	*/
	AKRESULT SetBusEffect(AkUniqueID InBusID, AkUInt32 InFXIndex, AkUniqueID InFXShareSetID);


	/** @brief Sets an Effect ShareSet at the specified audio node and Effect slot index.
	* 
	* The target node cannot be a Bus, to set effects on a bus, use SetBusEffect() instead.
	* The option "Override Parent" in the Effect section in Wwise must be enabled for this node, otherwise the parent's effect will
	* still be the one in use and the call to SetActorMixerEffect will have no impact.
	* ShareSet must be in a loaded bank.
	*
	* @param InAudioNodeID Can be a member of the Actor-Mixer or Interactive Music Hierarchy (not a bus).
	* @param InFXIndex Effect slot index (0-3)
	* @param InShareSetID ShareSets ID; pass AK_INVALID_UNIQUE_ID to clear the effect slot
	* @return Always returns AK_Success
	*/
	AKRESULT SetActorMixerEffect(AkUniqueID InAudioNodeID, AkUInt32 InFXIndex, AkUniqueID InShareSetID);
	AKRESULT SetActorMixerEffect(const FString& InBusName, AkUInt32 InFXIndex, AkUniqueID InFXShareSetID);

	/**
	 * Get an ak audio component, or create it if none exists that fit the attachment criteria.
	 */
	static class UAkComponent* GetAkComponent(
		class USceneComponent* AttachToComponent, FName AttachPointName, const FVector * Location, EAttachLocation::Type LocationType);

	static class UAkComponent* GetAkComponent(
		class USceneComponent* AttachToComponent, FName AttachPointName, const FVector * Location, EAttachLocation::Type LocationType, bool& ComponentCreated);

	/**
	 * Cancel the callback cookie for a dispatched event 
	 *
	 * @param in_cookie			The cookie to cancel
	 */
	void CancelEventCallbackCookie(void* in_cookie);

	void CancelEventCallbackDelegate(const FOnAkPostEventCallback& in_Delegate);

	 /** 
	  * Set the scaling factor of a game object.
	  * Modify the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect.
	  */
	AKRESULT SetAttenuationScalingFactor(AActor* Actor, float ScalingFactor);

	 /** 
	  * Set the scaling factor of a AkComponent.
	  * Modify the attenuation computations on this game object to simulate sounds with a a larger or smaller area of effect.
	  */
	AKRESULT SetAttenuationScalingFactor(UAkComponent* AkComponent, float ScalingFactor);

	/**
	 * Use the position of a separate AkComponent for distance calculations for a specified listener.
	 * When <tt>SetDistanceProbe()</tt> is called, Wwise calculates distance attenuation and filtering
	 * based on the distance between the distance probe AkComponent and the sound source.
	 * In third-person perspective applications, the distance probe may be set to the player character's position,
	 * and the listener position to that of the camera. In this scenario, attenuation is based on
	 * the distance between the character and the sound, whereas panning, spatialization, and spread and focus calculations are base on the camera.
	 */
	AKRESULT SetDistanceProbe(UAkComponent* Listener, UAkComponent* DistanceProbe);

	/**
	 * Starts a Wwise output capture. The output file will be located in the same folder as the SoundBanks.
	 * @param Filename - The name to give to the output file.
	 */
	void StartOutputCapture(const FString& Filename);

	/**
	 * Add text marker in output capture file.
	 * @param MarkerText - The name text to put in the marker.
	 */
	void AddOutputCaptureMarker(const FString& MarkerText);

	/**
	 * Stops a Wwise output capture. The output file will be located in the same folder as the SoundBanks.
	 */
	void StopOutputCapture();

	/**
	 * Starts a Wwise profiler capture. The output file will be located in the same folder as the SoundBanks.
	 * @param Filename - The name to give to the output file.
	 */
	void StartProfilerCapture(const FString& Filename);

	/**
	 * Stops a Wwise profiler capture. The output file will be located in the same folder as the SoundBanks.
	 */
	void StopProfilerCapture();

	/**
	 * Allows to register a Wwise plugin from a DLL name and path
	 */
	AKRESULT RegisterPluginDLL(const FString& in_DllName, const FString& in_DllPath);

	/**
	* Gets the path where the SoundBanks are located on disk
	*/
	FString GetBasePath();

	/**
	 * Suspends the SoundEngine
	 * @param in_bRenderAnyway	If set to true, audio processing will still occur, but not outputted. When set to false, no audio will be processed at all, even upon reception of RenderAudio().
	 */
	void Suspend(bool in_bRenderAnyway = false);

	/**
	 * Return from a suspended state
	 */
	void WakeupFromSuspend();

	/**
	 * Event called when the Wwise project is modified
	 */
	FOnWwiseProjectModification OnWwiseProjectModification;

	static inline void FVectorToAKVector( const FVector & in_vect, AkVector & out_vect )
	{
#if UE_5_0_OR_LATER
		checkf(in_vect.X <= FLT_MAX && in_vect.Y <= FLT_MAX && in_vect.Z <= FLT_MAX, TEXT("FVectorToAKVector: Data truncation when converting from FVector to AkVector."));
#endif
		out_vect.X = in_vect.X;
		out_vect.Y = in_vect.Y;
		out_vect.Z = in_vect.Z;
	}

	static inline AkVector FVectorToAKVector(const FVector& in_vect)
	{
#if UE_5_0_OR_LATER
		checkf(in_vect.X <= FLT_MAX && in_vect.Y <= FLT_MAX && in_vect.Z <= FLT_MAX, TEXT("FVectorToAKVector: Data truncation when converting from FVector to AkVector."));
#endif
		return AkVector{ (float)in_vect.X, (float)in_vect.Y, (float)in_vect.Z };
	}

	static inline void FVectorToAKVector64( const FVector & in_vect, AkVector64 & out_vect )
	{
		out_vect.X = in_vect.X;
		out_vect.Y = in_vect.Y;
		out_vect.Z = in_vect.Z;
	}
 
	static inline AkVector64 FVectorToAKVector64(const FVector& in_vect)
	{
		return AkVector64{ in_vect.X, in_vect.Y, in_vect.Z };
	}

	static inline AkExtent FVectorToAkExtent(const FVector& in_vect)
	{
#if UE_5_0_OR_LATER
		checkf(in_vect.X <= FLT_MAX && in_vect.Y <= FLT_MAX && in_vect.Z <= FLT_MAX, TEXT("FVectorToAkExtent: Data truncation when converting from FVector to AkExtent."));
#endif
		/* Unreal: right=y, up=z, front=x */
		return AkExtent{ (float)in_vect.Y, (float)in_vect.Z, (float)in_vect.X };
	}

	static inline void FVectorsToAKWorldTransform(const FVector& in_Position, const FVector& in_Front, const FVector& in_Up, AkWorldTransform& out_AkTransform)
	{
		// Convert from the UE axis system to the Wwise axis system
		out_AkTransform.Set(FVectorToAKVector64(in_Position), FVectorToAKVector(in_Front), FVectorToAKVector(in_Up));
	}

	static inline void AKVectorToFVector(const AkVector & in_vect, FVector & out_vect)
	{
		out_vect.X = in_vect.X;
		out_vect.Y = in_vect.Y;
		out_vect.Z = in_vect.Z;
	}

	static inline FVector AKVectorToFVector(const AkVector& in_vect)
	{
		return FVector(in_vect.X, in_vect.Y, in_vect.Z);
	}

	// Note that this is a loss of precision due to conversion from 64-bit to 32-bit
	static inline void AKVector64ToFVector(const AkVector64 & in_vect, FVector & out_vect)
	{
		out_vect.X = (float)in_vect.X;
		out_vect.Y = (float)in_vect.Y;
		out_vect.Z = (float)in_vect.Z;
	}

	// Note that this is a loss of precision due to conversion from 64-bit to 32-bit
	static inline FVector AKVector64ToFVector(const AkVector64& in_vect)
	{
		return FVector((float)in_vect.X, (float)in_vect.Y, (float)in_vect.Z);
	}

	FAkJobWorkerScheduler* GetAkJobWorkerScheduler() { return &AkJobWorkerScheduler; }

	uint8 GetMaxAuxBus() const { return MaxAuxBus; }

	AkCallbackInfoPool* GetAkCallbackInfoPool()
	{
		return CallbackInfoPool;
	}
	
#if WITH_EDITOR
	void SetMaxAuxBus(uint8 ValToSet) { MaxAuxBus = ValToSet; }
#endif

	static const int32 FIND_COMPONENTS_DEPTH_INFINITE = -1;

	/** Find UAkLateReverbComponents at a given location. */
	TArray<class UAkLateReverbComponent*> FindLateReverbComponentsAtLocation(const FVector& Loc, const UWorld* in_World);

	/** Add a UAkLateReverbComponent to the spatial index data structure. */
	void IndexLateReverb(class UAkLateReverbComponent* ComponentToAdd);

	/** Remove a UAkLateReverbComponent from the spatial index data structure. */
	void UnindexLateReverb(class UAkLateReverbComponent* ComponentToRemove);

	/** Update the bounds of a UAkLateReverbComponent in the spatial index data structure. Must be called if the component's transform changes.*/
	void ReindexLateReverb(class UAkLateReverbComponent* ComponentToAdd);

	/** Get whether the given world has room registered in it. */
	bool WorldHasActiveRooms(UWorld* World);

	/** Find UAkRoomComponents at a given location. */
	TArray<class UAkRoomComponent*> FindRoomComponentsAtLocation(const FVector& Loc, const UWorld* World);

	/** Return true if any UAkRoomComponents have been added to the prioritized list of rooms for the in_World**/
	bool UsingSpatialAudioRooms(const UWorld* World);

	/** Get the aux send values corresponding to a point in the world.**/
	void GetAuxSendValuesAtLocation(FVector Loc, TArray<AkAuxSendValue>& AkAuxSendValues, const UWorld* in_World);

	/** Update all portals. */
	void UpdateAllSpatialAudioPortals(UWorld* InWorld);
	
	/** Queue an update for all portals in a world to reconnect to their front and back rooms */
	void PortalsNeedRoomUpdate(UWorld* World) { WorldsInNeedOfPortalRoomsUpdate.Add(World); }

	/** Register a Portal in AK Spatial Audio.  Can be called again to update the portal parameters.	*/
	void SetSpatialAudioPortal(UAkPortalComponent* in_Portal);
	
	/** Remove a Portal from AK Spatial Audio	*/
	void RemoveSpatialAudioPortal(UAkPortalComponent* in_Portal);
	
	void OnActorSpawned(AActor* SpawnedActor);

	UAkComponentSet& GetDefaultListeners() { return m_defaultListeners; }

	void SetListeners(UAkComponent* in_pEmitter, const TArray<TWeakObjectPtr<UAkComponent>>& in_listenerSet);
	void AddDefaultListener(UAkComponent* in_pListener);
	void RemoveDefaultListener(UAkComponent* in_pListener);
	void UpdateDefaultActiveListeners();
#if WITH_EDITORONLY_DATA
	FTransform GetEditorListenerPosition(int32 ViewIndex) const;
#endif

	/** Specifies which listener is used for Wwise Spatial Audio**/
	bool SetSpatialAudioListener(UAkComponent* in_pListener);
	
	/** Get the listener that has been choosen to be used for Wwise Spatial Audio**/
	UAkComponent* GetSpatialAudioListener() const;

	AKRESULT SetPosition(UAkComponent* in_akComponent, const AkSoundPosition& in_SoundPosition);

	/** Add a UAkRoomComponent to the spatial index data structure. */
	void IndexRoom(class UAkRoomComponent* ComponentToAdd);

	/** Remove a UAkRoomComponent from the spatial index data structure. */
	void UnindexRoom(class UAkRoomComponent* ComponentToRemove);

	/** Update the bounds of a UAkRoomComponent in the spatial index data structure. Must be called if the component's transform changes.*/
	void ReindexRoom(class UAkRoomComponent* ComponentToAdd);

	AKRESULT AddRoom(UAkRoomComponent* in_pRoom, const AkRoomParams& in_RoomParams);
	AKRESULT UpdateRoom(UAkRoomComponent* in_pRoom, const AkRoomParams& in_RoomParams);
	AKRESULT RemoveRoom(UAkRoomComponent* in_pRoom);

	AKRESULT SetGameObjectRadius(UAkComponent* in_akComponent, float in_outerRadius, float in_innerRadius);

	AKRESULT SetImageSource(class AAkSpotReflector* in_pSpotReflector, const AkImageSourceSettings& in_ImageSourceInfo, AkUniqueID in_AuxBusID, UAkComponent* in_AkComponent);
	AKRESULT RemoveImageSource(class AAkSpotReflector* in_pSpotReflector, AkUniqueID in_AuxBusID, UAkComponent* in_AkComponent);
	AKRESULT ClearImageSources(AkUniqueID in_AuxBusID = AK_INVALID_AUX_ID, UAkComponent* in_AkComponent = NULL);

    static void GetChannelConfig(AkChannelConfiguration ChannelConfiguration, AkChannelConfig& config);
	static void GetChannelConfig(FAkChannelMask SpeakerConfiguration, AkChannelConfig& config);

	FAkEnvironmentIndex& GetRoomIndex() { return RoomIndex; }

	void AddPortalConnectionToOutdoors(const UWorld* in_world, UAkPortalComponent* in_pPortal);
	void RemovePortalConnectionToOutdoors(const UWorld* in_world, AkPortalID in_portalID);
	void GetObsOccServicePortalMap(const TWeakObjectPtr<UAkRoomComponent> InRoom, const UWorld* InWorld, AkObstructionAndOcclusionService::PortalMap& OutPortalMap);

	struct SetCurrentAudioCultureAsyncTask
	{
		enum CompletionType
		{
			LatentAction,
			Callback
		};

		FWwiseLanguageCookedData Language;
		FThreadSafeBool IsDone = false;
		FThreadSafeBool Succeeded = false;

		SetCurrentAudioCultureAsyncTask(FWwiseLanguageCookedData NewLanguage, FSetCurrentAudioCultureAction* LatentAction);
		SetCurrentAudioCultureAsyncTask(FWwiseLanguageCookedData NewLanguage, const FOnSetCurrentAudioCultureCompleted& CompletedCallback);

		bool Start();
		void Update();

	private:
		TSharedPtr<FPendingLatentActionValidityToken, ESPMode::ThreadSafe> LatentActionValidityToken;
		CompletionType CompletionActionType;
		FSetCurrentAudioCultureAction* SetAudioCultureLatentAction;
		FOnSetCurrentAudioCultureCompleted SetAudioCultureCompletedCallback;
	};

	void AddPlayingID(uint32 EventID, uint32 PlayingID, EAkAudioContext AudioContext);
	bool IsPlayingIDActive(uint32 EventID, uint32 PlayingID);
	bool IsEventIDActive(uint32 EventID);
	void RemovePlayingID(uint32 EventID, uint32 PlayingID);
	void StopEventID(uint32 EventID);

	FOnSwitchValueLoaded& GetOnSwitchValueLoaded(uint32 SwitchID);
	void BroadcastOnSwitchValueLoaded(UAkGroupValue* GroupValue);
	void SetLocalOutput();

	FAkComponentCallbackManager* GetCallbackManager() { return CallbackManager; }
	AKRESULT RegisterGameObject(AkGameObjectID GameObjectID, const FString& Name);

	/** Determine whether the Wwise sound engine should be updated for the given world type */
	static bool ShouldNotifySoundEngine(EWorldType::Type WorldType);

	static void LoadAudioObjectsAfterInitialization(TWeakObjectPtr<UAkAudioType>&& InAudioType);
	void LoadDelayedObjects();

private:
	bool EnsureInitialized();

	void* AllocatePermanentMemory( int32 Size, /*OUT*/ bool& AllocatedInPool );
	
	AKRESULT GetGameObjectID(AActor * in_pActor, AkGameObjectID& io_GameObject );

	template<typename FCreateCallbackPackage>
	AkPlayingID PostEventWithCallbackPackageOnAkGameObject(
		const AkUInt32 EventShortID,
		UAkGameObject* GameObject,
		const TArray<AkExternalSourceInfo>& ExternalSources,
		FCreateCallbackPackage CreateCallbackPackage,
		EAkAudioContext AudioContext
	);

	template<typename ChannelConfig>
	AKRESULT SetMultiplePositions(
		UAkComponent* in_pGameObjectAkComponent,
		const TArray<ChannelConfig>& in_aChannelConfigurations,
		const TArray<FTransform>& in_aPositions,
		AkMultiPositionType in_eMultiPositionType
	);

	// Overload allowing to modify StopWhenOwnerDestroyed after getting the AkComponent
	AKRESULT GetGameObjectID(AActor * in_pActor, AkGameObjectID& io_GameObject, bool in_bStopWhenOwnerDestroyed );

	/** Update the room connections for all portals */
	void UpdateRoomsForPortals();

#if WITH_EDITORONLY_DATA
	UAkComponent* CreateListener(UWorld* World, FEditorViewportClient* ViewportClient = nullptr);
	TArray<FTransform> ListenerTransforms;
	UAkComponent* EditorListener = nullptr;

	/**
	* Update Default Listeners when going in and out of Play mode in editor
	*/
	void EndPIE(const bool bIsSimulating);
	void BeginPIE(const bool bIsSimulating);
	void PausePIE(const bool bIsSimulating);
	void ResumePie(const bool bIsSimulating);
	void OnSwitchBeginPIEAndSIE(const bool bIsSimulating);

#endif

	/** Map to track whether new reverbs or rooms have been added to worlds in the previous frame.
	 *  AkComponent checks this in its TickComponent function and updates its assigned room and/or reverb volumes.
	 */
	TMap<UWorld*, bool> WorldVolumesUpdatedMap;
	void SAComponentAddedRemoved(UWorld* World);

	/** We keep a spatial index of UAkLateReverbComponents sorted by priority for faster finding of reverb volumes at a specific location.
	 */
	FAkEnvironmentIndex LateReverbIndex;

	/** We keep a spatial index of Spatial audio Rooms for faster finding of reverb volumes at a specific location.
	 */
	FAkEnvironmentIndex RoomIndex;

	/** We keep track of the portals in each world so their rooms can be updated when room and portal parameters change.
	*/
	TMap<UWorld*, TArray<TWeakObjectPtr<UAkPortalComponent>>> WorldPortalsMap;

	typedef WwiseUnrealHelper::AkSpatialAudioIDKeyFuncs<TWeakObjectPtr<UAkPortalComponent>, false> PortalComponentSpatialAudioIDKeyFuncs;
	typedef TMap<AkPortalID, TWeakObjectPtr<UAkPortalComponent>, FDefaultSetAllocator, PortalComponentSpatialAudioIDKeyFuncs> PortalComponentMap;
	TMap<const UWorld*, PortalComponentMap> OutdoorsConnectedPortals;

	void CleanupComponentMapsForWorld(UWorld* World);

	bool FindWwiseLanguage(const FString& NewAudioCulture, FString& FoundWwiseLanguage);
	void UpdateSetCurrentAudioCultureAsyncTasks();

	static bool m_bSoundEngineInitialized;
	UAkComponentSet m_defaultListeners;
	UAkComponent* m_SpatialAudioListener;

	bool m_isSuspended = false;

	uint8 MaxAuxBus;

	FAkComponentCallbackManager* CallbackManager;
	AkCallbackInfoPool* CallbackInfoPool;
	FAkJobWorkerScheduler AkJobWorkerScheduler;
	FWwiseIOHook* IOHook = nullptr;

	static bool m_EngineExiting;

	/* WAAPI Callback handles. */
	FDelegateHandle ProjectLoadedHandle;
	FDelegateHandle ConnectionLostHandle;
	FDelegateHandle ClientBeginDestroyHandle;

	struct FWaapiSubscriptionIds
	{
		uint64 Renamed = 0;
		uint64 PreDeleted = 0;
		uint64 ChildRemoved = 0;
		uint64 ChildAdded = 0;
		uint64 Created = 0;
	} WaapiSubscriptionIds;

	TArray<SetCurrentAudioCultureAsyncTask*> AudioCultureAsyncTasks;

	TSet<UWorld*> WorldsInNeedOfPortalRoomsUpdate;

#if !WITH_EDITOR
	TMap<FCulturePtr, FString> CachedUnrealToWwiseCulture;
#endif
	static FCriticalSection EventToPlayingIDMapCriticalSection;
	static TMap<uint32, TArray<uint32>> EventToPlayingIDMap;
	static TMap<uint32, EAkAudioContext> PlayingIDToAudioContextMap;

	static void PostEventAtLocationEndOfEventCallback(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo);

	static TMap<uint32, FOnSwitchValueLoaded> OnSwitchValueLoadedMap;

	static TArray<TWeakObjectPtr<UAkAudioType>> AudioObjectsToLoadAfterInitialization;

#if WITH_EDITORONLY_DATA
#ifndef AK_OPTIMIZED
	static AkErrorMessageTranslator* m_UnrealErrorTranslator;
#if AK_SUPPORT_WAAPI
	static AkWAAPIErrorMessageTranslator m_waapiErrorMessageTranslator;
#endif //AK_SUPPORT_WAAPI
#endif //AK_OPTIMIZED
#endif //WITH_EDITORONLY_DATA
};
