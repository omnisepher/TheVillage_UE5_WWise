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

#include "WaapiDataSource.h"

#include <Wwise/Stats/AudiokineticTools.h>

#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AkWaapiUtils.h"
#include "IAudiokineticTools.h"
#include "PackageTools.h"
#include "WwiseItemType.h"
#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Misc/Paths.h"
#include "WaapiPicker/WwiseTreeItem.h"

FWaapiDataSource::~FWaapiDataSource()
{
	TearDown();
}

bool FWaapiDataSource::Init()
{
	auto WaapiClient = FAkWaapiClient::Get();

	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to get the Waapi client."));
		return false;
	}

	ProjectLoadedHandle = WaapiClient->OnProjectLoaded.AddRaw(
		this, &FWaapiDataSource::OnProjectLoadedCallback);

	ConnectionLostHandle = WaapiClient->OnConnectionLost.AddRaw(
		this, &FWaapiDataSource::OnConnectionLostCallback);

	ClientBeginDestroyHandle = WaapiClient->OnClientBeginDestroy.AddRaw(
		this, &FWaapiDataSource::OnWaapiClientBeginDestroyCallback);

	SubscribeWaapiCallbacks();

	return true;
}

bool FWaapiDataSource::TearDown()
{
	auto WaapiClient = FAkWaapiClient::Get();

	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to get the Waapi client."));
		return false;
	}

	if (ProjectLoadedHandle.IsValid())
	{
		WaapiClient->OnProjectLoaded.Remove(ProjectLoadedHandle);
		ProjectLoadedHandle.Reset();
	}

	if (ConnectionLostHandle.IsValid())
	{
		WaapiClient->OnConnectionLost.Remove(ConnectionLostHandle);
		ConnectionLostHandle.Reset();
	}

	RemoveClientCallbacks();

	WaapiClient->OnClientBeginDestroy.Remove(ClientBeginDestroyHandle);

	return true;
}

void FWaapiDataSource::ConstructTree(bool bShouldRefresh)
{
	SCOPED_AUDIOKINETICTOOLS_EVENT_2(TEXT("FWaapiDataSource::ConstructTree"))

	if (IsProjectLoaded() != EWwiseConnectionStatus::Connected)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to construct Waapi Tree. The Wwise project is not connected."));
		return;
	}
	{
		FScopeLock AutoLock(&WaapiRootItemsLock);
		RootItems.Empty();
		NodesByPath.Empty();
	}

	for (int i = EWwiseItemType::Event; i <= EWwiseItemType::LastWwiseBrowserType; ++i)
	{

		FWwiseTreeItemPtr TreeRoot = ConstructTreeRoot(static_cast<EWwiseItemType::Type>(i));
		{
			FScopeLock AutoLock(&WaapiRootItemsLock);
			RootItems.Add(TreeRoot);
			NodesByPath.Add(TreeRoot->FolderPath, TreeRoot);
			LoadChildren(TreeRoot);
		}
	}

	if(bShouldRefresh)
	{
		WaapiDataSourceRefreshed.ExecuteIfBound();
	}
}

FWwiseTreeItemPtr FWaapiDataSource::ConstructTreeRoot(EWwiseItemType::Type Type)
{
	FGuid InItemGuid = FGuid();
	uint32 ShortId = 0;
	TSharedPtr<FJsonObject> Result;
	uint32_t ItemChildrenCount = 0;
	FString Path = WwiseWaapiHelper::BACK_SLASH + EWwiseItemType::FolderNames[static_cast<int>(Type)];

	if (IsProjectLoaded() != EWwiseConnectionStatus::Connected)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to construct Waapi Root Items. The Wwise project is not connected."));
		return nullptr;
	}

	// Get the root item of the given hierarchy using its path
	if (LoadWaapiInfo(WwiseWaapiHelper::PATH, Path, Result, {}))
	{
		const TSharedPtr<FJsonObject> ItemInfoObj = Result->GetArrayField(WwiseWaapiHelper::RETURN)[0]->AsObject();
		const FString ItemIdString = ItemInfoObj->GetStringField(WwiseWaapiHelper::ID);
		Path = ItemInfoObj->GetStringField(WwiseWaapiHelper::PATH);
		ItemChildrenCount = ItemInfoObj->GetNumberField(WwiseWaapiHelper::CHILDREN_COUNT);
		FGuid::ParseExact(ItemIdString, EGuidFormats::DigitsWithHyphensInBraces, InItemGuid);
	}

	else
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to get information for Waapi items at : %s"), *Path);
		
		if (Result && Result->GetStringField(TEXT("uri")) == TEXT("ak.wwise.locked"))
		{
			UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get information for Waapi items. The item is locked."));
		}
	}

	// Create the new tree item
	FWwiseTreeItemPtr NewRootParent = MakeShared<FWwiseTreeItem>(EWwiseItemType::BrowserDisplayNames[static_cast<int>(Type)], Path, nullptr, EWwiseItemType::Folder, InItemGuid);
	NewRootParent->ShortId = ShortId;
	NewRootParent->bWaapiRefExists = true;
	NewRootParent->ChildCountInWwise = ItemChildrenCount;

	return NewRootParent;
}

FWwiseTreeItemPtr FWaapiDataSource::ConstructWwiseTreeItem(const TSharedPtr<FJsonObject>& InItemInfoObj)
{
	static const FString ValidPaths[] = {
		EWwiseItemType::FolderNames[EWwiseItemType::Event],
		EWwiseItemType::FolderNames[EWwiseItemType::AuxBus],
		EWwiseItemType::FolderNames[EWwiseItemType::ActorMixer],
		EWwiseItemType::FolderNames[EWwiseItemType::GameParameter],
		EWwiseItemType::FolderNames[EWwiseItemType::State],
		EWwiseItemType::FolderNames[EWwiseItemType::Switch],
		EWwiseItemType::FolderNames[EWwiseItemType::Trigger],
		EWwiseItemType::FolderNames[EWwiseItemType::AcousticTexture],
		EWwiseItemType::FolderNames[EWwiseItemType::EffectShareSet]
	};

	static auto IsValidPath = [](const FString& Input, const auto& Source) -> bool {
		for (const auto& Item : Source)
		{
			if (Input.StartsWith(WwiseWaapiHelper::BACK_SLASH + Item))
			{
				return true;
			}
		}
		return false;
	};

	const FString ItemTypeString = InItemInfoObj->GetStringField(WwiseWaapiHelper::TYPE);

	auto ItemType = EWwiseItemType::FromString(ItemTypeString);
	if (ItemType == EWwiseItemType::None)
	{
		return {};
	}

	const FString ItemPath = InItemInfoObj->GetStringField(WwiseWaapiHelper::PATH);
	if (IsValidPath(ItemPath, ValidPaths))
	{
		const FString ItemIdString = InItemInfoObj->GetStringField(WwiseWaapiHelper::ID);
		FGuid InItemId = FGuid::NewGuid();
		FGuid::ParseExact(ItemIdString, EGuidFormats::DigitsWithHyphensInBraces, InItemId);
		const FString ItemName = InItemInfoObj->GetStringField(WwiseWaapiHelper::NAME);

		if (ItemName.IsEmpty())
		{
			return {};
		}

		const uint32_t ItemChildrenCount = InItemInfoObj->GetNumberField(WwiseWaapiHelper::CHILDREN_COUNT);

		if (ItemType == EWwiseItemType::StandaloneWorkUnit)
		{
			FString WorkUnitType;
			if (InItemInfoObj->TryGetStringField(WwiseWaapiHelper::WORKUNIT_TYPE, WorkUnitType) && WorkUnitType == "FOLDER")
			{
				ItemType = EWwiseItemType::PhysicalFolder;
			}
		}

		FWwiseTreeItemPtr TreeItem = MakeShared<FWwiseTreeItem>(ItemName, ItemPath, nullptr, ItemType, InItemId);
		TreeItem->WaapiName = ItemName;
		TreeItem->bWaapiRefExists = true;
		if (TreeItem->IsFolder())
		{
			NodesByPath.Add(TreeItem->FolderPath, TreeItem);
		}

		if ((ItemType != EWwiseItemType::Event) && (ItemType != EWwiseItemType::Sound))
		{
			TreeItem->ChildCountInWwise = ItemChildrenCount;
		}
		return TreeItem;
	}

	return {};
}

FWwiseTreeItemPtr FWaapiDataSource::ConstructWwiseTreeItem(const TSharedPtr<FJsonValue>& InJsonItem)
{
	return ConstructWwiseTreeItem(InJsonItem->AsObject());
}

FWwiseTreeItemPtr FWaapiDataSource::FindItemFromPath(const FWwiseTreeItemPtr& InParentItem,
	const FString& InCurrentItemPath)
{
	return InParentItem->FindItemRecursive(InCurrentItemPath);
}

FWwiseTreeItemPtr FWaapiDataSource::FindItemFromPath(const FString& InCurrentItemPath)
{
	auto FoundItem = NodesByPath.Find(InCurrentItemPath);

	if (FoundItem)
	{
		return *FoundItem;
	}

	return nullptr;
}

FWwiseTreeItemPtr FWaapiDataSource::FindOrConstructTreeItemFromJsonObject(
	const TSharedPtr<FJsonObject>& ObjectJson)
{
	FString objectPath;
	if (!ObjectJson->TryGetStringField(WwiseWaapiHelper::PATH, objectPath))
	{
		return {};
	}

	FString stringId;
	if (!ObjectJson->TryGetStringField(WwiseWaapiHelper::ID, stringId))
	{
		return {};
	}

	FGuid id;
	FGuid::ParseExact(stringId, EGuidFormats::DigitsWithHyphensInBraces, id);

	TArray<FString> pathParts;
	objectPath.ParseIntoArray(pathParts, *WwiseWaapiHelper::BACK_SLASH);

	if (pathParts.Num() == 0)
	{
		return {};
	}

	TSharedPtr<FWwiseTreeItem> treeItem;
	TArray<TSharedPtr<FWwiseTreeItem>>* children = &RootItems;
	TArray<TSharedPtr<FWwiseTreeItem>> itemsToExpand;
	FString folderPath;
	
	for (auto& part : pathParts)
	{
		folderPath += FString::Printf(TEXT("%s%s"), *WwiseWaapiHelper::BACK_SLASH, *part);

		bool found = false;
		for (auto& item : *children)
		{
			if (item->ItemId == id)
			{
				treeItem = item;
				break;
			}

			if (item->FolderPath == folderPath)
			{
				if (!item->IsExpanded)
				{
					const FString itemIdStringField = item->ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces);

					TSharedPtr<FJsonObject> Result;
					bool bIdFound = false;
					if(item->ItemId.IsValid())
					{
						// Request data from Wwise UI using WAAPI and use them to create a Wwise tree item, getting the informations from a specific "ID".
						bIdFound = CallWaapiGetInfoFrom(WwiseWaapiHelper::ID, itemIdStringField, Result, { { WwiseWaapiHelper::SELECT, { WwiseWaapiHelper::CHILDREN }, {} } });
					}

					if (!bIdFound)
					{
						// Folders do not have a specific "ID". Search with the "PATH" instead.
						if (!CallWaapiGetInfoFrom(WwiseWaapiHelper::PATH, item->FolderPath, Result, { { WwiseWaapiHelper::SELECT, { WwiseWaapiHelper::CHILDREN }, {} } }))
						{
							UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to get information from id : %s"), *itemIdStringField);
							return {};
						}
					}
					itemsToExpand.Add(item);
				}
				treeItem = item;
				children = treeItem->GetChildrenMutable();
				found = true;
			}
		}

		if (treeItem && treeItem->ItemId == id)
		{
			break;
		}

		if (!found)
		{
			return {};
		}
	}

	if (treeItem.IsValid() && treeItem->ItemId != id)
	{
		return {};
	}

	WwiseExpansionChange.ExecuteIfBound(itemsToExpand);

	return treeItem;
}

void FWaapiDataSource::FindAndCreateItems(FWwiseTreeItemPtr CurrentItem)
{
	FString LastPathVisited = CurrentItem->FolderPath;
	LastPathVisited.RemoveFromEnd(WwiseWaapiHelper::BACK_SLASH + CurrentItem->DisplayName);
	FWwiseTreeItemPtr RootItem = GetRootItem(CurrentItem->FolderPath);
	if (!RootItem || CurrentItem->FolderPath == RootItem->FolderPath)
	{
		return;
	}

	if (LastPathVisited == RootItem->FolderPath)
	{
		RootItem->AddChild(CurrentItem);
		return;
	}

	FWwiseTreeItemPtr ParentItem = FindItemFromPath(RootItem, LastPathVisited);
	if (ParentItem.IsValid())
	{
		ParentItem->AddChild(CurrentItem);
	}

	else
	{
		TSharedPtr<FJsonObject> Result;
		// Request data from Wwise UI using WAAPI and use them to create a Wwise tree item, getting the informations from a specific "PATH".
		if (LoadWaapiInfo(WwiseWaapiHelper::PATH, LastPathVisited, Result, {}))
		{
			// Recover the information from the Json object Result and use it to construct the tree item.
			FWwiseTreeItemPtr NewRootItem = ConstructWwiseTreeItem(Result->GetArrayField(WwiseWaapiHelper::RETURN)[0]);
			CurrentItem->Parent = NewRootItem;
			NewRootItem->AddChild(CurrentItem);
			FindAndCreateItems(NewRootItem);
		}
		else
		{
			UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to get information from Waapi path : %s"), *LastPathVisited);
		}
	}
}

FString FWaapiDataSource::GetItemWorkUnitPath(FWwiseTreeItemPtr InItem)
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Unable to connect to localhost"));
		return {};
	}

	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	{
		TSharedPtr<FJsonObject> From = MakeShared<FJsonObject>();
		From->SetArrayField(WwiseWaapiHelper::PATH, TArray<TSharedPtr<FJsonValue>> { MakeShared<FJsonValueString>(InItem->FolderPath) });
		Args->SetObjectField(WwiseWaapiHelper::FROM, From);
	}

	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	Options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>> { MakeShared<FJsonValueString>(WwiseWaapiHelper::FILEPATH) });

#if AK_SUPPORT_WAAPI
	TSharedPtr<FJsonObject> outJsonResult;
	if (!WaapiClient->Call(ak::wwise::core::object::get, Args, Options, outJsonResult))
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Call Failed to get %s's Work Unit Path"), *InItem->DisplayName);
		return {};
	}

	FString Path = outJsonResult->GetArrayField(WwiseWaapiHelper::RETURN)[0]->AsObject()->GetStringField(
		WwiseWaapiHelper::FILEPATH);

	return Path;
#else
	return {};
#endif
}

FWwiseTreeItemPtr FWaapiDataSource::GetRootItem(EWwiseItemType::Type RootType)
{
	if (IsProjectLoaded() != EWwiseConnectionStatus::Connected)
	{
		return nullptr;
	}

	check(RootType <= EWwiseItemType::LastWwiseBrowserType)

	if (RootType > RootItems.Num() - 1)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get Waapi Root Items. Index out of range."));
		return nullptr;
	}

	return RootItems[RootType];
}

FWwiseTreeItemPtr FWaapiDataSource::GetRootItem(const FString& InFullPath)
{
	for (int i = EWwiseItemType::Event; i <= EWwiseItemType::LastWwiseBrowserType; ++i)
	{
		if (InFullPath.StartsWith(RootItems[i]->FolderPath))
		{
			return RootItems[i];
		}
	}

	return {};
}

FWwiseTreeItemPtr FWaapiDataSource::LoadFilteredRootItem(EWwiseItemType::Type RootType, TSharedPtr<StringFilter> CurrentFilter)
{

	check(CurrentFilter.IsValid())

	if (RootType > RootItems.Num() - 1)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get Waapi Root Items. Index out of range."));
		return nullptr;
	}

	FString CurrentFilterText = CurrentFilter->GetRawFilterText().ToString();
	FString Path = WwiseWaapiHelper::BACK_SLASH + EWwiseItemType::FolderNames[static_cast<int>(RootType)];
	FScopeLock Autolock(&WaapiRootItemsLock);
	for (int i = EWwiseItemType::Event; i <= EWwiseItemType::LastWwiseBrowserType; ++i)
	{
		RootItems[i]->EmptyChildren();
	}
	TSharedPtr<FJsonObject> Result;
	if (LoadWaapiInfo(WwiseWaapiHelper::PATH, Path, Result,
		{
			{ WwiseWaapiHelper::SELECT , { WwiseWaapiHelper::DESCENDANTS }, {} },
			{ WwiseWaapiHelper::WHERE, { WwiseWaapiHelper::NAMECONTAINS, CurrentFilterText}, {} }
		}))
	{
		// Recover the information from the Json object Result and use it to construct the tree item.
		TArray<TSharedPtr<FJsonValue>> SearchResultArray = Result->GetArrayField(WwiseWaapiHelper::RETURN);
		if (SearchResultArray.Num())
		{
			// The map contains each path and the correspondent object of the search result.
			TMap < FString, FWwiseTreeItemPtr> SearchedResultTreeItem;
			for (int i = 0; i < SearchResultArray.Num(); i++)
			{
				// Fill the map with the path-object elements.
				FWwiseTreeItemPtr NewRootChild = ConstructWwiseTreeItem(SearchResultArray[i]);
				if (NewRootChild.IsValid())
				{
					FindAndCreateItems(NewRootChild);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Failed to get information from Waapi item search : %s"), *CurrentFilterText);
	}

	return GetRootItem(RootType);
}

bool FWaapiDataSource::LoadWaapiInfo(const FString& InFromField, const FString& InFromString,
                                     TSharedPtr<FJsonObject>& OutJsonResult, const TArray<WaapiTransformStringField>& TransformFields)
{
	auto WaapiClient = FAkWaapiClient::Get();

	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Unable to connect to localhost"));
		return false;
	}

#if AK_SUPPORT_WAAPI

	// Construct the Json arguments from a specific id/path
	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	{
		TSharedPtr<FJsonObject> From = MakeShared<FJsonObject>();
		From->SetArrayField(InFromField, TArray<TSharedPtr<FJsonValue>> {MakeShared<FJsonValueString>(InFromString)});
		Args->SetObjectField(WwiseWaapiHelper::FROM, From);
	}

	if (TransformFields.Num())
	{
		TArray<TSharedPtr<FJsonValue>> Transform;

		for (const auto& TransformValue : TransformFields)
		{
			TSharedPtr<FJsonObject> InsideTransform = MakeShared<FJsonObject>();
			TArray<TSharedPtr<FJsonValue>> JsonArray;
			for (auto TransformStringValueArg : TransformValue.valueStringArgs)
			{
				JsonArray.Add(MakeShared<FJsonValueString>(TransformStringValueArg));
			}
			for (auto TransformNumberValueArg : TransformValue.valueNumberArgs)
			{
				JsonArray.Add(MakeShared<FJsonValueNumber>(TransformNumberValueArg));
			}
			InsideTransform->SetArrayField(TransformValue.keyArg, JsonArray);
			Transform.Add(MakeShared<FJsonValueObject>(InsideTransform));
		}
		Args->SetArrayField(WwiseWaapiHelper::TRANSFORM, Transform);
	}

	// Construct the Options Json object : Getting specific infos to construct the WwiseTreeItem "id - name - type - childrenCount - path - parent"
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	Options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>>
	{
		MakeShared<FJsonValueString>(WwiseWaapiHelper::ID),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::NAME),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::TYPE),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::CHILDREN_COUNT),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::PATH),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::WORKUNIT_TYPE),
	});

	return WaapiClient->Call(ak::wwise::core::object::get, Args, Options, OutJsonResult);
#else
	return false;
#endif
}

bool FWaapiDataSource::IsTreeDirty()
{
	return false;
}

void FWaapiDataSource::Tick(const double InCurrentTime, const float InDeltaTime)
{
}

int32 FWaapiDataSource::LoadChildren(const FGuid& InParentId, const FString& InParentPath, TArray<FWwiseTreeItemPtr>& OutChildren)
{

	UE_LOG(LogAudiokineticTools, VeryVerbose, TEXT("Loading Children for Waapi item %s, id: %s"), *InParentPath, *InParentId.ToString())

	FString InFromField;
	FString InStringField;

	OutChildren = {};

	if (InParentId.IsValid())
	{
		InFromField = WwiseWaapiHelper::ID;
		InStringField = InParentId.ToString(EGuidFormats::DigitsWithHyphensInBraces);
	}

	else
	{
		InFromField = WwiseWaapiHelper::PATH;
		InStringField = InParentPath;
	}

	TSharedPtr<FJsonObject> Result;

	// Request data from Wwise UI using WAAPI and use them to create a Wwise tree item, getting the information from a specific "ID".
	if (!LoadWaapiInfo(WwiseWaapiHelper::PATH, InParentPath, Result,
		{
			{ WwiseWaapiHelper::SELECT , { WwiseWaapiHelper::DESCENDANTS }, {} },
			{ WwiseWaapiHelper::WHERE, { WwiseWaapiHelper::NAMECONTAINS, ""}, {}}
		}))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to load Waapi information for %s."), *InStringField);
		return 0;
	}

	TArray<TSharedPtr<FJsonValue>> StructJsonArray = Result->GetArrayField(WwiseWaapiHelper::RETURN);

	if (StructJsonArray.Num() == 0)
	{
		return 0;
	}

	FWwiseTreeItemPtr ExistingParent = nullptr;
	if (auto ParentPtr = NodesByPath.Find(InParentPath))
	{
		ExistingParent = *ParentPtr;
	}

	if(!ExistingParent)
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("No Parent found at %s."), *InParentPath)
		return 0;
	}

	ExistingParent->EmptyChildren();

	for (int i = 0; i < StructJsonArray.Num(); i++)
	{
		FWwiseTreeItemPtr NewChild = ConstructWwiseTreeItem(StructJsonArray[i]);
		if (NewChild)
		{
			FindAndCreateItems(NewChild);
		}
	}

	return OutChildren.Num();
}

int32 FWaapiDataSource::LoadChildren(FWwiseTreeItemPtr ParentTreeItem)
{
	SCOPED_AUDIOKINETICTOOLS_EVENT_2(TEXT("FWaapiDataSource::LoadChildren"))

	if (!ParentTreeItem)
	{
		return 0;
	}

	TArray<FWwiseTreeItemPtr> OutChildren;

	LoadChildren(ParentTreeItem->ItemId, ParentTreeItem->FolderPath, OutChildren);

	return ParentTreeItem->GetChildren().Num();
}

int32 FWaapiDataSource::GetChildItemCount(const FWwiseTreeItemPtr& InParentItem)
{
	return InParentItem->ChildCountInWwise;
}

FString FWaapiDataSource::LoadProjectName()
{
	auto WaapiClient = FAkWaapiClient::Get();

	if (!WaapiClient)
	{
		return {};
	}

	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	{
		TSharedPtr<FJsonObject> OfType = MakeShared<FJsonObject>();
		OfType->SetArrayField(WwiseWaapiHelper::OF_TYPE, TArray<TSharedPtr<FJsonValue>> { MakeShared<FJsonValueString>(WwiseWaapiHelper::PROJECT) });
		Args->SetObjectField(WwiseWaapiHelper::FROM, OfType);
	}

	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	{
		Options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>>
		{
			MakeShared<FJsonValueString>(WwiseWaapiHelper::NAME),
			MakeShared<FJsonValueString>(WwiseWaapiHelper::FILEPATH),
		});
	}

#if AK_SUPPORT_WAAPI
	TSharedPtr<FJsonObject> outJsonResult;
	if (WaapiClient->Call(ak::wwise::core::object::get, Args, Options, outJsonResult))
	{
		// Recover the information from the Json object Result and use it to get the item id.
		TArray<TSharedPtr<FJsonValue>> StructJsonArray = outJsonResult->GetArrayField(WwiseWaapiHelper::RETURN);
		if (StructJsonArray.Num())
		{
			FString Path = StructJsonArray[0]->AsObject()->GetStringField(WwiseWaapiHelper::FILEPATH);
			return FPaths::GetCleanFilename(Path);
		}
		else
		{
			UE_LOG(LogAudiokineticTools, Log, TEXT("Unable to get the project name"));
			return {};
		}
	}
#endif
	return {};
}

void FWaapiDataSource::HandleFindWwiseItemInProjectExplorerCommandExecute(
	const TArray<FWwiseTreeItemPtr>& SelectedItems) const
{
	auto waapiClient = FAkWaapiClient::Get();
	if (!waapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Unable to connect to localhost"));
		return;
	}

	if (SelectedItems.Num() == 0)
		return;

	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	Args->SetStringField(WwiseWaapiHelper::COMMAND, WwiseWaapiHelper::FIND_IN_PROJECT_EXPLORER);
	TArray<TSharedPtr<FJsonValue>> SelectedObjects;
	for (auto SelectedItem : SelectedItems)
	{
		if(SelectedItem->ItemId.IsValid())
		{
			SelectedObjects.Add(MakeShared<FJsonValueString>(SelectedItem->ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces)));
		}
	}
	Args->SetArrayField(WwiseWaapiHelper::OBJECTS, SelectedObjects);

#if AK_SUPPORT_WAAPI
	TSharedPtr<FJsonObject> Result;
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	if (!waapiClient->Call(ak::wwise::ui::commands::execute, Args, Options, Result))
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Call Failed"));
	}
#endif
}

void FWaapiDataSource::OnProjectLoadedCallback()
{
	SubscribeWaapiCallbacks();
	ConstructTree(true);
}

void FWaapiDataSource::OnConnectionLostCallback()
{
	{
		FScopeLock AutoLock(&WaapiRootItemsLock);
		RootItems.Empty();
		NodesByPath.Empty();
	}

	WaapiDataSourceRefreshed.ExecuteIfBound();

	UnsubscribeWaapiCallbacks();
}

void FWaapiDataSource::OnWaapiClientBeginDestroyCallback()
{
	auto waapiClient = FAkWaapiClient::Get();
	if (waapiClient == nullptr)
		return;

	if (ProjectLoadedHandle.IsValid())
	{
		waapiClient->OnProjectLoaded.Remove(ProjectLoadedHandle);
		ProjectLoadedHandle.Reset();
	}

	if (ConnectionLostHandle.IsValid())
	{
		waapiClient->OnConnectionLost.Remove(ConnectionLostHandle);
		ConnectionLostHandle.Reset();
	}

	UnsubscribeWaapiCallbacks();
}

void FWaapiDataSource::OnWaapiRenamed(uint64_t Id, TSharedPtr<FJsonObject> Response)
{
	FString OldName;
	if (Response->TryGetStringField(WwiseWaapiHelper::OLD_NAME, OldName) && OldName.IsEmpty())
	{
		return;
	}
	
	FString NewName;
	if (Response->TryGetStringField(WwiseWaapiHelper::NEW_NAME, NewName) && NewName.IsEmpty())
	{
		return;
	}

	const TSharedPtr<FJsonObject>* ObjectJsonPtr = nullptr;
	if (!Response->TryGetObjectField(WwiseWaapiHelper::OBJECT, ObjectJsonPtr))
	{
		return;
	}

	FString ObjectPath;
	if (!ObjectJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::PATH, ObjectPath))
	{
		return;
	}

	FString OldPath(ObjectPath);
	OldPath.RemoveFromEnd(WwiseWaapiHelper::BACK_SLASH + NewName);
	auto TreeItem = FindItemFromPath(OldPath);

	FString ItemIdString;
	if(ObjectJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::ID, ItemIdString))
	{
		FGuid ItemId = FGuid::NewGuid();
		FGuid::ParseExact(ItemIdString, EGuidFormats::DigitsWithHyphensInBraces, ItemId);
		if(ItemsToCreate.Contains(ItemId))
		{
			ItemsToCreate.Remove(ItemId);
			if (TreeItem)
			{
				auto ChildItem = ConstructWwiseTreeItem(*ObjectJsonPtr);
				if (ChildItem)
				{
					TreeItem->AddChild(ChildItem);
					if(ChildItem->IsFolder())
					{
						NodesByPath.Add(ChildItem->FolderPath, ChildItem);
					}
				}
				WaapiDataSourceRefreshed.ExecuteIfBound();
				return;
			}
		}
	}

	OldPath += WwiseWaapiHelper::BACK_SLASH + OldName;
	NodesByPath.Remove(OldPath);
	if(TreeItem)
	{
		for(auto& Child : TreeItem->GetChildren())
		{
			if(Child->DisplayName == OldName)
			{
				Child->DisplayName = UPackageTools::SanitizePackageName(NewName);
				Child->WaapiName = NewName;
				break;
			}
		}		
	}

	WaapiDataSourceRefreshed.ExecuteIfBound();
}

void FWaapiDataSource::OnWaapiChildAdded(uint64_t Id, TSharedPtr<FJsonObject> Response)
{
	const TSharedPtr<FJsonObject>* ChildJsonPtr = nullptr;
	if (!Response->TryGetObjectField(WwiseWaapiHelper::CHILD, ChildJsonPtr))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get Child Object field."));
		return;
	}

	FString ChildName;

	if (!ChildJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::NAME, ChildName))
	{
		return;
	}

	FString ChildPath;
	if (!ChildJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::PATH, ChildPath))
	{
		return;
	}

	const TSharedPtr<FJsonObject>* ParentJsonPtr = nullptr;
	if (!Response->TryGetObjectField(WwiseWaapiHelper::PARENT, ParentJsonPtr))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get Parent Object field."));
		return;
	}

	FString TreeItemParentPath;

	if (!ParentJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::PATH, TreeItemParentPath))
	{
		return;
	}

	FString TreeItemParentName;

	if (!ParentJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::NAME, TreeItemParentName))
	{
		return;
	}

	//The Item was copy pasted. Add it to the list to be created with the rename callback
	if(TreeItemParentPath == TreeItemParentName)
	{
		FString ItemIdString = ChildJsonPtr->Get()->GetStringField(WwiseWaapiHelper::ID);
		FGuid ItemId = FGuid::NewGuid();
		FGuid::ParseExact(ItemIdString, EGuidFormats::DigitsWithHyphensInBraces, ItemId);
		ItemsToCreate.Add(ItemId);
		return;
	}

	bool bChildAdded = false;
	if (auto ParentItem = NodesByPath.Find(TreeItemParentPath))
	{
		if (!ParentItem->Get()->FindItemRecursive(ChildPath))
		{
			auto ChildItem = ConstructWwiseTreeItem(*ChildJsonPtr);
			if (ChildItem)
			{
				ParentItem->Get()->AddChild(ChildItem);
				if(ChildItem->IsFolder())
				{
					NodesByPath.Add(ChildItem->FolderPath, ChildItem);
				}
				bChildAdded = true;
			}
		}
	}

	if(bChildAdded)
	{
		WaapiDataSourceRefreshed.ExecuteIfBound();
	}

}

void FWaapiDataSource::OnWaapiChildRemoved(uint64_t Id, TSharedPtr<FJsonObject> Response)
{
	const TSharedPtr<FJsonObject>* ChildJsonPtr = nullptr;
	if (!Response->TryGetObjectField(WwiseWaapiHelper::CHILD, ChildJsonPtr))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get Child Object field."));
		return;
	}

	const TSharedPtr<FJsonObject>* ParentJsonPtr = nullptr;
	if (!Response->TryGetObjectField(WwiseWaapiHelper::PARENT, ParentJsonPtr))
	{
		UE_LOG(LogAudiokineticTools, Error, TEXT("Failed to get Parent Object field."));
		return;
	}

	FString ItemIdString;
	FString ItemName;
	
	if (!ChildJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::ID, ItemIdString))
	{
		return;
	}

	if (!ChildJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::NAME, ItemName))
	{
		return;
	}
	
	FString TreeItemParentPath;

	if (!ParentJsonPtr->Get()->TryGetStringField(WwiseWaapiHelper::PATH, TreeItemParentPath))
	{
		return;
	}

	// This seems to be the path to the removed item, and not its parent?
	if (auto ParentTreeItem = NodesByPath.Find(TreeItemParentPath))
	{
		FGuid ItemId = FGuid(ItemIdString);
		NodesByPath.Remove(TreeItemParentPath / ItemName);
		(*ParentTreeItem)->RemoveChild(ItemId);
	}

	WaapiDataSourceRefreshed.ExecuteIfBound();

}

void FWaapiDataSource::OnWwiseSelectionChanged(uint64_t Id, TSharedPtr<FJsonObject> Response)
{
	AsyncTask(ENamedThreads::GameThread, [this, Response]()
	{
		const UAkSettingsPerUser* AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
		if (AkSettingsPerUser && AkSettingsPerUser->AutoSyncSelection)
		{
			const TArray<TSharedPtr<FJsonValue>>* objectsJsonArray = nullptr;
			if (Response->TryGetArrayField(WwiseWaapiHelper::OBJECTS, objectsJsonArray))
			{
				TArray<TSharedPtr<FWwiseTreeItem>> TreeItems;
				for (auto JsonObject : *objectsJsonArray)
				{
					auto TreeItem = FindOrConstructTreeItemFromJsonObject(JsonObject->AsObject());
					if (TreeItem)
					{
						TreeItems.Add(TreeItem);
					}
				}
				WwiseSelectionChange.ExecuteIfBound(TreeItems);
			}
		}
	});
}

void FWaapiDataSource::SubscribeWaapiCallbacks()
{
	struct SubscriptionData
	{
		const char* Uri;
		WampEventCallback Callback;
		uint64* SubscriptionId;
	};

#if AK_SUPPORT_WAAPI
	const SubscriptionData Subscriptions[] = {
		{ak::wwise::core::object::nameChanged, WampEventCallback::CreateRaw(this, &FWaapiDataSource::OnWaapiRenamed), &WaapiSubscriptionIds.Renamed},
		{ak::wwise::core::object::childAdded, WampEventCallback::CreateRaw(this, &FWaapiDataSource::OnWaapiChildAdded), &WaapiSubscriptionIds.ChildAdded},
		{ak::wwise::core::object::childRemoved, WampEventCallback::CreateRaw(this, &FWaapiDataSource::OnWaapiChildRemoved), &WaapiSubscriptionIds.ChildRemoved},
		{ak::wwise::ui::selectionChanged, WampEventCallback::CreateRaw(this, &FWaapiDataSource::OnWwiseSelectionChanged), &WaapiSubscriptionIds.SelectionChanged},
	};
#endif

	auto waapiClient = FAkWaapiClient::Get();
	if (!waapiClient)
	{
		return;
	}

	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	Options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>>
	{
		MakeShared<FJsonValueString>(WwiseWaapiHelper::ID),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::NAME),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::TYPE),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::CHILDREN_COUNT),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::PATH),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::PARENT),
		MakeShared<FJsonValueString>(WwiseWaapiHelper::WORKUNIT_TYPE),
	});

	TSharedPtr<FJsonObject> Result;
#if AK_SUPPORT_WAAPI
	for (auto& Subscription : Subscriptions)
	{
		if (*Subscription.SubscriptionId == 0)
		{
			waapiClient->Subscribe(Subscription.Uri,
				Options,
				Subscription.Callback,
				*Subscription.SubscriptionId,
				Result
			);
		}
	}
#endif
}

EWwiseConnectionStatus FWaapiDataSource::IsProjectLoaded()
{
	if(UAkSettingsPerUser* AkUserSettings = GetMutableDefault<UAkSettingsPerUser>())
	{
		if(!AkUserSettings->bAutoConnectToWAAPI)
		{
			return EWwiseConnectionStatus::SettingDisabled;
		}
	}
	if(FAkWaapiClient::IsProjectLoaded())
	{
		return EWwiseConnectionStatus::Connected;
	}
	if(FAkWaapiClient::Get()->bIsWrongProjectLoaded)
	{
		return EWwiseConnectionStatus::WrongProjectOpened;
	}
	return EWwiseConnectionStatus::WwiseNotOpen;
}

void FWaapiDataSource::SelectInProjectExplorer(TArray<FWwiseTreeItemPtr>& InTreeItems)
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Unable to connect to localhost"));
		return;
	}

	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	Args->SetStringField(WwiseWaapiHelper::COMMAND, WwiseWaapiHelper::FIND_IN_PROJECT_EXPLORER);
	TArray<TSharedPtr<FJsonValue>> SelectedObjects;
	for (const auto& TreeItem : InTreeItems)
	{
		SelectedObjects.Add(MakeShared<FJsonValueString>(TreeItem->ItemId.ToString(EGuidFormats::DigitsWithHyphensInBraces)));
	}
	Args->SetArrayField(WwiseWaapiHelper::OBJECTS, SelectedObjects);

#if AK_SUPPORT_WAAPI
	TSharedPtr<FJsonObject> Result;
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	if (!WaapiClient->Call(ak::wwise::ui::commands::execute, Args, Options, Result))
	{
		UE_LOG(LogAudiokineticTools, Log, TEXT("Call Failed to Select Items in Project Explorer."));
	}
#endif

}

bool FWaapiDataSource::CallWaapiGetInfoFrom(const FString& inFromField, const FString& inFromString, TSharedPtr<FJsonObject>& outJsonResult, const TArray<TransformStringField>& TransformFields)
{
	SCOPED_AUDIOKINETICTOOLS_EVENT_3(TEXT("FWaapiDataSource::CallWaapiGetInfoFrom"))
	
	auto waapiClient = FAkWaapiClient::Get();
	if (!waapiClient)
	{
		return false;
	}
#if AK_SUPPORT_WAAPI
	// Construct the arguments Json object : Getting infos "from - a specific id/path"
	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	{
		TSharedPtr<FJsonObject> from = MakeShared<FJsonObject>();
		from->SetArrayField(inFromField, TArray<TSharedPtr<FJsonValue>> { MakeShared<FJsonValueString>(inFromString) });
		Args->SetObjectField(WwiseWaapiHelper::FROM, from);

		// In case we would recover the children of the object that have the id : ID or the path : PATH, then we set isGetChildren to true.

		if (TransformFields.Num())
		{
			TArray<TSharedPtr<FJsonValue>> transform;

			for (auto TransformValue : TransformFields)
			{
				TSharedPtr<FJsonObject> insideTransform = MakeShared<FJsonObject>();
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				for (auto TransformStringValueArg : TransformValue.valueStringArgs)
				{
					JsonArray.Add(MakeShared<FJsonValueString>(TransformStringValueArg));
				}
				for (auto TransformNumberValueArg : TransformValue.valueNumberArgs)
				{
					JsonArray.Add(MakeShared<FJsonValueNumber>(TransformNumberValueArg));
				}
				insideTransform->SetArrayField(TransformValue.keyArg, JsonArray);
				transform.Add(MakeShared<FJsonValueObject>(insideTransform));
			}
			Args->SetArrayField(WwiseWaapiHelper::TRANSFORM, transform);
		}
	}

	// Construct the Options Json object : Getting specific infos to construct the wwise tree item "id - name - type - childrenCount - path - parent"
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	Options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>>
	{
		MakeShared<FJsonValueString>(WwiseWaapiHelper::ID),
			MakeShared<FJsonValueString>(WwiseWaapiHelper::NAME),
			MakeShared<FJsonValueString>(WwiseWaapiHelper::TYPE),
			MakeShared<FJsonValueString>(WwiseWaapiHelper::CHILDREN_COUNT),
			MakeShared<FJsonValueString>(WwiseWaapiHelper::PATH),
			MakeShared<FJsonValueString>(WwiseWaapiHelper::WORKUNIT_TYPE),
	});

	// Request data from Wwise using WAAPI

	return waapiClient->Call(ak::wwise::core::object::get, Args, Options, outJsonResult);
#endif
	return false;
}

void FWaapiDataSource::UnsubscribeWaapiCallbacks()
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (!WaapiClient)
	{
		return;
	}

	auto DoUnsubscribe = [WaapiClient](uint64& subscriptionId) {
		if (subscriptionId > 0)
		{
			TSharedPtr<FJsonObject> Result;
			WaapiClient->Unsubscribe(subscriptionId, Result);
			subscriptionId = 0;
		}
	};

	DoUnsubscribe(WaapiSubscriptionIds.Renamed);
	DoUnsubscribe(WaapiSubscriptionIds.ChildAdded);
	DoUnsubscribe(WaapiSubscriptionIds.ChildRemoved);
	DoUnsubscribe(WaapiSubscriptionIds.SelectionChanged);
}

void FWaapiDataSource::RemoveClientCallbacks()
{
	auto WaapiClient = FAkWaapiClient::Get();
	if (WaapiClient == nullptr)
	{
		return;
	}

	if (ProjectLoadedHandle.IsValid())
	{
		WaapiClient->OnProjectLoaded.Remove(ProjectLoadedHandle);
		ProjectLoadedHandle.Reset();
	}

	if (ConnectionLostHandle.IsValid())
	{
		WaapiClient->OnConnectionLost.Remove(ConnectionLostHandle);
		ConnectionLostHandle.Reset();
	}

	UnsubscribeWaapiCallbacks();
}
