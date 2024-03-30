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

#include "AkGenerateSoundBanksTask.h"
#include "AkWaapiClient.h"
#include "Async/Async.h"

AkGenerateSoundBanksTask::AkGenerateSoundBanksTask(const AkSoundBankGenerationManager::FInitParameters& InitParameters)
{
	GenerationManager = MakeShared<AkSoundBankGenerationManager, ESPMode::ThreadSafe>(InitParameters);
	GenerationManager->Init();
}

AkGenerateSoundBanksTask::~AkGenerateSoundBanksTask()
{
}

void AkGenerateSoundBanksTask::ExecuteForEditorPlatform()
{
	AkSoundBankGenerationManager::FInitParameters InitParameters;
	InitParameters.Platforms = { FPlatformProperties::IniPlatformName() };

	if (FAkWaapiClient::IsProjectLoaded())
	{
		InitParameters.GenerationMode = AkSoundBankGenerationManager::ESoundBankGenerationMode::WAAPI;
	}

	CreateAndExecuteTask(InitParameters);
}

void AkGenerateSoundBanksTask::CreateAndExecuteTask(const AkSoundBankGenerationManager::FInitParameters& InitParameters)
{
	AsyncTask(ENamedThreads::Type::AnyBackgroundThreadNormalTask, [InitParameters]
	{
		auto task = new FAsyncTask<AkGenerateSoundBanksTask>(InitParameters);
		task->StartSynchronousTask();
		task->EnsureCompletion();

		//Probably no longer necessary as we are no longer loading/unloading assets
		AsyncTask(ENamedThreads::GameThread, [task]
		{
			delete task;
		});
	});
}

void AkGenerateSoundBanksTask::DoWork()
{
	GenerationManager->DoGeneration();
}
