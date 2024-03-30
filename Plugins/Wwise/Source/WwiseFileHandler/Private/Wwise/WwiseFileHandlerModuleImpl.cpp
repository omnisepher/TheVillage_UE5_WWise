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

#include "Wwise/WwiseFileHandlerModuleImpl.h"
#include "Wwise/WwiseSoundBankManagerImpl.h"
#include "Wwise/WwiseExternalSourceManagerImpl.h"
#include "Wwise/WwiseFileCache.h"
#include "Wwise/WwiseMediaManagerImpl.h"
#include "Wwise/WwiseIOHookImpl.h"
#include "Wwise/Stats/FileHandler.h"

IMPLEMENT_MODULE(FWwiseFileHandlerModule, WwiseFileHandler)

FWwiseFileHandlerModule::FWwiseFileHandlerModule()
{
}

IWwiseSoundBankManager* FWwiseFileHandlerModule::GetSoundBankManager()
{
	Lock.ReadLock();
	if (LIKELY(SoundBankManager))
	{
		Lock.ReadUnlock();
	}
	else
	{
		Lock.ReadUnlock();
		Lock.WriteLock();
		if (LIKELY(!SoundBankManager))
		{
			UE_LOG(LogWwiseFileHandler, Display, TEXT("Initializing default SoundBank Manager."));
			SoundBankManager.Reset(InstantiateSoundBankManager());
		}
		Lock.WriteUnlock();
	}
	return SoundBankManager.Get();
}

IWwiseExternalSourceManager* FWwiseFileHandlerModule::GetExternalSourceManager()
{
	Lock.ReadLock();
	if (LIKELY(ExternalSourceManager))
	{
		Lock.ReadUnlock();
	}
	else
	{
		Lock.ReadUnlock();
		Lock.WriteLock();
		if (LIKELY(!ExternalSourceManager))
		{
			UE_LOG(LogWwiseFileHandler, Display, TEXT("Initializing default External Source Manager."));
			ExternalSourceManager.Reset(InstantiateExternalSourceManager());
		}
		Lock.WriteUnlock();
	}
	return ExternalSourceManager.Get();
}

IWwiseMediaManager* FWwiseFileHandlerModule::GetMediaManager()
{
	Lock.ReadLock();
	if (LIKELY(MediaManager))
	{
		Lock.ReadUnlock();
	}
	else
	{
		Lock.ReadUnlock();
		Lock.WriteLock();
		if (LIKELY(!MediaManager))
		{
			UE_LOG(LogWwiseFileHandler, Display, TEXT("Initializing default Media Manager."));
			MediaManager.Reset(InstantiateMediaManager());
		}
		Lock.WriteUnlock();
	}
	return MediaManager.Get();
}

FWwiseFileCache* FWwiseFileHandlerModule::GetFileCache()
{
	Lock.ReadLock();
	if (LIKELY(FileCache))
	{
		Lock.ReadUnlock();
	}
	else
	{
		Lock.ReadUnlock();
		Lock.WriteLock();
		if (LIKELY(!FileCache))
		{
			UE_LOG(LogWwiseFileHandler, Display, TEXT("Initializing default File Cache."));
			FileCache.Reset(InstantiateFileCache());
		}
		Lock.WriteUnlock();
	}
	return FileCache.Get();
}

FWwiseIOHook* FWwiseFileHandlerModule::InstantiateIOHook()
{
	return new FWwiseIOHookImpl;
}

IWwiseSoundBankManager* FWwiseFileHandlerModule::InstantiateSoundBankManager()
{
	return new FWwiseSoundBankManagerImpl;
}

IWwiseExternalSourceManager* FWwiseFileHandlerModule::InstantiateExternalSourceManager()
{
	return new FWwiseExternalSourceManagerImpl;
}

IWwiseMediaManager* FWwiseFileHandlerModule::InstantiateMediaManager()
{
	return new FWwiseMediaManagerImpl;
}

FWwiseFileCache* FWwiseFileHandlerModule::InstantiateFileCache()
{
	return new FWwiseFileCache;
}

void FWwiseFileHandlerModule::StartupModule()
{
	IWwiseFileHandlerModule::StartupModule();
}

void FWwiseFileHandlerModule::ShutdownModule()
{
	Lock.WriteLock();
	if (SoundBankManager.IsValid())
	{
		UE_LOG(LogWwiseFileHandler, Display, TEXT("Shutting down SoundBank Manager."));
		SoundBankManager.Reset();
	}
	if (ExternalSourceManager.IsValid())
	{
		UE_LOG(LogWwiseFileHandler, Display, TEXT("Shutting down External Source Manager."));
		ExternalSourceManager.Reset();
	}
	if (MediaManager.IsValid())
	{
		UE_LOG(LogWwiseFileHandler, Display, TEXT("Shutting down Media Manager."));
		MediaManager.Reset();
	}
	if (FileCache.IsValid())
	{
		UE_LOG(LogWwiseFileHandler, Display, TEXT("Shutting down File Cache."));
		FileCache.Reset();
	}
	Lock.WriteUnlock();
	IWwiseFileHandlerModule::ShutdownModule();
}
