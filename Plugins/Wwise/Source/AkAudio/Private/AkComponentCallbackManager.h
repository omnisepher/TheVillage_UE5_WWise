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

#include "AkAudioDevice.h"
#include "WwiseUnrealDefines.h"

class IAkUserEventCallbackPackage
{
public:
	/** Copy of the user callback flags, for use in our own callback */
	uint32 uUserFlags;

	uint32 KeyHash;

	bool HasExternalSources = false;

	IAkUserEventCallbackPackage()
		: uUserFlags(0)
	{}

	IAkUserEventCallbackPackage(uint32 in_Flags, uint32 in_Hash, bool in_HasExternalSources)
		: uUserFlags(in_Flags)
		, KeyHash(in_Hash)
		, HasExternalSources(in_HasExternalSources)
	{}

	virtual ~IAkUserEventCallbackPackage() {}

	virtual void HandleAction(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo) = 0;
	virtual void CancelCallback() {};
};

class FAkFunctionPtrEventCallbackPackage : public IAkUserEventCallbackPackage
{
public:
	FAkFunctionPtrEventCallbackPackage(AkCallbackFunc CbFunc, void* Cookie, uint32 Flags, uint32 in_Hash, bool in_HasExternalSources)
		: IAkUserEventCallbackPackage(Flags, in_Hash, in_HasExternalSources)
		  , pfnUserCallback(CbFunc)
		  , pUserCookie(Cookie)
		  , bShouldExecute(true)
	{
	}

	virtual void HandleAction(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo) override;
	virtual void CancelCallback() override;

private:
	/** Copy of the user callback, for use in our own callback */
	AkCallbackFunc pfnUserCallback;

	/** Copy of the user cookie, for use in our own callback */
	void* pUserCookie;

	/* Whether the callback should be executed, false if it has been cancelled. */
	TAtomic<bool> bShouldExecute;

	/* Prevent cancelling a callback while it is executing and vice-versa */
	static FCriticalSection CancelLock;
};

class FAkBlueprintDelegateEventCallbackPackage : public IAkUserEventCallbackPackage
{
public:
	FAkBlueprintDelegateEventCallbackPackage(FOnAkPostEventCallback PostEventCallback, uint32 Flags, uint32 in_Hash, bool in_HasExternalSources)
		: IAkUserEventCallbackPackage(Flags, in_Hash, in_HasExternalSources)
		, BlueprintCallback(PostEventCallback)
	{}

	virtual void HandleAction(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo) override;
	virtual void CancelCallback() override;

private:
	FOnAkPostEventCallback BlueprintCallback;
};

class FAkLatentActionEventCallbackPackage : public IAkUserEventCallbackPackage
{
public:
	FAkLatentActionEventCallbackPackage(FWaitEndOfEventAction* LatentAction, uint32 in_Hash, bool in_HasExternalSources)
		: IAkUserEventCallbackPackage(AK_EndOfEvent, in_Hash, in_HasExternalSources)
		, EndOfEventLatentAction(LatentAction)
	{
		LatentActionValidityToken = MakeShared<FPendingLatentActionValidityToken, ESPMode::ThreadSafe>();
		EndOfEventLatentAction->ValidityToken = LatentActionValidityToken;
	}

	virtual void HandleAction(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo) override;

private:
	TSharedPtr<FPendingLatentActionValidityToken, ESPMode::ThreadSafe> LatentActionValidityToken;
	FWaitEndOfEventAction* EndOfEventLatentAction;
};

class FAkComponentCallbackManager
{
public:
	static FAkComponentCallbackManager* GetInstance();

	static FAkComponentCallbackManager* Instance;

	/** Our own event callback */
	static void AkComponentCallback(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo);

	FAkComponentCallbackManager();
	~FAkComponentCallbackManager();

	IAkUserEventCallbackPackage* CreateCallbackPackage(AkCallbackFunc in_cbFunc, void* in_Cookie, uint32 in_Flags, AkGameObjectID in_gameObjID, bool HasExternalSources);
	IAkUserEventCallbackPackage* CreateCallbackPackage(FOnAkPostEventCallback BlueprintCallback, uint32 in_Flags, AkGameObjectID in_gameObjID, bool HasExternalSources);
	IAkUserEventCallbackPackage* CreateCallbackPackage(FWaitEndOfEventAction* LatentAction, AkGameObjectID in_gameObjID, bool HasExternalSources);
	void RemoveCallbackPackage(IAkUserEventCallbackPackage* in_Package, AkGameObjectID in_gameObjID);

	void CancelEventCallback(void* in_Cookie);
	void CancelEventCallback(const FOnAkPostEventCallback& in_Delegate);

	void RegisterGameObject(AkGameObjectID in_gameObjID);
	void UnregisterGameObject(AkGameObjectID in_gameObjID);

	bool HasActiveEvents(AkGameObjectID in_gameObjID);

private:
	typedef TSet<IAkUserEventCallbackPackage*> PackageSet;

	void RemovePackageFromSet(PackageSet* in_pPackageSet, IAkUserEventCallbackPackage* in_pPackage, AkGameObjectID in_gameObjID);

	FCriticalSection CriticalSection;

	typedef WwiseUnrealHelper::AkGameObjectIdKeyFuncs<PackageSet, false> PackageSetGameObjectIDKeyFuncs;
	TMap<AkGameObjectID, PackageSet, FDefaultSetAllocator, PackageSetGameObjectIDKeyFuncs> GameObjectToPackagesMap;

	// Used for quick lookup in cancel
	uint32 inline GetKeyHash(void* Key);
	uint32 inline GetKeyHash(const FOnAkPostEventCallback& Key);

	void CancelKeyHash(uint32 HashToCancel);
	TMultiMap<uint32, IAkUserEventCallbackPackage*> UserCookieHashToPackageMap;
};
