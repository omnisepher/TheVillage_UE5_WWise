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
*****************************************************************************/

#include "Wwise/WwiseUnitTests.h"

#if WWISE_UNIT_TESTS && UE_5_1_OR_LATER

#include "AkComponentCallbackManager.h"
#include "Tasks/Task.h"

WWISE_TEST_CASE(AkComponentCallback_Stress, "Audio::Wwise::AkAudio::AkComponentCallback", "[ApplicationContextMask][StressFilter]")
{
	struct TestCookie
	{
		bool AlreadyCancelled = false;
		bool bTestPassed = true;
	};

	SECTION("Safely use (or dispose of) Cookie after FAkComponentCallbackManager::CancelEventCallback")
	{
		FAkComponentCallbackManager* CallbackManager = FAkComponentCallbackManager::GetInstance();

		if (!CallbackManager)
			return;

		TArray<UE::Tasks::FTask> Tasks = {};
		// Joiner tasks makes sure that we trigger the tasks simultaneously
		UE::Tasks::FTaskEvent Joiner{UE_SOURCE_LOCATION};

		TArray<TestCookie*> CookiesToDelete;

		for (int i = 0; i < 1000; i++)
		{
			TestCookie* Cookie = new TestCookie();
			Cookie->AlreadyCancelled = false;
			CookiesToDelete.Add(Cookie);
			IAkUserEventCallbackPackage* CallbackPackage = CallbackManager->CreateCallbackPackage(
				[](AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo)
				{
					TestCookie* LocalCookie = static_cast<TestCookie*>(in_pCallbackInfo->pCookie);
					// If already cancelled, we should not be executing this callback -> test failure
					LocalCookie->bTestPassed = !LocalCookie->AlreadyCancelled;
				}, Cookie, AK_EndOfEvent, DUMMY_GAMEOBJ, false);

			AkEventCallbackInfo CallbackInfo =
			{
				AkCallbackInfo
				{
					CallbackPackage,
					DUMMY_GAMEOBJ,
				},
				1,
				1
			};

			// Create tasks to try to execute callbacks and cancel them at the same time
			UE::Tasks::FTask TaskA = UE::Tasks::Launch(UE_SOURCE_LOCATION,
				[CallbackManager, Cookie]
				{
					CallbackManager->CancelEventCallback(Cookie);
					Cookie->AlreadyCancelled = true;
				}, Joiner);

			UE::Tasks::FTask TaskB = UE::Tasks::Launch(UE_SOURCE_LOCATION,
				[CallbackManager, CallbackInfo]
				{
					CallbackManager->AkComponentCallback(AkCallbackType::AK_EndOfEvent,
					                                     const_cast<AkEventCallbackInfo*>(&CallbackInfo));
				}, Joiner);

			Tasks.Add(TaskA);
			Tasks.Add(TaskB);
		}

		Joiner.Trigger();
		Joiner.Wait();

		for (auto& Task : Tasks)
		{
			Task.Wait();
		}

		bool bTestsPassed = true;

		for (auto Cookie : CookiesToDelete)
		{
			bTestsPassed &= Cookie->bTestPassed;
			delete Cookie;
		}

		CHECK(bTestsPassed)
	}
}

#endif