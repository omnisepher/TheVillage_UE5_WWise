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

#include "Wwise/API/WwiseCommAPI.h"

class WWISESOUNDENGINE_API FWwiseCommAPI_2022_1 : public IWwiseCommAPI
{
public:
	UE_NONCOPYABLE(FWwiseCommAPI_2022_1);
	FWwiseCommAPI_2022_1() = default;

	///////////////////////////////////////////////////////////////////////
	/// @name Initialization
	//@{

	/// Initializes the communication module. When this is called, and AK::SoundEngine::RenderAudio()
	/// is called periodically, you may use the authoring tool to connect to the sound engine.
	///
	/// \warning This function must be called after the sound engine and memory manager have
	///          been properly initialized.
	///
	///
	/// \remark The AkCommSettings structure should be initialized with
	///         AK::Comm::GetDefaultInitSettings(). You can then change some of the parameters
	///			before calling this function.
	///
	/// \return
	///      - AK_Success if initialization was successful.
	///      - AK_InvalidParameter if one of the settings is invalid.
	///      - AK_InsufficientMemory if the specified pool size is too small for initialization.
	///      - AK_Fail for other errors.
	///		
	/// \sa
	/// - \ref initialization_comm
	/// - AK::Comm::GetDefaultInitSettings()
	/// - AkCommSettings::Ports
	virtual AKRESULT Init(
		const AkCommSettings& in_settings///< Initialization settings.			
		) override;

	/// Gets the last error from the OS-specific communication library.
	/// \return The system error code.  Check the code in the platform manufacturer documentation for details about the error.
	virtual AkInt32 GetLastError() override;

	/// Gets the communication module's default initialization settings values.
	/// \sa
	/// - \ref initialization_comm 
	/// - AK::Comm::Init()
	virtual void GetDefaultInitSettings(
		AkCommSettings& out_settings	///< Returned default initialization settings.
		) override;

	/// Terminates the communication module.
	/// \warning This function must be called before the memory manager is terminated.		
	/// \sa
	/// - \ref termination_comm 
	virtual void Term() override;

	/// Terminates and reinitialize the communication module using current settings.
	///
	/// \return
	///      - AK_Success if initialization was successful.
	///      - AK_InvalidParameter if one of the settings is invalid.
	///      - AK_InsufficientMemory if the specified pool size is too small for initialization.
	///      - AK_Fail for other errors.
	///
	/// \sa
	/// - \ref AK::SoundEngine::iOS::WakeupFromSuspend()
	virtual AKRESULT Reset() override;


	/// Get the initialization settings currently in use by the CommunicationSystem
	///
	/// \return
	///      - AK_Success if initialization was successful.
	virtual const AkCommSettings& GetCurrentSettings() override;


	/// Get the port currently in used by the command channel.
	///
	/// \return
	///      - Port number.
	virtual AkUInt16 GetCommandPort() override;

	//@}
};
