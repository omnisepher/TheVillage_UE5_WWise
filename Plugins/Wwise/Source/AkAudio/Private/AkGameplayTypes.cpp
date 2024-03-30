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
	AkAudioClasses.cpp:
=============================================================================*/

#include "AkGameplayTypes.h"

#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkCallbackInfoPool.h"
#include "AkComponent.h"
#include "WwiseUnrealDefines.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/GameEngine.h"
#include "EngineUtils.h"
#include "AkCallbackInfoPool.h"
#include "HAL/PlatformString.h"

UAkCallbackInfo* AkCallbackTypeHelpers::GetBlueprintableCallbackInfo(EAkCallbackType CallbackType, AkCallbackInfo* CallbackInfo)
{
	switch (CallbackType)
	{
	case EAkCallbackType::EndOfEvent:
		return UAkEventCallbackInfo::Create((AkEventCallbackInfo*)CallbackInfo);
	case EAkCallbackType::Marker:
		return UAkMarkerCallbackInfo::Create((AkMarkerCallbackInfo*)CallbackInfo);
	case EAkCallbackType::Duration:
		return UAkDurationCallbackInfo::Create((AkDurationCallbackInfo*)CallbackInfo);
	case EAkCallbackType::Starvation:
		return UAkEventCallbackInfo::Create((AkEventCallbackInfo*)CallbackInfo);
	case EAkCallbackType::MusicPlayStarted:
		return UAkEventCallbackInfo::Create((AkEventCallbackInfo*)CallbackInfo);
	case EAkCallbackType::MusicSyncBeat:
	case EAkCallbackType::MusicSyncBar:
	case EAkCallbackType::MusicSyncEntry:
	case EAkCallbackType::MusicSyncExit:
	case EAkCallbackType::MusicSyncGrid:
	case EAkCallbackType::MusicSyncUserCue:
	case EAkCallbackType::MusicSyncPoint:
		return UAkMusicSyncCallbackInfo::Create((AkMusicSyncCallbackInfo*)CallbackInfo);
	case EAkCallbackType::MIDIEvent:
		return UAkMIDIEventCallbackInfo::Create((AkMIDIEventCallbackInfo*)CallbackInfo);
	default: 
		return nullptr;
	}
	return nullptr;
}

AkCallbackInfo* AkCallbackTypeHelpers::CopyWwiseCallbackInfo(AkCallbackType CallbackType, AkCallbackInfo* SourceCallbackInfo)
{
	switch (CallbackType)
	{
	case AK_EndOfEvent:
	case AK_Starvation:
	case AK_MusicPlayStarted:
	{
		AkEventCallbackInfo* CbInfoCopy = (AkEventCallbackInfo*)FMemory::Malloc(sizeof(AkEventCallbackInfo));
		FMemory::Memcpy(CbInfoCopy, SourceCallbackInfo, sizeof(AkEventCallbackInfo));
		return CbInfoCopy;
	}
	case AK_Marker:
	{
		const char* SourceLabel = ((AkMarkerCallbackInfo*)SourceCallbackInfo)->strLabel;
		int32 LabelSize = SourceLabel ? FPlatformString::Strlen(SourceLabel) + 1 : 0;
		AkMarkerCallbackInfo* CbInfoCopy = (AkMarkerCallbackInfo*)FMemory::Malloc(sizeof(AkMarkerCallbackInfo) + LabelSize);
		FMemory::Memcpy(CbInfoCopy, SourceCallbackInfo, sizeof(AkMarkerCallbackInfo));

		if (SourceLabel)
		{
			CbInfoCopy->strLabel = reinterpret_cast<const char*>(CbInfoCopy) + sizeof(AkMarkerCallbackInfo);
			FPlatformString::Strcpy(const_cast<char*>(CbInfoCopy->strLabel), LabelSize - 1, SourceLabel);
		}
		return CbInfoCopy;
	}
	case AK_Duration:
	{
		AkDurationCallbackInfo* CbInfoCopy = (AkDurationCallbackInfo*)FMemory::Malloc(sizeof(AkDurationCallbackInfo));
		FMemory::Memcpy(CbInfoCopy, SourceCallbackInfo, sizeof(AkDurationCallbackInfo));
		return CbInfoCopy;
	}
	case AK_MusicSyncBeat:
	case AK_MusicSyncBar:
	case AK_MusicSyncEntry:
	case AK_MusicSyncExit:
	case AK_MusicSyncGrid:
	case AK_MusicSyncUserCue:
	case AK_MusicSyncPoint:
	{
		const char* SourceUserCue = ((AkMusicSyncCallbackInfo*)SourceCallbackInfo)->pszUserCueName;
		int32 UserCueSize = SourceUserCue ? FPlatformString::Strlen(SourceUserCue) + 1 : 0;
		AkMusicSyncCallbackInfo* CbInfoCopy = (AkMusicSyncCallbackInfo*)FMemory::Malloc(sizeof(AkMusicSyncCallbackInfo) + UserCueSize);
		FMemory::Memcpy(CbInfoCopy, SourceCallbackInfo, sizeof(AkMusicSyncCallbackInfo));

		//SourceUserCue is either null or a non-empty string
		if (SourceUserCue)
		{
			CbInfoCopy->pszUserCueName = reinterpret_cast<char*>(CbInfoCopy) + sizeof(AkMusicSyncCallbackInfo);
			FPlatformString::Strcpy(const_cast<char*>(CbInfoCopy->pszUserCueName), UserCueSize, SourceUserCue);
		}
		return CbInfoCopy;
	}
	case AK_MIDIEvent:
	{
		AkMIDIEventCallbackInfo* CbInfoCopy = (AkMIDIEventCallbackInfo*)FMemory::Malloc(sizeof(AkMIDIEventCallbackInfo));
		FMemory::Memcpy(CbInfoCopy, SourceCallbackInfo, sizeof(AkMIDIEventCallbackInfo));
		return CbInfoCopy;
	}
	default:
		return nullptr;
	}
	return nullptr;
}

AkCallbackType AkCallbackTypeHelpers::GetCallbackMaskFromBlueprintMask(int32 BlueprintCallbackType)
{
	return (AkCallbackType)BlueprintCallbackType;
}

EAkCallbackType AkCallbackTypeHelpers::GetBlueprintCallbackTypeFromAkCallbackType(AkCallbackType CallbackType)
{
	uint32 BitIndex = 0;
	uint32 CbType = (uint32)CallbackType >> 1;
	while (CbType != 0)
	{
		CbType >>= 1;
		BitIndex++;
	}
	return (EAkCallbackType)BitIndex;
}

UAkCallbackInfo::UAkCallbackInfo( class FObjectInitializer const & ObjectInitializer) :
	Super(ObjectInitializer)
{}

UAkCallbackInfo* UAkCallbackInfo::Create(AkGameObjectID GameObjectID)
{
	auto CbInfo = FAkAudioDevice::Get()->GetAkCallbackInfoPool()->Acquire<UAkCallbackInfo>();
	if (CbInfo)
	{
		CbInfo->AkComponent = UAkComponent::GetAkComponent(GameObjectID);
	}
	return CbInfo;
}

void UAkCallbackInfo::Reset()
{
	AkComponent = nullptr;
}

UAkEventCallbackInfo::UAkEventCallbackInfo(class FObjectInitializer const & ObjectInitializer) :
	Super(ObjectInitializer)
{}

UAkEventCallbackInfo* UAkEventCallbackInfo::Create(AkEventCallbackInfo* AkEventCbInfo)
{
	auto CbInfo = FAkAudioDevice::Get()->GetAkCallbackInfoPool()->Acquire<UAkEventCallbackInfo>();
	if (CbInfo)
	{
		CbInfo->AkComponent = UAkComponent::GetAkComponent(AkEventCbInfo->gameObjID);
		CbInfo->PlayingID = AkEventCbInfo->playingID;
		CbInfo->EventID = AkEventCbInfo->eventID;
	}
	return CbInfo;
}

UAkMIDIEventCallbackInfo::UAkMIDIEventCallbackInfo(class FObjectInitializer const & ObjectInitializer) :
	Super(ObjectInitializer)
{}

UAkMIDIEventCallbackInfo* UAkMIDIEventCallbackInfo::Create(AkMIDIEventCallbackInfo* AkMIDIEventCbInfo)
{
	auto CbInfo = FAkAudioDevice::Get()->GetAkCallbackInfoPool()->Acquire<UAkMIDIEventCallbackInfo>();
	if (CbInfo)
	{
		CbInfo->AkComponent = UAkComponent::GetAkComponent(AkMIDIEventCbInfo->gameObjID);
		CbInfo->PlayingID = AkMIDIEventCbInfo->playingID;
		CbInfo->EventID = AkMIDIEventCbInfo->eventID;
		CbInfo->AkMidiEvent = AkMIDIEventCbInfo->midiEvent;
	}
	return CbInfo;
}

UAkMarkerCallbackInfo::UAkMarkerCallbackInfo(class FObjectInitializer const & ObjectInitializer) :
	Super(ObjectInitializer)
{}

UAkMarkerCallbackInfo* UAkMarkerCallbackInfo::Create(AkMarkerCallbackInfo* AkMarkerCbInfo)
{
	auto CbInfo = FAkAudioDevice::Get()->GetAkCallbackInfoPool()->Acquire<UAkMarkerCallbackInfo>();
	if (CbInfo)
	{
		CbInfo->AkComponent = UAkComponent::GetAkComponent(AkMarkerCbInfo->gameObjID);
		CbInfo->PlayingID = AkMarkerCbInfo->playingID;
		CbInfo->EventID = AkMarkerCbInfo->eventID;
		CbInfo->Identifier = AkMarkerCbInfo->uIdentifier;
		CbInfo->Position = AkMarkerCbInfo->uPosition;
		CbInfo->Label = FString(AkMarkerCbInfo->strLabel);
	}
	return CbInfo;
}

UAkDurationCallbackInfo::UAkDurationCallbackInfo(class FObjectInitializer const & ObjectInitializer) :
	Super(ObjectInitializer)
{}

UAkDurationCallbackInfo* UAkDurationCallbackInfo::Create(AkDurationCallbackInfo* AkDurationCbInfo)
{
	auto CbInfo = FAkAudioDevice::Get()->GetAkCallbackInfoPool()->Acquire<UAkDurationCallbackInfo>();
	if (CbInfo)
	{
		CbInfo->AkComponent = UAkComponent::GetAkComponent(AkDurationCbInfo->gameObjID);
		CbInfo->PlayingID = AkDurationCbInfo->playingID;
		CbInfo->EventID = AkDurationCbInfo->eventID;
		CbInfo->Duration = AkDurationCbInfo->fDuration;
		CbInfo->EstimatedDuration = AkDurationCbInfo->fEstimatedDuration;
		CbInfo->AudioNodeID = AkDurationCbInfo->audioNodeID;
		CbInfo->MediaID = AkDurationCbInfo->mediaID;
		CbInfo->bStreaming = AkDurationCbInfo->bStreaming;
	}
	return CbInfo;
}

UAkMusicSyncCallbackInfo::UAkMusicSyncCallbackInfo(class FObjectInitializer const & ObjectInitializer) :
	Super(ObjectInitializer)
{}

UAkMusicSyncCallbackInfo* UAkMusicSyncCallbackInfo::Create(AkMusicSyncCallbackInfo* AkMusicCbInfo)
{
	auto CbInfo = FAkAudioDevice::Get()->GetAkCallbackInfoPool()->Acquire<UAkMusicSyncCallbackInfo>();
	if (CbInfo)
	{
		CbInfo->AkComponent = UAkComponent::GetAkComponent(AkMusicCbInfo->gameObjID);
		CbInfo->PlayingID = AkMusicCbInfo->playingID;
		CbInfo->SegmentInfo = AkMusicCbInfo->segmentInfo;
		CbInfo->MusicSyncType = AkCallbackTypeHelpers::GetBlueprintCallbackTypeFromAkCallbackType(AkMusicCbInfo->musicSyncType);
		CbInfo->UserCueName = FString(AkMusicCbInfo->pszUserCueName);
	}
	return CbInfo;
}

EAkMidiEventType UAkMIDIEventCallbackInfo::GetType()
{
	return (EAkMidiEventType)AkMidiEvent.byType;
}

uint8 UAkMIDIEventCallbackInfo::GetChannel()
{
	// Add one here so we report "Artist" channel number (between 1 and 16), instead of reporting the underlying value of 0-F.
	return AkMidiEvent.byChan + 1;
}

bool UAkMIDIEventCallbackInfo::GetGeneric(FAkMidiGeneric& AsGeneric)
{
	AsGeneric = FAkMidiGeneric(AkMidiEvent);
	return true;
}

bool UAkMIDIEventCallbackInfo::GetNoteOn(FAkMidiNoteOnOff& AsNoteOn)
{
	if (GetType() != EAkMidiEventType::AkMidiEventTypeNoteOn)
	{
		return false;
	}

	AsNoteOn = FAkMidiNoteOnOff(AkMidiEvent);
	return true;
}

bool UAkMIDIEventCallbackInfo::GetNoteOff(FAkMidiNoteOnOff& AsNoteOff)
{
	if (GetType() != EAkMidiEventType::AkMidiEventTypeNoteOff)
	{
		return false;
	}

	AsNoteOff = FAkMidiNoteOnOff(AkMidiEvent);
	return true;
}

bool UAkMIDIEventCallbackInfo::GetCc(FAkMidiCc& AsCc)
{
	if (GetType() != EAkMidiEventType::AkMidiEventTypeController)
	{
		return false;
	}

	AsCc = FAkMidiCc(AkMidiEvent);
	return true;
}

bool UAkMIDIEventCallbackInfo::GetPitchBend(FAkMidiPitchBend& AsPitchBend)
{
	if (GetType() != EAkMidiEventType::AkMidiEventTypePitchBend)
	{
		return false;
	}

	AsPitchBend = FAkMidiPitchBend(AkMidiEvent);
	return true;
}

bool UAkMIDIEventCallbackInfo::GetNoteAftertouch(FAkMidiNoteAftertouch& AsNoteAftertouch)
{
	if (GetType() != EAkMidiEventType::AkMidiEventTypeNoteAftertouch)
	{
		return false;
	}

	AsNoteAftertouch = FAkMidiNoteAftertouch(AkMidiEvent);
	return true;
}

bool UAkMIDIEventCallbackInfo::GetChannelAftertouch(FAkMidiChannelAftertouch& AsChannelAftertouch)
{
	if (GetType() != EAkMidiEventType::AkMidiEventTypeChannelAftertouch)
	{
		return false;
	}

	AsChannelAftertouch = FAkMidiChannelAftertouch(AkMidiEvent);
	return true;
}

bool UAkMIDIEventCallbackInfo::GetProgramChange(FAkMidiProgramChange& AsProgramChange)
{
	if (GetType() != EAkMidiEventType::AkMidiEventTypeProgramChange)
	{
		return false;
	}

	AsProgramChange = FAkMidiProgramChange(AkMidiEvent);
	return true;
}

FAkSDKExternalSourceArray::FAkSDKExternalSourceArray(const TArray<FAkExternalSourceInfo>& BlueprintArray)
{
	for (auto& ExternalSourceInfo : BlueprintArray)
	{
		AkOSChar* OsCharArray = nullptr;
		void* MediaData = nullptr;
		AkUInt32 MediaSize = 0;

		if (ExternalSourceInfo.ExternalSourceAsset)
		{
			UE_LOG(LogAkAudio, Error, TEXT("FAkSDKExternalSourceArray: ExternalSourceAssets are not supported. Please migrate your project and use AkAudioEvent."));
			return;
		}
		else
		{
			auto ExternalFileName = ExternalSourceInfo.FileName;
			if (FPaths::GetExtension(ExternalFileName).IsEmpty())
			{
				ExternalFileName += TEXT(".wem");
			}
			OsCharArray = (AkOSChar*)FMemory::Malloc((ExternalFileName.Len() + 1) * sizeof(AkOSChar));
			FPlatformString::Strcpy(OsCharArray, ExternalFileName.Len(), TCHAR_TO_AK(*(ExternalFileName)));

			ExternalSourceArray.Emplace(OsCharArray, FAkAudioDevice::GetShortIDFromString(ExternalSourceInfo.ExternalSrcName), (AkCodecID)ExternalSourceInfo.CodecID);
		}
	}
}

FAkSDKExternalSourceArray::~FAkSDKExternalSourceArray()
{
}

void FWaitEndOfEventAsyncAction::UpdateOperation(FLatentResponse& Response)
{
	if (FuturePlayingID.IsReady())
	{
		*PlayingID = FuturePlayingID.Get();
		if (*PlayingID == AK_INVALID_PLAYING_ID)
		{
			EventFinished = true;
		}

		if (EventFinished)
		{
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
}

AkDeviceAndWorld::AkDeviceAndWorld(AActor* in_pActor) :
	AkAudioDevice(FAkAudioDevice::Get()),
	CurrentWorld(in_pActor ? in_pActor->GetWorld() : nullptr)
{
}

AkDeviceAndWorld::AkDeviceAndWorld(const UObject* in_pWorldContextObject) :
	AkAudioDevice(FAkAudioDevice::Get()),
	CurrentWorld(GEngine->GetWorldFromContextObject(in_pWorldContextObject, EGetWorldErrorMode::ReturnNull))
{}

bool AkDeviceAndWorld::IsValid() const
{
	return (CurrentWorld && CurrentWorld->AllowAudioPlayback() && AkAudioDevice);
}