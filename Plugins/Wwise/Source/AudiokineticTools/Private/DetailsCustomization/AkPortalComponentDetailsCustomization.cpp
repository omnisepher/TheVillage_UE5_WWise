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

#include "AkPortalComponentDetailsCustomization.h"
#include "AkAcousticPortal.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "AudiokineticTools"


//////////////////////////////////////////////////////////////////////////
// FAkPortalComponentDetailsCustomization

FAkPortalComponentDetailsCustomization::FAkPortalComponentDetailsCustomization()
{
}

TSharedRef<IDetailCustomization> FAkPortalComponentDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FAkPortalComponentDetailsCustomization());
}

void FAkPortalComponentDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	DetailLayout.EditCategory("Fit to Geometry", FText::GetEmpty(), ECategoryPriority::Important);
	DetailLayout.EditCategory("AkPortalComponent", FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	
	for (TWeakObjectPtr<UObject>& Object : ObjectsBeingCustomized)
	{
		UAkPortalComponent* PortalBeingCustomized = Cast<UAkPortalComponent>(Object.Get());
		if (PortalBeingCustomized)
		{
			UObject* OuterObj = PortalBeingCustomized->GetOuter();
			UActorComponent* OuterComponent = Cast<UActorComponent>(OuterObj);
			AActor* OuterActor = Cast<AActor>(OuterObj);
			// Do not hide the transform if the component has been created from within a component or actor, as this will hide the transform for that component / actor as well
			// (i.e. - only hide the transform if the component has been added to the hierarchy of a blueprint class or actor instance from the editor)
			if (OuterComponent == nullptr && OuterActor == nullptr)
			{
				IDetailCategoryBuilder& TransformCategory = DetailLayout.EditCategory("TransformCommon", LOCTEXT("TransformCommonCategory", "Transform"), ECategoryPriority::Transform);
				TransformCategory.SetCategoryVisibility(false);
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE