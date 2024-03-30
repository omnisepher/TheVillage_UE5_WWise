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
#include "AkInclude.h"

#include "Wwise/WwiseSoundEngineModule.h"

class IWwiseSpatialAudioAPI
{
public:
	static IWwiseSpatialAudioAPI* Get()
	{
		IWwiseSoundEngineModule::ForceLoadModule();
		return IWwiseSoundEngineModule::SpatialAudio;
	}

	class IReverbEstimation;
	UE_NONCOPYABLE(IWwiseSpatialAudioAPI);

protected:
	IWwiseSpatialAudioAPI(IReverbEstimation* InReverbEstimation) :
		ReverbEstimation(InReverbEstimation)
	{}
public:
	virtual ~IWwiseSpatialAudioAPI()
	{
		delete ReverbEstimation;
	}

	////////////////////////////////////////////////////////////////////////
	/// @name Basic functions. 
	/// In order to use SpatialAudio, you need to initialize it using Init, and register the listeners that you plan on using with any of the services offered by SpatialAudio, using 
	/// RegisterListener respectively, _after_ having registered their corresponding game object to the sound engine.
	/// \akwarning At the moment, there can be only one Spatial Audio listener registered at any given time.
	//@{

	/// Initialize the SpatialAudio API.  
	virtual AKRESULT Init(const AkSpatialAudioInitSettings& in_initSettings) = 0;

	/// Assign a game object as the Spatial Audio listener.  There can be only one Spatial Audio listener registered at any given time; in_gameObjectID will replace any previously set Spatial Audio listener.
	/// The game object passed in must be registered by the client, at some point, for sound to be heard.  It is not necessary to be registered at the time of calling this function.
	/// If not listener is explicitly registered to spatial audio, then a default listener (set via \c AK::SoundEngine::SetDefaultListeners()) is selected.  If the are no default listeners, or there are more than one
	/// default listeners, then it is necessary to call RegisterListener() to specify which listener to use with Spatial Audio.
	virtual AKRESULT RegisterListener(
		AkGameObjectID in_gameObjectID				///< Game object ID
		) = 0;

	/// Unregister a game object as a listener in the SpatialAudio API; clean up Spatial Audio listener data associated with in_gameObjectID.  
	/// If in_gameObjectID is the current registered listener, calling this function will clear the Spatial Audio listener and
	/// Spatial Audio features will be disabled until another listener is registered.
	/// This function is optional - listener are automatically unregistered when their game object is deleted in the sound engine.
	/// \sa 
	/// - \ref AK::SpatialAudio::RegisterListener
	virtual AKRESULT UnregisterListener(
		AkGameObjectID in_gameObjectID				///< Game object ID
		) = 0;

	/// Define a inner and outer radius around each sound position for a specified game object. 
	/// The radii are used in spread and distance calculations, simulating a radial sound source.
	/// When applying attenuation curves, the distance between the listener and the inner sphere (defined by the sound position and \c in_innerRadius) is used. 
	/// The spread for each sound position is calculated as follows:
	/// - If the listener is outside the outer radius, then the spread is defined by the area that the sphere takes in the listener field of view. Specifically, this angle is calculated as 2.0*asinf( \c in_outerRadius / distance ), where distance is the distance between the listener and the sound position.
	///	- When the listener intersects the outer radius (the listener is exactly \c in_outerRadius units away from the sound position), the spread is exactly 50%.
	/// - When the listener is in between the inner and outer radius, the spread interpolates linearly from 50% to 100% as the listener transitions from the outer radius towards the inner radius.
	/// - If the listener is inside the inner radius, the spread is 100%.
	/// \aknote Transmission and diffraction calculations in Spatial Audio always use the center of the sphere (the position(s) passed into \c AK::SoundEngine::SetPosition or \c AK::SoundEngine::SetMultiplePositions) for raycasting. 
	/// To obtain accurate diffraction and transmission calculations for radial sources, where different parts of the volume may take different paths through or around geometry,
	/// it is necessary to pass multiple sound positions into \c AK::SoundEngine::SetMultiplePositions to allow the engine to 'sample' the area at different points.
	/// - \ref AK::SoundEngine::SetPosition
	/// - \ref AK::SoundEngine::SetMultiplePositions
	virtual AKRESULT SetGameObjectRadius(
		AkGameObjectID in_gameObjectID,				///< Game object ID
		AkReal32 in_outerRadius,					///< Outer radius around each sound position, defining 50% spread. Must satisfy \c in_innerRadius <= \c in outerRadius.
		AkReal32 in_innerRadius						///< Inner radius around each sound position, defining 100% spread and 0 attenuation distance. Must satisfy \c in_innerRadius <= \c in outerRadius.
		) = 0;

	//@}

	////////////////////////////////////////////////////////////////////////
	/// @name Helper functions for passing game data to the Reflect plug-in. 
	/// Use this API for detailed placement of reflection image sources.
	/// \aknote These functions are low-level and useful when your game engine already implements a geometrical approach to sound propagation such as an image-source or a ray tracing algorithm.
	/// Functions of Geometry are preferred and easier to use with the Reflect plug-in. \endaknote
	//@{

	/// Add or update an individual image source for processing via the AkReflect plug-in.  Use this API for detailed placement of
	/// reflection image sources, whose positions have been determined by the client, such as from the results of a ray cast, computation or by manual placement.  One possible
	/// use case is generating reflections that originate far enough away that they can be modeled as a static point source, for example, off of a distant mountain.
	/// The SpatialAudio API manages image sources added via SetImageSource() and sends them to the AkReflect plug-in that is on the aux bus with ID \c in_AuxBusID. 
	/// The image source applies all game objects that have a reflections aux send defined in the authoring tool, or only to a specific game object if \c in_gameObjectID is used.
	/// \aknote The \c AkImageSourceSettings struct passed in \c in_info must contain a unique image source ID to be able to identify this image source across frames and when updating and/or removing it later.  
	/// Each instance of AkReflect has its own set of data, so you may reuse ID, if desired, as long as \c in_gameObjectID and \c in_AuxBusID are different.
	/// \aknote It is possible for the AkReflect plugin to process reflections from both \c SetImageSource and the geometric reflections API on the same aux bus and game object, but be aware that image source ID collisions are possible.
	/// The image source IDs used by the geometric reflections API are generated from hashed data that uniquely identifies the reflecting surfaces. If a collision occurs, one of the reflections will not be heard.
	/// While collision are rare, to ensure that it never occurs use an aux bus for \c SetImageSource that is unique from the aux bus(es) defined in the authoring tool, and from those passed to \c SetEarlyReflectionsAuxSend.
	/// \endaknote
	/// \aknote For proper operation with AkReflect and the SpatialAudio API, any aux bus using AkReflect should have 'Listener Relative Routing' checked and the 3D Spatialization set to None in the Wwise authoring tool. See \ref spatial_audio_wwiseprojectsetup_businstances for more details. \endaknote
	/// \sa 
	/// - \ref AK::SpatialAudio::RemoveImageSource
	///	- \ref AK::SpatialAudio::ClearImageSources
	/// - \ref AK::SpatialAudio::SetGameObjectInRoom
	/// - \ref AK::SpatialAudio::SetEarlyReflectionsAuxSend
	virtual AKRESULT SetImageSource(
		AkImageSourceID in_srcID,								///< The ID of the image source being added.
		const AkImageSourceSettings& in_info,					///< Image source information.
		const char* in_name,									///< Name given to image source, can be used to identify the image source in the AK Reflect plugin UI.
		AkUniqueID in_AuxBusID = AK_INVALID_AUX_ID,				///< Aux bus that has the AkReflect plug in for early reflection DSP. 
																///< Pass AK_INVALID_AUX_ID to use the reflections aux bus defined in the authoring tool.
		AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT	///< The ID of the emitter game object to which the image source applies. 
																///< Pass AK_INVALID_GAME_OBJECT to apply to all game objects that have a reflections aux bus assigned in the authoring tool.
		) = 0;

	/// Remove an individual reflection image source that was previously added via \c SetImageSource.
	/// \sa 
	///	- \ref AK::SpatialAudio::SetImageSource
	///	- \ref AK::SpatialAudio::ClearImageSources
	virtual AKRESULT RemoveImageSource(
		AkImageSourceID in_srcID,									///< The ID of the image source to remove.
		AkUniqueID in_AuxBusID = AK_INVALID_AUX_ID,					///< Aux bus that was passed to SetImageSource.
		AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT		///< Game object ID that was passed to SetImageSource.
		) = 0;

	/// Remove all image sources matching \c in_AuxBusID and \c in_gameObjectID that were previously added via \c SetImageSource.
	/// Both \c in_AuxBusID and \c in_gameObjectID can be treated as wild cards matching all aux buses and/or all game object, by passing \c AK_INVALID_AUX_ID and/or \c AK_INVALID_GAME_OBJECT, respectively.
	/// \sa 
	///	- \ref AK::SpatialAudio::SetImageSource
	/// - \ref AK::SpatialAudio::RemoveImageSource
	virtual AKRESULT ClearImageSources(
		AkUniqueID in_AuxBusID = AK_INVALID_AUX_ID,							///< Aux bus that was passed to SetImageSource, or AK_INVALID_AUX_ID to match all aux buses.
		AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT				///< Game object ID that was passed to SetImageSource, or AK_INVALID_GAME_OBJECT to match all game objects.
		) = 0;

	//@}

	////////////////////////////////////////////////////////////////////////
	/// @name Geometry 
	/// Geometry API for early reflection processing using Reflect.
	//@{

	/// Add or update a set of geometry from the \c SpatialAudio module for geometric reflection and diffraction processing. A geometry set is a logical set of vertices, triangles, and acoustic surfaces,
	/// which are referenced by the same \c AkGeometrySetID. The ID (\c in_GeomSetID) must be unique and is also chosen by the client in a manner similar to \c AkGameObjectID's.
	/// It is necessary to create at least one geometry instance for each geometry set that is to be used for diffraction and reflection simulation.
	/// \sa 
	///	- \ref AkGeometryParams
	///	- \ref AK::SpatialAudio::SetGeometryInstance
	///	- \ref AK::SpatialAudio::RemoveGeometry
	virtual AKRESULT SetGeometry(
		AkGeometrySetID in_GeomSetID,		///< Unique geometry set ID, chosen by client.
		const AkGeometryParams& in_params	///< Geometry parameters to set.
		) = 0;

	/// Remove a set of geometry to the SpatialAudio API.
	/// Calling \c AK::SpatialAudio::RemoveGeometry will remove all instances of the geometry from the scene.
	/// \sa 
	///	- \ref AK::SpatialAudio::SetGeometry
	virtual AKRESULT RemoveGeometry(
		AkGeometrySetID in_SetID		///< ID of geometry set to be removed.
		) = 0;

	/// Add or update a geometry instance from the \c SpatialAudio module for geometric reflection and diffraction processing. 
	/// A geometry instance is a unique instance of a geometry set with a specified transform (position, rotation and scale). 
	/// It is necessary to create at least one geometry instance for each geometry set that is to be used for diffraction and reflection simulation.
	/// The ID (\c in_GeomSetInstanceID) must be unique amongst all geometry instances, including geometry instances referencing different geometry sets. The ID is chosen by the client in a manner similar to \c AkGameObjectID's.
	/// To update the transform of an existing geometry instance, call SetGeometryInstance again, passing the same \c AkGeometryInstanceID, with the updated transform. 
	/// \sa 
	///	- \ref AkGeometryInstanceParams
	///	- \ref AK::SpatialAudio::RemoveGeometryInstance
	virtual AKRESULT SetGeometryInstance(
		AkGeometryInstanceID in_GeometryInstanceID,	///< Unique geometry set instance ID, chosen by client.
		const AkGeometryInstanceParams& in_params	///< Geometry instance parameters to set.
		) = 0;

	/// Remove a geometry instance from the SpatialAudio API.
	/// \sa 
	///	- \ref AK::SpatialAudio::SetGeometryInstance
	virtual AKRESULT RemoveGeometryInstance(
		AkGeometryInstanceID in_GeometryInstanceID	///< ID of geometry set instance to be removed.
		) = 0;

	/// Query information about the reflection paths that have been calculated via geometric reflection processing in the SpatialAudio API. This function can be used for debugging purposes.
	/// This function must acquire the global sound engine lock and therefore, may block waiting for the lock.
	/// \sa
	/// - \ref AkReflectionPathInfo
	virtual AKRESULT QueryReflectionPaths(
		AkGameObjectID in_gameObjectID, ///< The ID of the game object that the client wishes to query.
		AkUInt32 in_positionIndex,		///< The index of the associated game object position.
		AkVector64& out_listenerPos,		///< Returns the position of the listener game object that is associated with the game object \c in_gameObjectID.
		AkVector64& out_emitterPos,		///< Returns the position of the emitter game object \c in_gameObjectID.
		AkReflectionPathInfo* out_aPaths,	///< Pointer to an array of \c AkReflectionPathInfo's which will be filled after returning.
		AkUInt32& io_uArraySize			///< The number of slots in \c out_aPaths, after returning the number of valid elements written.
		) = 0;

	//@}

	////////////////////////////////////////////////////////////////////////
	/// @name Rooms and Portals
	/// Sound Propagation API using rooms and portals.
	//@{

	/// Add or update a room. Rooms are used to connect portals and define an orientation for oriented reverbs. This function may be called multiple times with the same ID to update the parameters of the room.
	/// \akwarning The ID (\c in_RoomID) must be chosen in the same manner as \c AkGameObjectID's, as they are in the same ID-space. The spatial audio lib manages the 
	/// registration/unregistration of internal game objects for rooms that use these IDs and, therefore, must not collide. 
	/// Also, the room ID must not be in the reserved range (AkUInt64)(-32) to (AkUInt64)(-2) inclusively. You may, however, explicitly add the default room ID AK::SpatialAudio::kOutdoorRoomID (-1) 
	/// in order to customize its AkRoomParams, to provide a valid auxiliary bus, for example.\endakwarning
	/// \sa
	/// - \ref AkRoomID
	/// - \ref AkRoomParams
	/// - \ref AK::SpatialAudio::RemoveRoom
	virtual AKRESULT SetRoom(
		AkRoomID in_RoomID,				///< Unique room ID, chosen by the client.
		const AkRoomParams& in_Params,	///< Parameter for the room.
		const char* in_RoomName = nullptr ///< Name used to identify the room (optional)
		) = 0;

	/// Remove a room.
	/// \sa
	/// - \ref AkRoomID
	/// - \ref AK::SpatialAudio::SetRoom
	virtual AKRESULT RemoveRoom(
		AkRoomID in_RoomID	///< Room ID that was passed to \c SetRoom.
		) = 0;

	/// Add or update an acoustic portal. A portal is an opening that connects two or more rooms to simulate the transmission of reverberated (indirect) sound between the rooms. 
	/// This function may be called multiple times with the same ID to update the parameters of the portal. The ID (\c in_PortalID) must be chosen in the same manner as \c AkGameObjectID's, 
	/// as they are in the same ID-space. The spatial audio lib manages the registration/unregistration of internal game objects for portals that use these IDs, and therefore must not collide.
	/// \sa
	/// - \ref AkPortalID
	/// - \ref AkPortalParams
	/// - \ref AK::SpatialAudio::RemovePortal
	virtual AKRESULT SetPortal(
		AkPortalID in_PortalID,		///< Unique portal ID, chosen by the client.
		const AkPortalParams& in_Params,	///< Parameter for the portal.
		const char* in_PortalName = nullptr	///< Name used to identify portal (optional)
		) = 0;

	/// Remove a portal.
	/// \sa
	/// - \ref AkPortalID
	/// - \ref AK::SpatialAudio::SetPortal
	virtual AKRESULT RemovePortal(
		AkPortalID in_PortalID		///< ID of portal to be removed, which was originally passed to SetPortal.
		) = 0;

	/// Use a Room as a Reverb Zone.
	/// AK::SpatialAudio::SetReverbZone establishes a parent-child relationship between two Rooms and allows for sound propagation between them
	/// as if they were the same Room, without the need for a connecting Portal. Setting a Room as a Reverb Zone
	/// is useful in situations where two or more acoustic environments are not easily modeled as closed rooms connected by portals.
	/// Possible uses for Reverb Zones include: a covered area with no walls, a forested area within an outdoor space, or any situation
	/// where multiple reverb effects are desired within a common space. Reverb Zones have many advantages compared to standard Game-Defined
	/// Auxiliary Sends. They are part of the wet path, and form reverb chains with other Rooms; they are spatialized according to their 3D extent;
	/// they are also subject to other acoustic phenomena simulated in Wwise Spatial Audio, such as diffraction and transmission.
	/// A parent Room may have multiple Reverb Zones, but a Reverb Zone can only have a single Parent. If a Room is already assigned
	/// to a parent Room, it will first be removed from the old parent (exactly as if AK::SpatialAudio::RemoveReverbZone were called)
	/// before then being assigned to the new parent Room. A Room can not be its own parent.
	/// The Reverb Zone and its parent are both Rooms, and as such, must be specified using AK::SpatialAudio::SetRoom.
	/// If AK::SpatialAudio::SetReverbZone is called before AK::SpatialAudio::SetRoom, and either of the two rooms do not yet exist,
	/// placeholder Rooms with default parameters are created. They should be subsequently parameteized with AK::SpatialAudio::SetRoom.
	/// 
	/// To set which Reverb Zone a Game Object is in, use the AK::SpatialAudio::SetGameObjectInRoom API, and pass the Reverb Zone's Room ID.
	/// In Wwise Spatial Audio, a Game Object can only ever be inside a single room, and Reverb Zones are no different in this regard.
	/// \aknote
	/// The automatically created 'outdoors' Room is commonly used as a parent Room for Reverb Zones, since they often model open spaces.
	/// To attach a Reverb zone to outdoors, pass AK::SpatialAudio::kOutdoorRoomID as the \c in_ParentRoom argument. Like all Rooms, the 'outdoors' Room
	/// can be parameterized (for example, to assign a reverb bus) by passing AK::SpatialAudio::kOutdoorRoomID to AK::SpatialAudio::SetRoom.
	/// \sa
	/// - \ref AkRoomID
	///	- \ref AK::SpatialAudio::SetRoom
	///	- \ref AK::SpatialAudio::RemoveRoom
	///	- \ref AK::SpatialAudio::RemoveReverbZone
	/// - \ref AK::SpatialAudio::kOutdoorRoomID
	virtual AKRESULT SetReverbZone(
		AkRoomID in_ReverbZone,			///< ID of the Room which will be specified as a Reverb Zone.
		AkRoomID in_ParentRoom,			///< ID of the parent Room.
		AkReal32 in_transitionRegionWidth	///< Width of the transition region between the Reverb Zone and its parent. The transition region is centered around the Reverb Zone geometry. It only applies where triangle transmission loss is set to 0.
		) = 0;

	/// Remove a Reverb Zone from its parent.
	/// It will no longer be possible for sound to propagate between the two rooms, unless they are explicitly connected with a Portal.
	/// \sa
	///	- \ref AK::SpatialAudio::SetReverbZone
	///	- \ref AK::SpatialAudio::RemoveRoom
	///	- \ref AK::SpatialAudio::RemoveReverbZone
	virtual AKRESULT RemoveReverbZone(
		AkRoomID in_ReverbZone	///< ID of the Room which has been specified as a Reverb Zone.
		) = 0;

	/// Set the room that the game object is currently located in - usually the result of a containment test performed by the client. The room must have been registered with \c SetRoom.
	///	Setting the room for a game object provides the basis for the sound propagation service, and also sets which room's reverb aux bus to send to.  The sound propagation service traces the path
	/// of the sound from the emitter to the listener, and calculates the diffraction as the sound passes through each portal.  The portals are used to define the spatial location of the diffracted and reverberated
	/// audio.
	/// \sa 
	///	- \ref AK::SpatialAudio::SetRoom
	///	- \ref AK::SpatialAudio::RemoveRoom
	virtual AKRESULT SetGameObjectInRoom(
		AkGameObjectID in_gameObjectID, ///< Game object ID 
		AkRoomID in_CurrentRoomID		///< RoomID that was passed to \c AK::SpatialAudio::SetRoom
		) = 0;

	/// Unset the room that the game object is currently located in.
	///	When a game object has not been explicitly assigned to a room with \ref AK::SpatialAudio::SetGameObjectInRoom, the room is automatically computed.
	/// \sa 
	///	- \ref AK::SpatialAudio::SetRoom
	///	- \ref AK::SpatialAudio::RemoveRoom
	virtual AKRESULT UnsetGameObjectInRoom(
		AkGameObjectID in_gameObjectID ///< Game object ID
		) = 0;

	/// Set the early reflections order for reflection calculation. The reflections order indicates the number of times sound can bounce off of a surface. 
	/// A higher number requires more CPU resources but results in denser early reflections. Set to 0 to globally disable reflections processing.
	virtual AKRESULT SetReflectionsOrder(
		AkUInt32 in_uReflectionsOrder,	///< Number of reflections to calculate. Valid range [0,4]
		bool in_bUpdatePaths			///< Set to true to clear existing higher-order paths and to force the re-computation of new paths. If false, existing paths will remain and new paths will be computed when the emitter or listener moves.
		) = 0;

	/// Set the diffraction order for geometric path calculation. The diffraction order indicates the number of edges a sound can diffract around. 
	/// A higher number requires more CPU resources but results in paths found around more complex geometry. Set to 0 to globally disable geometric diffraction processing.
	/// \sa
	/// - \ref AkSpatialAudioInitSettings::uMaxDiffractionOrder
	virtual AKRESULT SetDiffractionOrder(
		AkUInt32 in_uDiffractionOrder,	///< Number of diffraction edges to consider in path calculations. Valid range [0,8]
		bool in_bUpdatePaths			///< Set to true to clear existing diffraction paths and to force the re-computation of new paths. If false, existing paths will remain and new paths will be computed when the emitter or listener moves.
		) = 0;

	/// Set the maximum number of game-defined auxiliary sends that can originate from a single emitter. 
	/// Set to 1 to only allow emitters to send directly to their current room. Set to 0 to disable the limit.
	/// \sa
	/// - \ref AkSpatialAudioInitSettings::uMaxEmitterRoomAuxSends
	virtual AKRESULT SetMaxEmitterRoomAuxSends(
		AkUInt32 in_uMaxEmitterRoomAuxSends		///< The maximum number of room aux send connections.
	) = 0;

	/// Set the number of rays cast from the listener by the stochastic ray casting engine.
	/// A higher number requires more CPU resources but provides more accurate results. Default value (100) should be good for most applications.
	///
	virtual AKRESULT SetNumberOfPrimaryRays(
		AkUInt32 in_uNbPrimaryRays		///< Number of rays cast from the listener
		) = 0;

	/// Set the number of frames on which the path validation phase will be spread. Value between [1..[
	/// High values delay the validation of paths. A value of 1 indicates no spread at all.
	///
	virtual AKRESULT SetLoadBalancingSpread(
		AkUInt32 in_uNbFrames		///< Number of spread frames
		) = 0;

	/// Set an early reflections auxiliary bus for a particular game object. 
	/// Geometrical reflection calculation inside spatial audio is enabled for a game object if any sound playing on the game object has a valid early reflections aux bus specified in the authoring tool,
	/// or if an aux bus is specified via \c SetEarlyReflectionsAuxSend.
	/// The \c in_auxBusID parameter of SetEarlyReflectionsAuxSend applies to sounds playing on the game object \c in_gameObjectID which have not specified an early reflection bus in the authoring tool -
	/// the parameter specified on individual sounds' reflection bus takes priority over the value passed in to \c SetEarlyReflectionsAuxSend.
	/// \aknote 
	/// Users may apply this function to avoid duplicating sounds in the actor-mixer hierarchy solely for the sake of specifying a unique early reflection bus, or in any situation where the same 
	/// sound should be played on different game objects with different early reflection aux buses (the early reflection bus must be left blank in the authoring tool if the user intends to specify it through the API). \endaknote
	virtual AKRESULT SetEarlyReflectionsAuxSend(
		AkGameObjectID in_gameObjectID, ///< Game object ID 
		AkAuxBusID in_auxBusID			///< Auxiliary bus ID. Applies only to sounds which have not specified an early reflection bus in the authoring tool. Pass \c AK_INVALID_AUX_ID to set only the send volume.
		) = 0;

	/// Set an early reflections send volume for a particular game object. 
	/// The \c in_fSendVolume parameter is used to control the volume of the early reflections send. It is combined with the early reflections volume specified in the authoring tool, and is applied to all sounds 
	/// playing on the game object.
	/// Setting \c in_fSendVolume to 0.f will disable all reflection processing for this game object.
	virtual AKRESULT SetEarlyReflectionsVolume(
		AkGameObjectID in_gameObjectID, ///< Game object ID 
		AkReal32 in_fSendVolume			///< Send volume (linear) for auxiliary send. Set 0.f to disable reflection processing. Valid range 0.f-1.f. 
		) = 0;

	/// Set the obstruction and occlusion value for a portal that has been registered with Spatial Audio.
	/// Portal obstruction simulates objects that block the direct sound path between the portal and the listener, but
	/// allows indirect sound to pass around the obstacle. For example, use portal obstruction 
	/// when a piece of furniture is blocking the line of sight of the portal opening.
	/// Portal obstruction is applied on the connection between the emitter and the listener, and only affects the dry signal path.
	/// Portal occlusion simulates a complete blockage of both the direct and indirect sound through a portal. For example, use portal occlusion for 
	/// opening or closing a door or window.
	/// Portal occlusion is applied on the connection between the emitter and the first room in the chain, as well as the connection between the emitter and listener.
	/// Portal occlusion affects both the dry and wet (reverberant) signal paths.
	/// To apply detailed obstruction to specific sound paths but not others, use \c AK::SpatialAudio::SetGameObjectToPortalObstruction and \c AK::SpatialAudio::SetPortalToPortalObstruction.
	/// To apply occlusion and obstruction to the direct line of sight between the emitter and listener use \c AK::SoundEngine::SetObjectObstructionAndOcclusion.
	/// \sa
	/// - \ref AK::SpatialAudio::SetGameObjectToPortalObstruction
	/// - \ref AK::SpatialAudio::SetPortalToPortalObstruction
	/// - \ref AK::SoundEngine::SetObjectObstructionAndOcclusion
	virtual AKRESULT SetPortalObstructionAndOcclusion(
		AkPortalID in_PortalID,				///< Portal ID.
		AkReal32 in_fObstruction,			///< Obstruction value.  Valid range 0.f-1.f
		AkReal32 in_fOcclusion				///< Occlusion value.  Valid range 0.f-1.f
		) = 0;

	/// Set the obstruction value of the path between a game object and a portal that has been created by Spatial Audio.
	/// The obstruction value is applied on one of the path segments of the sound between the emitter and the listener.
	/// Diffraction must be enabled on the sound for a diffraction path to be created.
	/// Also, there should not be any portals between the provided game object and portal ID parameters.
	/// The obstruction value is used to simulate objects between the portal and the game object that are obstructing the sound.
	/// Send an obstruction value of 0 to ensure the value is removed from the internal data structure.
	/// \sa
	/// - \ref AK::SpatialAudio::SetPortalObstructionAndOcclusion
	virtual AKRESULT SetGameObjectToPortalObstruction(
		AkGameObjectID in_gameObjectID,		///< Game object ID
		AkPortalID in_PortalID,				///< Portal ID
		AkReal32 in_fObstruction			///< Obstruction value.  Valid range 0.f-1.f
		) = 0;

	/// Set the obstruction value of the path between two portals that has been created by Spatial Audio.
	/// The obstruction value is applied on one of the path segments of the sound between the emitter and the listener.
	/// Diffraction must be enabled on the sound for a diffraction path to be created.
	/// Also, there should not be any portals between the two provided ID parameters.
	/// The obstruction value is used to simulate objects between the portals that are obstructing the sound.
	/// Send an obstruction value of 0 to ensure the value is removed from the internal data structure.
	/// \sa
	/// - \ref AK::SpatialAudio::SetPortalObstructionAndOcclusion
	virtual AKRESULT SetPortalToPortalObstruction(
		AkPortalID in_PortalID0,			///< Portal ID
		AkPortalID in_PortalID1,			///< Portal ID
		AkReal32 in_fObstruction			///< Obstruction value.  Valid range 0.f-1.f
		) = 0;

	/// Query information about the wet diffraction amount for the portal \c in_portal, returned as a normalized value \c out_wetDiffraction in the range [0,1].  
	/// The wet diffraction is calculated from how far into the 'shadow region' the listener is from the portal.  Unlike dry diffraction, the 
	/// wet diffraction does not depend on the incident angle, but only the normal of the portal.
	/// This value is applied by spatial audio, to the Diffraction value and built-in game parameter of the room game object that is
	/// on the other side of the portal (relative to the listener).
	/// This function must acquire the global sound engine lock and therefore, may block waiting for the lock.
	/// \sa
	/// - \ref AkSpatialAudioInitSettings
	virtual AKRESULT QueryWetDiffraction(
		AkPortalID in_portal,			///< The ID of the game object that the client wishes to query.
		AkReal32& out_wetDiffraction	///< The number of slots in \c out_aPaths, after returning the number of valid elements written.
		) = 0;

	/// Query information about the diffraction state for a particular listener and emitter, which has been calculated using the data provided via the spatial audio emitter API. This function can be used for debugging purposes.
	/// Returned in \c out_aPaths, this array contains the sound paths calculated from diffraction around a geometric edge and/or diffraction through portals connecting rooms.
	/// No paths will be returned in any of the following conditions: (1) the emitter game object has a direct line of sight to the listener game object, (2) the emitter and listener are in the same room, and the listener is completely outside the radius of the emitter, 
	/// or (3) The emitter and listener are in different rooms, but there are no paths found via portals between the emitter and the listener.
	/// A single path with zero diffraction nodes is returned when all of the following conditions are met: (1) the emitter and listener are in the same room, (2) there is no direct line of sight, and (3) either the Voice's Attenuation's curve max distance is exceeded or the accumulated diffraction coefficient exceeds 1.0.
	/// This function must acquire the global sound engine lock and, therefore, may block waiting for the lock.
	/// \sa
	/// - \ref AkDiffractionPathInfo
	virtual AKRESULT QueryDiffractionPaths(
		AkGameObjectID in_gameObjectID,		///< The ID of the game object that the client wishes to query.
		AkUInt32 in_positionIndex,			///< The index of the associated game object position.
		AkVector64& out_listenerPos,		///< Returns the position of the listener game object that is associated with the game object \c in_gameObjectID.
		AkVector64& out_emitterPos,			///< Returns the position of the emitter game object \c in_gameObjectID.
		AkDiffractionPathInfo* out_aPaths,	///< Pointer to an array of \c AkDiffractionPathInfo's which will be filled on return.
		AkUInt32& io_uArraySize				///< The number of slots in \c out_aPaths, after returning the number of valid elements written.
		) = 0;

	/// Reset the stochastic engine state by re-initializing the random seeds.
	///
	virtual AKRESULT ResetStochasticEngine() = 0;

	//@}

	class IReverbEstimation
	{
	public:
		UE_NONCOPYABLE(IReverbEstimation);
	protected:
		IReverbEstimation() = default;
	public:
		virtual ~IReverbEstimation() {}

		////////////////////////////////////////////////////////////////////////
		/// @name Reverb estimation. 
		/// These functions can be used to estimate the reverb parameters of a physical environment, using its volume and surface area
		//@{

		/// This is used to estimate the line of best fit through the absorption values of an Acoustic Texture.
		/// This value is what's known as the HFDamping.
		/// return Gradient of line of best fit through y = mx + c.
		virtual float CalculateSlope(const AkAcousticTexture& texture) = 0;

		/// Calculate average absorption values using each of the textures in in_textures, weighted by their corresponding surface area.
		virtual void GetAverageAbsorptionValues(AkAcousticTexture* in_textures, float* in_surfaceAreas, int in_numTextures, AkAcousticTexture& out_average) = 0;

		/// Estimate the time taken (in seconds) for the sound reverberation in a physical environment to decay by 60 dB.
		/// This is estimated using the Sabine equation. The T60 decay time can be used to drive the decay parameter of a reverb effect.
		virtual AKRESULT EstimateT60Decay(
			AkReal32 in_volumeCubicMeters,				///< The volume (in cubic meters) of the physical environment. 0 volume or negative volume will give a decay estimate of 0.
			AkReal32 in_surfaceAreaSquaredMeters,		///< The surface area (in squared meters) of the physical environment. Must be >= AK_SA_MIN_ENVIRONMENT_SURFACE_AREA
			AkReal32 in_environmentAverageAbsorption,	///< The average absorption amount of the surfaces in the environment. Must be between AK_SA_MIN_ENVIRONMENT_ABSORPTION and 1.
			AkReal32& out_decayEstimate					///< Returns the time taken (in seconds) for the reverberation to decay bu 60 dB.
			) = 0;

		/// Estimate the time taken (in milliseconds) for the first reflection to reach the listener.
		/// This assumes the emitter and listener are both positioned in the centre of the environment.
		virtual AKRESULT EstimateTimeToFirstReflection(
			AkVector in_environmentExtentMeters,	///< Defines the dimensions of the environment (in meters) relative to the center; all components must be positive numbers.
			AkReal32& out_timeToFirstReflectionMs,	///< Returns the time taken (in milliseconds) for the first reflection to reach the listener.
			AkReal32 in_speedOfSound = 343.0f		///< Defaults to 343.0 - the speed of sound in dry air. Must be > 0.
			) = 0;

		/// Estimate the high frequency damping from a collection of AkAcousticTextures and corresponding surface areas. 
		/// The high frequency damping is a measure of how much high frequencies are dampened compared to low frequencies. 
		/// The value is comprised between -1 and 1. A value > 0 indicates more high frequency damping than low frequency damping. < 0 indicates more low frequency damping than high frequency damping. 0 indicates uniform damping.
		/// The average absorption values are calculated using each of the textures in the collection, weighted by their corresponding surface area.
		/// The HFDamping is then calculated as the line-of-best-fit through the average absorption values.
		virtual AkReal32 EstimateHFDamping(
			AkAcousticTexture* in_textures,	///< A collection of AkAcousticTexture structs from which to calculate the average high frequency damping.
			float* in_surfaceAreas,			///< Surface area values for each of the textures in in_textures.
			int in_numTextures				///< The number of textures in in_textures (and the number of surface area values in in_surfaceAreas).
			) = 0;

		//@}
	}* const ReverbEstimation;
};
