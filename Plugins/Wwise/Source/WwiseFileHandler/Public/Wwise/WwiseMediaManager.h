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

#include "AkInclude.h"
#include "Wwise/WwiseFileHandlerModule.h"
#include "Wwise/WwiseStreamableFileHandler.h"

struct FWwiseMediaCookedData;

class IWwiseMediaManager : public IWwiseStreamableFileHandler
{
public:
	inline static IWwiseMediaManager* Get()
	{
		if (auto* Module = IWwiseFileHandlerModule::GetModule())
		{
			return Module->GetMediaManager();
		}
		return nullptr;
	}

	using FLoadMediaCallback = TUniqueFunction<void(bool bSuccess)>;
	using FUnloadMediaCallback = TUniqueFunction<void()>;

	virtual void LoadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FLoadMediaCallback&& InCallback) = 0;
	virtual void UnloadMedia(const FWwiseMediaCookedData& InMediaCookedData, const FString& InRootPath, FUnloadMediaCallback&& InCallback) = 0;
	virtual void SetGranularity(AkUInt32 Uint32) = 0;
};
