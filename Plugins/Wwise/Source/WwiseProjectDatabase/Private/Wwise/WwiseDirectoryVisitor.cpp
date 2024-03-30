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

#include "Wwise/WwiseDirectoryVisitor.h"

#include "WwiseUnrealHelper.h"
#include "Wwise/Metadata/WwiseMetadataRootFile.h"
#include "Wwise/Metadata/WwiseMetadataProjectInfo.h"
#include "Wwise/Metadata/WwiseMetadataPlatform.h"
#include "Wwise/Metadata/WwiseMetadataLanguage.h"
#include "Wwise/Stats/ProjectDatabase.h"

#include "Async/Async.h"
#include "Misc/Paths.h"

//
// FPlatformRootDirectoryVisitor
//
class FWwiseDirectoryVisitor::FPlatformRootDirectoryVisitor : public IPlatformFile::FDirectoryVisitor, public FWwiseDirectoryVisitor::IGettableVisitor
{
public:
	FPlatformRootDirectoryVisitor(
		const FWwiseSharedPlatformId& InPlatform,
		IPlatformFile& InFileInterface) :
		Platform(InPlatform),
		FileInterface(InFileInterface)
	{}
	bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override;
	bool StartJobIfValid();
	FWwiseGeneratedFiles::FPlatformFiles& Get() override;

	const FWwiseSharedPlatformId Platform;
	IPlatformFile& FileInterface;

	FWwiseGeneratedFiles::FPlatformFiles PlatformFiles;
	TArray<TFuture<FWwiseDirectoryVisitor::IGettableVisitor*>> Futures;
};

//
// FSoundBankVisitor
//
class FWwiseDirectoryVisitor::FSoundBankVisitor : public IPlatformFile::FDirectoryVisitor, public FWwiseDirectoryVisitor::IGettableVisitor
{
public:
	FSoundBankVisitor(IPlatformFile& InFileInterface) :
		FileInterface(InFileInterface)
	{}
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory);
	FWwiseGeneratedFiles::FPlatformFiles& Get() override;

	IPlatformFile& FileInterface;
	FWwiseGeneratedFiles::FPlatformFiles Result;
};

//
// FMediaVisitor
//
class FWwiseDirectoryVisitor::FMediaVisitor : public IPlatformFile::FDirectoryVisitor, public FWwiseDirectoryVisitor::IGettableVisitor
{
public:
	FMediaVisitor(IPlatformFile& InFileInterface) :
		FileInterface(InFileInterface)
	{}
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory);
	FWwiseGeneratedFiles::FPlatformFiles& Get() override;

	IPlatformFile& FileInterface;
	FWwiseGeneratedFiles::FPlatformFiles Result;
};

//
// FWwiseDirectoryVisitor
//
bool FWwiseDirectoryVisitor::Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_2(TEXT("FWwiseDirectoryVisitor::Visit"));
	// make sure all paths are "standardized" so the other end can match up with it's own standardized paths
	FString RelativeFilename = FilenameOrDirectory;
	FPaths::MakeStandardFilename(RelativeFilename);
	const auto Filename = FPaths::GetCleanFilename(RelativeFilename);

	if (Filename.StartsWith(TEXT(".")))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("[WwiseDirectoryVisitor] Skipping: %s"), *RelativeFilename);
		return true;
	}

	UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("[WwiseDirectoryVisitor] Visiting %s"), *RelativeFilename);

	if (bIsDirectory)
	{
		// Skip directories, they are to be processed by ProjectInfo.json's Path
		return true;
	}

	FWwiseGeneratedFiles::FileTuple FileToAdd(RelativeFilename, FileInterface.GetTimeStamp(FilenameOrDirectory));
	const auto Extension = FPaths::GetExtension(RelativeFilename);

	if (Filename.Equals(TEXT("ProjectInfo.json"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found ProjectInfo: %s"), *RelativeFilename);

		// We need to retrieve the Path from ProjectInfo. Parse this file immediately
		// (will be parsed twice. Now once, and officially later - since the file is small, it's not a big worry)
		auto Root = FWwiseMetadataRootFile::LoadFile(RelativeFilename);
		if (!Root || !Root->ProjectInfo)
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not read ProjectInfo to retrieve paths."), *RelativeFilename);
			return true;
		}

		GeneratedDirectory.ProjectInfo = Root;

		const auto Path = FPaths::GetPath(FilenameOrDirectory);

		auto& Platforms = Root->ProjectInfo->Platforms;
		bool bFoundPlatform = false;
		if (!PlatformName)
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("Skipping loading all platforms"));
		}
		else
		{
			for (auto& Platform : Platforms)
			{
				if (PlatformName->ToString().Equals(Platform.Name.ToString(), ESearchCase::IgnoreCase))
				{
					bFoundPlatform = true;
					break;
				}
			}

			UE_CLOG(UNLIKELY(!bFoundPlatform), LogWwiseProjectDatabase, Log, TEXT("Requested platform not found: %s"), *PlatformName->ToString());
		}

		if (bFoundPlatform)
		{
			for (auto& Platform : Platforms)
			{
				const FString PlatformPath = Platform.Path.ToString();

				FString RequestedPlatformPath = ""; 

				if (FPaths::IsRelative(PlatformPath))
				{
					RequestedPlatformPath = Path / Platform.Path.ToString();
				}
				else
				{
					RequestedPlatformPath = PlatformPath;
				}

				if (!PlatformName->ToString().Equals(Platform.Name.ToString(), ESearchCase::IgnoreCase))
				{
					UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Skipping platform %s"), *Platform.Name.ToString());
					continue;
				}
				if (PlatformGuid && *PlatformGuid != Platform.BasePlatformGUID)
				{
					UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Skipping platform %s (Base %s)"), *Platform.Name.ToString(), *Platform.BasePlatform.ToString());
					continue;
				}

				UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Visiting platform %s at: %s"), *Platform.Name.ToString(), *Platform.Path.ToString());

				FWwisePlatformId CurrentPlatform;
				CurrentPlatform.PlatformGuid = Platform.GUID;
				CurrentPlatform.PlatformName = Platform.Name;
				FString RelativePlatformPath(RequestedPlatformPath);
				FPaths::MakePathRelativeTo(RelativePlatformPath, *WwiseUnrealHelper::GetSoundBankDirectory());
				CurrentPlatform.PathRelativeToGeneratedSoundBanks = FName(RelativePlatformPath);
				FWwiseSharedPlatformId PlatformRef;
				PlatformRef.Platform = MakeShared<FWwisePlatformId, ESPMode::ThreadSafe>(CurrentPlatform);

				Futures.Add(Async(EAsyncExecution::TaskGraph, [this, PlatformRef, RequestedPlatformPath] {
					auto* RootVisitor = new FPlatformRootDirectoryVisitor(PlatformRef, FileInterface);
					if (!FileInterface.IterateDirectory(*RequestedPlatformPath, *RootVisitor) ||
						!RootVisitor->StartJobIfValid())
					{
						UE_LOG(LogWwiseProjectDatabase, Warning, TEXT("Could not find generated platform %s at: %s"), *PlatformRef.GetPlatformName().ToString(), *PlatformRef.Platform->PathRelativeToGeneratedSoundBanks.ToString());
						delete RootVisitor;
						RootVisitor = nullptr;
					}
					return RootVisitor;
				}));
			}
		}

		GeneratedDirectory.GeneratedRootFiles.ProjectInfoFile = MoveTemp(FileToAdd);
	}
	else if (Filename.Equals(TEXT("Wwise_IDs.h"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found Wwise IDs: %s"), *RelativeFilename);
		GeneratedDirectory.GeneratedRootFiles.WwiseIDsFile = MoveTemp(FileToAdd);
	}
	else if (Filename.Equals(TEXT("SoundBanksGeneration.log"), ESearchCase::IgnoreCase)
		|| Extension.Equals(TEXT("xml"), ESearchCase::IgnoreCase))
	{
		// Nothing to do
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Log, TEXT("Unknown file. Not in a platform. Will be ignored: %s"), *RelativeFilename);
	}

	return true;
}

FWwiseGeneratedFiles& FWwiseDirectoryVisitor::Get()
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_4(TEXT("FWwiseDirectoryVisitor::Get"));
	for (const auto& Future : Futures)
	{
		auto* Result = Future.Get();
		if (Result)
		{
			auto& PlatformFiles = Result->Get();
			if (PlatformFiles.IsValid())
			{
				GeneratedDirectory.Platforms.Add(Result->Platform, PlatformFiles);
			}
			delete Result;
		}
	}
	Futures.Empty();
	return GeneratedDirectory;
}



//
// FPlatformRootDirectoryVisitor
//
bool FWwiseDirectoryVisitor::FPlatformRootDirectoryVisitor::Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_2(TEXT("FPlatformRootDirectoryVisitor::Visit"));
	// make sure all paths are "standardized" so the other end can match up with it's own standardized paths
	FString RelativeFilename = FilenameOrDirectory;
	FPaths::MakeStandardFilename(RelativeFilename);
	const auto Filename = FPaths::GetCleanFilename(RelativeFilename);

	if (Filename.StartsWith(TEXT(".")))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("[RootFilesVisitor] Skipping: %s"), *RelativeFilename);
		return true;
	}

	UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("[RootFilesVisitor] Visiting %s"), *RelativeFilename);

	if (bIsDirectory)
	{
		PlatformFiles.DirectoriesToWatch.Add(RelativeFilename);

		if (Filename.Equals(TEXT("Media"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found media directory: %s"), *RelativeFilename);
			PlatformFiles.MediaDirectory = FilenameOrDirectory;
		}
		else if (Filename.Equals(TEXT("Bus"), ESearchCase::IgnoreCase)
			|| Filename.Equals(TEXT("Event"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found auto SoundBank directory: %s"), *RelativeFilename);
			PlatformFiles.AutoSoundBankDirectories.Add(FilenameOrDirectory);
		}
		else {
			UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found language directory: %s"), *RelativeFilename);
			PlatformFiles.LanguageDirectories.Add(FilenameOrDirectory);
		}

		return true;
	}

	FWwiseGeneratedFiles::FileTuple FileToAdd(RelativeFilename, FileInterface.GetTimeStamp(FilenameOrDirectory));
	const auto Extension = FPaths::GetExtension(RelativeFilename);

	if (!Extension.Equals(TEXT("json"), ESearchCase::IgnoreCase) 
		&& !Extension.Equals(TEXT("txt"), ESearchCase::IgnoreCase)
		&& !Extension.Equals(TEXT("bnk"), ESearchCase::IgnoreCase)
		&& !Extension.Equals(TEXT("xml"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Adding extra file: %s"), *RelativeFilename);
		PlatformFiles.ExtraFiles.Add(MoveTemp(FileToAdd));
		return true;
	}

	if (Extension.Equals(TEXT("bnk"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Adding SoundBank file: %s"), *RelativeFilename);
		PlatformFiles.SoundBankFiles.Add(MoveTemp(FileToAdd));
		return true;
	}
	else if (Extension.Equals(TEXT("xml"), ESearchCase::IgnoreCase)
		|| Extension.Equals(TEXT("txt"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Skipping file: %s"), *RelativeFilename);
		return true;
	}
	else if (Filename.Equals(TEXT("SoundbanksInfo.json"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found monolithic SoundBank info: %s"), *RelativeFilename);
		PlatformFiles.SoundbanksInfoFile = MoveTemp(FileToAdd);
	}
	else if (Filename.Equals(TEXT("PlatformInfo.json"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found platform info: %s"), *RelativeFilename);
		PlatformFiles.PlatformInfoFile = MoveTemp(FileToAdd);
	}
	else if (Filename.Equals(TEXT("PluginInfo.json"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Found plugin info: %s"), *RelativeFilename);
		PlatformFiles.PluginInfoFile = MoveTemp(FileToAdd);
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Adding metadata file: %s"), *RelativeFilename);
		PlatformFiles.MetadataFiles.Add(MoveTemp(FileToAdd));
	}

	return true;
}

bool FWwiseDirectoryVisitor::FPlatformRootDirectoryVisitor::StartJobIfValid()
{
	if (!PlatformFiles.IsValid())
	{
		return false;
	}

	if (!PlatformFiles.MediaDirectory.IsEmpty())
	{
		const auto& Elem = PlatformFiles.MediaDirectory;
		Futures.Add(Async(EAsyncExecution::TaskGraph, [this, Elem] {
			UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Visiting media directory: %s"), *Elem);
			auto* MediaVisitor = new FMediaVisitor(FileInterface);
			FileInterface.IterateDirectory(*Elem, *MediaVisitor);
			return static_cast<FWwiseDirectoryVisitor::IGettableVisitor*>(MediaVisitor);
		}));
	}

	for (const auto& Elem : PlatformFiles.AutoSoundBankDirectories)
	{
		Futures.Add(Async(EAsyncExecution::TaskGraph, [this, Elem] {
			UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Visiting auto SoundBank directory: %s"), *Elem);
			auto* SoundBankVisitor = new FSoundBankVisitor(FileInterface);
			FileInterface.IterateDirectory(*Elem, *SoundBankVisitor);
			return static_cast<FWwiseDirectoryVisitor::IGettableVisitor*>(SoundBankVisitor);
		}));
	}

	for (const auto& Elem : PlatformFiles.LanguageDirectories)
	{
		Futures.Add(Async(EAsyncExecution::TaskGraph, [this, Elem] {
			UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("Visiting language directory: %s"), *Elem);
			auto* SoundBankVisitor = new FSoundBankVisitor(FileInterface);
			FileInterface.IterateDirectory(*Elem, *SoundBankVisitor);
			return static_cast<FWwiseDirectoryVisitor::IGettableVisitor*>(SoundBankVisitor);
		}));
	}

	return true;
}

FWwiseGeneratedFiles::FPlatformFiles& FWwiseDirectoryVisitor::FPlatformRootDirectoryVisitor::Get()
{
	SCOPED_WWISEPROJECTDATABASE_EVENT_4(TEXT("FPlatformRootDirectoryVisitor::Get"));
	for (const auto& Future : Futures)
	{
		auto* Result = Future.Get();
		if (Result)
		{
			PlatformFiles.Append(MoveTemp(Result->Get()));
			delete Result;
		}
	}
	Futures.Empty();
	return PlatformFiles;
}

//
// FSoundBankVisitor
//
bool FWwiseDirectoryVisitor::FSoundBankVisitor::Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
{
	// make sure all paths are "standardized" so the other end can match up with it's own standardized paths
	FString RelativeFilename = FilenameOrDirectory;
	FPaths::MakeStandardFilename(RelativeFilename);
	const auto Filename = FPaths::GetCleanFilename(RelativeFilename);
	const auto Extension = FPaths::GetExtension(RelativeFilename);

	if (Filename.StartsWith(TEXT(".")))
	{
		UE_LOG(LogWwiseProjectDatabase, Verbose, TEXT("[SoundBankVisitor] Skipping: %s"), *RelativeFilename);
		return true;
	}

	UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("[SoundBankVisitor] Visiting %s"), *RelativeFilename);

	if (bIsDirectory)
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Iterating folder: %s"), *RelativeFilename);
		Result.DirectoriesToWatch.Add(RelativeFilename);
		FileInterface.IterateDirectory(FilenameOrDirectory, *this);
		return true;
	}

	FWwiseGeneratedFiles::FileTuple FileToAdd(RelativeFilename, FileInterface.GetTimeStamp(FilenameOrDirectory));

	if (Extension.Equals(TEXT("json"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Adding metadata file: %s"), *RelativeFilename);
		Result.MetadataFiles.Add(MoveTemp(FileToAdd));
		return true;
	}
	else if (Extension.Equals(TEXT("bnk"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Adding SoundBank file: %s"), *RelativeFilename);
		Result.SoundBankFiles.Add(MoveTemp(FileToAdd));
		return true;
	}
	else if (Extension.Equals(TEXT("xml"), ESearchCase::IgnoreCase)
		|| Extension.Equals(TEXT("txt"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Skipping file: %s"), *RelativeFilename);
		return true;
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Adding extra file: %s"), *RelativeFilename);
		Result.ExtraFiles.Add(MoveTemp(FileToAdd));
		return true;
	}
}

FWwiseGeneratedFiles::FPlatformFiles& FWwiseDirectoryVisitor::FSoundBankVisitor::Get()
{
	return Result;
}

//
// FMediaVisitor
//
bool FWwiseDirectoryVisitor::FMediaVisitor::Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
{
	// make sure all paths are "standardized" so the other end can match up with it's own standardized paths
	FString RelativeFilename = FilenameOrDirectory;
	FPaths::MakeStandardFilename(RelativeFilename);

	UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("[MediaVisitor] Visiting %s"), *RelativeFilename);

	if (bIsDirectory)
	{
		Result.DirectoriesToWatch.Add(RelativeFilename);
		FileInterface.IterateDirectory(FilenameOrDirectory, *this);
	}
	else
	{
		FWwiseGeneratedFiles::FileTuple FileToAdd(RelativeFilename, FileInterface.GetTimeStamp(FilenameOrDirectory));
		const auto Extension = FPaths::GetExtension(RelativeFilename);

		if (Extension.Equals(TEXT("wem"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogWwiseProjectDatabase, VeryVerbose, TEXT("Adding media file: %s"), *RelativeFilename);
			Result.MediaFiles.Add(MoveTemp(FileToAdd));
		}
		else
		{
			UE_LOG(LogWwiseProjectDatabase, Log, TEXT("Adding unexpected extra file: %s"), *RelativeFilename);
			Result.ExtraFiles.Add(MoveTemp(FileToAdd));
		}
	}
	return true;
}

FWwiseGeneratedFiles::FPlatformFiles& FWwiseDirectoryVisitor::FMediaVisitor::Get()
{
	return Result;
}
