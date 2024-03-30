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

#include "IAudioLink.h"
#include "Misc/ScopeRWLock.h"
#include "Templates/SharedPointer.h"

#include "AkInclude.h"

struct FWwiseAudioLinkSynchronizer : IAudioLinkSynchronizer, TSharedFromThis<FWwiseAudioLinkSynchronizer, ESPMode::ThreadSafe>
{
	FRWLock RwLock;

	// Registered with AudioLink
	FOnSuspend		OnSuspend;
	FOnResume		OnResume;
	FOnOpenStream	OnOpenStream;
	FOnCloseStream	OnCloseStream;
	FOnBeginRender	OnBeginRender;
	FOnEndRender	OnEndRender;

	bool bIsBound { false };

	FWwiseAudioLinkSynchronizer();
	~FWwiseAudioLinkSynchronizer() override;

	void ExecuteBeginRender(AK::IAkGlobalPluginContext* InContext);
	void ExecuteEndRender(AK::IAkGlobalPluginContext* InContext);
	void ExecuteOpenStream();
	void ExecuteCloseStream();
	void Bind();
	void Unbind();

	#define MAKE_DELEGATE_FUNC(X)\
		FDelegateHandle Register##X##Delegate(const FOn##X::FDelegate& InDelegate) override\
		{\
			FWriteScopeLock WriteLock(RwLock);\
			return On##X.Add(InDelegate);\
		}\
		bool Remove##X##Delegate(const FDelegateHandle& InHandle) override\
		{\
			FWriteScopeLock WriteLock(RwLock);\
			return On##X.Remove(InHandle);\
		}

	MAKE_DELEGATE_FUNC(Suspend)
	MAKE_DELEGATE_FUNC(Resume)
	MAKE_DELEGATE_FUNC(OpenStream)
	MAKE_DELEGATE_FUNC(CloseStream)
	MAKE_DELEGATE_FUNC(BeginRender)
	MAKE_DELEGATE_FUNC(EndRender)

	#undef MAKE_DELEGATE_FUNC
};
