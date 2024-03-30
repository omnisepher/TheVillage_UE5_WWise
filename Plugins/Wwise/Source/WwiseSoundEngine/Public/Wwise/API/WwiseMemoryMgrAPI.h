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

#include "CoreMinimal.h"
#include "AkInclude.h"

#include "Wwise/WwiseSoundEngineModule.h"

class IWwiseMemoryMgrAPI
{
public:
	static IWwiseMemoryMgrAPI* Get()
	{
		IWwiseSoundEngineModule::ForceLoadModule();
		return IWwiseSoundEngineModule::MemoryMgr;
	}

	UE_NONCOPYABLE(IWwiseMemoryMgrAPI);
protected:
	inline IWwiseMemoryMgrAPI() = default;
public:
	virtual ~IWwiseMemoryMgrAPI() {}

	/// Initialize the default implementation of the Memory Manager.
	/// \sa AK::MemoryMgr
	virtual AKRESULT Init(
		AkMemSettings* in_pSettings        ///< Memory manager initialization settings.
		) = 0;

	/// Obtain the default initialization settings for the default implementation of the Memory Manager.
	virtual void GetDefaultSettings(
		AkMemSettings& out_pMemSettings	///< Memory manager default initialization settings.
		) = 0;

	////////////////////////////////////////////////////////////////////////
	/// @name Initialization
	//@{

	/// Query whether the Memory Manager has been successfully initialized.
	/// \warning This function is not thread-safe. It should not be called at the same time as MemoryMgr::Init or MemoryMgr::Term.
	/// \return True if the Memory Manager is initialized, False otherwise
	/// \sa
	/// - AK::MemoryMgr::Init()
	/// - \ref memorymanager
	virtual bool IsInitialized() = 0;

	/// Terminate the Memory Manager.
	/// \warning This function is not thread-safe. It is not valid to allocate memory or otherwise interact with the memory manager during or after this call.
	/// \sa
	/// - \ref memorymanager
	virtual void Term() = 0;

	/// Performs whatever steps are required to initialize a thread for use with the memory manager.
	/// For example initializing thread local storage that the allocator requires to work.
	/// The default implementation of the memory manager performs thread initialization automatically and therefore this call is optional.
	/// For implementations where the cost of automatically initializing a thread for use with an allocator would be prohibitively expensive
	/// this call allows you to perform the initialization once during, for example, thread creation.
	/// \sa
	/// - AkMemInitForThread
	virtual void InitForThread() = 0;

	/// Allows you to manually terminate a thread for use with the memory manager.
	/// The default implementation of the memory manager requires that all threads that interact with the memory manager call this function prior
	/// to either their termination or the termination of the memory manager. Threads not created by the sound engine itself will not have this
	/// function called for them automatically.
	/// Take care to call this function for any thread, not owned by wwise, that may have interacted with the memory manager. For example job system workers.
	/// \sa
	/// - AkMemTermForThread
	virtual void TermForThread() = 0;

	/// Allows you to "trim" a thread being used with the memory manager.
	/// This is a function that will be called periodically by some Wwise-owned threads,
	/// so that any thread-local state can be cleaned up in order to return memory for other systems to use.
	/// For example, this can be used to return thread-local heaps to global stores or to finalize other deferred operations.
	/// This function is only required for optimization purposes and does not have to be defined.
	/// Therefore, unlike TermForThread, this is not expected to be called in all scenarios by Wwise.
	/// It is also recommended to be called by game engine integrations in any worker threads that run Wwise jobs.
	/// Refer to \ref eventmgrthread_jobmgr_best_practices for more information.
	/// \sa
	/// - AkMemTrimForThread
	virtual void TrimForThread() = 0;

	//@}

	////////////////////////////////////////////////////////////////////////
	/// @name Memory Allocation
	//@{

	/// Allocate memory: debug version.
	/// \return A pointer to the start of the allocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* dMalloc(
		AkMemPoolId in_poolId,				///< ID of the memory category (AkMemID)
		size_t		in_uSize,				///< Number of bytes to allocate
		const char* in_pszFile,				///< Debug file name
		AkUInt32	in_uLine				///< Debug line number
		) = 0;

	/// Allocate memory.
	/// \return A pointer to the start of the allocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* Malloc(
		AkMemPoolId in_poolId,				///< ID of the memory category (AkMemID)
		size_t		in_uSize 				///< Number of bytes to allocate
		) = 0;

	/// Reallocate memory: debug version.
	/// \return A pointer to the start of the reallocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* dRealloc(
		AkMemPoolId	in_poolId,
		void* in_pAlloc,
		size_t		in_uSize,
		const char* in_pszFile,
		AkUInt32	in_uLine
		) = 0;

	/// Reallocate memory.
	/// \return A pointer to the start of the reallocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* Realloc(
		AkMemPoolId in_poolId,				///< ID of the memory category (AkMemID)
		void* in_pAlloc,					///< Pointer to the start of the allocated memory
		size_t		in_uSize 				///< Number of bytes to allocate
		) = 0;

	/// Reallocate memory: debug version.
	/// \return A pointer to the start of the reallocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* dReallocAligned(
		AkMemPoolId	in_poolId,				///< ID of the memory category (AkMemID)
		void* in_pAlloc,					///< Pointer to the start of the allocated memory
		size_t		in_uSize,				///< Number of bytes to allocate
		AkUInt32	in_uAlignment,			///< Alignment (in bytes)
		const char* in_pszFile,				///< Debug file name
		AkUInt32	in_uLine				///< Debug line number
		) = 0;

	/// Reallocate memory.
	/// \return A pointer to the start of the reallocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* ReallocAligned(
		AkMemPoolId in_poolId,				///< ID of the memory category (AkMemID)
		void* in_pAlloc,					///< Pointer to the start of the allocated memory
		size_t		in_uSize, 				///< Number of bytes to allocate
		AkUInt32	in_uAlignment			///< Alignment (in bytes)
		) = 0;

	/// Free memory allocated with the memory manager.
	/// \sa
	/// - \ref memorymanager
	virtual void Free(
		AkMemPoolId in_poolId,				///< ID of the memory category (AkMemID)
		void* in_pMemAddress				///< Pointer to the start of memory
		) = 0;

	/// Allocate memory with a specific alignment. debug version.
	/// \return A pointer to the start of the allocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* dMalign(
		AkMemPoolId in_poolId,				///< ID of the memory category (AkMemID)
		size_t		in_uSize,				///< Number of bytes to allocate
		AkUInt32	in_uAlignment, 			///< Alignment (in bytes)
		const char* in_pszFile,				///< Debug file name
		AkUInt32	in_uLine				///< Debug line number
		) = 0;

	/// Allocate memory with a specific alignment.
	/// \return A pointer to the start of the allocated memory (NULL if the allocation could not be completed)
	/// \sa
	/// - \ref memorymanager
	virtual void* Malign(
		AkMemPoolId in_poolId,				///< ID of the memory category (AkMemID)
		size_t		in_uSize, 				///< Number of bytes to allocate
		AkUInt32	in_uAlignment 			///< Alignment (in bytes)
		) = 0;

	//@}

	////////////////////////////////////////////////////////////////////////
	/// @name Memory Profiling
	//@{

	/// Get statistics for a given memory category.
	/// \note Be aware of the potentially incoherent nature of reporting such information during concurrent modification by multiple threads.
	virtual void GetCategoryStats(
		AkMemPoolId	in_poolId,				///< ID of the memory category (AkMemID)
		AK::MemoryMgr::CategoryStats& out_poolStats		///< Returned statistics.
		) = 0;

	/// Get statistics for overall memory manager usage.
	/// \note Be aware of the potentially incoherent nature of reporting such information during concurrent modification by multiple threads.
	virtual void GetGlobalStats(
		AK::MemoryMgr::GlobalStats& out_stats				///< Returned statistics.
		) = 0;

	/// Called to start profiling memory usage for one thread (the calling thread).
	/// \note Not implementing this will result in the Soundbank tab of the Wwise Profiler to show 0 bytes for memory usage.
	virtual void StartProfileThreadUsage(
		) = 0;

	/// Called to stop profiling memory usage for the current thread.
	/// \return The amount of memory allocated by this thread since StartProfileThreadUsage was called.
	/// \note Not implementing this will result in the Soundbank tab of the Wwise Profiler to show 0 bytes for memory usage.
	virtual AkUInt64 StopProfileThreadUsage(
		) = 0;

	/// Dumps the currently tracked allocations to a file
	/// \note AkMemSettings::uMemoryDebugLevel must be enabled and the build must define AK_MEMDEBUG for this to work
	virtual void DumpToFile(
		const AkOSChar* pszFilename			///< Filename.
		) = 0;

	//@}
};
