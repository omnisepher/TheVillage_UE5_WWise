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

#include "AkGeometryComponentDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "Editor/TransBuffer.h"
#include "UI/SAkGeometrySurfaceOverrideController.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"

#include "AkGeometryComponent.h"

#define LOCTEXT_NAMESPACE "AudiokineticTools"

static int SurfacePropertiesHeight = 72;

//////////////////////////////////////////////////////////////////////////
// FAkGeometryDetailsCustomization

FAkGeometryComponentDetailsCustomization::FAkGeometryComponentDetailsCustomization()
{
	ComponentBeingCustomized = nullptr;
}

FAkGeometryComponentDetailsCustomization::~FAkGeometryComponentDetailsCustomization()
{
	if (ComponentBeingCustomized.IsValid() && ComponentBeingCustomized->GetOnRefreshDetails())
	{
		if (ComponentBeingCustomized->GetOnRefreshDetails()->IsBoundToObject(this))
		{
			ComponentBeingCustomized->ClearOnRefreshDetails();
		}
	}

	ComponentBeingCustomized.Reset();
}

void FAkGeometryComponentDetailsCustomization::BeginModify(FText TransactionText)
{
	if (GEditor && GEditor->Trans)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		if (TransBuffer != nullptr)
			TransBuffer->Begin(*FString("AkGeometry Acoustic Surfaces"), TransactionText);
	}

	if (ComponentBeingCustomized.IsValid())
		ComponentBeingCustomized->Modify();
}

void FAkGeometryComponentDetailsCustomization::EndModify()
{
	if (GEditor && GEditor->Trans)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		if (TransBuffer != nullptr)
			TransBuffer->End();
	}
}

TSharedRef<IDetailCustomization> FAkGeometryComponentDetailsCustomization::MakeInstance()
{
	return MakeShared<FAkGeometryComponentDetailsCustomization>();
}

void FAkGeometryComponentDetailsCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder)
{
	DetailBuilder = InDetailBuilder;
	CustomizeDetails(*InDetailBuilder);
}

void FAkGeometryComponentDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	InDetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	InDetailBuilder.EditCategory("Geometry", FText::GetEmpty(), ECategoryPriority::TypeSpecific);

	for (TWeakObjectPtr<UObject>& Object : ObjectsBeingCustomized)
	{
		UAkGeometryComponent* GeometryComponentBeingCustomized = Cast<UAkGeometryComponent>(Object.Get());
		if (GeometryComponentBeingCustomized)
		{
			UObject* OuterObj = GeometryComponentBeingCustomized->GetOuter();
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

	auto Component = Cast<UAkGeometryComponent>(ObjectsBeingCustomized[0].Get());
	if (Component == nullptr)
	{
		return;
	}

	auto LockedDetailBuilder = DetailBuilder.Pin();
	if (UNLIKELY(!LockedDetailBuilder))
	{
		return;
	}

	ComponentBeingCustomized = TWeakObjectPtr<UAkGeometryComponent>(Component);
	auto meshTypeChangedHandle = LockedDetailBuilder->GetProperty("MeshType");
	meshTypeChangedHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FAkGeometryComponentDetailsCustomization::RefreshDetails));

	FOnRefreshDetails refreshDetails = FOnRefreshDetails::CreateSP(this, &FAkGeometryComponentDetailsCustomization::RefreshDetails);
	ComponentBeingCustomized->SetOnRefreshDetails(refreshDetails);

	if (ComponentBeingCustomized->MeshType == AkMeshType::StaticMesh)
	{
		InDetailBuilder.HideProperty("CollisionMeshSurfaceOverride");
		if (ObjectsBeingCustomized.Num() == 1)
		{
			IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Surface Overrides", FText::GetEmpty(), ECategoryPriority::TypeSpecific);

			InDetailBuilder.HideProperty("StaticMeshSurfaceOverride");
			auto SurfacesPropHandle = InDetailBuilder.GetProperty("StaticMeshSurfaceOverride");
			CategoryBuilder.HeaderContent
			(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(FText::FromString("Reset to Defaults"))
					.ToolTipText(FText::FromString(FString("Reset the surface properties to their default values.")))
					.OnClicked_Lambda([this, SurfacesPropHandle]()
						{
							BeginModify(FText::FromString(FString("Reset surface properties to defaults")));
							if (ComponentBeingCustomized->HasAnyFlags(RF_ArchetypeObject)
								|| ComponentBeingCustomized->CreationMethod == EComponentCreationMethod::Instance)
							{
								TArray<UMaterialInterface*> Materials;
								ComponentBeingCustomized->StaticMeshSurfaceOverride.GetKeys(Materials);
								for (UMaterialInterface* Material : Materials)
								{
									ComponentBeingCustomized->StaticMeshSurfaceOverride[Material].AcousticTexture = nullptr;
									ComponentBeingCustomized->StaticMeshSurfaceOverride[Material].bEnableOcclusionOverride = false;
									ComponentBeingCustomized->StaticMeshSurfaceOverride[Material].OcclusionValue = 1.0f;
								}
							}
							else
							{
								ComponentBeingCustomized->StaticMeshSurfaceOverride.Empty();
								SurfacesPropHandle->ResetToDefault();
							}
							EndModify();
							return FReply::Handled();
						})
				]
			);
			ComponentBeingCustomized->UpdateStaticMeshOverride();
			TArray<UMaterialInterface*> Materials;
			ComponentBeingCustomized->StaticMeshSurfaceOverride.GetKeys(Materials);
			for (UMaterialInterface* Material : Materials)
			{
				FDetailWidgetRow& SurfacesRow = CategoryBuilder.AddCustomRow(FText::FromString("Texture Surface Occlusion"));
				SurfacesRow.NameContent()
				[
					SNew(SBox)
					.HeightOverride(SurfacePropertiesHeight)
					.ToolTipText(FText::FromString("The material(s) in this list are populated using the materials assigned to the AkGeometry Component's Static Mesh parent Component."))
					[
						SNew(SObjectPropertyEntryBox)
						.AllowedClass(UMaterialInterface::StaticClass())
						.ObjectPath_Lambda([Material]() { return FSoftObjectPath(Material).GetAssetPathString(); })
						.ToolTipText(FText::FromString("The material(s) in this list are populated using the materials assigned to the AkGeometry Component's Static Mesh parent Component."))
						.IsEnabled(false)
					]
				];
				SurfacesRow.ValueContent()
				[
					SNew(SBox)
					.HeightOverride(SurfacePropertiesHeight)
					[
						SNew(SAkGeometryStaticMeshSurfaceController, ObjectsBeingCustomized[0], LockedDetailBuilder, Material)
					]
				];
			}
		}
	}
	else if (ComponentBeingCustomized->MeshType == AkMeshType::CollisionMesh)
	{
		InDetailBuilder.HideProperty("LOD");
		InDetailBuilder.HideProperty("WeldingThreshold");
		InDetailBuilder.HideProperty("StaticMeshSurfaceOverride");

		if (ObjectsBeingCustomized.Num() == 1)
		{
			IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Surface Overrides", FText::GetEmpty(), ECategoryPriority::TypeSpecific);

			auto SurfacesPropHandle = InDetailBuilder.GetProperty("CollisionMeshSurfaceOverride");
			InDetailBuilder.HideProperty("CollisionMeshSurfaceOverride");

			CategoryBuilder.HeaderContent
			(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(FText::FromString("Reset to Defaults"))
					.ToolTipText(FText::FromString(FString("Reset the surface properties to their default values.")))
					.OnClicked_Lambda([this, SurfacesPropHandle]() 
					{
						BeginModify(FText::FromString(FString("Reset surface properties to defaults")));
						if (ComponentBeingCustomized->HasAnyFlags(RF_ArchetypeObject)
							|| ComponentBeingCustomized->CreationMethod == EComponentCreationMethod::Instance)
						{
							ComponentBeingCustomized->CollisionMeshSurfaceOverride.AcousticTexture = nullptr;
							ComponentBeingCustomized->CollisionMeshSurfaceOverride.bEnableOcclusionOverride = false;
							ComponentBeingCustomized->CollisionMeshSurfaceOverride.OcclusionValue = 1.0f;
						}
						else
						{
							SurfacesPropHandle->ResetToDefault();
						}
						EndModify();
						return FReply::Handled();
					})
				]
			);

			FDetailWidgetRow& SurfacesRow = CategoryBuilder.AddCustomRow(FText::FromString("Texture Surface Occlusion"));

			SurfacesRow.ValueContent()
			[
				SNew(SBox)
				.HeightOverride(SurfacePropertiesHeight)
				[
					SNew(SAkGeometryCollisionMeshSurfaceController, ObjectsBeingCustomized[0], LockedDetailBuilder)
				]
			];
		}
	}
}

void FAkGeometryComponentDetailsCustomization::RefreshDetails()
{
	if (ComponentBeingCustomized.IsValid())
	{
		ComponentBeingCustomized->ClearOnRefreshDetails();
	}

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