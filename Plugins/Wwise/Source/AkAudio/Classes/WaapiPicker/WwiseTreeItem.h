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
	WwiseTreeItem.h
------------------------------------------------------------------------------------*/
#pragma once

#include "AkAudioType.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameEngine.h"
#include "Widgets/Views/STableRow.h"
#include "WwiseItemType.h"
#if WITH_EDITORONLY_DATA
#include "Wwise/Metadata/WwiseMetadataBasicReference.h"
#endif
/*------------------------------------------------------------------------------------
	WwiseTreeItem
------------------------------------------------------------------------------------*/

struct AKAUDIO_API FWwiseTreeItem : public TSharedFromThis<FWwiseTreeItem>
{
private:
	TArray< TSharedPtr<FWwiseTreeItem> > m_Children;

public:
	/** Name to display */
	FString DisplayName;
	/** The path of the tree item including the name */
	FString FolderPath;
	/** Type of the item */
	EWwiseItemType::Type ItemType = EWwiseItemType::None;
	/** Id of the item */
	FGuid ItemId;
	/** ShortId of the item*/
	uint32 ShortId = 0;

	/** The children of this tree item */
	const TArray< TSharedPtr<FWwiseTreeItem> > GetChildren() { return m_Children;  }
	TArray< TSharedPtr<FWwiseTreeItem> >* GetChildrenMutable() { return &m_Children;  }
	
	/** The number of children of this tree item requested from Wwise*/
	uint32_t ChildCountInWwise = 0;

	/** The parent folder for this item */
	TWeakPtr<FWwiseTreeItem> Parent;

	/** The row in the tree view associated to this item */
	TWeakPtr<ITableRow> TreeRow;

	/** Should this item be visible? */
	bool IsVisible = true;

	/** The Assets associated with the Tree Item*/
	TArray<FAssetData> Assets;

	/** The name of the UAsset referenced by this item. If there are more than one, takes the first one found*/
	FName UAssetName;

	/** The name of the item in the Wwise project*/
	FString WaapiName;

#if WITH_EDITORONLY_DATA
	/** Reference to the item in the Wwise Project database */
	TSharedPtr<FWwiseMetadataBasicReference> WwiseItemRef;
#endif

	/** Is this item active in the currently opened project? */
	bool bWaapiRefExists = false;

	/** Is this item in the same path in Wwise and the SoundBanks? */
	bool bSameLocation = true;

	bool IsExpanded = false;

	bool UEAssetExists() const;

	bool WwiseBankRefExists() const;

	bool WaapiRefExists() const;

	bool IsRenamedInWwise() const;

	bool IsDeletedInWwise() const;

	bool IsNotInWwiseOrSoundBank() const;

	bool IsNewInWwise() const;

	bool IsMovedInWwise() const;

	bool IsSoundBankUpToDate() const;

	bool IsRenamedInSoundBank() const;

	bool IsUAssetMissing() const;

	bool IsUAssetOrphaned() const;

	bool IsNotInSoundBankOrUnreal() const;

	bool IsUAssetUpToDate() const;

	bool HasUniqueUAsset() const;

	bool HasMultipleUAssets() const;

	bool IsUAssetOutOfDate() const;
	
	bool IsItemUpToDate() const;

	bool IsFolder() const;

	bool IsAuxBus() const;

	bool ShouldDisplayInfo() const;

	bool IsRootItem() const;

	TSharedPtr<FWwiseTreeItem> GetRoot();

	void SetWaapiRef(bool bExistsInWaapi);

	FString GetSwitchAssetName() const;

	const FString GetDefaultAssetName() const;

	/** Constructor */
	FWwiseTreeItem(FString InDisplayName, FString InFolderPath, TSharedPtr<FWwiseTreeItem> InParent, EWwiseItemType::Type InItemType, const FGuid& InItemId)
		: DisplayName(MoveTemp(InDisplayName))
		, FolderPath(MoveTemp(InFolderPath))
		, ItemType(MoveTemp(InItemType))
		, ItemId(InItemId)
		, ChildCountInWwise(m_Children.Num())
		, Parent(MoveTemp(InParent))
	{
	}

#if WITH_EDITORONLY_DATA
	FWwiseTreeItem(const FWwiseMetadataBasicReference& ItemRef, TSharedPtr<FWwiseTreeItem> InParent, EWwiseItemType::Type InItemType)
		: ItemType(InItemType)
		, ChildCountInWwise(m_Children.Num())
		, Parent(MoveTemp(InParent))
	{
		WwiseItemRef = MakeShared<FWwiseMetadataBasicReference>(ItemRef.Id, ItemRef.Name, ItemRef.ObjectPath, ItemRef.GUID);
		ItemId = WwiseItemRef->GUID;
		DisplayName = WwiseItemRef->Name.ToString();
		FolderPath = WwiseItemRef->ObjectPath.ToString();
	}
#endif

	void AddChild(TSharedPtr<FWwiseTreeItem> Child);

	void AddChildren(TArray<TSharedPtr<FWwiseTreeItem>> Children);

	void EmptyChildren();

	void RemoveChild(const FGuid& childGuid);

	void RemoveChild(const TSharedPtr< FWwiseTreeItem> child);

	void RemoveChildren(const TArray<TSharedPtr<FWwiseTreeItem>> Children);
	
	/** Returns true if this item is a child of the specified item */
	bool IsChildOf(const FWwiseTreeItem& InParent);

	bool IsBrowserType() const;

	bool IsOfType(const TArray<EWwiseItemType::Type>& Types) const;

	bool IsNotOfType(const TArray<EWwiseItemType::Type>& Types) const;

	/** Returns the child item by name or NULL if the child does not exist */
	TSharedPtr<FWwiseTreeItem> GetChild(const FString& InChildName);

	/** Returns the child item by name or NULL if the child does not exist */
	TSharedPtr<FWwiseTreeItem> GetChild(const FGuid& InGuid, const AkUInt32 InShortId, const FString& InChildName);

	/** Finds the child who's path matches the one specified */
	TSharedPtr<FWwiseTreeItem> FindItemRecursive(const FString& InFullPath);

	/** Finds the child who's Guid matches the one specified */
	TSharedPtr<FWwiseTreeItem> FindItemRecursive(const TSharedPtr<FWwiseTreeItem>& InItem);

	struct FCompareWwiseTreeItem
	{
		template<typename CT>
		inline int StringCompareLogical(const CT* pA1, const CT* pA2) const
		{
			if (pA1 && pA2)
			{
				while (*pA1)
				{
					if (!*pA2)
					{
						// We've iterated through all the characters of the RHS but
						// there are characters left on the LHS
						return 1;
					}
					else if (TChar<CT>::IsDigit(*pA1))
					{
						// LHS is a digit but RHS is not
						if (!TChar<CT>::IsDigit(*pA2))
							return -1;

						// Both sides are digits, parse the numbers and compare them
						CT* pEnd1 = nullptr;
						CT* pEnd2 = nullptr;
						const auto i1 = TCString<CT>::Strtoi(pA1, &pEnd1, 10);
						const auto i2 = TCString<CT>::Strtoi(pA2, &pEnd2, 10);

						if (i1 < i2)
							return -1;
						else if (i1 > i2)
							return 1;

						pA1 = pEnd1;
						pA2 = pEnd2;
					}
					else if (TChar<CT>::IsDigit(*pA2))
					{
						// LHS is not a digit but RHS is
						return 1;
					}
					else
					{
						// Neither side is a digit, do a case-insensitive comparison
						int diff = TChar<CT>::ToLower(*pA1) - TChar<CT>::ToLower(*pA2);
						if (diff > 0)
							return 1;
						else if (diff < 0)
							return -1;

						++pA1;
						++pA2;
					}
				}

				if (*pA2)
				{
					// We've iterated through all the characters of the LHS but
					// there are characters left on the RHS
					return -1;
				}
			}

			return 0;
		}

		FORCEINLINE bool operator()( TSharedPtr<FWwiseTreeItem> A, TSharedPtr<FWwiseTreeItem> B ) const
		{
			// Items are sorted like so:
			// 1- Physical folders, sorted alphabetically
			// 1- WorkUnits, sorted alphabetically
			// 2- Virtual folders, sorted alphabetically
			// 3- Normal items, sorted alphabetically
			if( A->ItemType == B->ItemType)
			{
				return StringCompareLogical(*A->DisplayName, *B->DisplayName) < 0;
			}
			else if( A->ItemType == EWwiseItemType::PhysicalFolder )
			{
				return true;
			}
			else if( B->ItemType == EWwiseItemType::PhysicalFolder )
			{
				return false;
			}
			else if( A->ItemType == EWwiseItemType::StandaloneWorkUnit || A->ItemType == EWwiseItemType::NestedWorkUnit )
			{
				return true;
			}
			else if( B->ItemType == EWwiseItemType::StandaloneWorkUnit || B->ItemType == EWwiseItemType::NestedWorkUnit )
			{
				return false;
			}
			else if( A->ItemType == EWwiseItemType::Folder )
			{
				return true;
			}
			else if( B->ItemType == EWwiseItemType::Folder )
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	};

	/** Sort the children by name */
	void SortChildren();
};