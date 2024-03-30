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

#include "FSoundPlayingColumn.h"

#include "WwiseUEFeatures.h"
#include "EditorStyleSet.h"
#include "SWwiseBrowser.h"
#include "WaapiPicker/WwiseTreeItem.h"

#define LOCTEXT_NAMESPACE "AkAudio"

FSoundPlayingColumn::FSoundPlayingColumn(SWwiseBrowser& WwiseBrowser)
	: WwiseBrowserWeak(StaticCastSharedRef<SWwiseBrowser>(WwiseBrowser.AsShared()))
{
}

FName FSoundPlayingColumn::GetColumnId()
{
	return FName("SoundPlaying");
}

const TSharedRef<SWidget> FSoundPlayingColumn::ConstructRowWidget(FWwiseTreeItemPtr TreeItem,
                                                                 const STableRow<FWwiseTreeItemPtr>& Row)
{
	auto WwiseBrowser = WwiseBrowserWeak.Pin();
	return SNew(SImage)
	#if UE_5_0_OR_LATER
		.Image(FAkAppStyle::Get().GetBrush("ClassIcon.AmbientSound"))
	#else
		.Image(FEditorStyle::Get().GetBrush("Sequencer.Tracks.Audio"))
	#endif
		.Visibility_Raw(WwiseBrowser.Get(), &SWwiseBrowser::IsItemPlaying, TreeItem->ItemId);
}

SHeaderRow::FColumn::FArguments FSoundPlayingColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments PlayingStatusColumnHeader = SHeaderRow::Column(GetColumnId());
	TAttribute<FText> StatusLabel;
	PlayingStatusColumnHeader.DefaultLabel(LOCTEXT("SoundPlayingColumnLabel", ""));
	PlayingStatusColumnHeader.DefaultTooltip(LOCTEXT("SoundplayingColumn_Tooltip", "Is the Sound Playing in Wwise?"));
	PlayingStatusColumnHeader.FixedWidth(20.f);
	return PlayingStatusColumnHeader;
}

#undef LOCTEXT_NAMESPACE