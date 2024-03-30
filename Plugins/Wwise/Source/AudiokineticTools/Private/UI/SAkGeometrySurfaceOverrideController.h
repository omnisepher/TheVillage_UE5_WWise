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

#include "Framework/Application/SlateApplication.h"
#include "Widgets/SCompoundWidget.h"
#include "Editor.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;
class UMaterialInterface;

class UAkGeometryComponent;
struct FAkGeometrySurfaceOverride;
class UAkAcousticTexture;

class SSurfacePropertiesLabels : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSurfacePropertiesLabels) {}
	SLATE_END_ARGS()

	AUDIOKINETICTOOLS_API void Construct(const FArguments& InArgs, FAkGeometrySurfaceOverride* InSurfaceOverride);

private:
	FAkGeometrySurfaceOverride* SurfaceOverride = nullptr;

	EVisibility TransmissionLossVisibility() const;
};

class SAkGeometrySurfaceController : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAkGeometrySurfaceController) {}
	SLATE_END_ARGS()

	AUDIOKINETICTOOLS_API void Construct(const FArguments& InArgs
		, TWeakObjectPtr<UObject> ObjectBeingCustomized
		, const TSharedPtr<IDetailLayoutBuilder>& InLayoutBuilder
	);

	~SAkGeometrySurfaceController();

private:
	void BeginModify(FText TransactionText);
	void EndModify();

	virtual FAkGeometrySurfaceOverride* GetSurfaceOverride() const { return nullptr; }
	void BuildSlate();

	FString GetSelectedTextureAssetPath() const;
	void OnTextureAssetChanged(const FAssetData& InAssetData);

	TOptional<float> GetTransmissionLossValue() const;
	void OnTransmissionLossChanged(float NewValue, ETextCommit::Type Commit);
	EVisibility TransmissionLossVisibility() const;

	ECheckBoxState GetEnableTransmissionLossOverrideCheckBoxState() const;
	void OnEnableTransmissionLossOverrideChanged(ECheckBoxState NewState);


	UAkAcousticTexture* CurrentTexture = nullptr;
	/** The details layout in the editor */
	TWeakPtr<IDetailLayoutBuilder> LayoutBuilder = nullptr;
	/** The AkGeometryComponent being customized */
	mutable UAkGeometryComponent* ComponentBeingCustomized = nullptr;

	friend class SAkGeometryCollisionMeshSurfaceController;
	friend class SAkGeometryStaticMeshSurfaceController;
};

class SAkGeometryCollisionMeshSurfaceController : public SAkGeometrySurfaceController
{
private:
	virtual FAkGeometrySurfaceOverride* GetSurfaceOverride() const override;
};

class SAkGeometryStaticMeshSurfaceController : public SAkGeometrySurfaceController
{
public:
	AUDIOKINETICTOOLS_API void Construct(const FArguments& InArgs
		, TWeakObjectPtr<UObject> ObjectBeingCustomized
		, const TSharedPtr<IDetailLayoutBuilder>& InLayoutBuilder
		, UMaterialInterface* InMaterialKey
	);

private:
	virtual FAkGeometrySurfaceOverride* GetSurfaceOverride() const override;
	UMaterialInterface* MaterialKey;
};