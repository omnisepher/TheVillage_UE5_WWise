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

#include "WaapiPicker/WwiseTreeItem.h"

bool FWwiseTreeItem::UEAssetExists() const
{
	return Assets.Num() > 0;
}

bool FWwiseTreeItem::WwiseBankRefExists() const
{
#if WITH_EDITORONLY_DATA
	return WwiseItemRef.IsValid() && WwiseItemRef->GUID.IsValid();
#endif
	return false;
}

bool FWwiseTreeItem::WaapiRefExists() const
{
	return bWaapiRefExists;
}

bool FWwiseTreeItem::IsRenamedInWwise() const
{
	if (!WwiseBankRefExists() || !WaapiRefExists())
	{
		return false;
	}
	return DisplayName != WaapiName;
}

bool FWwiseTreeItem::IsDeletedInWwise() const
{
	return !WaapiRefExists() && WwiseBankRefExists();
}

bool FWwiseTreeItem::IsNotInWwiseOrSoundBank() const
{
	return !WaapiRefExists() && !WwiseBankRefExists();
}

bool FWwiseTreeItem::IsNewInWwise() const
{
	return WaapiRefExists() && !WwiseBankRefExists();
}

bool FWwiseTreeItem::IsMovedInWwise() const
{
	return !bSameLocation && !IsRenamedInWwise();
}

bool FWwiseTreeItem::IsSoundBankUpToDate() const
{
	return WwiseBankRefExists() && WaapiRefExists()
		&& !IsRenamedInWwise() && !IsMovedInWwise();
}

bool FWwiseTreeItem::IsRenamedInSoundBank() const
{
	if (!WwiseBankRefExists() || !HasUniqueUAsset())
	{
		return false;
	}
	FString NameToCompare = DisplayName;
	if(IsOfType({ EWwiseItemType::Switch , EWwiseItemType::State }))
	{
		NameToCompare = GetSwitchAssetName();
	}
	return FName(NameToCompare) != UAssetName;
}

bool FWwiseTreeItem::IsUAssetMissing() const
{
	return WwiseBankRefExists() && !UEAssetExists();
}

bool FWwiseTreeItem::IsUAssetOrphaned() const
{
	return !WwiseBankRefExists() && UEAssetExists();
}

bool FWwiseTreeItem::IsNotInSoundBankOrUnreal() const
{
	return !WwiseBankRefExists() && !UEAssetExists();
}

bool FWwiseTreeItem::IsUAssetUpToDate() const
{
	return WwiseBankRefExists() && HasUniqueUAsset() && !IsRenamedInSoundBank() && !IsUAssetOutOfDate();
}

bool FWwiseTreeItem::HasUniqueUAsset() const
{
	return Assets.Num() == 1;
}

bool FWwiseTreeItem::HasMultipleUAssets() const
{
	return Assets.Num() > 1;
}

bool FWwiseTreeItem::IsUAssetOutOfDate() const
{
	for(auto& Asset : Assets)
	{
		uint32 AssetShortId = 0;
		FGuid AssetGuid;
		auto GuidValue = Asset.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseGuid));
		auto ShortIdValue = Asset.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseShortId));
		auto WwiseName = Asset.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(FWwiseObjectInfo, WwiseName));
		if (GuidValue.IsSet())
		{
			FString GuidAsString = GuidValue.GetValue();
			FGuid Guid = FGuid(GuidAsString);
			AssetGuid = Guid;
		}
		if (ShortIdValue.IsSet())
		{
			AssetShortId = FCString::Strtoui64(*ShortIdValue.GetValue(), NULL, 10);
		}
		if (AssetGuid != ItemId || AssetShortId != ShortId || WwiseName != DisplayName)
		{
			return true;
		}
	}
	return false;
}

bool FWwiseTreeItem::IsItemUpToDate() const
{
	return IsUAssetUpToDate() && IsSoundBankUpToDate();
}

bool FWwiseTreeItem::IsFolder() const
{
	return IsAuxBus() || IsOfType({ 
					EWwiseItemType::StandaloneWorkUnit, 
					EWwiseItemType::NestedWorkUnit, 
					EWwiseItemType::Folder,
					EWwiseItemType::Bus,
					EWwiseItemType::MotionBus,
					EWwiseItemType::PhysicalFolder,  
					EWwiseItemType::SwitchContainer, 
					EWwiseItemType::SwitchGroup, 
					EWwiseItemType::StateGroup, 
					EWwiseItemType::RandomSequenceContainer 
					});
}

bool FWwiseTreeItem::IsAuxBus() const
{
	return IsOfType({
					EWwiseItemType::AuxBus
					});
}

bool FWwiseTreeItem::ShouldDisplayInfo() const
{
	return !(IsFolder() && !IsAuxBus());
}

bool FWwiseTreeItem::IsRootItem() const
{
	return !Parent.Pin();
}

TSharedPtr<FWwiseTreeItem> FWwiseTreeItem::GetRoot()
{
	if (!Parent.IsValid())
	{
		return MakeShared<FWwiseTreeItem>(*this);
	}

	auto Root = Parent.Pin();
	while (Root->Parent.IsValid())
	{
		Root = Root->Parent.Pin();
	}
	return Root;
}

void FWwiseTreeItem::SetWaapiRef(bool bExistsInWaapi)
{
	bWaapiRefExists = bExistsInWaapi;
}

FString FWwiseTreeItem::GetSwitchAssetName() const
{
	if(Parent.IsValid())
	{
		return Parent.Pin()->DisplayName + "-" + DisplayName;
	}
	return FString();
}

const FString FWwiseTreeItem::GetDefaultAssetName() const
{
	if (IsOfType({ EWwiseItemType::Switch , EWwiseItemType::State }))
	{
		return GetSwitchAssetName();
	}
	return DisplayName;
}

void FWwiseTreeItem::AddChild(TSharedPtr<FWwiseTreeItem> Child)
{
	Child->Parent = TWeakPtr<FWwiseTreeItem>(this->AsShared());
	m_Children.Add(Child);
	ChildCountInWwise = m_Children.Num();
}

void FWwiseTreeItem::AddChildren(TArray<TSharedPtr<FWwiseTreeItem>> Children)
{
	for (auto Child : Children)
	{
		Child->Parent = TWeakPtr<FWwiseTreeItem>(this->AsShared());
		m_Children.Add(Child);
	}
	ChildCountInWwise = m_Children.Num();
}

void FWwiseTreeItem::EmptyChildren()
{
	m_Children.Empty();
	ChildCountInWwise = m_Children.Num();
}

void FWwiseTreeItem::RemoveChild(const FGuid& childGuid)
{
	m_Children.RemoveAll([childGuid](TSharedPtr<FWwiseTreeItem> child) { return child->ItemId == childGuid; });
	ChildCountInWwise = m_Children.Num();
}

void FWwiseTreeItem::RemoveChild(const TSharedPtr< FWwiseTreeItem> child)
{
	m_Children.Remove(child);
	ChildCountInWwise = m_Children.Num();
}

void FWwiseTreeItem::RemoveChildren(const TArray<TSharedPtr<FWwiseTreeItem>> Children)
{
	for (auto& Child : Children)
	{
		m_Children.Remove(Child);
	}
	ChildCountInWwise = m_Children.Num();
}

/** Returns true if this item is a child of the specified item */
bool FWwiseTreeItem::IsChildOf(const FWwiseTreeItem& InParent)
{
	auto CurrentParent = Parent.Pin();
	while (CurrentParent.IsValid())
	{
		if (CurrentParent.Get() == &InParent)
		{
			return true;
		}

		CurrentParent = CurrentParent->Parent.Pin();
	}

	return false;
}

bool FWwiseTreeItem::IsBrowserType() const
{
	return IsOfType({ EWwiseItemType::Event,
			EWwiseItemType::Bus,
			EWwiseItemType::AuxBus,
			EWwiseItemType::AcousticTexture,
			EWwiseItemType::State,
			EWwiseItemType::Switch,
			EWwiseItemType::GameParameter,
			EWwiseItemType::Trigger,
			EWwiseItemType::EffectShareSet
		});
}

bool FWwiseTreeItem::IsOfType(const TArray<EWwiseItemType::Type>& Types) const
{
	for (const auto& Type : Types)
	{
		if (ItemType == Type)
		{
			return true;
		}
	}

	return false;
}

bool FWwiseTreeItem::IsNotOfType(const TArray<EWwiseItemType::Type>& Types) const
{
	return !IsOfType(Types);
}

/** Returns the child item by name or NULL if the child does not exist */
TSharedPtr<FWwiseTreeItem> FWwiseTreeItem::GetChild(const FString& InChildName)
{
	for (int32 ChildIdx = 0; ChildIdx < m_Children.Num(); ++ChildIdx)
	{
		if (m_Children[ChildIdx]->DisplayName == InChildName)
		{
			return m_Children[ChildIdx];
		}
	}

	return TSharedPtr<FWwiseTreeItem>();
}

/** Returns the child item by name or NULL if the child does not exist */
TSharedPtr<FWwiseTreeItem> FWwiseTreeItem::GetChild(const FGuid& InGuid, const AkUInt32 InShortId,  const FString& InChildName)
{
	for (int32 ChildIdx = 0; ChildIdx < m_Children.Num(); ++ChildIdx)
	{
		if (m_Children[ChildIdx]->ItemId == InGuid && !IsFolder())
		{
			return m_Children[ChildIdx];
		}
		if (m_Children[ChildIdx]->ShortId == InShortId && InShortId > 0)
		{
			return m_Children[ChildIdx];
		}
		if (m_Children[ChildIdx]->DisplayName == InChildName)
		{
			return m_Children[ChildIdx];
		}
	}

	return TSharedPtr<FWwiseTreeItem>();
}

/** Finds the child who's path matches the one specified */
TSharedPtr<FWwiseTreeItem> FWwiseTreeItem::FindItemRecursive(const FString& InFullPath)
{
	if (InFullPath == FolderPath)
	{
		return SharedThis(this);
	}

	for (int32 ChildIdx = 0; ChildIdx < m_Children.Num(); ++ChildIdx)
	{
		if (InFullPath.StartsWith(m_Children[ChildIdx]->FolderPath))
		{
			const TSharedPtr<FWwiseTreeItem>& Item = m_Children[ChildIdx]->FindItemRecursive(InFullPath);
			if (Item.IsValid())
			{
				return Item;
			}
		}
	}

	return TSharedPtr<FWwiseTreeItem>(NULL);

}

/** Finds the child who's Guid matches the one specified */
TSharedPtr<FWwiseTreeItem> FWwiseTreeItem::FindItemRecursive(const  TSharedPtr<FWwiseTreeItem>& InItem)
{
	if(InItem->ItemType == ItemType)
	{
		if (InItem->ItemId == ItemId)
		{
			return SharedThis(this);
		}
	
		if(InItem->ShortId == ShortId && ShortId > 0)
		{
			return SharedThis(this);
		}
	
		if(InItem->DisplayName == DisplayName && !InItem->ItemId.IsValid())
		{
			return SharedThis(this);
		}
	}

	for (int32 ChildIdx = 0; ChildIdx < m_Children.Num(); ++ChildIdx)
	{
		const TSharedPtr<FWwiseTreeItem>& Item = m_Children[ChildIdx]->FindItemRecursive(InItem);
		if (Item.IsValid())
		{
			return Item;
		}
	}

	return TSharedPtr<FWwiseTreeItem>(NULL);
}
/** Sort the children by name */
void FWwiseTreeItem::SortChildren()
{
	m_Children.Sort(FCompareWwiseTreeItem());
}