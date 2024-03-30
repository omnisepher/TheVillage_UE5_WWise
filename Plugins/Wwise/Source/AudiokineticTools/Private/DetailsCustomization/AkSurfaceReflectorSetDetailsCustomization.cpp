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

#include "AkSurfaceReflectorSetDetailsCustomization.h"
#include "AkComponent.h"
#include "AkSurfaceReflectorSetComponent.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorModeManager.h"
#include "EditorSupportDelegates.h"
#include "IPropertyUtilities.h"
#include "LevelEditorActions.h"
#include "Model.h"
#include "GameFramework/Volume.h"
#include "UI/SAcousticSurfacesController.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#include "Builders/ConeBuilder.h"
#include "Builders/CubeBuilder.h"
#include "Builders/CurvedStairBuilder.h"
#include "Builders/CylinderBuilder.h"
#include "Builders/LinearStairBuilder.h"
#include "Builders/SpiralStairBuilder.h"
#include "Builders/TetrahedronBuilder.h"


#define LOCTEXT_NAMESPACE "AudiokineticTools"


//////////////////////////////////////////////////////////////////////////
// FAkSurfaceReflectorSetDetailsCustomization

FAkSurfaceReflectorSetDetailsCustomization::FAkSurfaceReflectorSetDetailsCustomization()
{
	ReflectorSetBeingCustomized = nullptr;
	FCoreUObjectDelegates::OnObjectModified.AddRaw(this, &FAkSurfaceReflectorSetDetailsCustomization::OnObjectModified);
	FEditorSupportDelegates::RedrawAllViewports.AddRaw(this, &FAkSurfaceReflectorSetDetailsCustomization::OnRedrawViewports);
}

FAkSurfaceReflectorSetDetailsCustomization::~FAkSurfaceReflectorSetDetailsCustomization()
{
	FCoreUObjectDelegates::OnObjectModified.RemoveAll(this);
	FEditorSupportDelegates::RedrawAllViewports.RemoveAll(this);

	if (ReflectorSetBeingCustomized.IsValid() && ReflectorSetBeingCustomized->GetOnRefreshDetails())
	{
		if (ReflectorSetBeingCustomized->GetOnRefreshDetails()->IsBoundToObject(this))
		{
			ReflectorSetBeingCustomized->ClearOnRefreshDetails();
		}
	}

	ReflectorSetBeingCustomized.Reset();
}

FReply FAkSurfaceReflectorSetDetailsCustomization::OnEnableEditModeClicked()
{
	FLevelEditorActionCallbacks::OnShowOnlySelectedActors();
	GLevelEditorModeTools().ActivateMode(FEditorModeID(TEXT("EM_Geometry")), false);
	return FReply::Handled();
}

FReply FAkSurfaceReflectorSetDetailsCustomization::OnDisableEditModeClicked()
{
	GLevelEditorModeTools().DeactivateMode(FEditorModeID(TEXT("EM_Geometry")));
	FLevelEditorActionCallbacks::ExecuteExecCommand(FString(TEXT("ACTOR UNHIDE ALL")));
	return FReply::Handled();
}

void FAkSurfaceReflectorSetDetailsCustomization::OnObjectModified(UObject* Object)
{
	if (!SelectedObjectModifiedThisFrame)
	{
		for (TWeakObjectPtr<UObject> UObjectPtr : ObjectsBeingCustomized)
		{
			if (Object == UObjectPtr.Get())
			{
				SelectedObjectModifiedThisFrame = true;
				return;
			}
		}
	}
}

void FAkSurfaceReflectorSetDetailsCustomization::OnRedrawViewports()
{
	if (SelectedObjectModifiedThisFrame && DetailBuilder.IsValid())
	{
		// If there is any user interaction going on, we don't want to refresh the details panel.
		// (This would interrupt the interaction and make sliders unusable)
		for (TWeakObjectPtr<UObject> UObjectPtr : ObjectsBeingCustomized)
		{
			if (UAkSurfaceReflectorSetComponent* reflectorSet = Cast<UAkSurfaceReflectorSetComponent>(UObjectPtr.Get()))
			{
				if (reflectorSet->UserInteractionInProgress)
				{
					return;
				}
			}
		}

		IDetailLayoutBuilder* Layout = nullptr;
		if (auto LockedDetailBuilder = DetailBuilder.Pin())
		{
			Layout = LockedDetailBuilder.Get();
		}
		if (LIKELY(Layout))
		{
			Layout->ForceRefreshDetails();
		}

		SelectedObjectModifiedThisFrame = false;
	}
}

TSharedRef<IDetailCustomization> FAkSurfaceReflectorSetDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FAkSurfaceReflectorSetDetailsCustomization());
}

void FAkSurfaceReflectorSetDetailsCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder)
{
	InDetailBuilder->EditCategory("EnableComponent", FText::GetEmpty(), ECategoryPriority::Important);
	InDetailBuilder->EditCategory("SurfaceReflectorSet", FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	DetailBuilder = InDetailBuilder;

	CustomizeDetails(*InDetailBuilder);
}

void FAkSurfaceReflectorSetDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	InDetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	auto AcousticPolysPropHandle = InDetailBuilder.GetProperty("AcousticPolys");
	InDetailBuilder.HideProperty("AcousticPolys");
	
	bool showGeometrySettings = false;
	for (int i = 0; i < ObjectsBeingCustomized.Num(); ++i)
	{
		auto Component = Cast<UAkSurfaceReflectorSetComponent>(ObjectsBeingCustomized[i].Get());
		if (Component)
		{
			ReflectorSetBeingCustomized = TWeakObjectPtr<UAkSurfaceReflectorSetComponent>(Component);
			if (ReflectorSetBeingCustomized->bEnableSurfaceReflectors)
			{
				showGeometrySettings = true;
				break;
			}
		}
	}

	if (!showGeometrySettings)
		InDetailBuilder.HideCategory("SurfaceReflectorSet");

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Surface Properties", FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	FString enableEditSurfacesTooltip(FString("Enable ") + GEOMETRY_EDIT_DISPLAY_NAME + " and show only selected actors");
	FString disableEditSurfacesTooltip(FString("Disable ") + GEOMETRY_EDIT_DISPLAY_NAME + " and show all actors");

	CategoryBuilder.HeaderContent
	(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SAssignNew(SelectionInfoLabel, STextBlock)
			// SelectionInfoLabel is passed in to a SAcousticSurfacesController below, and the selection text is set during its construction.
			.Text(FText::FromString(""))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(FText::FromString("Enable Edit Surfaces "))
			.ToolTipText(FText::FromString(enableEditSurfacesTooltip))
			.OnClicked(this, &FAkSurfaceReflectorSetDetailsCustomization::OnEnableEditModeClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(FText::FromString("Disable Edit Surfaces"))
			.ToolTipText(FText::FromString(disableEditSurfacesTooltip))
			.OnClicked(this, &FAkSurfaceReflectorSetDetailsCustomization::OnDisableEditModeClicked)
		]
	);
	// Assume Transmission Loss controls will be hidden
	int surfacePropertiesControlsHeight = 48;
	for (TWeakObjectPtr<UObject> ObjectBeingCustomized : ObjectsBeingCustomized)
	{
		UAkSurfaceReflectorSetComponent* reflectorSetComponent = Cast<UAkSurfaceReflectorSetComponent>(ObjectBeingCustomized.Get());
		if (reflectorSetComponent && reflectorSetComponent->bEnableSurfaceReflectors)
		{
			// There is a surface reflector set with bEnableSurfaceReflectors enabled - add room for Transmission Loss controls.
			surfacePropertiesControlsHeight = 72;
			break;
		}
	}

	if (auto LockedDetailBuilder = DetailBuilder.Pin())
	{
		FDetailWidgetRow& acousticSurfacesRow = CategoryBuilder.AddCustomRow(AcousticPolysPropHandle->GetPropertyDisplayName());
		acousticSurfacesRow.NameContent()
		[
			SNew(SBox)
			.HeightOverride(surfacePropertiesControlsHeight)
			[
				SNew(SAcousticSurfacesLabels, ObjectsBeingCustomized)
			]
		];
		acousticSurfacesRow.ValueContent()
		[
			SNew(SBox)
			.HeightOverride(surfacePropertiesControlsHeight)
			[
				SNew(SAcousticSurfacesController, ObjectsBeingCustomized, LockedDetailBuilder)
			]
		];
	}

	if (ObjectsBeingCustomized.Num() == 1)
	{
		auto Component = Cast<UAkSurfaceReflectorSetComponent>(ObjectsBeingCustomized[0].Get());
		if (Component)
		{
			ReflectorSetBeingCustomized = TWeakObjectPtr<UAkSurfaceReflectorSetComponent>(Component);
			SetupGeometryModificationHandlers();
		}
		else
		{
			ReflectorSetBeingCustomized.Reset();
			UE_LOG(LogAkAudio, Log, TEXT("FAkSurfaceReflectorSetDetailsCustomization::CustomizeDetails: Could not get ObjectsBeingCustomized."));
		}
	}
}

#define REGISTER_PROPERTY_CHANGED(Class, Property, LockedDetailBuilder) \
	auto Property ## Handle = LockedDetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(Class, Property), Class::StaticClass(), BrushBuilderName); \
	if (Property ## Handle->IsValidHandle()) Property ## Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FAkSurfaceReflectorSetDetailsCustomization::OnGeometryChanged))

void FAkSurfaceReflectorSetDetailsCustomization::SetupGeometryModificationHandlers()
{
	if (!ReflectorSetBeingCustomized.IsValid())
	{
		return;
	}

	static const FName BrushBuilderName(TEXT("BrushBuilder"));
	auto ParentBrush = ReflectorSetBeingCustomized->ParentBrush;
	if(!ParentBrush)
		return;

	if (auto LockedDetailBuilder = DetailBuilder.Pin())
	{
		auto EnableHandle = LockedDetailBuilder->GetProperty("bEnableSurfaceReflectors");
		EnableHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FAkSurfaceReflectorSetDetailsCustomization::OnEnableValueChanged));

		// This is to detect if the BrushBuilder changed.
		if (ReflectorSetBeingCustomized->AcousticPolys.Num() != ParentBrush->Nodes.Num())
			LockedDetailBuilder->GetPropertyUtilities()->EnqueueDeferredAction(FSimpleDelegate::CreateSP(this, &FAkSurfaceReflectorSetDetailsCustomization::OnGeometryChanged));
	}

	// Need to register to a LOT of different properties, because some change the geometry but don't force a refresh of the details panel
	AVolume* ParentVolume = Cast<AVolume>(ReflectorSetBeingCustomized->GetOwner());
	UClass* BrushBuilderClass = nullptr;
	if (ParentVolume && ParentVolume->BrushBuilder)
	{
		BrushBuilderClass = ParentVolume->BrushBuilder->GetClass();
		if (BrushBuilderClass == nullptr)
		{
			return;
		}
	}

	if (auto LockedDetailBuilder = DetailBuilder.Pin())
	{
		if (BrushBuilderClass == UConeBuilder::StaticClass())
		{
			REGISTER_PROPERTY_CHANGED(UConeBuilder, Sides, LockedDetailBuilder);
			REGISTER_PROPERTY_CHANGED(UConeBuilder, Hollow, LockedDetailBuilder);
		}
		else if (BrushBuilderClass == UCubeBuilder::StaticClass())
		{
			REGISTER_PROPERTY_CHANGED(UCubeBuilder, Hollow, LockedDetailBuilder);
			REGISTER_PROPERTY_CHANGED(UCubeBuilder, Tessellated, LockedDetailBuilder);
		}
		else if (BrushBuilderClass == UCurvedStairBuilder::StaticClass())
		{
			REGISTER_PROPERTY_CHANGED(UCurvedStairBuilder, NumSteps, LockedDetailBuilder);
		}
		else if (BrushBuilderClass == UCylinderBuilder::StaticClass())
		{
			REGISTER_PROPERTY_CHANGED(UCylinderBuilder, Sides, LockedDetailBuilder);
			REGISTER_PROPERTY_CHANGED(UCylinderBuilder, Hollow, LockedDetailBuilder);
		}
		else if (BrushBuilderClass == ULinearStairBuilder::StaticClass())
		{
			REGISTER_PROPERTY_CHANGED(ULinearStairBuilder, NumSteps, LockedDetailBuilder);
		}
		else if (BrushBuilderClass == USpiralStairBuilder::StaticClass())
		{
			REGISTER_PROPERTY_CHANGED(USpiralStairBuilder, NumSteps, LockedDetailBuilder);
		}
		else if (BrushBuilderClass == UTetrahedronBuilder::StaticClass())
		{
			REGISTER_PROPERTY_CHANGED(UTetrahedronBuilder, SphereExtrapolation, LockedDetailBuilder);
		}
	}

	FOnRefreshDetails DetailsChanged = FOnRefreshDetails::CreateRaw(this, &FAkSurfaceReflectorSetDetailsCustomization::OnEnableValueChanged);
	ReflectorSetBeingCustomized->SetOnRefreshDetails(DetailsChanged);
}

void FAkSurfaceReflectorSetDetailsCustomization::OnEnableValueChanged()
{
	if (ReflectorSetBeingCustomized.IsValid())
	{
		ReflectorSetBeingCustomized->ClearOnRefreshDetails();
	}
}

void FAkSurfaceReflectorSetDetailsCustomization::OnGeometryChanged()
{
	if (ReflectorSetBeingCustomized.IsValid())
	{
		ReflectorSetBeingCustomized->UpdatePolys();
		ReflectorSetBeingCustomized->ClearOnRefreshDetails();
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE