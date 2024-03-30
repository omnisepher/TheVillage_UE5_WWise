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
	SWwiseBrowser.cpp
------------------------------------------------------------------------------------*/

#include "SWwiseBrowser.h"

#include "IDesktopPlatform.h"
#include "ISettingsModule.h"
#include "Async/Async.h"
#include "AkSettingsPerUser.h"
#include "DesktopPlatformModule.h"
#include "Async/Async.h"
#include "EditorDirectories.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "Widgets/Images/SLayeredImage.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"

#include "AkAudioBankGenerationHelpers.h"
#include "AkAudioModule.h"
#include "AkAudioStyle.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "WwiseUnrealHelper.h"

#if !UE_5_0_OR_LATER
#include "EditorFontGlyphs.h"
#endif

#include "FSoundPlayingColumn.h"
#include "IAudiokineticTools.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "Wwise/WwiseProjectDatabase.h"
#include "WwiseBrowser/WwiseAssetDragDropOp.h"
#include "WwiseBrowser/WwiseBrowserHelpers.h"
#include "WwiseBrowser/WwiseBrowserViewCommands.h"

#include "SWwiseBrowserTreeView.h"
#include "SoundBankStatusColumn.h"
#include "WwiseBrowserTreeColumn.h"
#include "WwiseStatusColumn.h"
#include "WwiseUEAssetStatusColumn.h"
#include "DataSource/WwiseBrowserDataSource.h"
#include "AkWaapiUMG/Components/AkBoolPropertyToControlCustomization.h"
#include "Wwise/WwiseReconcile.h"
#include "Widgets/SWwiseReconcile.h"


#define LOCTEXT_NAMESPACE "AkAudio"

const FName SWwiseBrowser::WwiseBrowserTabName = "WwiseBrowser";

namespace SWwiseBrowser_Helper
{
	const FName DirectoryWatcherModuleName = "DirectoryWatcher";
}

namespace FilterMenuTitles
{
	FText GetMenuTitle(ESoundBankStatusFilter Filter)
	{
		switch (Filter)
		{
		case NewInWwise:
			return LOCTEXT("NewInWwise", "New In Wwise");
		case DeletedInWwise:
			return LOCTEXT("DeletedInWwise", "Deleted In Wwise");
		case RenamedInWwise:
			return LOCTEXT("RenamedInWwise", "Renamed In Wwise");
		case NotInWwise:
			return LOCTEXT("NotInWwiseOrSoundBank", "Not In Wwise or SoundBank");
		case MovedInWwise:
			return LOCTEXT("MovedInWwise", "Moved In Wwise");
		case UpToDate:
			return LOCTEXT("SoundBankUpToDate", "SoundBank Up To Date");
		}
		return FText();
	}

	FText GetMenuTooltip(ESoundBankStatusFilter Filter)
	{
		switch (Filter)
		{
		case NewInWwise:
			return LOCTEXT("NewInWwise_Tooltip", "Filter the New in Wwise Items.");
		case DeletedInWwise:
			return LOCTEXT("DeletedInWwise_Tooltip", "Filter the Deleted Items in Wwise.");
		case RenamedInWwise:
			return LOCTEXT("RenamedInWwise_Tooltip", "Filter the Renamed Items in Wwise.");
		case NotInWwise:
			return LOCTEXT("NotInWwiseOrSoundBank_Tooltip", "Filter the Items not in Wwise or in SoundBank.");
		case MovedInWwise:
			return LOCTEXT("MovedInWwise_Tooltip", "Filter the Items in different locations in Wwise and in SoundBank.");
		case UpToDate:
			return LOCTEXT("SoundBankUpToDate_Tooltip", "Filter the Up to date Wwise Items.");
		}
		return FText();
	}

	FText GetMenuTitle(EUAssetStatusFilter Filter)
	{
		switch (Filter)
		{
		case UAssetMissing:
			return LOCTEXT("UAssetMissing", "UAsset Missing");
		case NotInSoundBankOrUnreal:
			return LOCTEXT("NotInSoundBankOrUnreal", "Not in SoundBank or Unreal");
		case UAssetOrphaned:
			return LOCTEXT("UAssetOrphaned", "UAsset Orphaned");
		case MultipleUAssets:
			return LOCTEXT("UAssetMultiple", "Multiple UAssets");
		case RenamedInSoundBank:
			return LOCTEXT("RenamedInSoundBank", "Renamed In SoundBank");
		case UAssetNeedsUpdate:
			return LOCTEXT("UAssetNeedsUpdate", "UAsset Needs Update");
		case UAssetUpToDate:
			return LOCTEXT("UAssetUpToDate", "UAsset Up to Date");
		}
		return FText();
	}

	FText GetMenuTooltip(EUAssetStatusFilter Filter)
	{
		switch (Filter)
		{
		case UAssetMissing:
			return LOCTEXT("UAssetMissing_Tooltip", "Filter the Missing UAssets.");
		case NotInSoundBankOrUnreal:
			return LOCTEXT("NotInSoundBankOrUnreal_Tooltip", "Filter the UAssets not in SoundBanks or Unreal.");
		case UAssetOrphaned:
			return LOCTEXT("UAssetOrphaned_Tooltip", "Filter UAssets with no counterpart in Wwise.");
		case MultipleUAssets:
			return LOCTEXT("UAssetMultiple_Tooltip", "Filter Wwise items with multiple UAssets.");
		case RenamedInSoundBank:
			return LOCTEXT("UAssetOrphaned_Tooltip", "Filter UAssets with different names in the SoundBank.");
		case UAssetNeedsUpdate:
			return LOCTEXT("UAssetNeedsUpdate_Tooltip", "Filter UAssets with different Guid or ShortId in the SoundBank.");
		case UAssetUpToDate:
			return LOCTEXT("UAssetUpToDate_Tooltip", "Filter UAssets that are up to date with Wwise.");
		}
		return FText();
	}

	FText GetMenuTitle(EWwiseTypeFilter Filter)
	{
		switch (Filter)
		{
		case AcousticTexture:
			return LOCTEXT("AcousticTexture", "Acoustic Texture");
		case Effects:
			return LOCTEXT("Effects", "Effects");
		case Events:
			return LOCTEXT("Events", "Events");
		case GameParameters:
			return LOCTEXT("GameParameters", "GameParameters");
		case MasterMixerHierarchy:
			return LOCTEXT("MasterMixerHierarchy", "Master Mixer Hierarchy (Bus)");
		case State:
			return LOCTEXT("States", "States");
		case Switch:
			return LOCTEXT("Switches", "Switches");
		case Trigger:			
			return LOCTEXT("Triggers", "Triggers");
		}
		return FText();
	}

	FText GetMenuTooltip(EWwiseTypeFilter Filter)
	{
		switch (Filter)
		{
		case AcousticTexture:
			return LOCTEXT("AcousticTexture_Tooltip", "Filter Wwise Acoustic Texture Types.");
		case Effects:
			return LOCTEXT("Effects_Tooltip", "Filter Wwise Effect Types.");
		case Events:
			return LOCTEXT("Events_Tooltip", "Filter Wwise Event Types.");
		case GameParameters:
			return LOCTEXT("GameParameters_Tooltip", "Filter Wwise Game Parameter Types.");
		case MasterMixerHierarchy:
			return LOCTEXT("MasterMixerHierarchy_Tooltip", "Filter Wwise Bus Types.");
		case State:
			return LOCTEXT("State_Tooltip", "Filter Wwise State Types.");
		case Switch:
			return LOCTEXT("Switch_Tooltip", "Filter Wwise Switch Types.");
		case Trigger:
			return LOCTEXT("Trigger_Tooltip", "Filter Wwise Trigger Types.");
		}
		return FText();
	}
}

SWwiseBrowser::SWwiseBrowser(): CommandList(MakeShared<FUICommandList>())
{
	AllowTreeViewDelegates = true;
	DataSource = MakeUnique<FWwiseBrowserDataSource>();
	Transport = MakeUnique<WaapiPlaybackTransport>();

	DataSource->WwiseBrowserDataSourceRefreshed.BindLambda([this]
	{
		AsyncTask(ENamedThreads::Type::GameThread, [this]
		{
			this->ConstructTree();
		});
	});

	DataSource->WwiseSelectionChange.BindRaw(this, &SWwiseBrowser::UpdateWaapiSelection);
	DataSource->WwiseExpansionChange.BindRaw(this, &SWwiseBrowser::ExpandItems);
}

void SWwiseBrowser::CreateWwiseBrowserCommands()
{
	const FWwiseBrowserViewCommands& Commands = FWwiseBrowserViewCommands::Get();
	FUICommandList& ActionList = *CommandList;
	// Action to start playing an event from the Wwise Browser.
	ActionList.MapAction(
		Commands.RequestPlayWwiseItem,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandlePlayWwiseItemCommandExecute),
		FCanExecuteAction::CreateSP(this, &SWwiseBrowser::HandlePlayOrStopWwiseItemCanExecute));

	// Action to stop all playing Wwise item (event).
	ActionList.MapAction(
		Commands.RequestStopAllWwiseItem,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandleStopAllWwiseItemCommandExecute));

	// Action to explore an item (workunit) in the containing folder.
	ActionList.MapAction(
		Commands.RequestExploreWwiseItem,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandleExploreWwiseItemCommandExecute),
		FCanExecuteAction::CreateSP(this, &SWwiseBrowser::HandleExploreWwiseItemCanExecute));

	// Action find an item in the Wwise Project Explorer
	ActionList.MapAction(
		Commands.RequestFindInProjectExplorerWwiseItem,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandleFindInProjectExplorerWwiseItemCommandExecute),
		FCanExecuteAction::CreateSP(this, &SWwiseBrowser::HandleFindInProjectExplorerWwiseItemCanExecute));

	// Action to find the Unreal asset in the Content Browser that maps to this item
	ActionList.MapAction(
		Commands.RequestFindInContentBrowser,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandleFindInContentBrowserCommandExecute),
		FCanExecuteAction::CreateSP(this, &SWwiseBrowser::HandleFindInContentBrowserCanExecute));

	// Action to refresh the Browser
	ActionList.MapAction(
		Commands.RequestRefreshWwiseBrowser,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandleRefreshWwiseBrowserCommandExecute));

	// Action for importing the selected items from the Wwise Browser.
	ActionList.MapAction(
		Commands.RequestImportWwiseItem,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandleImportWwiseItemCommandExecute));

	// Action for reconciling the selected items from the Wwise Browser.
	ActionList.MapAction(
		Commands.RequestReconcileWwiseItem,
		FExecuteAction::CreateSP(this, &SWwiseBrowser::HandleReconcileWwiseItemCommandExecute));
}

TSharedPtr<SWidget> SWwiseBrowser::MakeWwiseBrowserContextMenu()
{
	const FWwiseBrowserViewCommands& Commands = FWwiseBrowserViewCommands::Get();

	// Build up the menu
	FMenuBuilder MenuBuilder(true, CommandList);
	{
		MenuBuilder.BeginSection("WwiseBrowserTransport", LOCTEXT("MenuHeader", "WwiseBrowser"));
		{
			MenuBuilder.AddMenuEntry(Commands.RequestPlayWwiseItem);
			MenuBuilder.AddMenuEntry(Commands.RequestStopAllWwiseItem);
		}
		MenuBuilder.EndSection();
		MenuBuilder.BeginSection("WwiseBrowserFindOptions", LOCTEXT("ExploreMenuHeader", "Explore"));
		{
			MenuBuilder.AddMenuEntry(Commands.RequestFindInProjectExplorerWwiseItem);
			MenuBuilder.AddMenuEntry(Commands.RequestFindInContentBrowser);
			MenuBuilder.AddMenuEntry(Commands.RequestExploreWwiseItem);
		}
		MenuBuilder.EndSection();
		MenuBuilder.BeginSection("WwiseBrowserRefreshAll", LOCTEXT("RefreshHeader", "Refresh"));
		{
			MenuBuilder.AddMenuEntry(Commands.RequestRefreshWwiseBrowser);
		}
		MenuBuilder.EndSection();
		MenuBuilder.BeginSection("WwiseBrowserImport", LOCTEXT("ImportHeader", "Import"));
		{
			MenuBuilder.AddMenuEntry(Commands.RequestImportWwiseItem);
		}
		MenuBuilder.EndSection();

		{
			MenuBuilder.AddMenuEntry(Commands.RequestReconcileWwiseItem);
		}
		MenuBuilder.EndSection();

	}
	return MenuBuilder.MakeWidget();
}

void SWwiseBrowser::HandlePlayWwiseItemCommandExecute()
{
	TArray<FWwiseTreeItemPtr> SelectedItems = TreeViewPtr->GetSelectedItems();
	int32 TransportID = -1;
	for (auto WwiseTreeItem : SelectedItems)
	{
		bool bPlaying = Transport->IsPlaying(WwiseTreeItem->ItemId);

		if (WwiseTreeItem->bWaapiRefExists)
		{
			TransportID = Transport->FindOrAdd(WwiseTreeItem->ItemId);
		}
		if(TransportID >= 0 && !bPlaying)
		{
			Transport->TogglePlay(TransportID);
		}
		if(bPlaying)
		{
			Transport->Remove(WwiseTreeItem->ItemId);
		}
	}
}

bool SWwiseBrowser::HandlePlayOrStopWwiseItemCanExecute()
{
	if (IsWaapiAvailable() != EWwiseConnectionStatus::Connected)
	{
		return false;
	}

	TArray<FWwiseTreeItemPtr> SelectedItems = TreeViewPtr->GetSelectedItems();
	if (SelectedItems.Num() == 0)
	{
		return false;
	}

	for (auto SelectedItem: SelectedItems)
	{

		if (!SelectedItem->bWaapiRefExists)
		{
			return false;
		}

		if (SelectedItem->IsNotOfType({ EWwiseItemType::Event, EWwiseItemType::Sound, EWwiseItemType::BlendContainer, EWwiseItemType::SwitchContainer, EWwiseItemType::RandomSequenceContainer }))
		{
			return false;
		}
	}

	return true;
}

void SWwiseBrowser::HandleStopWwiseItemCommandExecute()
{
	TArray<FWwiseTreeItemPtr> SelectedItems = TreeViewPtr->GetSelectedItems();
	int32 TransportID = -1;
	for (auto WwiseTreeItem : SelectedItems)
	{
		if (WwiseTreeItem->bWaapiRefExists)
		{
			TransportID = Transport->FindOrAdd(WwiseTreeItem->ItemId);
		}
		Transport->Stop(TransportID);
	}
}

void SWwiseBrowser::HandleStopAllWwiseItemCommandExecute()
{
	Transport->StopAndDestroyAll();
}

void SWwiseBrowser::HandleExploreWwiseItemCommandExecute()
{
	FWwiseTreeItemPtr SelectedItem = TreeViewPtr->GetSelectedItems()[0];
	FString WorkUnitPath = DataSource->GetItemWorkUnitPath(SelectedItem);
	FPlatformProcess::ExploreFolder(*WorkUnitPath);
}

bool SWwiseBrowser::HandleExploreWwiseItemCanExecute()
{
	return TreeViewPtr->GetSelectedItems().Num() == 1 && IsWaapiAvailable() == EWwiseConnectionStatus::Connected;
}

void SWwiseBrowser::HandleFindInProjectExplorerWwiseItemCommandExecute()
{
	TArray<FWwiseTreeItemPtr> SelectedItems = TreeViewPtr->GetSelectedItems();

	DataSource->SelectInWwiseProjectExplorer(SelectedItems);
}

bool SWwiseBrowser::HandleFindInProjectExplorerWwiseItemCanExecute()
{
	return TreeViewPtr->GetSelectedItems().Num() >= 1 && IsWaapiAvailable() == EWwiseConnectionStatus::Connected;
}

bool SWwiseBrowser::HandleFindInContentBrowserCanExecute()
{
	TArray<FWwiseTreeItemPtr> SelectedItems = TreeViewPtr->GetSelectedItems();

	for (const auto& Asset : SelectedItems)
	{
		if (Asset->IsOfType({ EWwiseItemType::Event,
			EWwiseItemType::Bus,
			EWwiseItemType::AuxBus,
			EWwiseItemType::AcousticTexture,
			EWwiseItemType::State,
			EWwiseItemType::Switch,
			EWwiseItemType::GameParameter,
			EWwiseItemType::Trigger,
			EWwiseItemType::EffectShareSet
			}))
		{
			return true;
		}
	}

	return false;
}

void SWwiseBrowser::HandleFindInContentBrowserCommandExecute()
{
	TArray<FWwiseTreeItemPtr> SelectedItems = TreeViewPtr->GetSelectedItems();
	TArray<FAssetData> AssetsToSync;

	for (const auto& Asset : SelectedItems)
	{
		AssetsToSync.Append(Asset->Assets);
	}

	if (AssetsToSync.Num())
	{
		GEditor->SyncBrowserToObjects(AssetsToSync);
	}
}

void SWwiseBrowser::HandleRefreshWwiseBrowserCommandExecute()
{
	DataSource->ConstructTree();
}

SWwiseBrowser::~SWwiseBrowser()
{
	RootItems.Empty();
	SearchBoxFilter->OnChanged().RemoveAll(this);
	DataSource->WwiseSelectionChange.Unbind();
	DataSource->WwiseExpansionChange.Unbind();
}

void SWwiseBrowser::Construct(const FArguments& InArgs)
{

	FGenericCommands::Register();
	FWwiseBrowserViewCommands::Register();
	CreateWwiseBrowserCommands();

	SearchBoxFilter = MakeShareable(new StringFilter(StringFilter::FItemToStringArray::CreateSP(this, &SWwiseBrowser::PopulateSearchStrings)));
	SearchBoxFilter->OnChanged().AddSP(this, &SWwiseBrowser::OnTextFilterUpdated);

	HeaderRowWidget =
		SNew(SHeaderRow)
		.CanSelectGeneratedColumn(true);

#if UE_5_0_OR_LATER
	TSharedPtr<SLayeredImage> FilterImage = SNew(SLayeredImage)
		.Image(FAkAppStyle::Get().GetBrush("Icons.Filter"))
		.ColorAndOpacity(FSlateColor::UseForeground());
#if UE_5_1_OR_LATER
	// Badge the filter icon if there are filters active
	FilterImage->AddLayer(TAttribute<const FSlateBrush*>(this, &SWwiseBrowser::GetFilterBadgeIcon));
#endif
#endif

	SetupColumns(*HeaderRowWidget);

	ChildSlot
		[
			SNew(SBorder)
			.Padding(7.f)
		.BorderImage(FAkAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SOverlay)

		// Browser
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)

		// Search
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 1, 0, 3)
		[
			SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			InArgs._SearchContent.Widget
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SComboButton)
#if UE_5_0_OR_LATER
			.ComboButtonStyle(FAkAppStyle::Get(), "SimpleComboButton")
#else
			.ComboButtonStyle(FEditorStyle::Get(), "GenericFilters.ComboButtonStyle")
		.ForegroundColor(FLinearColor::White)
#endif
		.ToolTipText(LOCTEXT("Browser_AddFilterToolTip", "Add filters to the Wwise Browser."))
		.OnGetMenuContent(this, &SWwiseBrowser::MakeAddFilterMenu)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
#if UE_5_0_OR_LATER
			FilterImage.ToSharedRef()
#else
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
			.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
			.Text(FText::FromString(FString(TEXT("\xf0b0"))) /*fa-filter*/)
#endif
		]
		]
		]

					+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("WwiseBrowserSearchTooltip", "Search..."))
		.OnTextChanged(this, &SWwiseBrowser::OnSearchBoxChanged)
		.SelectAllTextWhenFocused(false)
		.DelayChangeNotificationsWhileTyping(true)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.Padding(FMargin(5.0f,0.0f,20.0f,0.0f))
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("WwiseBrowserRefresh", "Refresh the Wwise Browser"))
				.ButtonStyle(FAkAudioStyle::Get(), "AudiokineticTools.HoverHintOnly")
				.OnClicked(this, &SWwiseBrowser::OnRefreshClicked)
				.VAlign(VAlign_Center)
				[

#if UE_5_0_OR_LATER
					SNew(SImage)
					.Image(FAkAppStyle::Get().GetBrush("Icons.Refresh"))
#else
					SNew(STextBlock)
					.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
					.Text(FEditorFontGlyphs::Repeat)
					.ColorAndOpacity(FLinearColor::White)
#endif
				]	
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("AkBrowserGenerate", "Generate SoundBanks..."))
				.VAlign(VAlign_Center)
			.OnClicked(this, &SWwiseBrowser::OnGenerateSoundBanksClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(FMargin(20.0f,0.0f,0.0f,0.0f))
		.AutoWidth()
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("Reconcile_Tooltip", "Reconcile Selected."))
			.ButtonStyle(FAkAudioStyle::Get(), "AudiokineticTools.HoverHintOnly")
			.OnClicked(this, &SWwiseBrowser::OnReconcileClicked)
			[
				SNew(SImage)
				.Image(FAkAppStyle::Get().GetBrush("Graph.Node.Loop"))
			]
		]
		
	]

	// Tree
	+ SVerticalBox::Slot()
	.FillHeight(1.f)
	[
		SAssignNew(TreeViewPtr, SWwiseBrowserTreeView, StaticCastSharedRef<SWwiseBrowser>(AsShared()))
		.TreeItemsSource(&RootItems).Visibility(this, &SWwiseBrowser::IsWarningNotVisible)
		.OnGenerateRow(this, &SWwiseBrowser::GenerateRow)
		.ItemHeight(18)
		.SelectionMode(InArgs._SelectionMode)
		.OnSelectionChanged(this, &SWwiseBrowser::TreeSelectionChanged)
		.OnExpansionChanged(this, &SWwiseBrowser::TreeExpansionChanged)
		.OnGetChildren(this, &SWwiseBrowser::GetChildrenForTree)
		.OnContextMenuOpening(this, &SWwiseBrowser::MakeWwiseBrowserContextMenu)
		.ClearSelectionOnClick(false)
		.HeaderRow(HeaderRowWidget)
		.OnMouseButtonDoubleClick(this, &SWwiseBrowser::OnTreeItemDoubleClicked)
	]

	//Footer
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 6.f, 0.f, 0.f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(this, &SWwiseBrowser::GetConnectionStatusText)
						.Visibility(this, &SWwiseBrowser::IsWarningNotVisible)
					]
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(this, &SWwiseBrowser::GetSoundBanksLocationText)
						.ColorAndOpacity(this, &SWwiseBrowser::GetSoundBanksLocationTextColor)
					]
				]
			]
		]
	]
]

	// Empty Browser
	+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Visibility(this, &SWwiseBrowser::IsWarningVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.Text(this, &SWwiseBrowser::GetWarningText)
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("AkBrowserOpenSettings", "Open Wwise Integration Settings"))
				.Visibility(this, &SWwiseBrowser::IsWarningVisible)
				.OnClicked(this, &SWwiseBrowser::OnOpenSettingsClicked)
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoHeight()
		]
		]
	];

	InitialParse();
}

TSharedRef<SWidget> SWwiseBrowser::MakeAddFilterMenu()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/false, nullptr);

	MenuBuilder.BeginSection("SoundBankStatusFilters", LOCTEXT("SoundBankStatusFilters", "SoundBank Status Filters"));
	{
		for (int i = 0; i < NumberOfSoundBankStatus; i++)
		{
			ESoundBankStatusFilter Status = (ESoundBankStatusFilter)i;
			MenuBuilder.AddMenuEntry(
				FilterMenuTitles::GetMenuTitle(Status),
				FilterMenuTitles::GetMenuTooltip(Status),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SWwiseBrowser::SoundBankFilterExecute, Status),
					FCanExecuteAction::CreateLambda([] { return true; }),
					FIsActionChecked::CreateSP(this, &SWwiseBrowser::SoundBankFilterIsChecked, Status)),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}


		MenuBuilder.AddMenuEntry(
			LOCTEXT("SoundBankNotUpToDate", "SoundBank Not Up to Date"),
			LOCTEXT("SoundBankNotUpToDate_Tooltip", "Activate all filters of this section that is not \"SoundBank Up to Date\""),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SWwiseBrowser::SoundBankNotUpToDateExecute)),
			NAME_None,
			EUserInterfaceActionType::Button
			);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("UAssetStatusFilters", LOCTEXT("UAssetStatusFilters", "UAsset Status Filters"));
	{
		for(int i = 0; i < NumberOfUAssetStatus; i++)
		{
			EUAssetStatusFilter Status = (EUAssetStatusFilter)i;
			MenuBuilder.AddMenuEntry(
				FilterMenuTitles::GetMenuTitle(Status),
				FilterMenuTitles::GetMenuTooltip(Status),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SWwiseBrowser::UAssetFilterExecute, Status),
					FCanExecuteAction::CreateLambda([] { return true; }),
					FIsActionChecked::CreateSP(this, &SWwiseBrowser::UAssetFilterIsChecked, Status)),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}

		MenuBuilder.AddMenuEntry(
			LOCTEXT("UAssetNotUpToDate", "UAsset Not Up to Date"),
			LOCTEXT("UAssetNotUpToDate_Tooltip", "Activate all filters of this section that is not \"UAsset Up to Date\""),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SWwiseBrowser::UAssetNotUpToDateExecute)),
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("TypeFilters", LOCTEXT("TypeFilters", "Type Filters"));
	{
		for (int i = 0; i < NumberOfWwiseTypes; i++)
		{
			EWwiseTypeFilter Type = (EWwiseTypeFilter)i;
			MenuBuilder.AddMenuEntry(
				FilterMenuTitles::GetMenuTitle(Type),
				FilterMenuTitles::GetMenuTooltip(Type),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SWwiseBrowser::WwiseTypeFilterExecute, Type),
					FCanExecuteAction::CreateLambda([] { return true; }),
					FIsActionChecked::CreateSP(this, &SWwiseBrowser::WwiseTypeFilterIsChecked, Type)),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("RemoveFilters", LOCTEXT("RemoveFilters", "Remove Filters"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Remove_Title", "Remove All Filters"),
			LOCTEXT("Remove_Tooltip", "Removes all selected filters."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SWwiseBrowser::RemoveFiltersExecute),
				FCanExecuteAction::CreateLambda([] { return true; })),
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}

	return MenuBuilder.MakeWidget();
}

const FSlateBrush* SWwiseBrowser::GetFilterBadgeIcon() const
{
	if (!SoundBankStatusFilter.AreFiltersOff())
	{
		return FAkAppStyle::Get().GetBrush("Icons.BadgeModified");
	}
	if (!UAssetStatusFilter.AreFiltersOff())
	{
		return FAkAppStyle::Get().GetBrush("Icons.BadgeModified");
	}
	if (!WwiseTypeFilter.AreFiltersOff())
	{
		return FAkAppStyle::Get().GetBrush("Icons.BadgeModified");
	}
	return nullptr;
}

void SWwiseBrowser::SoundBankFilterExecute(ESoundBankStatusFilter Filter)
{
	SoundBankStatusFilter.bFilters[Filter] = !SoundBankStatusFilter.bFilters[Filter];
	OnFilterUpdated();
}

bool SWwiseBrowser::SoundBankFilterIsChecked(ESoundBankStatusFilter Filter)
{
	return SoundBankStatusFilter.bFilters[Filter];
}

void SWwiseBrowser::UAssetFilterExecute(EUAssetStatusFilter Filter)
{
	UAssetStatusFilter.bFilters[Filter] = !UAssetStatusFilter.bFilters[Filter];
	OnFilterUpdated();
}

bool SWwiseBrowser::UAssetFilterIsChecked(EUAssetStatusFilter Filter)
{
	return UAssetStatusFilter.bFilters[Filter];
}

void SWwiseBrowser::WwiseTypeFilterExecute(EWwiseTypeFilter Filter)
{
	WwiseTypeFilter.bFilters[Filter] = !WwiseTypeFilter.bFilters[Filter];
	OnFilterUpdated();
}

bool SWwiseBrowser::WwiseTypeFilterIsChecked(EWwiseTypeFilter Filter)
{
	return WwiseTypeFilter.bFilters[Filter];
}

void SWwiseBrowser::RemoveFiltersExecute()
{
	bool bShouldUpdate = false;

	for (auto& bFilter : WwiseTypeFilter.bFilters)
	{
		if(bFilter)
		{
			bShouldUpdate = true;
			bFilter = false;
		}
	}

	for(auto& bFilter : SoundBankStatusFilter.bFilters)
	{
		if (bFilter)
		{
			bShouldUpdate = true;
			bFilter = false;
		}
	}

	for(auto& bFilter : UAssetStatusFilter.bFilters)
	{
		if (bFilter)
		{
			bShouldUpdate = true;
			bFilter = false;
		}
	}

	if(bShouldUpdate)
	{
		OnFilterUpdated();
	}
}

void SWwiseBrowser::SoundBankNotUpToDateExecute()
{
	bool bShouldUpdate = false;
	for (int i = 0; i < ESoundBankStatusFilter::UpToDate; i++)
	{
		if (!SoundBankStatusFilter.bFilters[i])
		{
			SoundBankStatusFilter.bFilters[i] = true;
			bShouldUpdate = true;
		}
	}

	if (SoundBankStatusFilter.bFilters[ESoundBankStatusFilter::UpToDate])
	{
		SoundBankStatusFilter.bFilters[ESoundBankStatusFilter::UpToDate] = false;
		bShouldUpdate = true;
	}

	if (bShouldUpdate)
	{
		OnFilterUpdated();
	}
}

void SWwiseBrowser::UAssetNotUpToDateExecute()
{
	bool bShouldUpdate = false;
	for (int i = 0; i < EUAssetStatusFilter::UAssetUpToDate; i++)
	{
		if (!UAssetStatusFilter.bFilters[i])
		{
			UAssetStatusFilter.bFilters[i] = true;
			bShouldUpdate = true;
		}
	}

	if (UAssetStatusFilter.bFilters[EUAssetStatusFilter::UAssetUpToDate])
	{
		UAssetStatusFilter.bFilters[EUAssetStatusFilter::UAssetUpToDate] = false;
		bShouldUpdate = true;
	}

	if (bShouldUpdate)
	{
		OnFilterUpdated();
	}
}


void SWwiseBrowser::ForceRefresh()
{
	if(!GetProjectName().IsEmpty())
	{
		DataSource->ConstructTree();
	}
}

void SWwiseBrowser::InitialParse()
{
	if(!GetProjectName().IsEmpty())
	{
		DataSource->ConstructTree();
		ConstructTree();
		TreeViewPtr->RequestTreeRefresh();
		ExpandFirstLevel();
	}
}

FText SWwiseBrowser::GetProjectName() const
{
	return DataSource->GetProjectName();
}

FText SWwiseBrowser::GetConnectedWwiseProjectName() const
{
	return DataSource->GetConnectedWwiseProjectName();
}

EWwiseConnectionStatus SWwiseBrowser::IsWaapiAvailable() const
{
	return DataSource->GetWaapiConnectionStatus();
}

EVisibility SWwiseBrowser::IsWarningVisible() const
{
	// Also need to check if WAAPI available
	return GetProjectName().IsEmpty() ? EVisibility::Visible : EVisibility::Hidden;
}

EVisibility SWwiseBrowser::IsWarningNotVisible() const
{
	return GetProjectName().IsEmpty() ? EVisibility::Hidden : EVisibility::Visible;
}

EVisibility SWwiseBrowser::IsItemPlaying(FGuid ItemId) const
{
	return Transport->IsPlaying(ItemId) ? EVisibility::Visible : EVisibility::Hidden;
}

FText SWwiseBrowser::GetWarningText() const
{
	FString soundBankDirectory = WwiseUnrealHelper::GetSoundBankDirectory();
	if (soundBankDirectory.IsEmpty())
	{
		const FText WarningText = LOCTEXT("BrowserSoundBanksFolderEmpty", "Root Output Path in Wwise Integration settings is empty.\nThis folder should match the \"Root Output Path\" in the Wwise Project's SoundBanks settings.");
		return WarningText;
	}

	const FText WarningText = FText::FormatOrdered(LOCTEXT("BrowserMissingSoundBanks", "SoundBank metadata was not found at path specified by the \"Root Output Path\" setting: {0}.\nThis folder should match the \"Root Output Path\" in the Wwise Project's SoundBanks settings.\nEnsure the folders match, and that SoundBanks and JSON metadata are generated.\nPress the \"Refresh\" button to re-parse the generated metadata."), FText::FromString(soundBankDirectory));
	return WarningText;
}

FText SWwiseBrowser::GetConnectionStatusText() const
{
	switch(IsWaapiAvailable())
	{
	case Connected:
		return FText::Format(LOCTEXT("WwiseConnected", "Connected to Wwise Project: {0}"), GetProjectName());
	case SettingDisabled:
		return LOCTEXT("WwiseSettingDisabled", "Not Connected to Wwise. Only objects on disk shown. Enable \"Auto Connect to Waapi\" in the Wwise User Settings to see the Wwise project in the Browser.");
	case WrongProjectOpened:
		return LOCTEXT("WwiseWrongProject", "Not Connected to Wwise. Only objects on disk shown. The wrong Wwise project is opened.");
	case WwiseNotOpen:
	default:
		return LOCTEXT("WwiseNotConnected", "Not Connected to Wwise. Only objects on disk shown. Open Wwise to see the Wwise project in the Browser.");
	}
}

FText SWwiseBrowser::GetSoundBanksLocationText() const
{
	FString RootOutputPath = TEXT("Root Output Path");
	UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>();
	if(UserSettings && !UserSettings->RootOutputPathOverride.Path.IsEmpty())
	{
		RootOutputPath = TEXT("Root Output Path Override");
	}
	return FText::Format(LOCTEXT("RootOutputPath", "{0}: {1}"), FText::FromString(RootOutputPath), FText::FromString(WwiseUnrealHelper::GetSoundBankDirectory()));
}

FSlateColor SWwiseBrowser::GetSoundBanksLocationTextColor() const
{
	UAkSettingsPerUser* UserSettings = GetMutableDefault<UAkSettingsPerUser>();
	if(UserSettings && !UserSettings->RootOutputPathOverride.Path.IsEmpty())
	{
		return FLinearColor(1.f, 0.33f, 0);
	}
	return FLinearColor::Gray;
}

FReply SWwiseBrowser::OnOpenSettingsClicked()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("Project"), FName("Wwise"), FName("Integration"));
	return FReply::Handled();
}

FReply SWwiseBrowser::OnRefreshClicked()
{
	if (FModuleManager::Get().IsModuleLoaded("AudiokineticTools"))
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("SWwiseBrowser::OnRefreshClicked: Reloading project data."));
		FModuleManager::Get().GetModuleChecked<IAudiokineticTools>(FName("AudiokineticTools")).RefreshWwiseProject();
	}
	ForceRefresh();
	return FReply::Handled();
}

FReply SWwiseBrowser::OnGenerateSoundBanksClicked()
{
	UE_LOG(LogAudiokineticTools, Verbose, TEXT("SWwiseBrowser::OnGenerateSoundBanksClicked: Opening Generate SoundBanks Window."));
	AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow();
	return FReply::Handled();
}

FReply SWwiseBrowser::OnReconcileClicked()
{
	if(IsWarningVisible() == EVisibility::Hidden)
	{
		UE_LOG(LogAudiokineticTools, Verbose, TEXT("SWwiseBrowser::OnReconcileClicked: Opening Reconcile Window."));
		CreateReconcileTab();
	}
	else if(EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("NoSoundBanksReconcile", "SoundBanks must be generated before reconciliation. Generate SoundBanks now?")))
	{
		AkAudioBankGenerationHelper::CreateGenerateSoundDataWindow();
	}
	return FReply::Handled();
}

void SWwiseBrowser::OnTreeItemDoubleClicked(FWwiseTreeItemPtr TreeItem)
{
	if (TreeItem->IsBrowserType())
	{
		HandleFindInContentBrowserCommandExecute();
	}

	else if (TreeItem->IsFolder() && TreeItem->GetChildren().Num() > 0)
	{
		ExpandItem(TreeItem, !TreeViewPtr->IsItemExpanded(TreeItem));
	}
}

void SWwiseBrowser::ConstructTree()
{

	UE_LOG(LogAudiokineticTools, Verbose, TEXT("SWwiseBrowser::ConstructTree: Rebuilding Wwise Browser Tree"));

	RootItems.Empty(EWwiseItemType::LastWwiseBrowserType - EWwiseItemType::Event + 1);

	for (int i = EWwiseItemType::Event; i <= EWwiseItemType::LastWwiseBrowserType + 1; ++i)
	{
		FWwiseTreeItemPtr NewRoot = DataSource->GetTreeRootForType(static_cast<EWwiseItemType::Type>(i));

		RootItems.Add(NewRoot);
	}		

	RestoreTreeExpansion(RootItems);
	TreeViewPtr->RequestTreeRefresh();
}

void SWwiseBrowser::ExpandFirstLevel()
{
	// Expand root items and first-level work units.
	for(int32 i = 0; i < RootItems.Num(); i++)
	{
		ExpandItem(RootItems[i], true);
	}
}

void SWwiseBrowser::ExpandParents(FWwiseTreeItemPtr Item)
{
	if(Item->Parent.IsValid())
	{
		ExpandParents(Item->Parent.Pin());
		ExpandItem(Item->Parent.Pin(), true);
	}
}

TSharedRef<ITableRow> SWwiseBrowser::GenerateRow( FWwiseTreeItemPtr TreeItem, const TSharedRef<STableViewBase>& OwnerTable )
{
	check(TreeItem.IsValid());

	TSharedRef<SWwiseBrowserTreeRow> NewRow = SNew(SWwiseBrowserTreeRow, TreeViewPtr.ToSharedRef(), SharedThis(this)).
		Item(TreeItem);

	TreeItem->TreeRow = NewRow;

	return NewRow;
}

void SWwiseBrowser::GetChildrenForTree(FWwiseTreeItemPtr TreeItem, TArray< FWwiseTreeItemPtr >& OutChildren)
{
	if (TreeItem)
	{
		if(TreeItem->ChildCountInWwise)
		{
			if (!LastExpandedPaths.Contains(TreeItem->FolderPath))
			{
				DataSource->ClearEmptyChildren(TreeItem);
				// We add a placeholder item if the children exist, but are not loaded (e.g. for WAAPI). This should never be visible
				if (!TreeItem->GetChildren().Num())
				{
					FWwiseTreeItemPtr EmptyTreeItem = MakeShared<FWwiseTreeItem>(FString::Format(TEXT("Expansion placeholder for {0} "), { TreeItem->FolderPath }), "", nullptr, EWwiseItemType::None, FGuid());
					EmptyTreeItem->Parent = TreeItem;
					TreeItem->AddChild(EmptyTreeItem);
				}
			}
			else
			{
				ExpandItem(TreeItem, true);
			}
		}

		OutChildren = TreeItem->GetChildren();
	}
}

FReply SWwiseBrowser::OnDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	return DoDragDetected(MouseEvent, TreeViewPtr->GetSelectedItems());
}

FReply SWwiseBrowser::DoDragDetected(const FPointerEvent& MouseEvent, const TArray<FWwiseTreeItemPtr>& SelectedItems)
{
	if (!MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Unhandled();
	}

	if (SelectedItems.Num() == 0)
	{
		return FReply::Unhandled();
	}

	UE_LOG(LogAudiokineticTools, Verbose, TEXT("SWwiseBrowser::OnDragDetected: User drag operation started."));

    auto AkSettings = GetMutableDefault<UAkSettings>();
	const FString DefaultPath = AkSettings->DefaultAssetCreationPath;

	TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> DragAssets;
	TSet<FGuid> SeenGuids;
	bool bAllItemsCanBeCreated = true;
	for (auto& WwiseTreeItem : SelectedItems)
	{
		if (!WwiseTreeItem->ItemId.IsValid() && !WwiseTreeItem->IsFolder())
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Cannot drag selected Wwise asset: %s does not have a valid ID"), *(WwiseTreeItem->FolderPath));
			continue;
		}

		if(!WwiseBrowserHelpers::CanCreateAsset(WwiseTreeItem))
		{
			bAllItemsCanBeCreated = false;
		}

		WwiseBrowserHelpers::FindOrCreateAssetsRecursive(WwiseTreeItem,DragAssets,SeenGuids, WwiseBrowserHelpers::EAssetCreationMode::Transient);
	}

	if (DragAssets.Num() == 0 && bAllItemsCanBeCreated)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to find or create Wwise asset '%s'%s in Browser operation"),
			*(SelectedItems[0]->FolderPath), SelectedItems.Num() > 1 ? TEXT(" and others"): TEXT(""));
		return FReply::Unhandled();
	}

	return FReply::Handled().BeginDragDrop(FWwiseAssetDragDropOp::New(DragAssets));
}

void SWwiseBrowser::ImportWwiseAssets(const TArray<FWwiseTreeItemPtr>& SelectedItems, const FString& PackagePath)
{
	UE_LOG(LogAudiokineticTools, Verbose, TEXT("SWwiseBrowser::ImportWwiseAssets: Creating %d assets in %s."), SelectedItems.Num(), *PackagePath);
	TArray<WwiseBrowserHelpers::WwiseBrowserAssetPayload> AssetsToImport;
	TSet<FGuid> SeenGuids;
	for(auto WwiseTreeItem: SelectedItems)
	{
		WwiseBrowserHelpers::FindOrCreateAssetsRecursive(WwiseTreeItem, AssetsToImport, SeenGuids, 
			WwiseBrowserHelpers::EAssetCreationMode::InPackage, PackagePath);

	}

	WwiseBrowserHelpers::SaveSelectedAssets(AssetsToImport, PackagePath, 
			WwiseBrowserHelpers::EAssetCreationMode::InPackage, WwiseBrowserHelpers::EAssetDuplicationMode::DoDuplication);
}

FReply SWwiseBrowser::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey KeyPressed = InKeyEvent.GetKey();

	if (KeyPressed == EKeys::SpaceBar)
	{
		// Play the wwise item.
		if (HandlePlayOrStopWwiseItemCanExecute())
		{
			HandlePlayWwiseItemCommandExecute();
			return FReply::Handled();
		}
	}

	else if (KeyPressed == EKeys::F5)
	{	// Populates the Wwise Browser.
		HandleRefreshWwiseBrowserCommandExecute();
		return FReply::Handled();
	}

	else if ((KeyPressed == EKeys::One) && InKeyEvent.IsControlDown() && InKeyEvent.IsShiftDown())
	{
		// Finds the specified object in the Project Explorer (Sync Group 1).
		if (HandleFindInProjectExplorerWwiseItemCanExecute())
		{
			HandleFindInProjectExplorerWwiseItemCommandExecute();
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

void SWwiseBrowser::PopulateSearchStrings(const FString& FolderName, OUT TArray< FString >& OutSearchStrings) const
{
	OutSearchStrings.Add(FolderName);
}

void SWwiseBrowser::OnSearchBoxChanged(const FText& InSearchText)
{
	SearchBoxFilter->SetRawFilterText(InSearchText);
}

void SWwiseBrowser::SetItemVisibility(FWwiseTreeItemPtr Item, bool IsVisible)
{
	if (Item.IsValid())
	{
		if (IsVisible)
		{
			// Propagate visibility to parents.
			SetItemVisibility(Item->Parent.Pin(), IsVisible);
		}
		Item->IsVisible = IsVisible;
		if (Item->TreeRow.IsValid())
		{
			TSharedRef<SWidget> wid = Item->TreeRow.Pin()->AsWidget();
			wid->SetVisibility(IsVisible ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
}

void SWwiseBrowser::SaveCurrentTreeExpansion()
{
	TSet<FWwiseTreeItemPtr> ExpandedItemSet;
	TreeViewPtr->GetExpandedItems(ExpandedItemSet);

	LastExpandedPaths.Empty();

	for (const auto& Item : ExpandedItemSet)
	{
		if (Item.IsValid())
		{
			// Keep track of the last paths that we broadcasted for expansion reasons when filtering
			LastExpandedPaths.Add(Item->FolderPath);
		}
	}
}

void SWwiseBrowser::RestoreTreeExpansion(const TArray< FWwiseTreeItemPtr >& Items)
{
	TSet<FWwiseTreeItemPtr> ExpandedItemSet;
	TreeViewPtr->GetExpandedItems(ExpandedItemSet);

	LastExpandedPaths.Empty();

	for (const auto& Item : ExpandedItemSet)
	{
		if (Item.IsValid())
		{
			// Keep track of the last paths that we broadcasted for expansion reasons when filtering
			LastExpandedPaths.Add(Item->FolderPath);
		}
	}
}

void SWwiseBrowser::TreeSelectionChanged( FWwiseTreeItemPtr TreeItem, ESelectInfo::Type SelectInfo )
{
	if (AllowTreeViewDelegates)
	{
		const TArray<FWwiseTreeItemPtr> SelectedItems = TreeViewPtr->GetSelectedItems();

		LastSelectedPaths.Empty();
		for (int32 ItemIdx = 0; ItemIdx < SelectedItems.Num(); ++ItemIdx)
		{
			const FWwiseTreeItemPtr Item = SelectedItems[ItemIdx];
			if (Item.IsValid())
			{
				LastSelectedPaths.Add(Item->FolderPath);
			}
		}

		const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
		if (AkSettingsPerUser && AkSettingsPerUser->AutoSyncSelection &&
			DataSource->GetWaapiConnectionStatus() == Connected && SelectInfo != ESelectInfo::Direct)
		{
			DataSource->HandleFindWwiseItemInProjectExplorerCommandExecute(SelectedItems);
		}
	}
}

void SWwiseBrowser::TreeExpansionChanged( FWwiseTreeItemPtr TreeItem, bool bIsExpanded )
{
	auto Items = TreeViewPtr->GetSelectedItems();
	if (bIsExpanded)
	{
		// If we have a filter applied, it is assumed that everything is loaded
		if (!IsFiltering())
		{
			TArray<FWwiseTreeItemPtr> OutChildren;
		}
	}
	// If the item is not expanded we don't need to request the server to get any information(the children are hidden).
	else
	{
		LastExpandedPaths.Remove(TreeItem->FolderPath);
		return;
	}

	LastExpandedPaths.Add(TreeItem->FolderPath);

	TreeViewPtr->SetSelectedItems(Items);
}

void SWwiseBrowser::ExpandItem(FWwiseTreeItemPtr TreeItem, bool bShouldExpand)
{
	TreeViewPtr->SetItemExpansion(TreeItem, bShouldExpand);
	TreeItem->IsExpanded = bShouldExpand;
}

void SWwiseBrowser::ExpandItem(FWwiseTreeItemPtr WaapiTreeItem)
{
	if(auto TreeItem = GetTreeItemFromWaapiItem(WaapiTreeItem))
	{
		LastExpandedPaths.Add(TreeItem->FolderPath);
		ExpandItem(TreeItem, true);
	}
}

bool SWwiseBrowser::IsItemExpanded(FWwiseTreeItemPtr TreeItem)
{
	return TreeViewPtr->IsItemExpanded(TreeItem);
}

void SWwiseBrowser::GetTreeItemsFromWaapi(const TArray<TSharedPtr<FWwiseTreeItem>>& WaapiTreeItems, TArray<TSharedPtr<FWwiseTreeItem>>& TreeItems)
{
	for(auto& WaapiTreeItem : WaapiTreeItems)
	{
		if(auto TreeItem = GetTreeItemFromWaapiItem(WaapiTreeItem))
		{
			TreeItems.Add(TreeItem);
		}
	}
}

FWwiseTreeItemPtr SWwiseBrowser::GetTreeItemFromWaapiItem(FWwiseTreeItemPtr WaapiTreeItem)
{
	for (auto& RootItem : RootItems)
	{
		if (auto ResultTreeItem = RootItem->FindItemRecursive(WaapiTreeItem))
		{
			return ResultTreeItem;
		}
		if (auto ResultTreeItem = RootItem->FindItemRecursive(WaapiTreeItem->FolderPath))
		{
			return ResultTreeItem;
		}
	}
	return FWwiseTreeItemPtr();
}

void SWwiseBrowser::ExpandItems(const TArray<FWwiseTreeItemPtr>& Items)
{
	for(auto& Item : Items)
	{
		ExpandItem(Item, true);
	}
}

void SWwiseBrowser::UpdateWaapiSelection(const TArray<TSharedPtr<FWwiseTreeItem>>& WaapiTreeItems)
{
	TArray<TSharedPtr<FWwiseTreeItem>> TreeItems;
	GetTreeItemsFromWaapi(WaapiTreeItems, TreeItems);
	TreeViewPtr->RequestTreeRefresh();
	if (TreeItems.Num() > 0)
	{
		
		TreeViewPtr->ClearSelection();
		TreeViewPtr->SetSelectedItems(TreeItems);
		TreeViewPtr->RequestScrollIntoView(TreeItems[0]);
	}
	AllowTreeViewDelegates = true;
}

void SWwiseBrowser::HandleImportWwiseItemCommandExecute() const
{
	// If not prompting individual files, prompt the user to select a target directory.
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		FString LastWwiseImportPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT);
		FString FolderName;
		const FString Title = NSLOCTEXT("UnrealEd", "ChooseADirectory", "Choose A Directory").ToString();
		const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			Title,
			LastWwiseImportPath,
			FolderName
		);

		if (bFolderSelected)
		{
			if (!FPaths::IsUnderDirectory(FolderName, FPaths::ProjectContentDir()))
			{

				const FText FailReason = FText::FormatOrdered(LOCTEXT("CannotImportWwiseItem", "Cannot import into {0}. Folder must be in content directory."), FText::FromString(FolderName));
				FMessageDialog::Open(EAppMsgType::Ok, FailReason);
				UE_LOG(LogAudiokineticTools, Error, TEXT("%s"), *FailReason.ToString());
				return;
			}
			FPaths::MakePathRelativeTo(FolderName, *FPaths::ProjectContentDir());
			FString PackagePath = TEXT("/Game");
			if (!FolderName.IsEmpty())
			{
				PackagePath = PackagePath / FolderName;
			}
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_IMPORT, PackagePath);
			ImportWwiseAssets(TreeViewPtr->GetSelectedItems(), PackagePath);
		}
	}
}

void SWwiseBrowser::HandleReconcileWwiseItemCommandExecute() const
{
	CreateReconcileTab();
}

void SWwiseBrowser::SetupColumns(SHeaderRow& HeaderRow)
{

	Columns.Empty();
	HeaderRow.ClearColumns();

	auto SoundColumn = MakeShared<FSoundPlayingColumn>(*this);
	auto WwiseObjectsColumn = MakeShared<FWwiseBrowserTreeColumn>(*this);
	auto WwiseStatusColumn = MakeShared<FWwiseStatusColumn>(*this);
	auto WwiseUEAssetStatusColumn = MakeShared<FWwiseUEAssetStatusColumn>();
	auto SoundBankStatusColumn = MakeShared<FSoundBankStatusColumn>();

	auto SoundArgs = SoundColumn->ConstructHeaderRowColumn();
	auto WwiseObjectArgs = WwiseObjectsColumn->ConstructHeaderRowColumn();
	auto WwiseStatusArgs = WwiseStatusColumn->ConstructHeaderRowColumn();
	auto WwiseUEAssetStatusArgs = WwiseUEAssetStatusColumn->ConstructHeaderRowColumn();
	auto SoundBankStatusArgs = SoundBankStatusColumn->ConstructHeaderRowColumn();

	Columns.Add(SoundArgs._ColumnId, SoundColumn);
	Columns.Add(WwiseObjectArgs._ColumnId, WwiseObjectsColumn);
	Columns.Add(WwiseStatusArgs._ColumnId, WwiseStatusColumn);
	Columns.Add(WwiseUEAssetStatusArgs._ColumnId, WwiseUEAssetStatusColumn);
	Columns.Add(SoundBankStatusArgs._ColumnId, SoundBankStatusColumn);

	HeaderRow.AddColumn(SoundArgs);
	HeaderRowWidget->AddColumn(WwiseObjectArgs);
	HeaderRowWidget->AddColumn(WwiseStatusArgs);
	HeaderRowWidget->AddColumn(WwiseUEAssetStatusArgs);
	HeaderRowWidget->AddColumn(SoundBankStatusArgs);
}

void SWwiseBrowser::OnTextFilterUpdated()
{
	if (GetFilterText().IsEmpty())
	{
		bIsFilterApplied = false;
	}

	DataSource->ApplyTextFilter(SearchBoxFilter);
}

void SWwiseBrowser::OnFilterUpdated()
{
	if (RootItems.Num() == 0)
	{
		return;
	}

	ApplyFilter();
	TreeViewPtr->RequestTreeRefresh();
}

void SWwiseBrowser::ApplyFilter()
{
	AllowTreeViewDelegates = false;

	if (!bIsFilterApplied)
	{
		SaveCurrentTreeExpansion();
	}

	DataSource->ApplyFilter(SoundBankStatusFilter, UAssetStatusFilter, WwiseTypeFilter);

	RootItems.Empty();
	for (int i = EWwiseItemType::Event; i <= EWwiseItemType::LastWwiseBrowserType + 1; ++i)
	{
		FWwiseTreeItemPtr NewRoot = DataSource->GetTreeRootForType(static_cast<EWwiseItemType::Type>(i));

		RootItems.Add(NewRoot);
	}

	bIsFilterApplied = true;
	AllowTreeViewDelegates = true;
}

bool SWwiseBrowser::IsFiltering()
{
	return !GetFilterText().IsEmpty();
}

FText SWwiseBrowser::GetFilterText()
{
	return SearchBoxFilter.IsValid() ? SearchBoxFilter->GetRawFilterText(): FText();
}

FString SWwiseBrowser::GetFilterString()
{
	return GetFilterText().ToString();
}

TAttribute<FText> SWwiseBrowser::GetFilterHighlightText() const
{
	return SearchBoxFilter.IsValid() ? SearchBoxFilter->GetRawFilterText(): FText();
}


void SWwiseBrowser::CreateReconcileTab() const
{
	auto SelectedItems = TreeViewPtr->GetSelectedItems();

	//Nothing items are selected, Select everything
	if(SelectedItems.Num() == 0)
	{
		SelectedItems = RootItems;
	}
	TArray<FWwiseReconcileItem> ReconcileItems;
	if (auto WwiseReconcile = IWwiseReconcile::Get())
	{
		WwiseReconcile->ConvertWwiseItemTypeToReconcileItem(SelectedItems, ReconcileItems);
		if (ReconcileItems.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoReconcile", "Selected Items do not need to be reconciled."));
			return;
		}

		auto Window = SNew(SWindow)
			.Title(LOCTEXT("AkAudioWwiseReconcileTabTitle", "Reconcile Unreal Assets"))
			.SizingRule(ESizingRule::UserSized)
			.MinWidth(850)
			.MinHeight(400)
			.ClientSize(FVector2D(1100.f, 400.f));

		auto ReconcileWindow = SNew(SWwiseReconcile, ReconcileItems, Window);
		Window->SetContent(ReconcileWindow);
		Window->SetAsModalWindow();
		GEditor->EditorAddModalWindow(Window);
	}
	else
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get Wwise Reconcile Module."));
	}
}

#undef LOCTEXT_NAMESPACE
