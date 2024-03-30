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

#include "AkCallbackInfoPool.h"
#include "AkGameplayTypes.h"

UAkCallbackInfo* AkCallbackInfoPool::InternalAcquire(UClass* type)
{
	ensure(IsInGameThread());
	auto& poolArray = Pool.FindOrAdd(type);
	if (poolArray.Num() > 0)
	{
		return poolArray.Pop();
	}

	auto instance = NewObject<UAkCallbackInfo>((UObject*)GetTransientPackage(), type, NAME_None, RF_Public | RF_Standalone);
	gcStorage.Emplace(instance);
	return instance;
}

void AkCallbackInfoPool::Release(UAkCallbackInfo* instance)
{
	ensure(IsInGameThread());
	if (Pool.Contains(instance->GetClass()))
	{
		instance->Reset();
		Pool[instance->GetClass()].Push(instance);
	}
}
