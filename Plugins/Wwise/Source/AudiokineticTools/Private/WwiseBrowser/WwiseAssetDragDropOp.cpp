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

#include "WwiseBrowser/WwiseAssetDragDropOp.h"

#include "AkAudioType.h"
#include "AkSettings.h"
#include "WwiseUEFeatures.h"
#include "WwiseUnrealHelper.h"
#include "AssetToolsModule.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "ContentBrowserModule.h"
#include "FileHelpers.h"
#include "Misc/Paths.h"
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "AkAudio"

TSharedRef<FAssetDragDropOp> FWwiseAssetDragDropOp::New(TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> InAssetData, UActorFactory* ActorFactory)
{
	return New(MoveTemp(InAssetData), TArray<FString>(), ActorFactory);
}

TSharedRef<FAssetDragDropOp> FWwiseAssetDragDropOp::New(TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> InAssetData, TArray<FString> InAssetPaths, UActorFactory* ActorFactory)
{
	TArray<FAssetData> NewAssets;
	for (WwiseBrowserHelpers::WwiseBrowserAssetPayload AssetResult : InAssetData)
	{
		if (AssetResult.ExistingAssets.Num() > 0)
		{
			NewAssets.Add(AssetResult.ExistingAssets[0]);
		}
		else
		{
			NewAssets.Add(AssetResult.CreatedAsset);
		}
	}
	TSharedRef<FAssetDragDropOp> ParentOperation = FAssetDragDropOp::New(NewAssets, InAssetPaths, ActorFactory);

	FWwiseAssetDragDropOp* RawPointer = new FWwiseAssetDragDropOp();
	TSharedRef<FWwiseAssetDragDropOp> Operation = MakeShareable(RawPointer);
	// ugly hack since FAssetDragDropOp data is private
	static_cast<FAssetDragDropOp*>(RawPointer)->operator=(ParentOperation.Get());

	FAssetViewDragAndDropExtender::FOnDropDelegate DropDelegate = FAssetViewDragAndDropExtender::FOnDropDelegate::CreateRaw(RawPointer, &FWwiseAssetDragDropOp::OnAssetViewDrop);
	FAssetViewDragAndDropExtender::FOnDragOverDelegate DragOverDelegate = FAssetViewDragAndDropExtender::FOnDragOverDelegate::CreateRaw(RawPointer, &FWwiseAssetDragDropOp::OnAssetViewDragOver);
	Operation->Extender = new FAssetViewDragAndDropExtender(DropDelegate, DragOverDelegate);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetViewDragAndDropExtender>& AssetViewDragAndDropExtenders = ContentBrowserModule.GetAssetViewDragAndDropExtenders();
	AssetViewDragAndDropExtenders.Add(*(Operation->Extender));

	Operation->WwiseAssetsToDrop = InAssetData;

	return Operation;
}

FWwiseAssetDragDropOp::~FWwiseAssetDragDropOp()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetViewDragAndDropExtender>& AssetViewDragAndDropExtenders = ContentBrowserModule.GetAssetViewDragAndDropExtenders();
	for (int32 i = 0; i < AssetViewDragAndDropExtenders.Num(); i++)
	{
		if (AssetViewDragAndDropExtenders[i].OnDropDelegate.IsBoundToObject(this))
		{
			AssetViewDragAndDropExtenders.RemoveAt(i);
		}
	}

	delete Extender;
}

bool FWwiseAssetDragDropOp::OnAssetViewDrop(const FAssetViewDragAndDropExtender::FPayload& Payload)
{
	if (!Payload.DragDropOp->IsOfType<FWwiseAssetDragDropOp>())
	{
		SetCanDrop(false);
		return false;
	}

	if (CanDrop)
	{
		bDroppedOnContentBrowser =true;
		if (Payload.PackagePaths.Num() <= 0)
		{
			return false;
		}

		const auto AssetDragDrop = static_cast<FWwiseAssetDragDropOp*>(Payload.DragDropOp.Get());
		auto PackagePath = Payload.PackagePaths[0].ToString();

		// UE5 adds "/All" to all game content folder paths, but CreateAsset doesn't like it
		PackagePath.RemoveFromStart(TEXT("/All"));

		// UE5 adds "/All/Plugins" to all plugin content folder paths, but CreateAsset doesn't like it
		PackagePath.RemoveFromStart(TEXT("/Plugins"));
		AssetViewDropTargetPackagePath = PackagePath;
	}

	return CanDrop;
}

bool FWwiseAssetDragDropOp::OnAssetViewDragOver(const FAssetViewDragAndDropExtender::FPayload& Payload)
{
	if (!Payload.DragDropOp->IsOfType<FWwiseAssetDragDropOp>())
	{
		SetCanDrop(false);
		return false;
	}
	SetCanDrop(true);
	return true;
}


void FWwiseAssetDragDropOp::SaveAssets()
{
	FString DefaultPath = FPaths::ProjectContentDir();
	auto AkSettings = GetDefault<UAkSettings>();
	if (LIKELY(AkSettings))
	{
		DefaultPath = AkSettings->DefaultAssetCreationPath;
	}

	FString TargetRootPackagePath = DefaultPath;
	WwiseBrowserHelpers::EAssetDuplicationMode DuplicationMode =  WwiseBrowserHelpers::EAssetDuplicationMode::NoDuplication;
	if (bDroppedOnContentBrowser)
	{
		TargetRootPackagePath = AssetViewDropTargetPackagePath;
		DuplicationMode = WwiseBrowserHelpers::EAssetDuplicationMode::DoDuplication;
	}

	WwiseBrowserHelpers::SaveSelectedAssets(WwiseAssetsToDrop, TargetRootPackagePath, WwiseBrowserHelpers::EAssetCreationMode::Transient, DuplicationMode);
}

void FWwiseAssetDragDropOp::DeleteAssets()
{
	TArray<FAssetData> AssetsToDelete;
	for (WwiseBrowserHelpers::WwiseBrowserAssetPayload& AssetResult : WwiseAssetsToDrop)
	{
		if (AssetResult.CreatedAsset.IsValid())
		{
			AssetsToDelete.Add(AssetResult.CreatedAsset);
		}
	}

	if (AssetsToDelete.Num() > 0)
	{
		ObjectTools::DeleteAssets(AssetsToDelete, false);
	}
}

void FWwiseAssetDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	if (!bDropWasHandled)
	{
		DeleteAssets();
		return;
	}

	SaveAssets();
}

void FWwiseAssetDragDropOp::SetCanDrop(const bool InCanDrop)
{
	CanDrop = InCanDrop;
	SetTooltipText();

	if (InCanDrop)
	{
		MouseCursor = EMouseCursor::GrabHandClosed;
		SetToolTip(GetTooltipText(), FAkAppStyle::Get().GetBrush(TEXT("Graph.ConnectorFeedback.Ok")));
	}
	else
	{
		MouseCursor = EMouseCursor::SlashedCircle;
		SetToolTip(GetTooltipText(), FAkAppStyle::Get().GetBrush(TEXT("Graph.ConnectorFeedback.Error")));
	}
}

void FWwiseAssetDragDropOp::SetTooltipText()
{
	if (CanDrop)
	{
		auto& assets = GetAssets();
		FString Text = assets.Num() ? assets[0].AssetName.ToString() : TEXT("");

		if (assets.Num() > 1)
		{
			Text += TEXT(" ");
			Text += FString::Printf(TEXT("and %d other(s)"), assets.Num() - 1);
		}
		CurrentHoverText = FText::FromString(Text);
	}
	else
	{
		CurrentHoverText = LOCTEXT("OnDragAkEventsOverFolder_CannotDrop", "Wwise assets can only be dropped in the right folder");
	}
}

FText FWwiseAssetDragDropOp::GetTooltipText() const
{
	return CurrentHoverText;
}

#undef LOCTEXT_NAMESPACE