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

#include "Wwise/Metadata/WwiseMetadataForwardDeclarations.h"
#include "Wwise/Ref/WwiseRefRootFile.h"
#include "Wwise/Ref/WwiseRefType.h"

class WWISEPROJECTDATABASE_API FWwiseAnyRef
{
public:
	TSharedPtr<FWwiseRefRootFile> Ref;

	template<typename EWwiseRefType>
	static FWwiseAnyRef Create(const EWwiseRefType& inRef)
	{
		return FWwiseAnyRef(new EWwiseRefType(inRef));
	}

	FWwiseAnyRef() :
		Ref()
	{}

	FWwiseAnyRef(FWwiseAnyRef&& InRef) :
		Ref(MoveTemp(InRef.Ref))
	{}

	FWwiseAnyRef(const FWwiseAnyRef& InRef) :
		Ref(InRef.Ref)
	{}

private:
	FWwiseAnyRef(FWwiseRefRootFile*&& InRef) :
		Ref(InRef)
	{
	}

public:
	~FWwiseAnyRef()
	{
	}

	EWwiseRefType GetType() const
	{
		if (!Ref)
		{
			return EWwiseRefType::None;
		}
		return (EWwiseRefType)Ref->Type();
	}
	operator bool() const { return Ref != nullptr; }
	bool IsValid() const { return Ref != nullptr; }

	const FWwiseRefLanguage* GetLanguageRef() const;
	const FWwiseRefPlatform* GetPlatformRef() const;
	const FWwiseRefPluginLib* GetPluginLibRef() const;
	const FWwiseRefSoundBank* GetSoundBankRef() const;
	const FWwiseRefMedia* GetMediaRef() const;
	const FWwiseRefCustomPlugin* GetCustomPluginRef() const;
	const FWwiseRefPluginShareSet* GetPluginShareSetRef() const;
	const FWwiseRefAudioDevice* GetAudioDeviceRef() const;
	const FWwiseRefEvent* GetEventRef() const;
	const FWwiseRefSwitchContainer* GetSwitchContainerRef() const;
	const FWwiseRefDialogueEvent* GetDialogueEventRef() const;
	const FWwiseRefDialogueArgument* GetDialogueArgumentRef() const;
	const FWwiseRefBus* GetBusRef() const;
	const FWwiseRefAuxBus* GetAuxBusRef() const;
	const FWwiseRefGameParameter* GetGameParameterRef() const;
	const FWwiseRefStateGroup* GetStateGroupRef() const;
	const FWwiseRefState* GetStateRef() const;
	const FWwiseRefSwitchGroup* GetSwitchGroupRef() const;
	const FWwiseRefSwitch* GetSwitchRef() const;
	const FWwiseRefTrigger* GetTriggerRef() const;
	const FWwiseRefExternalSource* GetExternalSourceRef() const;
	const FWwiseRefAcousticTexture* GetAcousticTextureRef() const;

	const FWwiseMetadataLanguage* GetLanguage() const;
	const FWwiseMetadataPlatform* GetPlatform() const;
	const FWwiseMetadataPlatformReference* GetPlatformReference() const;
	const FWwiseMetadataPluginLib* GetPluginLib() const;
	const FWwiseMetadataSoundBank* GetSoundBank() const;
	const FWwiseMetadataMedia* GetMedia() const;
	const FWwiseMetadataPlugin* GetCustomPlugin() const;
	const FWwiseMetadataPlugin* GetPluginShareSet() const;
	const FWwiseMetadataPlugin* GetAudioDevice() const;
	const FWwiseMetadataEvent* GetEvent() const;
	const FWwiseMetadataSwitchContainer* GetSwitchContainer() const;
	const FWwiseMetadataDialogueEvent* GetDialogueEvent() const;
	const FWwiseMetadataDialogueArgument* GetDialogueArgument() const;
	const FWwiseMetadataBus* GetBus() const;
	const FWwiseMetadataBus* GetAuxBus() const;
	const FWwiseMetadataGameParameter* GetGameParameter() const;
	const FWwiseMetadataStateGroup* GetStateGroup() const;
	const FWwiseMetadataState* GetState() const;
	const FWwiseMetadataSwitchGroup* GetSwitchGroup() const;
	const FWwiseMetadataSwitch* GetSwitch() const;
	const FWwiseMetadataTrigger* GetTrigger() const;
	const FWwiseMetadataExternalSource* GetExternalSource() const;
	const FWwiseMetadataAcousticTexture* GetAcousticTexture() const;

	bool GetRef(FWwiseRefLanguage& OutRef) const;
	bool GetRef(FWwiseRefPlatform& OutRef) const;
	bool GetRef(FWwiseRefPluginLib& OutRef) const;
	bool GetRef(FWwiseRefSoundBank& OutRef) const;
	bool GetRef(FWwiseRefMedia& OutRef) const;
	bool GetRef(FWwiseRefCustomPlugin& OutRef) const;
	bool GetRef(FWwiseRefPluginShareSet& OutRef) const;
	bool GetRef(FWwiseRefAudioDevice& OutRef) const;
	bool GetRef(FWwiseRefEvent& OutRef) const;
	bool GetRef(FWwiseRefSwitchContainer& OutRef) const;
	bool GetRef(FWwiseRefDialogueEvent& OutRef) const;
	bool GetRef(FWwiseRefDialogueArgument& OutRef) const;
	bool GetRef(FWwiseRefBus& OutRef) const;
	bool GetRef(FWwiseRefAuxBus& OutRef) const;
	bool GetRef(FWwiseRefGameParameter& OutRef) const;
	bool GetRef(FWwiseRefStateGroup& OutRef) const;
	bool GetRef(FWwiseRefState& OutRef) const;
	bool GetRef(FWwiseRefSwitchGroup& OutRef) const;
	bool GetRef(FWwiseRefSwitch& OutRef) const;
	bool GetRef(FWwiseRefTrigger& OutRef) const;
	bool GetRef(FWwiseRefExternalSource& OutRef) const;
	bool GetRef(FWwiseRefAcousticTexture& OutRef) const;

	FGuid GetGuid(const EWwiseRefType* TypeOverride = nullptr) const;
	uint32 GetGroupId(const EWwiseRefType* TypeOverride = nullptr) const;
	uint32 GetId(const EWwiseRefType* TypeOverride = nullptr) const;
	FName GetName(const EWwiseRefType* TypeOverride = nullptr) const;
	FName GetObjectPath(const EWwiseRefType* TypeOverride = nullptr) const;

	bool operator ==(const FWwiseAnyRef& Rhs) const
	{
		return GetType() == Rhs.GetType()
			&& operator bool() == Rhs.operator bool()
			&& (!operator bool()
				|| Ref->Hash() == Rhs.Ref->Hash());
	}

	bool operator !=(const FWwiseAnyRef& Rhs) const
	{
		return !(operator == (Rhs));
	}

};

inline uint32 GetTypeHash(const FWwiseAnyRef& InValue)
{
	if ((bool)InValue)
	{
		return InValue.Ref->Hash();
	}
	return 0;
}
