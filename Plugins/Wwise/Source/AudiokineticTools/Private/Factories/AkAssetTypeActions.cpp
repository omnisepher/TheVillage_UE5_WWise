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

#include "AkAssetTypeActions.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IAssetTools.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/ScopeLock.h"
#include "Toolkits/SimpleAssetEditor.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "AkAssetTypeActions"

namespace FAkAssetTypeActions_Helpers
{
	FCriticalSection CriticalSection;
	TMap<FString, AkPlayingID> PlayingAkEvents;

	void AkEventPreviewCallback(AkCallbackType in_eType, AkCallbackInfo* in_pCallbackInfo)
	{
		auto EventInfo = static_cast<AkEventCallbackInfo*>(in_pCallbackInfo);
		if (!EventInfo)
			return;

		FScopeLock Lock(&CriticalSection);
		for (auto& PlayingEvent : PlayingAkEvents)
		{
			if (PlayingEvent.Value == EventInfo->playingID)
			{
				PlayingAkEvents.Remove(PlayingEvent.Key);
				return;
			}
		}
	}

	template<bool PlayOne>
	void PlayEvents(const TArray<TWeakObjectPtr<UAkAudioEvent>>& InObjects)
	{
		auto AudioDevice = FAkAudioDevice::Get();
		if (!AudioDevice)
			return;

		for (auto& Obj : InObjects)
		{
			auto Event = Obj.Get();
			if (!Event)
				continue;

			AkPlayingID* foundID;
			{
				FScopeLock Lock(&CriticalSection);
				foundID = PlayingAkEvents.Find(Event->GetName());
			}

			if (foundID)
			{
				AudioDevice->StopPlayingID(*foundID);
			}
			else
			{
				if(!Event->bAutoLoad)
				{
					Event->LoadEventDataForContentBrowserPreview();
				}
				const auto CurrentPlayingID = Event->PostAmbient(nullptr, &AkEventPreviewCallback, nullptr, AK_EndOfEvent, nullptr, EAkAudioContext::EditorAudio);
				if (CurrentPlayingID != AK_INVALID_PLAYING_ID)
				{
					FScopeLock Lock(&CriticalSection);
					PlayingAkEvents.FindOrAdd(Event->GetName()) = CurrentPlayingID;
				}
			}

			if (PlayOne)
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkAcousticTexture

void FAssetTypeActions_AkAcousticTexture::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}


//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkAudioEvent

void FAssetTypeActions_AkAudioEvent::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto Events = GetTypedWeakObjectPtrs<UAkAudioEvent>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AkAudioEvent_PlayEvent", "Play Event"),
		LOCTEXT("AkAudioEvent_PlayEventTooltip", "Plays the selected event."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioEvent::PlayEvent, Events),
			FCanExecuteAction()
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AkAudioEvent_StopEvent", "Stop Event"),
		LOCTEXT("AkAudioEvent_StopEventTooltip", "Stops the selected event."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_AkAudioEvent::StopEvent, Events),
			FCanExecuteAction()
		)
	);
}

void FAssetTypeActions_AkAudioEvent::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}

bool FAssetTypeActions_AkAudioEvent::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
{
	if (ActivationType == EAssetTypeActivationMethod::DoubleClicked || ActivationType == EAssetTypeActivationMethod::Opened)
	{
		if (InObjects.Num() == 1)
		{
			return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(InObjects[0]);
		}
		else if (InObjects.Num() > 1)
		{
			return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(InObjects);
		}
	}
	else if (ActivationType == EAssetTypeActivationMethod::Previewed)
	{
		auto Events = GetTypedWeakObjectPtrs<UAkAudioEvent>(InObjects);
		FAkAssetTypeActions_Helpers::PlayEvents<true>(Events);
	}

	return true;
}

void FAssetTypeActions_AkAudioEvent::PlayEvent(TArray<TWeakObjectPtr<UAkAudioEvent>> Objects)
{
	FAkAssetTypeActions_Helpers::PlayEvents<false>(Objects);
}

void FAssetTypeActions_AkAudioEvent::StopEvent(TArray<TWeakObjectPtr<UAkAudioEvent>> Objects)
{
	FAkAudioDevice * AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice)
	{
		AudioDevice->StopGameObject(nullptr);
	}
}

//////////////////////////////////////////////////////////////////////////
// FAssetTypeActions_AkAuxBus

void FAssetTypeActions_AkAuxBus::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}

void FAssetTypeActions_AkAuxBus::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto AuxBusses = GetTypedWeakObjectPtrs<UAkAuxBus>(InObjects);
}
#undef LOCTEXT_NAMESPACE
