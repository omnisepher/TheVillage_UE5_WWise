/***********************************************************************
  The content of this file includes source code for the sound engine
  portion of the AUDIOKINETIC Wwise Technology and constitutes "Level
  Two Source Code" as defined in the Source Code Addendum attached
  with this file.  Any use of the Level Two Source Code shall be
  subject to the terms and conditions outlined in the Source Code
  Addendum and the End User License Agreement for Wwise(R).

  Copyright (c) 2024 Audiokinetic Inc.
 ***********************************************************************/

#pragma once

#include <AK/SoundEngine/Common/AkHashTableTypes.h>
#include <AK/Tools/Common/AkBitFuncs.h>

// Inline-able functions for AK::HashTable 
// Exposing enough functionality to be use by plug-in services and other things in non-header files
// For plug-ins, further functionality is provided by IAkPluginServiceHashTable
// For core soundengine execution, further functionality is provided in AkHashTable.h
namespace AK
{

namespace HashTable
{
namespace Internal
{
	template<typename KeyType>
	inline AkInt32 IdealPos(KeyType uKey, AkInt32 iEntriesMask)
	{
		return AkInt32(uKey) & iEntriesMask;
	}

	// returns how far away the current slot is from the key's ideal position
	template<typename KeyType>
	AkInt32 DistanceFromIdealPos(AkInt32 iSlot, KeyType uKey, AkInt32 iEntriesMask)
	{
		// subtraction against an unmasked key and then masking afterwards,
		// will give same result as if we masked the slot and key individually, first
		// (also wraparound of the slot relative to the ukey also washes away with the masking)
		return (iSlot - AkInt32(uKey)) & iEntriesMask;
	}

	// Internal helper function for AkHashTableBase; don't call this directly, use AK::HashTable::Get* functions instead
	template<typename KeyType>
	inline AkInt32 LinearProbe(const KeyType* pKeys, const bool* pbSlotOccupied, KeyType uKey, AkInt32 iSlot, AkUInt32 uNumEntries)
	{
		AkInt32 iDistFromBestSlot = 0;
		AkInt32 iEntriesMask = (AkInt32)uNumEntries - 1;
		for (; ; )
		{
			if (!pbSlotOccupied[iSlot])
				return -1;
			KeyType keyInSlot = pKeys[iSlot];
			if (keyInSlot == uKey)
				break;
			if (iDistFromBestSlot > DistanceFromIdealPos(iSlot, keyInSlot, iEntriesMask))
				return -1;
			iSlot = (iSlot + 1) & iEntriesMask;
			++iDistFromBestSlot;
		}
		return iSlot;
	}

	// Internal helper function for AkHashTableBase; don't call this directly, use AkHashTableGet* functions instead
	inline AkInt32 OccupiedProbe(const bool* pbSlotOccupied, AkInt32 iSlot, AkInt32 iNumEntries)
	{
		while (iSlot < iNumEntries)
		{
			// 64-bit load to scan 8 pbSlotOccupieds at once
			// (safe to load a bit past the end of slotOccupied region because we cap against iNumEntries anyway)
			if (AkUInt64 slotOccCompressed = *((AkUInt64*)(pbSlotOccupied + iSlot)))
			{
				iSlot = iSlot + AKPLATFORM::AkBitScanForward64(slotOccCompressed) / 8;
				iSlot = (iSlot >= iNumEntries) ? -1 : iSlot;
				return iSlot;
			}
			else
			{
				iSlot += 8;
			}
		}
		return -1;
	}
} // namespace Internal

	// returns either:
	// the slot of the first valid entry that uKey maps to
	// -1 if there are no valid entries at the table that uKey maps to
	template<typename KeyType>
	inline AkInt32 GetFirstSlotForKey(const AkHashTableBase<KeyType>* pHashTable, KeyType uKey)
	{
		if (pHashTable->uNumReservedEntries == 0)
			return -1;
		AkUInt32 uNumEntries = pHashTable->uNumReservedEntries;
		AkInt32 iBestSlot = uKey & (uNumEntries - 1);
		return Internal::LinearProbe(pHashTable->pKeys, pHashTable->pbSlotOccupied, uKey, iBestSlot, uNumEntries);
	}

	// returns either:
	// the next slot after iPreviousIndex which contains a valid entry
	// -1 if the next table after iPreviousIndex doesn't contain a valid entry
	template<typename KeyType>
	inline AkInt32 GetNextSlotForKey(const AkHashTableBase<KeyType>* pHashTable, KeyType uKey, AkInt32 iPreviousSlot)
	{
		if (pHashTable->uNumReservedEntries == 0)
			return -1;
		AkUInt32 uNumEntries = pHashTable->uNumReservedEntries;
		AkInt32 iNextSlot = (iPreviousSlot + 1) & (uNumEntries - 1);
		return Internal::LinearProbe(pHashTable->pKeys, pHashTable->pbSlotOccupied, uKey, iNextSlot, uNumEntries);
	}

	// returns either:
	// the slot of the first occupied entry in the hash table
	// -1 if there are no occupied entries in the table
	template<typename KeyType>
	inline AkInt32 GetFirstActiveSlot(const AkHashTableBase<KeyType>* pHashTable)
	{
		return Internal::OccupiedProbe(pHashTable->pbSlotOccupied, 0, (AkInt32)pHashTable->uNumReservedEntries);
	}

	// returns either:
	// the slot of the next occupied entry in the hash table (relative to iPreviousSlot)
	// -1 if there are no occupied entries in the table after iPreviousSlot
	template<typename KeyType>
	inline AkInt32 GetNextActiveSlot(const AkHashTableBase<KeyType>* pHashTable, AkInt32 iPreviousSlot)
	{
		return Internal::OccupiedProbe(pHashTable->pbSlotOccupied, iPreviousSlot + 1, (AkInt32)pHashTable->uNumReservedEntries);
	}

	// runs the provided function over every active slot in the hashtable
	// functype(AkUInt32 in_slot)
	template<typename KeyType, typename FuncType> 
	inline void ForEachSlot(const AkHashTableBase<KeyType>* in_pHashTable, FuncType in_func)
	{
		AkUInt32 uNumReservedEntries = in_pHashTable->uNumReservedEntries;
		bool* pbSlotOccupied = in_pHashTable->pbSlotOccupied;
		for (AkUInt32 uBaseSlot = 0; uBaseSlot < uNumReservedEntries; uBaseSlot += 8)
		{
			AkUInt64 slotOccMask = *(AkUInt64*)(pbSlotOccupied + uBaseSlot);
			while (slotOccMask)
			{
				AkUInt32 slotSubIdx = AKPLATFORM::AkBitScanReverse64(slotOccMask);
				in_func(uBaseSlot + 7 - (slotSubIdx / 8));
				slotOccMask ^= (0x8000000000000000ULL >> slotSubIdx);
			}
		}
	}

} // namespace HashTable
} // namespace AK
