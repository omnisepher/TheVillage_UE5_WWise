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

/*=============================================================================
	AkJsonFactory.cpp:
=============================================================================*/
#include "Factories/AkJsonFactory.h"

#include "AkAudioEvent.h"
#include "AkSettings.h"
#include "WwiseUnrealHelper.h"
#include "Misc/Paths.h"

/*------------------------------------------------------------------------------
	UAkJsonFactory.
------------------------------------------------------------------------------*/
UAkJsonFactory::UAkJsonFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAudioEvent::StaticClass();
	Formats.Add(TEXT("json;Audiokinetic SoundBank Metadata"));
	bCreateNew = true;
	bEditorImport = true;
	ImportPriority = 101;
}

UObject* UAkJsonFactory::FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn )
{
	return nullptr;
}

bool UAkJsonFactory::FactoryCanImport(const FString& Filename)
{
	//check extension
	if (FPaths::GetExtension(Filename) == TEXT("json"))
	{
		const UAkSettings* AkSettings = GetDefault<UAkSettings>();

		if (Filename.Contains(WwiseUnrealHelper::GetSoundBankDirectory()))
		{
			return true;
		}
	}

	return false;
}

bool UAkJsonFactory::ShouldShowInNewMenu() const
{
	return false;
}
