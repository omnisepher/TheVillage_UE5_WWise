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

#include "Wwise/WwiseReconcileModuleImpl.h"
#include "Wwise/WwiseReconcile.h"
#include "Wwise/Stats/Reconcile.h"
#include "Wwise/WwiseReconcileImpl.h"

IMPLEMENT_MODULE(FWwiseReconcileModule, WwiseReconcileModule)

IWwiseReconcile* FWwiseReconcileModule::GetReconcile()
{
	Lock.ReadLock();
	if (LIKELY(Reconcile))
	{
		Lock.ReadUnlock();
	}
	else
	{
		Lock.ReadUnlock();
		Lock.WriteLock();
		if (LIKELY(!Reconcile))
		{
			UE_LOG(LogWwiseReconcile, Display, TEXT("Initializing default Reconcile."));
			Reconcile.Reset(InstantiateReconcile());
		}
		Lock.WriteUnlock();
	}
	return Reconcile.Get();
}

IWwiseReconcile* FWwiseReconcileModule::InstantiateReconcile()
{
	SCOPED_WWISERECONCILE_EVENT(TEXT("FWwiseReconcileModule::InstantiateReconcileImpl"));
	return new FWwiseReconcileImpl;
}

void FWwiseReconcileModule::ShutdownModule()
{
	Lock.WriteLock();
	if (Reconcile.IsValid())
	{
		UE_LOG(LogWwiseReconcile, Display, TEXT("Shutting down default Reconcile."));
		Reconcile.Reset();
	}
	Lock.WriteUnlock();
	IWwiseReconcileModule::ShutdownModule();
}
