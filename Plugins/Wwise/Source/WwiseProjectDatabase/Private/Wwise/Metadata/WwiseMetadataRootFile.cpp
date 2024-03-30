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

#include "Wwise/Metadata/WwiseMetadataRootFile.h"

#include "Async/AsyncWork.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/Metadata/WwiseMetadataPluginInfo.h"
#include "Wwise/Metadata/WwiseMetadataProjectInfo.h"
#include "Wwise/Metadata/WwiseMetadataSoundBanksInfo.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "Wwise/Stats/ProjectDatabase.h"

#include "WwiseDefines.h"

FWwiseMetadataRootFile::FWwiseMetadataRootFile(FWwiseMetadataLoader& Loader) :
	PlatformInfo(Loader.GetObjectPtr<FWwiseMetadataPlatformInfo>(this, TEXT("PlatformInfo"))),
	PluginInfo(Loader.GetObjectPtr<FWwiseMetadataPluginInfo>(this, TEXT("PluginInfo"))),
	ProjectInfo(Loader.GetObjectPtr<FWwiseMetadataProjectInfo>(this, TEXT("ProjectInfo"))),
	SoundBanksInfo(Loader.GetObjectPtr<FWwiseMetadataSoundBanksInfo>(this, TEXT("SoundBanksInfo")))
{
	if (Loader.bResult && !PlatformInfo && !PluginInfo && !ProjectInfo && !SoundBanksInfo)
	{
		Loader.Fail(TEXT("FWwiseMetadataRootFile"));
	}
	IncLoadedSize(sizeof(FWwiseMetadataRootFile));
}

FWwiseMetadataRootFile::~FWwiseMetadataRootFile()
{
	if (PlatformInfo)
	{
		delete PluginInfo;
		PluginInfo = nullptr;
	}
	if (PluginInfo)
	{
		delete PluginInfo;
		PluginInfo = nullptr;
	}
	if (ProjectInfo)
	{
		delete PluginInfo;
		PluginInfo = nullptr;
	}
	if (SoundBanksInfo)
	{
		delete SoundBanksInfo;
		SoundBanksInfo = nullptr;
	}
}

class FWwiseAsyncLoadFileTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FWwiseAsyncLoadFileTask>;

	WwiseMetadataSharedRootFilePtr& Output;
	const FString& FilePath;

public:
	FWwiseAsyncLoadFileTask(
		WwiseMetadataSharedRootFilePtr& OutputParam,
		const FString& FilePathParam) :
		Output(OutputParam),
		FilePath(FilePathParam)
	{
	}

protected:
	void DoWork()
	{
		FString FileContents;
		if (!FFileHelper::LoadFileToString(FileContents, *FilePath))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Error while loading file %s to string"), *FilePath);
			return;
		}

		Output = FWwiseMetadataRootFile::LoadFile(MoveTemp(FileContents), *FilePath);
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FWwiseAsyncLoadFileTask, STATGROUP_WwiseProjectDatabase);
	}
};

WwiseMetadataSharedRootFilePtr FWwiseMetadataRootFile::LoadFile(FString&& File, const FString& FilePath)
{
	UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Parsing file in: %s"), *FilePath);

	auto JsonReader = TJsonReaderFactory<>::Create(MoveTemp(File));
	TSharedPtr<FJsonObject> RootJsonObject;
	if (!FJsonSerializer::Deserialize(JsonReader, RootJsonObject))
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Error while decoding json"));
		return {};
	}

	FWwiseMetadataLoader Loader(RootJsonObject.ToSharedRef());
	auto Result = MakeShared<FWwiseMetadataRootFile>(Loader);

	if (!Loader.bResult)
	{
		Loader.LogParsed(TEXT("LoadFile"), 0, FName(FilePath));
		return {};
	}

	return Result;
}

WwiseMetadataSharedRootFilePtr FWwiseMetadataRootFile::LoadFile(const FString& FilePath)
{
	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *FilePath))
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Error while loading file %s to string"), *FilePath);
		return nullptr;
	}

	return LoadFile(MoveTemp(FileContents), FilePath);
}

WwiseMetadataFileMap FWwiseMetadataRootFile::LoadFiles(const TArray<FString>& FilePaths)
{
	WwiseMetadataFileMap Result;
	for (const auto& FilePath : FilePaths)
	{
		Result.Add(FilePath, {});
	}

	TArray<FAsyncTask<FWwiseAsyncLoadFileTask>> Tasks;
	Tasks.Empty(Result.Num());

	for (auto& Elem : Result)
	{
		Tasks.Emplace(Elem.Value, Elem.Key);
	}

	if (Result.Num() > 1)
	{
		// Create a temporary Thread Pool to fully load the file paths with a large Stack size.
		// We typically use way less than that, but some functions are recursive, and Json parsing can be memory intensive.
		const auto WorkersToSpawn = FMath::Min(FPlatformMisc::NumberOfWorkerThreadsToSpawn(), Result.Num());
		static constexpr int32 StackSize = 2 * 1024 * 1024;
		const auto MetadataLoadingThreadPool = FQueuedThreadPool::Allocate();
		verify(MetadataLoadingThreadPool->Create(WorkersToSpawn, StackSize, TPri_BelowNormal, TEXT("Wwise ProjectDatabase Loading Pool")));

		for (auto& Task : Tasks)
		{
			Task.StartBackgroundTask(MetadataLoadingThreadPool);
		}

		for (auto& Task : Tasks)
		{
			Task.EnsureCompletion();
		}

		delete MetadataLoadingThreadPool;
	}
	else
	{
		// We have only one (or zero) task. Do it synchronously.
		for (auto& Task : Tasks)
		{
			Task.StartSynchronousTask();
		}

		for (auto& Task : Tasks)
		{
			Task.EnsureCompletion();
		}
	}

	return Result;
}
