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

#include "PropertyEditorModule.h"
#include "IDetailCustomization.h"
#include "UObject/StrongObjectPtr.h"
//////////////////////////////////////////////////////////////////////////
// FAkSurfaceReflectorSetDetailsCustomization

class IDetailCategoryBuilder;
class STextBlock;
class UAkSurfaceReflectorSetComponent;

class FAkSurfaceReflectorSetDetailsCustomization : public IDetailCustomization
{
public:
	FAkSurfaceReflectorSetDetailsCustomization();
	~FAkSurfaceReflectorSetDetailsCustomization();
	// Makes a new instance of this detail layout class for a specific detail view requesting it
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder) override;
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;
	// End of IDetailCustomization interface

private:
	TWeakPtr<IDetailLayoutBuilder> DetailBuilder;
	TSharedPtr<STextBlock> SelectionInfoLabel;
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	TWeakObjectPtr<UAkSurfaceReflectorSetComponent> ReflectorSetBeingCustomized;
	FReply OnEnableEditModeClicked();
	FReply OnDisableEditModeClicked();
	// In geometry edit mode, when the face selection is changed for an actor, the OnObjectModified delegate is broadcast before the selected faces are updated.
	// For that reason, we use a flag to indicate when an object has been modified this frame.
	// That flag is used in OnRedrawViewports - which happens later - to refresh the details panel in case the selected faces have changed.
	bool SelectedObjectModifiedThisFrame = false;
	void OnObjectModified(UObject* Object);
	void OnRedrawViewports();

	void OnEnableValueChanged();
	void OnGeometryChanged();
	void SetupGeometryModificationHandlers();
};
