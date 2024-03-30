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

#include "Containers/UnrealString.h"
#include "Engine/EngineTypes.h"

namespace EWwiseItemType
{
	UENUM()
	enum Type
	{
		Event,
		AuxBus,
		AcousticTexture,
		State,
		Switch,
		GameParameter,
		Trigger,
		EffectShareSet,
		ActorMixer,
		Bus,
		Project,
		StandaloneWorkUnit,
		NestedWorkUnit,
		PhysicalFolder,
		Folder,
		Sound,
		SwitchContainer,
		RandomSequenceContainer,
		BlendContainer,
		MotionBus,
		StateGroup,
		SwitchGroup,
		InitBank,
		LastWwiseBrowserType = EffectShareSet,

		None = -1,
	};

	static const FString EventsBrowserName = TEXT("Events");
	static const FString BussesBrowserName = TEXT("Busses");
	static const FString AcousticTexturesBrowserName = TEXT("AcousticTextures");
	static const FString StatesBrowserName = TEXT("States");
	static const FString SwitchesBrowserName = TEXT("Switches");
	static const FString GameParametersBrowserName = TEXT("GameParameters");
	static const FString TriggersBrowserName = TEXT("Triggers");
	static const FString ShareSetsBrowserName =	TEXT("Effect ShareSets");
	static const FString OrphanAssetsBrowserName = TEXT("Orphan Assets");

	//Name to show in the Browser
	static const FString BrowserDisplayNames[] = {
		EventsBrowserName,
		BussesBrowserName,
		AcousticTexturesBrowserName,
		StatesBrowserName,
		SwitchesBrowserName,
		GameParametersBrowserName,
		TriggersBrowserName,
		ShareSetsBrowserName,
		OrphanAssetsBrowserName
	};

	//Tag in the work unit XML for this WwiseObjectType
	static const FString WorkUnitTagNames[] = {
		TEXT("Events"),
		TEXT("Busses"),
		TEXT("VirtualAcoustics"),
		TEXT("States"),
		TEXT("Switches"),
		TEXT("GameParameters"),
		TEXT("Triggers"),
		TEXT("Effects"),
	};

	//Name of the folder containing the work units of this WwiseObjectType
	static const FString FolderNames[] = {
		TEXT("Events"),
		TEXT("Master-Mixer Hierarchy"),
		TEXT("Virtual Acoustics"),
		TEXT("States"),
		TEXT("Switches"),
		TEXT("Game Parameters"),
		TEXT("Triggers"),
		TEXT("Effects"),
		TEXT("Actor-Mixer Hierarchy"),
	};

	static const TArray<FString> PhysicalFoldersToIgnore = {
		TEXT("Actor-Mixer Hierarchy"),
		TEXT("Attenuations"),
		TEXT("Audio Devices"),
		TEXT("Control Surface Sessions"),
		TEXT("Conversion Settings"),
		TEXT("Dynamic Dialogue"),
		TEXT("Interactive Music Hierarchy"),
		TEXT("Metadata"),
		TEXT("Mixing Sessions"),
		TEXT("Modulators"),
		TEXT("Presets"),
		TEXT("Queries"),
		TEXT("SoundBanks"),
		TEXT("Soundcaster Sessions"),
	};

	inline Type FromString(const FString& ItemName)
	{
		struct TypePair
		{
			FString Name;
			Type Value;
		};

		static const TypePair ValidTypes[] = {
			{TEXT("AcousticTexture"), Type::AcousticTexture},
			{TEXT("ActorMixer"), Type::ActorMixer},
			{TEXT("AuxBus"), Type::AuxBus},
			{TEXT("BlendContainer"), Type::BlendContainer},
			{TEXT("Bus"), Type::Bus},
			{TEXT("Event"), Type::Event},
			{TEXT("Folder"), Type::Folder},
			{TEXT("GameParameter"), Type::GameParameter},
			{TEXT("MotionBus"), Type::MotionBus},
			{TEXT("PhysicalFolder"), Type::PhysicalFolder},
			{TEXT("Project"), Type::Project},
			{TEXT("RandomSequenceContainer"), Type::RandomSequenceContainer},
			{TEXT("Sound"), Type::Sound},
			{TEXT("State"), Type::State},
			{TEXT("StateGroup"), Type::StateGroup},
			{TEXT("Switch"), Type::Switch},
			{TEXT("SwitchContainer"), Type::SwitchContainer},
			{TEXT("SwitchGroup"), Type::SwitchGroup},
			{TEXT("Trigger"), Type::Trigger},
			{TEXT("WorkUnit"), Type::StandaloneWorkUnit},
			{TEXT("Effect"), Type::EffectShareSet},

		};

		for (const auto& type : ValidTypes)
		{
			if (type.Name == ItemName)
			{
				return type.Value;
			}
		}

		return Type::None;
	}

	inline Type FromFolderName(const FString& ItemName)
	{
		struct TypePair
		{
			FString Name;
			Type Value;
		};

		static const TypePair ValidTypes[] = {
			{TEXT("Virtual Acoustics"), Type::AcousticTexture},
			{TEXT("Master-Mixer Hierarchy"), Type::AuxBus},
			{TEXT("Events"), Type::Event},
			{TEXT("Game Parameters"), Type::GameParameter},
			{TEXT("States"), Type::State},
			{TEXT("Switches"), Type::Switch},
			{TEXT("Triggers"), Type::Trigger},
			{TEXT("Effects"), Type::EffectShareSet},
		};

		for (const auto& type : ValidTypes)
		{
			if (type.Name == ItemName)
			{
				return type.Value;
			}
		}

		return Type::None;
	}
};
