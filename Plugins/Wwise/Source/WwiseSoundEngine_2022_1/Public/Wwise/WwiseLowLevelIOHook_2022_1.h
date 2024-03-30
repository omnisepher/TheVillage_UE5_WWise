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

class WWISESOUNDENGINE_API FWwiseLowLevelIOHook : public AK::StreamMgr::IAkIOHookDeferredBatch
{
public:
	virtual AKRESULT Close(AkFileDesc* in_pFileDesc) = 0;

	virtual void BatchRead(
		AkUInt32 in_uNumTransfers,
		BatchIoTransferItem* in_pTransferItems
	) = 0;

	virtual void BatchWrite(
		AkUInt32 in_uNumTransfers,
		BatchIoTransferItem* in_pTransferItems
	) = 0;

private:
	virtual AKRESULT Close(
		AkFileDesc& in_fileDesc
	) override final;

	virtual AKRESULT BatchRead(
		AkUInt32				in_uNumTransfers,
		BatchIoTransferItem*	in_pTransferItems,
		AkBatchIOCallback		in_pBatchIoCallback,
		AKRESULT*				io_pDispatchResults
	) override final;

	virtual AKRESULT BatchWrite(
		AkUInt32				in_uNumTransfers,		///< Number of transfers to process
		BatchIoTransferItem*	in_pTransferItems,		///< List of transfer items to process
		AkBatchIOCallback		in_pBatchIoCallback,	///< Callback to execute to handle completion of multiple items simultaneously
		AKRESULT*				io_pDispatchResults		///< Output result codes to indicate if a transfer was successfully dispatched
	) override final;
};
