/***********************************************************************
  The content of this file includes source code for the sound engine
  portion of the AUDIOKINETIC Wwise Technology and constitutes "Level
  Two Source Code" as defined in the Source Code Addendum attached
  with this file.  Any use of the Level Two Source Code shall be
  subject to the terms and conditions outlined in the Source Code
  Addendum and the End User License Agreement for Wwise(R).

  Copyright (c) 2024 Audiokinetic Inc.
 ***********************************************************************/

#pragma once

 /// Defines the parameters of a marker.
struct AkAudioMarker
{
	AkUInt32        dwIdentifier;       ///< Identifier.
	AkUInt32        dwPosition;         ///< Position in the audio data in sample frames.
	const char*     strLabel;           ///< Label of the marker taken from the file.
	AkUInt32        dwLabelSize;        ///< Length of label read the from the file + terminating null character.
};
