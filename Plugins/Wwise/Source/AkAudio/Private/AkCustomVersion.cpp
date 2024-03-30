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

#include "AkCustomVersion.h"
#include "AkAudioDevice.h"
#include "Serialization/CustomVersion.h"

const FGuid FAkCustomVersion::GUID(0xE2717C7E, 0x52F544D3, 0x950C5340, 0xB315035E);

// Register the custom version with core
FCustomVersionRegistration GRegisterAkCustomVersion(FAkCustomVersion::GUID, FAkCustomVersion::LatestVersion, TEXT("AkAudioVersion"));
