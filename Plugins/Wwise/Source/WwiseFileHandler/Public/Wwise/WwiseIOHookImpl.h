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

#include "Wwise/WwiseIOHook.h"
#include "Wwise/WwiseExecutionQueue.h"

#include "Templates/Atomic.h"

class IWwiseStreamingManagerHooks;

class WWISEFILEHANDLER_API FWwiseIOHookImpl : public FWwiseDefaultIOHook
{
public:
	FWwiseIOHookImpl();
	bool Init(const AkDeviceSettings& InDeviceSettings) override;

	/**
	 * Opens a file, asynchronously.
	 *
	 * @param io_pOpenData		File open information (name, flags, etc). 
	 * Also contain the callback to call when the open operation is completed,
	 * and the AkFileDesc to fill out.	 
	 */
	virtual void BatchOpen(
		AkUInt32				in_uNumFiles,	// Number of files to open
		AkAsyncFileOpenData**	in_ppItems		// File open information (name, flags, etc)
												// Also contain the callback to call when the open operation is completed,
												// and the AkFileDesc to fill out.
	);

	/**
	 * @brief Reads multiple data from multiple files (asynchronous).
	 * 
	 * @param in_uNumTransfers		Number of transfers to process
	 * @param in_pTransferItems		List of transfer items to process	 
	 */
	virtual void BatchRead(
		AkUInt32				in_uNumTransfers,
		BatchIoTransferItem*	in_pTransferItems		
	) override;

	/**
	 * @brief Write multiple data from multiple files (asynchronous).
	 *
	 * @param in_uNumTransfers		Number of transfers to process
	 * @param in_pTransferItems		List of transfer items to process
	 */
	virtual void BatchWrite(
		AkUInt32				in_uNumTransfers,
		BatchIoTransferItem*	in_pTransferItems
	) override;

	/**
	 * @brief Cancel IO from multiple files (asynchronous).
	 *
	 * @param in_uNumTransfers		Number of transfers to process
	 * @param in_pTransferItems		List of transfer items to process
	 */
	virtual void BatchCancel(
		AkUInt32				in_uNumTransfers, 
		BatchIoTransferItem*	in_pTransferItems,
		bool**					io_ppbCancelAllTransfersForThisFile
	) override;

	/**
	 * Cleans up a file.
	 *
	 * @param in_fileDesc		File descriptor.
	 * @return	AK_Success if operation was successful, error code otherwise
	 */
	AKRESULT Close(AkFileDesc* in_pFileDesc) override;

	// Returns the block size for the file or its storage device.
	AkUInt32 GetBlockSize(AkFileDesc& in_fileDesc) override;

	// Returns a description for the streaming device above this low-level hook.
	void GetDeviceDesc(AkDeviceDesc& out_deviceDesc) override;

	// Returns custom profiling data: 1 if file opens are asynchronous, 0 otherwise.
	AkUInt32 GetDeviceData() override;

protected:
	FWwiseExecutionQueue BatchExecutionQueue;
	AkDeviceID m_deviceID = AK_INVALID_DEVICE_ID;

#ifndef AK_OPTIMIZED
	TAtomic<uint32> CurrentDeviceData;
	TAtomic<uint32> MaxDeviceData;
#endif

	virtual IWwiseStreamingManagerHooks* GetStreamingHooks(const AkFileSystemFlags& InFileSystemFlag);

	virtual AKRESULT OpenFileForWrite(AkAsyncFileOpenData *io_pOpenData);

	virtual AKRESULT Open(AkAsyncFileOpenData* io_pOpenData);

	virtual AKRESULT Read(
		AkFileDesc& in_fileDesc,
		const AkIoHeuristics& in_heuristics,
		AkAsyncIOTransferInfo& io_transferInfo);

	virtual AKRESULT Write(
		AkFileDesc& in_fileDesc,
		const AkIoHeuristics& in_heuristics,
		AkAsyncIOTransferInfo& io_transferInfo);
};
