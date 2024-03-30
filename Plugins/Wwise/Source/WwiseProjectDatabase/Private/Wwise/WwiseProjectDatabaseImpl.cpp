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

#include "Wwise/WwiseProjectDatabaseImpl.h"

#include "WwiseUnrealHelper.h"
#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/WwiseResourceLoader.h"
#include "Wwise/WwiseProjectDatabaseDelegates.h"

#include "Async/Async.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "WwiseProjectDatabase"

FWwiseProjectDatabaseImpl::FWwiseProjectDatabaseImpl() :
	ResourceLoaderOverride(nullptr),
	LockedDataStructure(new FWwiseDataStructure())
{
}

FWwiseProjectDatabaseImpl::~FWwiseProjectDatabaseImpl()
{
}

void FWwiseProjectDatabaseImpl::UpdateDataStructure(const FDirectoryPath* InUpdateGeneratedSoundBanksPath, const FGuid* InBasePlatformGuid)
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_2(TEXT("FWwiseProjectDatabaseImpl::UpdateDataStructure"));
	FWwiseSharedPlatformId Platform;
	FDirectoryPath SourcePath;
	{
		auto* ResourceLoader = GetResourceLoader();
		if (UNLIKELY(!ResourceLoader))
		{
			return;
		}
		if (InUpdateGeneratedSoundBanksPath)
		{
			ResourceLoader->SetUnrealGeneratedSoundBanksPath(*InUpdateGeneratedSoundBanksPath);
		}

		Platform = ResourceLoader->GetCurrentPlatform();
		SourcePath = ResourceLoader->GetUnrealGeneratedSoundBanksPath();
	}

	{
		FWriteScopeLock WLock(LockedDataStructure->Lock);
		auto& DataStructure = LockedDataStructure.Get();

		if (DisableDefaultPlatforms())
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("UpdateDataStructure: Retrieving root data structure in (%s)"), *SourcePath.Path);
			FScopedSlowTask SlowTask(0, LOCTEXT("WwiseProjectDatabaseUpdate", "Retrieving Wwise data structure root..."));

			{
				FWwiseDataStructure UpdatedDataStructure(SourcePath, nullptr, nullptr);
				DataStructure = MoveTemp(UpdatedDataStructure);
			}
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("UpdateDataStructure: Retrieving data structure for %s (Base: %s) in (%s)"),
				*Platform.GetPlatformName().ToString(), InBasePlatformGuid ? *InBasePlatformGuid->ToString() : TEXT("null"), *SourcePath.Path);
			FScopedSlowTask SlowTask(0, FText::Format(
				LOCTEXT("WwiseProjectDatabaseUpdate", "Retrieving Wwise data structure for platform {0}..."),
				FText::FromName(Platform.GetPlatformName())));

			{
				FWwiseDataStructure UpdatedDataStructure(SourcePath, &Platform.GetPlatformName(), InBasePlatformGuid);
				DataStructure = MoveTemp(UpdatedDataStructure);

				// Update platform according to data found if different
				FWwiseSharedPlatformId FoundSimilarPlatform = Platform;
				for (const auto& LoadedPlatform : DataStructure.Platforms)
				{
					FoundSimilarPlatform = LoadedPlatform.Key;
					if (FoundSimilarPlatform == Platform)
					{
						break;
					}
				}

				//Update SharedPlatformId with parsed root paths
				if (DataStructure.Platforms.Contains(FoundSimilarPlatform) )
				{
					const FWwisePlatformDataStructure& PlatformEntry = DataStructure.Platforms.FindRef(FoundSimilarPlatform);
					FoundSimilarPlatform.Platform->ExternalSourceRootPath = PlatformEntry.PlatformRef.GetPlatformInfo()->RootPaths.ExternalSourcesOutputRoot;
				}
				//Update the resource loader current platform as internal data may have changed
				auto* ResourceLoader = GetResourceLoader();
				if (UNLIKELY(!ResourceLoader))
				{
					return;
				}

				ResourceLoader->SetPlatform(FoundSimilarPlatform);
			}

			if (UNLIKELY(DataStructure.Platforms.Num() == 0))
			{
				if(!InBasePlatformGuid)
				{
					UE_LOG(LogWwiseProjectDatabase, Error, TEXT("JSON metadata files are not generated. Make sure the SoundBanks are generated and that the \"Generate JSON Metadata\" setting is enabled in your Wwise Project Settings, under the SoundBanks tab."));
					return;
				}
				UE_LOG(LogWwiseProjectDatabase, Error, TEXT("UpdateDataStructure: Could not find suitable platform for %s (Base: %s) in (%s)"),
					*Platform.GetPlatformName().ToString(), InBasePlatformGuid ? *InBasePlatformGuid->ToString() : TEXT("null"), *SourcePath.Path);
				return;
			}
		}
		bIsDatabaseParsed = true;
		UE_LOG(LogWwiseProjectDatabase, Log, TEXT("UpdateDataStructure: Done."));
	}
	if (Get() == this && bShouldBroadcast)		// Only broadcast database updates on main project.
	{
		//Stop multiple threads from Broadcasting this delegate at the same time.
		bShouldBroadcast = false;
		FWwiseProjectDatabaseDelegates::Get()->GetOnDatabaseUpdateCompletedDelegate().Broadcast();
		bShouldBroadcast = true;
	}
}

void FWwiseProjectDatabaseImpl::PrepareProjectDatabaseForPlatform(FWwiseResourceLoader*&& InResourceLoader)
{
	ResourceLoaderOverride.Reset(InResourceLoader);
}

FWwiseResourceLoader* FWwiseProjectDatabaseImpl::GetResourceLoader()
{
	if (ResourceLoaderOverride.IsValid())
	{
		return ResourceLoaderOverride.Get();
	}
	else
	{
		return FWwiseResourceLoader::Get();
	}
}

const FWwiseResourceLoader* FWwiseProjectDatabaseImpl::GetResourceLoader() const
{
	if (ResourceLoaderOverride.IsValid())
	{
		return ResourceLoaderOverride.Get();
	}
	else
	{
		return FWwiseResourceLoader::Get();
	}
}

#undef LOCTEXT_NAMESPACE
