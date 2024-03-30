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

#include "Wwise/WwiseFileStateTools.h"
#include "Wwise/WwiseFileCache.h"
#include "Wwise/Stats/AsyncStats.h"
#include "Wwise/Stats/FileHandler.h"

#include "WwiseDefines.h"
#include "WwiseUnrealDefines.h"

#include "Misc/Paths.h"
#include "Async/MappedFileHandle.h"
#include "Async/AsyncFileHandle.h"
#include "HAL/FileManager.h"
#if UE_5_0_OR_LATER
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif

#include <inttypes.h>

uint8* FWwiseFileStateTools::AllocateMemory(int64 InMemorySize, bool bInDeviceMemory, int32 InMemoryAlignment,
                                            bool bInEnforceMemoryRequirements,
                                            const FName& InStat, const FName& InStatDevice)
{
	uint8* Result = nullptr;

	if (bInDeviceMemory && bInEnforceMemoryRequirements)
	{
#if AK_SUPPORT_DEVICE_MEMORY
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Allocating %" PRIi64 " (%" PRIi32 ") bytes in Device Memory"), InMemorySize, InMemoryAlignment);
		Result = static_cast<uint8*>(AKPLATFORM::AllocDevice((size_t)InMemorySize, nullptr));
		if (Result)
		{
			ASYNC_INC_MEMORY_STAT_BY_FName(InStatDevice, InMemorySize);
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("Could not allocate %" PRIi64 " (%" PRIi32 ") bytes in Device Memory"), InMemorySize, InMemoryAlignment);
		}
#else
		UE_LOG(LogWwiseFileHandler, Error, TEXT("No Device Memory, but trying to allocate %" PRIi64 " (%" PRIi32 ") bytes"), InMemorySize, InMemoryAlignment);
		return AllocateMemory(InMemorySize, false, InMemoryAlignment, bInEnforceMemoryRequirements, InStat, InStatDevice);
#endif
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Allocating %" PRIi64 " (%" PRIi32 ") bytes in Unreal memory"), InMemorySize, InMemoryAlignment);
		Result = static_cast<uint8*>(FMemory::Malloc(InMemorySize, bInEnforceMemoryRequirements ? InMemoryAlignment : 0));
		if (Result)
		{
			ASYNC_INC_MEMORY_STAT_BY_FName(InStat, InMemorySize);
		}
		UE_CLOG(UNLIKELY(!Result), LogWwiseFileHandler, Error, TEXT("Could not allocate %" PRIi64 " (%" PRIi32 ") bytes in Unreal memory"), InMemorySize, InMemoryAlignment);
	}
	return Result;
}

void FWwiseFileStateTools::DeallocateMemory(const uint8* InMemoryPtr, int64 InMemorySize, bool bInDeviceMemory,
	int32 InMemoryAlignment, bool bInEnforceMemoryRequirements,
	const FName& InStat, const FName& InStatDevice)
{
	if (!InMemoryPtr)
	{
		return;
	}

	if (bInDeviceMemory && bInEnforceMemoryRequirements)
	{
#if AK_SUPPORT_DEVICE_MEMORY
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Deallocating %" PRIi64 " (%" PRIi32 ") bytes in Device Memory"), InMemorySize, InMemoryAlignment);
		ASYNC_DEC_MEMORY_STAT_BY_FName(InStatDevice, InMemorySize);
		AKPLATFORM::FreeDevice((void*)InMemoryPtr, InMemorySize, 0, true);
#else
		UE_LOG(LogWwiseFileHandler, Error, TEXT("No Device Memory, but trying to deallocate %" PRIi64 " (%" PRIi32 ") bytes"), InMemorySize, InMemoryAlignment);
		return DeallocateMemory(InMemoryPtr, InMemorySize, false, InMemoryAlignment, bInEnforceMemoryRequirements, InStat, InStatDevice);

#endif
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Deallocating %" PRIi64 " (%" PRIi32 ") bytes in Unreal memory"), InMemorySize, InMemoryAlignment);
		FMemory::Free(const_cast<uint8*>(InMemoryPtr));
		ASYNC_DEC_MEMORY_STAT_BY_FName(InStat, InMemorySize);
	}
}

bool FWwiseFileStateTools::GetMemoryMapped(IMappedFileHandle*& OutMappedHandle, IMappedFileRegion*& OutMappedRegion,
	int64& OutSize, const FString& InFilePathname, int32 InMemoryAlignment, const FName& InStat)
{
	if (!GetMemoryMapped(OutMappedHandle, OutSize, InFilePathname, InMemoryAlignment, InStat))
	{
		return false;
	}
	if (UNLIKELY(!GetMemoryMappedRegion(OutMappedRegion, *OutMappedHandle)))
	{
		UnmapHandle(*OutMappedHandle, InStat);
		OutMappedHandle = nullptr;
		return false;
	}
	return true;
}

bool FWwiseFileStateTools::GetMemoryMapped(IMappedFileHandle*& OutMappedHandle, int64& OutSize,
	const FString& InFilePathname, int32 InMemoryAlignment,
	const FName& InStat)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("Memory mapping %s"), *InFilePathname);
	auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	auto* Handle = PlatformFile.OpenMapped(*InFilePathname);
	if (UNLIKELY(!Handle))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("OpenMapped %s failed"), *InFilePathname);
		return false;
	}

	OutMappedHandle = Handle;
	OutSize = Handle->GetFileSize();
	ASYNC_INC_MEMORY_STAT_BY_FName(InStat, OutSize);
	return true;
}

bool FWwiseFileStateTools::GetMemoryMappedRegion(IMappedFileRegion*& OutMappedRegion, IMappedFileHandle& InMappedHandle)
{
	auto* Region = InMappedHandle.MapRegion(0, MAX_int64, true);
	if (UNLIKELY(!Region))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("MapRegion failed"));
		return false;
	}

	OutMappedRegion = Region;
	return true;
}

void FWwiseFileStateTools::UnmapRegion(IMappedFileRegion& InMappedRegion)
{
	delete &InMappedRegion;
}

void FWwiseFileStateTools::UnmapHandle(IMappedFileHandle& InMappedHandle, const FName& InStat)
{
	const auto Size = InMappedHandle.GetFileSize(); 
	delete &InMappedHandle;
	ASYNC_INC_MEMORY_STAT_BY_FName(InStat, Size);
}

void FWwiseFileStateTools::GetFileToPtr(TUniqueFunction<void(bool bResult, const uint8* Ptr, int64 Size)>&& InCallback,
	const FString& InFilePathname, bool bInDeviceMemory, int32 InMemoryAlignment, bool bInEnforceMemoryRequirements,
	const FName& InStat, const FName& InStatDevice,
	EAsyncIOPriorityAndFlags InPriority, int64 ReadFirstBytes)
{
	SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileStateTools::GetFileToPtr"));

	auto* FileCache = FWwiseFileCache::Get();
	if (UNLIKELY(!FileCache))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseFileStateTools::GetFileToPtr Failed to get FileCache instance while reading %s"), *InFilePathname);
		return InCallback(false, nullptr, 0);
	}

	auto* HandlePtr = new FWwiseFileCacheHandle*;
	if (UNLIKELY(!HandlePtr))
	{
		UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseFileStateTools::GetFileToPtr Failed to allocate FileCacheHandle pointer while reading %s"), *InFilePathname);
		return InCallback(false, nullptr, 0);
	}

	FileCache->CreateFileCacheHandle(*HandlePtr, InFilePathname,
	[HandlePtr, InCallback = MoveTemp(InCallback), InFilePathname, bInDeviceMemory, InMemoryAlignment, bInEnforceMemoryRequirements, InStat, InStatDevice, InPriority, ReadFirstBytes](bool bResult) mutable
	{
		SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileStateTools::GetFileToPtr Opened"));
		auto* Handle = *HandlePtr;

		if (UNLIKELY(!bResult) || UNLIKELY(!Handle))
		{
			UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseFileStateTools::GetFileToPtr Failed to get FileCache instance while reading %s"), *InFilePathname);
			delete HandlePtr;
			InCallback(false, nullptr, 0);
			if (Handle) Handle->CloseAndDelete();
			return;
		}
			
		auto Size = Handle->GetFileSize();
		
		if (UNLIKELY(!Size))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseFileStateTools::GetFileToPtr File not found %s"), *InFilePathname);
			delete HandlePtr;
			InCallback(false, nullptr, 0);
			Handle->CloseAndDelete();
			return;
		}

		if (ReadFirstBytes >= 0 && Size > ReadFirstBytes)
		{
			Size = ReadFirstBytes;
		}

		auto MemoryAlignment = InMemoryAlignment;
		if (UNLIKELY((MemoryAlignment & (MemoryAlignment - 1)) != 0))
		{
			UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseFileStateTools::GetFileToPtr Invalid non-2^n Memory Alignment (%" PRIi32 ") while getting file %s. Resetting to 0."), InMemoryAlignment, *InFilePathname);
			MemoryAlignment = 0;
		}

		uint8* Ptr = AllocateMemory(Size, bInDeviceMemory, MemoryAlignment, bInEnforceMemoryRequirements, InStat, InStatDevice);
		if (UNLIKELY(!Ptr))
		{
			UE_LOG(LogWwiseFileHandler, Warning, TEXT("FWwiseFileStateTools::GetFileToPtr Could not Allocate %" PRIi64 " for %s"), Size, *InFilePathname);
			delete HandlePtr;
			InCallback(false, nullptr, 0);
			Handle->CloseAndDelete();
			return;
		}

		UE_CLOG(ReadFirstBytes >= 0, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileStateTools::GetFileToPtr Getting the first bytes of file %s (%" PRIi64 " bytes)"), *InFilePathname, Size);
		UE_CLOG(ReadFirstBytes == 0, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileStateTools::GetFileToPtr Getting the entire file %s (%" PRIi64 " bytes)"), *InFilePathname, Size);

		Handle->ReadData(Ptr, 0, Size, InPriority,
		[HandlePtr, InCallback = MoveTemp(InCallback), InFilePathname, Ptr, Size, bInDeviceMemory, InMemoryAlignment, bInEnforceMemoryRequirements, InStat, InStatDevice, ReadFirstBytes](bool bResult) mutable
		{
			SCOPED_WWISEFILEHANDLER_EVENT_4(TEXT("FWwiseFileStateTools::GetFileToPtr Done"));
			auto* Handle = *HandlePtr;
			delete HandlePtr;

			if (LIKELY(bResult))
			{
				UE_CLOG(ReadFirstBytes >= 0, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileStateTools::GetFileToPtr Done getting the first bytes of file %s (%" PRIi64 " bytes)"), *InFilePathname, Size);
				UE_CLOG(ReadFirstBytes == 0, LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseFileStateTools::GetFileToPtr Done getting the entire file %s (%" PRIi64 " bytes)"), *InFilePathname, Size);
			}
			else
			{
				UE_CLOG(ReadFirstBytes >= 0, LogWwiseFileHandler, Error, TEXT("FWwiseFileStateTools::GetFileToPtr Failed getting the first bytes of file %s (%" PRIi64 " bytes)"), *InFilePathname, Size);
				UE_CLOG(ReadFirstBytes == 0, LogWwiseFileHandler, Error, TEXT("FWwiseFileStateTools::GetFileToPtr Failed getting the entire file %s (%" PRIi64 " bytes)"), *InFilePathname, Size);
				DeallocateMemory(Ptr, Size, bInDeviceMemory, InMemoryAlignment, bInEnforceMemoryRequirements, InStat, InStatDevice);
			}

			LaunchWwiseTask(WWISEFILEHANDLER_ASYNC_NAME("FWwiseFileStateTools::GetFileToPtr Callback"), [InCallback = MoveTemp(InCallback), bResult, Ptr, Size]
			{
				InCallback(bResult, Ptr, Size);
			});
			
			Handle->CloseAndDelete();
		});
	});
}
