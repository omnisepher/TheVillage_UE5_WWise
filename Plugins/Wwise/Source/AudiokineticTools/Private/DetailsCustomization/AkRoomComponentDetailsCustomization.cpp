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

#include "AkRoomComponentDetailsCustomization.h"
#include "AkRoomComponent.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "AudiokineticTools"


//////////////////////////////////////////////////////////////////////////
// FAkRoomComponentDetailsCustomization

FAkRoomComponentDetailsCustomization::FAkRoomComponentDetailsCustomization()
{
}

TSharedRef<IDetailCustomization> FAkRoomComponentDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FAkRoomComponentDetailsCustomization());
}

void FAkRoomComponentDetailsCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder)
{
	InDetailBuilder->EditCategory("EnableComponent", FText::GetEmpty(), ECategoryPriority::Important);
	InDetailBuilder->EditCategory("Room", FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	InDetailBuilder->EditCategory("AkEvent", FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	DetailBuilder = InDetailBuilder;

	CustomizeDetails(*InDetailBuilder);
}

void FAkRoomComponentDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	InDetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	for (TWeakObjectPtr<UObject>& Object : ObjectsBeingCustomized)
	{
		UAkRoomComponent* RoomBeingCustomized = Cast<UAkRoomComponent>(Object.Get());
		if (RoomBeingCustomized)
		{
			UObject* OuterObj = RoomBeingCustomized->GetOuter();
			UActorComponent* OuterComponent = Cast<UActorComponent>(OuterObj);
			AActor* OuterActor = Cast<AActor>(OuterObj);
			// Do not hide the transform if the component has been created from within a component or actor, as this will hide the transform for that component / actor as well
			// (i.e. - only hide the transform if the component has been added to the hierarchy of a blueprint class or actor instance from the editor)
			if (OuterComponent == nullptr && OuterActor == nullptr)
			{
				IDetailCategoryBuilder& TransformCategory = InDetailBuilder.EditCategory("TransformCommon", LOCTEXT("TransformCommonCategory", "Transform"), ECategoryPriority::Transform);
				TransformCategory.SetCategoryVisibility(false);
				break;
			}
		}
	}

	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	UAkRoomComponent* RoomBeingCustomized = Cast<UAkRoomComponent>(ObjectsBeingCustomized[0].Get());
	if (RoomBeingCustomized)
	{
		IDetailCategoryBuilder& ToggleDetailCategory = InDetailBuilder.EditCategory("EnableComponent");
		auto EnableHandle = InDetailBuilder.GetProperty("bEnable");
		EnableHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FAkRoomComponentDetailsCustomization::OnEnableValueChanged));

		if (!RoomBeingCustomized->bEnable)
		{
			InDetailBuilder.HideCategory("Room");
			InDetailBuilder.HideCategory("AkEvent");
			InDetailBuilder.HideCategory("ReverbZone");
		}
	}
}

void FAkRoomComponentDetailsCustomization::OnEnableValueChanged()
{
	if (DetailBuilder.IsValid())
	{
		IDetailLayoutBuilder* Layout = nullptr;
		if (auto LockedDetailBuilder = DetailBuilder.Pin())
		{
			Layout = LockedDetailBuilder.Get();
		}
		if (LIKELY(Layout))
		{
			Layout->ForceRefreshDetails();
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE