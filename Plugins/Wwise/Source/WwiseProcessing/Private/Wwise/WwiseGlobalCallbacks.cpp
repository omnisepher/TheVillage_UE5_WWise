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

#include "Wwise/WwiseGlobalCallbacks.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"

#include "WwiseUnrealHelper.h"
#include "Wwise/WwiseDeferredQueue.h"
#include "Wwise/Stats/Processing.h"
#include "Wwise/WwiseProcessingModule.h"
#include "Wwise/WwiseSoundEngineUtils.h"

FWwiseGlobalCallbacks::FWwiseGlobalCallbacks() :
	RegisterQueue(WWISE_EQ_NAME("RegisterQueue Global Callback")),
	BeginQueue(WWISE_EQ_NAME("BeginQueue Global Callback")),
	PreProcessMessageQueueForRenderQueue(WWISE_EQ_NAME("PreProcessMessageQueueForRenderQueue Global Callback")),
	PostMessagesProcessedQueue(WWISE_EQ_NAME("PostMessagesProcessedQueue Global Callback")),
	BeginRenderQueue(WWISE_EQ_NAME("BeginRenderQueue Global Callback")),
	EndRenderQueue(WWISE_EQ_NAME("EndRenderQueue Global Callback")),
	EndQueue(WWISE_EQ_NAME("EndQueue Global Callback")),
	TermQueue(WWISE_EQ_NAME("TermQueue Global Callback")),
	MonitorQueue(WWISE_EQ_NAME("MonitorQueue Global Callback")),
	MonitorRecapQueue(WWISE_EQ_NAME("MonitorRecapQueue Global Callback")),
	InitQueue(WWISE_EQ_NAME("InitQueue Global Callback")),
	SuspendQueue(WWISE_EQ_NAME("SuspendQueue Global Callback")),
	WakeupFromSuspendQueue(WWISE_EQ_NAME("WakeupFromSuspendQueue Global Callback"))
{
}

FWwiseGlobalCallbacks::~FWwiseGlobalCallbacks()
{
	if (UNLIKELY(bInitialized))
	{
		UE_LOG(LogWwiseProcessing, Error, TEXT("GlobalCallbacks not terminated at destruction."));
	}
}

bool FWwiseGlobalCallbacks::Initialize()
{
	if (UNLIKELY(bInitialized))
	{
		UE_LOG(LogWwiseProcessing, Fatal, TEXT("Global Callbacks already initialized"));
		return false;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseProcessing, Fatal, TEXT("Could not implement callbacks."));
		return false;
	}
	bInitialized = true;

	bool bResult = true;
	AKRESULT Result;
	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnRegisterCallbackStatic, AkGlobalCallbackLocation_Register, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `Register` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginCallbackStatic, AkGlobalCallbackLocation_Begin, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `Begin` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallbackStatic, AkGlobalCallbackLocation_PreProcessMessageQueueForRender, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `PreProcessMessageQueueForRender` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnPostMessagesProcessedCallbackStatic, AkGlobalCallbackLocation_PostMessagesProcessed, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `PostMessagesProcessed` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginRenderCallbackStatic, AkGlobalCallbackLocation_BeginRender, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `BeginRender` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndRenderCallbackStatic, AkGlobalCallbackLocation_EndRender, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `EndRender` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndCallbackStatic, AkGlobalCallbackLocation_End, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `End` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnTermCallbackStatic, AkGlobalCallbackLocation_Term, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `Term` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorCallbackStatic, AkGlobalCallbackLocation_Monitor, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `Monitor` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorRecapCallbackStatic, AkGlobalCallbackLocation_MonitorRecap, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `MonitorRecap` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnInitCallbackStatic, AkGlobalCallbackLocation_Init, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `Init` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnSuspendCallbackStatic, AkGlobalCallbackLocation_Suspend, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `Suspend` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnWakeupFromSuspendCallbackStatic, AkGlobalCallbackLocation_WakeupFromSuspend, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseProcessing, Error, TEXT("Cannot Register `WakeupFromSuspend` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	return bResult;
}

void FWwiseGlobalCallbacks::Terminate()
{
	if (!bInitialized)
	{
		return;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		return;
	}
	bInitialized = false;

	AKRESULT Result;
	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnRegisterCallbackStatic, AkGlobalCallbackLocation_Register);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `Register` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginCallbackStatic, AkGlobalCallbackLocation_Begin);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `Begin` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallbackStatic, AkGlobalCallbackLocation_PreProcessMessageQueueForRender);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `PreProcessMessageQueueForRender` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnPostMessagesProcessedCallbackStatic, AkGlobalCallbackLocation_PostMessagesProcessed);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `PostMessagesProcessed` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginRenderCallbackStatic, AkGlobalCallbackLocation_BeginRender);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `BeginRender` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndRenderCallbackStatic, AkGlobalCallbackLocation_EndRender);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `EndRender` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndCallbackStatic, AkGlobalCallbackLocation_End);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `End` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnTermCallbackStatic, AkGlobalCallbackLocation_Term);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `Term` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorCallbackStatic, AkGlobalCallbackLocation_Monitor);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `Monitor` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorRecapCallbackStatic, AkGlobalCallbackLocation_MonitorRecap);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `MonitorRecap` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnInitCallbackStatic, AkGlobalCallbackLocation_Init);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `Init` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnSuspendCallbackStatic, AkGlobalCallbackLocation_Suspend);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `Suspend` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnWakeupFromSuspendCallbackStatic, AkGlobalCallbackLocation_WakeupFromSuspend);
	UE_CLOG(Result != AK_Success && Result != AK_InvalidParameter, LogWwiseProcessing, Verbose, TEXT("Cannot Unregister `WakeupFromSuspend` Callback: %d (%s)"), Result, WwiseUnrealHelper::GetResultString(Result));
}

void FWwiseGlobalCallbacks::RegisterSync(FSyncFunction&& InFunction)
{
	RegisterQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::RegisterCompletion(FCompletionPromise&& Promise)
{
	RegisterAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForRegister()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForRegister"));
	FEventRef Event;
	RegisterAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::BeginSync(FSyncFunction&& InFunction)
{
	BeginQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::BeginCompletion(FCompletionPromise&& Promise)
{
	BeginAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForBegin()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForBegin"));
	FEventRef Event;
	BeginAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::PreProcessMessageQueueForRenderSync(FSyncFunction&& InFunction)
{
	PreProcessMessageQueueForRenderQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::PreProcessMessageQueueForRenderCompletion(FCompletionPromise&& Promise)
{
	PreProcessMessageQueueForRenderAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForPreProcessMessageQueueForRender()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForPreProcessMessageQueueForRender"));
	FEventRef Event;
	PreProcessMessageQueueForRenderAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::PostMessagesProcessedSync(FSyncFunction&& InFunction)
{
	PostMessagesProcessedQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::PostMessagesProcessedCompletion(FCompletionPromise&& Promise)
{
	PostMessagesProcessedAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForPostMessagesProcessed()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForPostMessagesProcessed"));
	FEventRef Event;
	PostMessagesProcessedAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::BeginRenderSync(FSyncFunction&& InFunction)
{
	BeginRenderQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::BeginRenderCompletion(FCompletionPromise&& Promise)
{
	BeginRenderAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForBeginRender()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForBeginRender"));
	FEventRef Event;
	BeginRenderAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::EndRenderSync(FSyncFunction&& InFunction)
{
	EndRenderQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::EndRenderCompletion(FCompletionPromise&& Promise)
{
	EndRenderAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForEndRender()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForEndRender"));
	FEventRef Event;
	EndRenderAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::EndSync(FSyncFunction&& InFunction)
{
	EndQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::EndCompletion(FCompletionPromise&& Promise, int Count)
{
	if (Count <= 0)
	{
		return Promise.EmplaceValue();
	}

	EndAsync([Promise = MoveTemp(Promise), Count = Count - 1]() mutable
	{
		auto* WwiseGlobalCallbacks = FWwiseGlobalCallbacks::Get();
		WwiseGlobalCallbacks->EndCompletion(MoveTemp(Promise), Count);
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForEnd()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForEnd"));
	FEventRef Event;
	EndAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::TermSync(FSyncFunction&& InFunction)
{
	TermQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::TermCompletion(FCompletionPromise&& Promise)
{
	TermAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForTerm()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForTerm"));
	FEventRef Event;
	TermAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::MonitorSync(FSyncFunction&& InFunction)
{
	MonitorQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::MonitorCompletion(FCompletionPromise&& Promise)
{
	MonitorAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForMonitor()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForMonitor"));
	FEventRef Event;
	MonitorAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::MonitorRecapSync(FSyncFunction&& InFunction)
{
	MonitorRecapQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::MonitorRecapCompletion(FCompletionPromise&& Promise)
{
	MonitorRecapAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForMonitorRecap()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForMonitorRecap"));
	FEventRef Event;
	MonitorRecapAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::InitSync(FSyncFunction&& InFunction)
{
	InitQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::InitCompletion(FCompletionPromise&& Promise)
{
	InitAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForInit()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForInit"));
	FEventRef Event;
	InitAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::SuspendSync(FSyncFunction&& InFunction)
{
	SuspendQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::SuspendCompletion(FCompletionPromise&& Promise)
{
	SuspendAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForSuspend()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForSuspend"));
	FEventRef Event;
	SuspendAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WakeupFromSuspendSync(FSyncFunction&& InFunction)
{
	WakeupFromSuspendQueue.SyncDefer(MoveTemp(InFunction));
}

void FWwiseGlobalCallbacks::WakeupFromSuspendCompletion(FCompletionPromise&& Promise)
{
	WakeupFromSuspendAsync([Promise = MoveTemp(Promise)]() mutable
	{
		Promise.EmplaceValue();
		return EWwiseDeferredAsyncResult::Done;
	});
}

void FWwiseGlobalCallbacks::WaitForWakeupFromSuspend()
{
	SCOPED_WWISEPROCESSING_EVENT_4(TEXT("FWwiseGlobalCallbacks::WaitForWakeupFromSuspend"));
	FEventRef Event;
	WakeupFromSuspendAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::OnRegisterCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	RegisterQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnBeginCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	BeginQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	PreProcessMessageQueueForRenderQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnPostMessagesProcessedCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	PostMessagesProcessedQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnBeginRenderCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	BeginRenderQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnEndRenderCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	EndRenderQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnEndCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	EndQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnTermCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	TermQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnMonitorCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	MonitorQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnMonitorRecapCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	MonitorRecapQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnInitCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	InitQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnSuspendCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	SuspendQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnWakeupFromSuspendCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	WakeupFromSuspendQueue.Run(in_pContext);
}

void FWwiseGlobalCallbacks::OnRegisterCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnRegisterCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnBeginCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnBeginCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnPreProcessMessageQueueForRenderCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnPostMessagesProcessedCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnPostMessagesProcessedCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnBeginRenderCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnBeginRenderCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnEndRenderCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnEndRenderCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnEndCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnEndCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnTermCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnTermCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnMonitorCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnMonitorCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnMonitorRecapCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnMonitorRecapCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnInitCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnInitCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnSuspendCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnSuspendCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnWakeupFromSuspendCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnWakeupFromSuspendCallback(in_pContext);
}
