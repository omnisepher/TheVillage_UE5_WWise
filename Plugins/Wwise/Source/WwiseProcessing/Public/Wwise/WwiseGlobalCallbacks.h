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

#include "Wwise/WwiseDeferredQueue.h"

#include "AkInclude.h"
#include "WwiseProcessingModule.h"

#include "Async/Future.h"

class WWISEPROCESSING_API FWwiseGlobalCallbacks
{
public:
	using FCompletionPromise = TPromise<void>;
	using FAsyncFunction = FWwiseDeferredQueue::FFunction;
	using FGameFunction = FWwiseDeferredQueue::FFunction;
	using FSyncFunction = FWwiseDeferredQueue::FSyncFunction;
	using FGameThreadDelegate = FWwiseDeferredQueue::FGameThreadDelegate;
	using FThreadSafeDelegate = FWwiseDeferredQueue::FThreadSafeDelegate;

protected:
	bool bInitialized = false;
	FWwiseDeferredQueue RegisterQueue;
	FWwiseDeferredQueue BeginQueue;
	FWwiseDeferredQueue PreProcessMessageQueueForRenderQueue;
	FWwiseDeferredQueue PostMessagesProcessedQueue;
	FWwiseDeferredQueue BeginRenderQueue;
	FWwiseDeferredQueue EndRenderQueue;
	FWwiseDeferredQueue EndQueue;
	FWwiseDeferredQueue TermQueue;
	FWwiseDeferredQueue MonitorQueue;
	FWwiseDeferredQueue MonitorRecapQueue;
	FWwiseDeferredQueue InitQueue;
	FWwiseDeferredQueue SuspendQueue;
	FWwiseDeferredQueue WakeupFromSuspendQueue;

public:
	static FWwiseGlobalCallbacks* Get()
	{
		const auto ProcessingModule = IWwiseProcessingModule::GetModule();
		if (UNLIKELY(!ProcessingModule))
		{
			return nullptr;
		}
		return ProcessingModule->GetGlobalCallbacks();
	}

	FWwiseGlobalCallbacks();
	virtual ~FWwiseGlobalCallbacks();

	virtual bool Initialize();
	virtual void Terminate();
	bool IsInitialized() const { return bInitialized; }

	// AkGlobalCallbackLocation_Register: Right after successful registration of callback/plugin. Typically used by plugins along with AkGlobalCallbackLocation_Term for allocating memory for the lifetime of the sound engine.
	void RegisterAsync(FAsyncFunction&& InFunction) { RegisterQueue.AsyncDefer(MoveTemp(InFunction)); }
	void RegisterGame(FGameFunction&& InFunction) { RegisterQueue.GameDefer(MoveTemp(InFunction)); }
	void RegisterSync(FSyncFunction&& InFunction);
	void RegisterCompletion(FCompletionPromise&& Promise);
	void WaitForRegister();
	FGameThreadDelegate& OnRegister { RegisterQueue.OnGameRun };
	FThreadSafeDelegate& OnRegisterTS { RegisterQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_Begin: Start of audio processing. The number of frames about to be rendered depends on the sink/end-point and can be zero.
	void BeginAsync(FAsyncFunction&& InFunction) { BeginQueue.AsyncDefer(MoveTemp(InFunction)); }
	void BeginGame(FGameFunction&& InFunction) { BeginQueue.GameDefer(MoveTemp(InFunction)); }
	void BeginSync(FSyncFunction&& InFunction);
	void BeginCompletion(FCompletionPromise&& Promise);
	void WaitForBegin();
	FGameThreadDelegate& OnBegin { BeginQueue.OnGameRun };
	FThreadSafeDelegate& OnBeginTS { BeginQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_PreProcessMessageQueueForRender: Start of frame rendering, before having processed game messages.
	void PreProcessMessageQueueForRenderAsync(FAsyncFunction&& InFunction) { PreProcessMessageQueueForRenderQueue.AsyncDefer(MoveTemp(InFunction)); }
	void PreProcessMessageQueueForRenderGame(FGameFunction&& InFunction) { PreProcessMessageQueueForRenderQueue.GameDefer(MoveTemp(InFunction)); }
	void PreProcessMessageQueueForRenderSync(FSyncFunction&& InFunction);
	void PreProcessMessageQueueForRenderCompletion(FCompletionPromise&& Promise);
	void WaitForPreProcessMessageQueueForRender();
	FGameThreadDelegate& OnPreProcessMessageQueueForRender { PreProcessMessageQueueForRenderQueue.OnGameRun };
	FThreadSafeDelegate& OnPreProcessMessageQueueForRenderTS { PreProcessMessageQueueForRenderQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_PostMessagesProcessed: After one or more messages have been processed, but before updating game object and listener positions internally.
	void PostMessagesProcessedAsync(FAsyncFunction&& InFunction) { PostMessagesProcessedQueue.AsyncDefer(MoveTemp(InFunction)); }
	void PostMessagesProcessedGame(FGameFunction&& InFunction) { PostMessagesProcessedQueue.GameDefer(MoveTemp(InFunction)); }
	void PostMessagesProcessedSync(FSyncFunction&& InFunction);
	void PostMessagesProcessedCompletion(FCompletionPromise&& Promise);
	void WaitForPostMessagesProcessed();
	FGameThreadDelegate& OnPostMessagesProcessed { PostMessagesProcessedQueue.OnGameRun };
	FThreadSafeDelegate& OnPostMessagesProcessedTS { PostMessagesProcessedQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_BeginRender: Start of frame rendering, after having processed game messages.
	void BeginRenderAsync(FAsyncFunction&& InFunction) { BeginRenderQueue.AsyncDefer(MoveTemp(InFunction)); }
	void BeginRenderGame(FGameFunction&& InFunction) { BeginRenderQueue.GameDefer(MoveTemp(InFunction)); }
	void BeginRenderSync(FSyncFunction&& InFunction);
	void BeginRenderCompletion(FCompletionPromise&& Promise);
	void WaitForBeginRender();
	FGameThreadDelegate& OnBeginRender { BeginRenderQueue.OnGameRun };
	FThreadSafeDelegate& OnBeginRenderTS { BeginRenderQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_EndRender: End of frame rendering.
	void EndRenderAsync(FAsyncFunction&& InFunction) { EndRenderQueue.AsyncDefer(MoveTemp(InFunction)); }
	void EndRenderGame(FGameFunction&& InFunction) { EndRenderQueue.GameDefer(MoveTemp(InFunction)); }
	void EndRenderSync(FSyncFunction&& InFunction);
	void EndRenderCompletion(FCompletionPromise&& Promise);
	void WaitForEndRender();
	FGameThreadDelegate& OnEndRender { EndRenderQueue.OnGameRun };
	FThreadSafeDelegate& OnEndRenderTS { EndRenderQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_End: End of audio processing.
	void EndAsync(FAsyncFunction&& InFunction) { EndQueue.AsyncDefer(MoveTemp(InFunction)); }
	void EndGame(FGameFunction&& InFunction) { EndQueue.GameDefer(MoveTemp(InFunction)); }
	void EndSync(FSyncFunction&& InFunction);
	void EndCompletion(FCompletionPromise&& Promise, int Count = 1);
	void WaitForEnd();
	FGameThreadDelegate& OnEnd { EndQueue.OnGameRun };
	FThreadSafeDelegate& OnEndTS { EndQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_Term: Sound engine termination.
	void TermAsync(FAsyncFunction&& InFunction) { TermQueue.AsyncDefer(MoveTemp(InFunction)); }
	void TermGame(FGameFunction&& InFunction) { TermQueue.GameDefer(MoveTemp(InFunction)); }
	void TermSync(FSyncFunction&& InFunction);
	void TermCompletion(FCompletionPromise&& Promise);
	void WaitForTerm();
	FGameThreadDelegate& OnTerm { TermQueue.OnGameRun };
	FThreadSafeDelegate& OnTermTS { TermQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_Monitor: Send monitor data
	void MonitorAsync(FAsyncFunction&& InFunction) { MonitorQueue.AsyncDefer(MoveTemp(InFunction)); }
	void MonitorGame(FGameFunction&& InFunction) { MonitorQueue.GameDefer(MoveTemp(InFunction)); }
	void MonitorSync(FSyncFunction&& InFunction);
	void MonitorCompletion(FCompletionPromise&& Promise);
	void WaitForMonitor();
	FGameThreadDelegate& OnMonitor { MonitorQueue.OnGameRun };
	FThreadSafeDelegate& OnMonitorTS { MonitorQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_MonitorRecap: Send monitor data connection to recap.
	void MonitorRecapAsync(FAsyncFunction&& InFunction) { MonitorRecapQueue.AsyncDefer(MoveTemp(InFunction)); }
	void MonitorRecapGame(FGameFunction&& InFunction) { MonitorRecapQueue.GameDefer(MoveTemp(InFunction)); }
	void MonitorRecapSync(FSyncFunction&& InFunction);
	void MonitorRecapCompletion(FCompletionPromise&& Promise);
	void WaitForMonitorRecap();
	FGameThreadDelegate& OnMonitorRecap { MonitorRecapQueue.OnGameRun };
	FThreadSafeDelegate& OnMonitorRecapTS { MonitorRecapQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_Init: Sound engine initialization.
	void InitAsync(FAsyncFunction&& InFunction) { InitQueue.AsyncDefer(MoveTemp(InFunction)); }
	void InitGame(FGameFunction&& InFunction) { InitQueue.GameDefer(MoveTemp(InFunction)); }
	void InitSync(FSyncFunction&& InFunction);
	void InitCompletion(FCompletionPromise&& Promise);
	void WaitForInit();
	FGameThreadDelegate& OnInit { InitQueue.OnGameRun };
	FThreadSafeDelegate& OnInitTS { InitQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_Suspend: Sound engine suspension through AK::SoundEngine::Suspend
	void SuspendAsync(FAsyncFunction&& InFunction) { SuspendQueue.AsyncDefer(MoveTemp(InFunction)); }
	void SuspendGame(FGameFunction&& InFunction) { SuspendQueue.GameDefer(MoveTemp(InFunction)); }
	void SuspendSync(FSyncFunction&& InFunction);
	void SuspendCompletion(FCompletionPromise&& Promise);
	void WaitForSuspend();
	FGameThreadDelegate& OnSuspend { SuspendQueue.OnGameRun };
	FThreadSafeDelegate& OnSuspendTS { SuspendQueue.OnSyncRunTS };

	// AkGlobalCallbackLocation_WakeupFromSuspend: Sound engine awakening through AK::SoundEngine::WakeupFromSuspend
	void WakeupFromSuspendAsync(FAsyncFunction&& InFunction) { WakeupFromSuspendQueue.AsyncDefer(MoveTemp(InFunction)); }
	void WakeupFromSuspendGame(FGameFunction&& InFunction) { WakeupFromSuspendQueue.GameDefer(MoveTemp(InFunction)); }
	void WakeupFromSuspendSync(FSyncFunction&& InFunction);
	void WakeupFromSuspendCompletion(FCompletionPromise&& Promise);
	void WaitForWakeupFromSuspend();
	FGameThreadDelegate& OnWakeupFromSuspend { WakeupFromSuspendQueue.OnGameRun };
	FThreadSafeDelegate& OnWakeupFromSuspendTS { WakeupFromSuspendQueue.OnSyncRunTS };

protected:
	virtual void OnRegisterCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnBeginCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnPreProcessMessageQueueForRenderCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnPostMessagesProcessedCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnBeginRenderCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnEndRenderCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnEndCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnTermCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnMonitorCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnMonitorRecapCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnInitCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnSuspendCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnWakeupFromSuspendCallback(AK::IAkGlobalPluginContext* in_pContext);

private:
	static void OnRegisterCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnBeginCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnPreProcessMessageQueueForRenderCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnPostMessagesProcessedCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnBeginRenderCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnEndRenderCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnEndCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnTermCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnMonitorCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnMonitorRecapCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnInitCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnSuspendCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnWakeupFromSuspendCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
};
