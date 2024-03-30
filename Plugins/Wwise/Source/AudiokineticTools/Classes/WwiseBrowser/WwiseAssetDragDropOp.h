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

/*------------------------------------------------------------------------------------
	WwiseEventDragDropOp.h
------------------------------------------------------------------------------------*/
#pragma once

#include "Containers/Map.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "ContentBrowserDelegates.h"
#include "AssetToolsModule.h"
#include "WwiseBrowser/WwiseBrowserHelpers.h"

class FWwiseAssetDragDropOp : public FAssetDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FWwiseEventDragDropOp, FAssetDragDropOp)

	static TSharedRef<FAssetDragDropOp> New(TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> InAssetData, UActorFactory* ActorFactory = nullptr);

	static TSharedRef<FAssetDragDropOp> New(TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> InAssetData, TArray<FString> InAssetPaths, UActorFactory* ActorFactory);

	bool OnAssetViewDrop(const FAssetViewDragAndDropExtender::FPayload& Payload);
	bool OnAssetViewDragOver(const FAssetViewDragAndDropExtender::FPayload& Payload);
	void SaveAssets();
	void DeleteAssets();

	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	void SetCanDrop(const bool InCanDrop);

	void SetTooltipText();
	FText GetTooltipText() const;

	~FWwiseAssetDragDropOp();

public:
	FText CurrentHoverText;
	bool CanDrop = true;
	FAssetViewDragAndDropExtender* Extender = nullptr;

private:
	// Assets contained within the drag operation. Value is true if the asset existed prior to initiating the drop.
	TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> WwiseAssetsToDrop;
	bool bDroppedOnContentBrowser = false;
	FString AssetViewDropTargetPackagePath;
};