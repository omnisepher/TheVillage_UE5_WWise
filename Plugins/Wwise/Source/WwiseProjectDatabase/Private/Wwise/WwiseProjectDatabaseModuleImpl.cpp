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

#include "Wwise/WwiseProjectDatabaseModuleImpl.h"

#include "Wwise/WwiseProjectDatabaseDelegates.h"
#include "Wwise/WwiseProjectDatabaseImpl.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "WwiseUnrealHelper.h"

IMPLEMENT_MODULE(FWwiseProjectDatabaseModule, WwiseProjectDatabase)

FWwiseProjectDatabase* FWwiseProjectDatabaseModule::GetProjectDatabase()
{
	SCOPED_WWISEPROJECTDATABASE_EVENT(TEXT("GetProjectDatabase"));
	Lock.ReadLock();
	if (LIKELY(ProjectDatabase))
	{
		Lock.ReadUnlock();
	}
	else
	{
		Lock.ReadUnlock();
		Lock.WriteLock();
		if (LIKELY(!ProjectDatabase))
		{
			UE_LOG(LogWwiseProjectDatabase, Display, TEXT("Initializing default Project Database."));
			ProjectDatabase.Reset(InstantiateProjectDatabase());
		}
		Lock.WriteUnlock();
	}
	return ProjectDatabase.Get();
}

FWwiseProjectDatabase* FWwiseProjectDatabaseModule::InstantiateProjectDatabase()
{
	SCOPED_WWISEPROJECTDATABASE_EVENT(TEXT("InstantiateProjectDatabase"));
	return new FWwiseProjectDatabaseImpl;
}

bool FWwiseProjectDatabaseModule::CanHaveDefaultInstance()
{
	if (IsRunningCommandlet())
	{
		if (WwiseUnrealHelper::GetSoundBankDirectory().IsEmpty())
		{
			return false;
		}
	}
	return true;
}

FWwiseProjectDatabaseDelegates* FWwiseProjectDatabaseModule::GetProjectDatabaseDelegates()
{
	if (!ProjectDatabaseDelegates)
	{
		ProjectDatabaseDelegates = InstantiateProjectDatabaseDelegates();
	}

	return ProjectDatabaseDelegates;
}

FWwiseProjectDatabaseDelegates* FWwiseProjectDatabaseModule::InstantiateProjectDatabaseDelegates()
{
	SCOPED_WWISEPROJECTDATABASE_EVENT(TEXT("InstantiateProjectDatabaseDelegates"));
	return new FWwiseProjectDatabaseDelegates;
}

void FWwiseProjectDatabaseModule::ShutdownModule()
{
	SCOPED_WWISEPROJECTDATABASE_EVENT(TEXT("ShutdownModule"));
	Lock.WriteLock();
	if (ProjectDatabase.IsValid())
	{
		UE_LOG(LogWwiseProjectDatabase, Display, TEXT("Shutting down default Project Database."));
		ProjectDatabase.Reset();
	}
	Lock.WriteUnlock();

	if (ProjectDatabaseDelegates)
	{
		delete ProjectDatabaseDelegates;
	}

	IWwiseProjectDatabaseModule::ShutdownModule();
}
