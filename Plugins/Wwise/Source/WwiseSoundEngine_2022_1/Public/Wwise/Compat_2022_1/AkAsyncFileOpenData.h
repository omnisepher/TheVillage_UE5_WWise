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

#include <AK/SoundEngine/Common/IAkStreamMgr.h>

struct AkAsyncFileOpenData;
using AkFileOpenCallback = void(*)(AkAsyncFileOpenData* in_pOpenInfo, AKRESULT in_eResult);
struct AkAsyncFileOpenData
{
	const AkOSChar*     pszFileName;		///< File name. Only one of pszFileName or fileID should be valid (pszFileName null while fileID is not AK_INVALID_FILE_ID, or vice versa)
	AkFileID			fileID;				///< File ID. Only one of pszFileName or fileID should be valid (pszFileName null while fileID is not AK_INVALID_FILE_ID, or vice versa)
	AkFileSystemFlags*  pFlags;				///< Flags for opening, null when unused
	AkOpenMode			eOpenMode;			///< Open mode.
	AkFileOpenCallback	pCallback;			///< Callback function used to notify the high-level device when Open is done
	void*				pCookie;			///< Reserved. Pass this unchanged to the callback function. The I/O device uses this cookie to retrieve the owner of the transfer.
	AkFileDesc*			pFileDesc;			///< File Descriptor to fill once the Open operation is complete. 
	void*				pCustomData;		///< Convenience pointer for the IO hook implementer. Useful for additional data used in asynchronous implementations, for example.
};
