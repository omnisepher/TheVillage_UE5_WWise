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

#include "Engine/EngineTypes.h"
#include "UObject/Object.h"

#include "AkInclude.h"
#include "AkAudioSession.generated.h"

UENUM()
enum class EAkAudioSessionCategory
{
    Ambient,
    SoloAmbient,
    PlayAndRecord,
};

UENUM(Meta = (Bitmask))
enum class EAkAudioSessionCategoryOptions : uint32
{
    MixWithOthers,
    DuckOthers,
    AllowBluetooth,
    DefaultToSpeaker,
};

UENUM()
enum class EAkAudioSessionMode
{
    Default,
    VoiceChat,
    GameChat,
    VideoRecording,
    Measurement,
    MoviePlayback,
    VideoChat,
};

struct FAkInitializationStructure;

USTRUCT()
struct FAkAudioSession
{
    GENERATED_BODY()

    UPROPERTY(Config, Category = "Ak Initialization Settings|Audio Session", EditAnywhere, meta = (ToolTip = "The IDs of the iOS audio session categories, useful for defining app-level audio behaviours such as inter-app audio mixing policies and audio routing behaviours.These IDs are functionally equivalent to the corresponding constants defined by the iOS audio session service back-end (AVAudioSession). Refer to Xcode documentation for details on the audio session categories."))
    EAkAudioSessionCategory AudioSessionCategory = EAkAudioSessionCategory::Ambient;

    UPROPERTY(Config, Category = "Ak Initialization Settings|Audio Session", EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/AkAudio.EAkAudioSessionCategoryOptions", ToolTip = "The IDs of the iOS audio session category options, used for customizing the audio session category features. These IDs are functionally equivalent to the corresponding constants defined by the iOS audio session service back-end (AVAudioSession). Refer to Xcode documentation for details on the audio session category options."))
    uint32 AudioSessionCategoryOptions = 1 << (uint32)EAkAudioSessionCategoryOptions::DuckOthers;

    UPROPERTY(Config, Category = "Ak Initialization Settings|Audio Session", EditAnywhere, meta = (ToolTip = "The IDs of the iOS audio session modes, used for customizing the audio session for typical app types. These IDs are functionally equivalent to the corresponding constants defined by the iOS audio session service back-end (AVAudioSession). Refer to Xcode documentation for details on the audio session category options."))
    EAkAudioSessionMode AudioSessionMode = EAkAudioSessionMode::Default;

    void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;
};
