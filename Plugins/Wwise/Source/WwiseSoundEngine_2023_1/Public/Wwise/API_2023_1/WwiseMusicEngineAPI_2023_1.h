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

#include "Wwise/API/WwiseMusicEngineAPI.h"

class WWISESOUNDENGINE_API FWwiseMusicEngineAPI_2023_1 : public IWwiseMusicEngineAPI
{
public:
	UE_NONCOPYABLE(FWwiseMusicEngineAPI_2023_1);
	FWwiseMusicEngineAPI_2023_1() = default;

	///////////////////////////////////////////////////////////////////////
	/// @name Initialization
	//@{

	/// Initialize the music engine.
	/// \warning This function must be called after the base sound engine has been properly initialized. 
	/// There should be no AK API call between AK::SoundEngine::Init() and this call. Any call done in between is potentially unsafe.
	/// \return AK_Success if the Init was successful, AK_Fail otherwise.
	/// \sa
	/// - \ref workingwithsdks_initialization
	virtual AKRESULT Init(
		AkMusicSettings* in_pSettings	///< Initialization settings (can be NULL, to use the default values)
		) override;

	/// Get the music engine's default initialization settings values
	/// \sa
	/// - \ref soundengine_integration_init_advanced
	/// - AK::MusicEngine::Init()
	virtual void GetDefaultInitSettings(
		AkMusicSettings& out_settings	///< Returned default platform-independent music engine settings
		) override;

	/// Terminate the music engine.
	/// \warning This function must be called before calling Term() on the base sound engine.
	/// \sa
	/// - \ref workingwithsdks_termination
	virtual void Term(
		) override;

	/// Query information on the active segment of a music object that is playing. Use the playing ID 
	/// that was returned from AK::SoundEngine::PostEvent(), provided that the event contained a play
	/// action that was targeting a music object. For any configuration of interactive music hierarchy, 
	/// there is only one segment that is active at a time. 
	/// To be able to query segment information, you must pass the AK_EnableGetMusicPlayPosition flag 
	/// to the AK::SoundEngine::PostEvent() method. This informs the sound engine that the source associated 
	/// with this event should be given special consideration because GetPlayingSegmentInfo() can be called 
	/// at any time for this AkPlayingID.
	/// Notes:
	/// - If the music object is a single segment, you will get negative values for AkSegmentInfo::iCurrentPosition
	///		during the pre-entry. This will never occur with other types of music objects because the 
	///		pre-entry of a segment always overlaps another active segment.
	///	- The active segment during the pre-entry of the first segment of a Playlist Container or a Music Switch 
	///		Container is "nothing", as well as during the post-exit of the last segment of a Playlist (and beyond).
	///	- When the active segment is "nothing", out_uSegmentInfo is filled with zeros.
	/// - If in_bExtrapolate is true (default), AkSegmentInfo::iCurrentPosition is corrected by the amount of time elapsed
	///		since the beginning of the audio frame. It is thus possible that it slightly overshoots the total segment length.
	/// \return AK_Success if there is a playing music structure associated with the specified playing ID.
	/// \sa
	/// - AK::SoundEngine::PostEvent
	/// - AkSegmentInfo
	virtual AKRESULT GetPlayingSegmentInfo(
		AkPlayingID		in_PlayingID,			///< Playing ID returned by AK::SoundEngine::PostEvent().
		AkSegmentInfo& out_segmentInfo,		///< Structure containing information about the active segment of the music structure that is playing.
		bool			in_bExtrapolate = true	///< Position is extrapolated based on time elapsed since last sound engine update.
		) override;

	//@}
};
