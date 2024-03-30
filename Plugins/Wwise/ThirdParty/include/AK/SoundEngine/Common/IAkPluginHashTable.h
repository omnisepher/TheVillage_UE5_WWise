/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the 
"Apache License"); you may not use this file except in compliance with the 
Apache License. You may obtain a copy of the Apache License at 
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

/// \file
/// Plug-in interface for HashTable

#pragma once

#include <type_traits>
#include <AK/SoundEngine/Common/AkHashTableTypes.h>
#include <AK/SoundEngine/Common/AkHashTableFuncs.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Tools/Common/AkMurMurHash.h>
#include <AK/Tools/Common/AkObject.h>

namespace AK
{

/// Interface for an open-addressed hash table (or key-value array).
/// Relatively low-cost (O(1)) lookup, add, and removal by key. Typical use is to add values by key,
/// and then using GetFirstSlotByKey (and GetNextSlotByKey, if keys are not guaranteed to be unique)
/// to get a Slot into the AkHashTableBase, and then indexing AkHashTableBase::pValues by the provided Slot.
/// 
/// Note that the AkHashTableBase referred to is not inherently thread-safe, so appropriate protections
/// will be required if the same hashTable is shared across multiple plug-in instances.
///
/// Some hash functions are provided in AkMurMurHash.h which can be used for key generation.
/// In particular, this can be very useful for storing data on audio objects, or audio object channels,
/// over time. Note that if indexing by object-channel is desired, GetObjectChannelHash is recommended
/// to mitigate key-collisions from multiple channels of data. However, Audio Object IDs themselves are
/// chaotic enough that they can be used directly.

class IAkPluginServiceHashTable : public IAkPluginService
{
protected:
	virtual ~IAkPluginServiceHashTable() {}
public:

	// initializes and preallocates storage for specified number of entries
	// 
	// returns either:
	// AK_InsufficientMemory if a required allocation could not be made
	// AK_Success if the hash table was initialized successfully
	virtual AKRESULT InitBase(AK::AkHashTableBase<AkUInt64>* io_pHashTable, AK::IAkPluginMemAlloc* in_pAllocator,
		AkUInt32 in_uInitialReserveCount, AkUInt32 in_uValueElementSize, AkUInt32 in_uValueElementAlign) = 0;

	// frees memory allocated for entries
	virtual void Term(AkHashTableBase<AkUInt64>* io_pHashTable, AK::IAkPluginMemAlloc* in_pAllocator) = 0;

	// resets the table, clearing all keys and values to zero, but leaving the memory allocated
	virtual void Reset(AkHashTableBase<AkUInt64>* io_pHashTable) = 0;

	// adds the provided key to the hash table
	// and returns a pointer to the value made available, or nullptr if a grow occurred but memory could not be provided
	// - keys can be added to the table multiple times
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	virtual void* AddKey(AkHashTableBase<AkUInt64>* io_pHashTable, AK::IAkPluginMemAlloc* in_pAllocator, AkUInt64 in_uKey) = 0;

	// removes the entry at the slot provided
	// Returns true when a move of data around the end of the list occurred, e.g. in case iteration over the table needs to be modified
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	virtual bool RemoveSlot(AkHashTableBase<AkUInt64>* io_pHashTable, AkInt32 in_iSlot) = 0;

	// finds a slot with the provided key and deletes it from the table
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	virtual void RemoveKey(AkHashTableBase<AkUInt64>* io_pHashTable, AkUInt64 in_uKey) = 0;

	///////////////////////////////////////////////////////////////////////////
	// All virtual/interface specific functions should only be above this point.
	// Inline/static helpers that call the interface functions, or other hashtable funcs are as follows:

	// returns either:
	// the slot of the first valid entry that in_uKey maps to
	// -1 if there are no valid entries at the table that uKey maps to
	static AkInt32 GetFirstSlotByKey(AkHashTableBase<AkUInt64>* in_pHashTable, AkUInt64 in_uKey) {
		return AK::HashTable::GetFirstSlotForKey(in_pHashTable, in_uKey);
	}

	// returns either:
	// the next slot after iPreviousSlot which contains a valid entry
	// -1 if the next slot after iPreviousSlot doesn't contain a valid entry
	static AkInt32 GetNextSlotByKey(AkHashTableBase<AkUInt64>* in_pHashTable, AkUInt64 in_uKey, AkInt32 in_iPreviousSlot) {
		return AK::HashTable::GetNextSlotForKey(in_pHashTable, in_uKey, in_iPreviousSlot);
	}

	// helper function for initialization of the hashtable against a specific value type
	template<typename ValueType>
	AkForceInline AKRESULT Init(AkHashTable<AkUInt64, ValueType>* io_pHashTable, AK::IAkPluginMemAlloc* in_pAllocator,
		AkUInt32 in_uInitialReserveCount)
	{
		return InitBase(io_pHashTable, in_pAllocator, in_uInitialReserveCount, sizeof(ValueType), alignof(ValueType));
	}

	// frees memory allocated for entries, and invokes the destructor for all value types along the way
	template<typename KeyType, typename ValueType>
	AkForceInline void TermValues(AkHashTable<KeyType, ValueType>* io_pHashTable, AK::IAkPluginMemAlloc* in_pAllocator)
	{
		if (!std::is_trivially_destructible<ValueType>::value)
		{
			AK::HashTable::ForEachSlot(io_pHashTable, [&](AkUInt32 uSlot)
				{
					(io_pHashTable->ValueAt(uSlot)).~ValueType();
				});
		}
		Term((AkHashTableBase<KeyType>*)io_pHashTable, in_pAllocator);
	}

	// adds the provided key to the hash table, performs default initialization of the table's valueType,
	// and returns a pointer to the value made available, or nullptr if a grow occurred but memory could not be provided
	// - keys can be added to the table multiple times
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	template<typename KeyType, typename ValueType>
	AkForceInline void* AddKeyDefaultValue(AkHashTable<KeyType, ValueType>* io_pHashTable, AK::IAkPluginMemAlloc* in_pAllocator,
		KeyType in_uKey)
	{
		if (ValueType* pInsertedValue = (ValueType*)AddKey(io_pHashTable, in_pAllocator, in_uKey))
		{
			AKASSERT(sizeof(ValueType) == io_pHashTable->uValueElementSize);
			AkPlacementNew(pInsertedValue) (ValueType);
			return pInsertedValue;
		}
		return nullptr;
	}

	// adds the provided key to the hash table, copies the provided value into the newly allocated value,
	// and returns a pointer to the value made available, or nullptr if a grow occurred but memory could not be provided
	// - keys can be added to the table multiple times
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	template<typename ValueType>
	AkForceInline ValueType* AddKeyValue(AkHashTable<AkUInt64, ValueType>* io_pHashTable, AK::IAkPluginMemAlloc* in_pAllocator,
		AkUInt64 in_uKey, ValueType* in_pNewValue)
	{
		if (ValueType* pInsertedValue = (ValueType*)AddKey(io_pHashTable, in_pAllocator, in_uKey))
		{
			AKASSERT(sizeof(ValueType) == io_pHashTable->uValueElementSize);
			AKPLATFORM::AkMemCpy(pInsertedValue, in_pNewValue, sizeof(ValueType));
			return pInsertedValue;
		}
		return nullptr;
	}

	// removes the entry at the slot provided, and performed destruction of the table's valuetype
	// Returns true when a move of data around the end of the list occurred, e.g. in case iteration over the table needs to be modified
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	template<typename KeyType, typename ValueType>
	AkForceInline bool RemoveSlotValue(AkHashTable<KeyType, ValueType>* io_pHashTable, AkInt32 in_iSlotToRemove)
	{
		io_pHashTable->ValueAt(in_iSlotToRemove).~ValueType();
		return RemoveSlot(io_pHashTable, in_iSlotToRemove);
	}

	// runs the provided function over every active slot in the hashtable, which is used to determine if the element should be removed
	// For each element where this function returns true, it removes the entry at the slot provided
	// 
	// in_func should be of type "bool functype(AkUInt32 in_slot)"
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	template<typename KeyType, typename FuncType>
	AkForceInline void RemoveIf(AkHashTableBase<KeyType>* in_pHashTable, FuncType in_func)
	{
		AkUInt32 uNumReservedEntries = in_pHashTable->uNumReservedEntries;
		bool* pbSlotOccupied = in_pHashTable->pbSlotOccupied;
		AkUInt32 uSlot = 0;
		while (uSlot < uNumReservedEntries)
		{
			if (pbSlotOccupied[uSlot] && in_func(uSlot))
			{
				// if slot is occupied, and the function confirmed removal, remove this slot, but don't advance uSlot
				bool removeWrappedData = RemoveSlot(in_pHashTable, (AkInt32)uSlot);
				// if the removeSlot process moved data around the end of the table, then we need to lower numReservedEntries to compensate
				if (removeWrappedData)
				{
					uNumReservedEntries--;
				}
			}
			else
			{
				uSlot++;
			}
		}
	}

	// runs the provided function over every active slot in the hashtable, which is used to determine if the element should be removed
	// For each element where this function returns true, it removes the entry at the slot provided, and performs destruction of the table's valuetype
	// 
	// in_func should be of type "bool functype(AkUInt32 in_slot)"
	// - this function can move entries in the table around, so previous pointers to elements, etc, should be considered invalidated
	template<typename KeyType, typename ValueType, typename FuncType>
	AkForceInline void RemoveIfValue(AkHashTable<KeyType, ValueType>* in_pHashTable, FuncType in_func)
	{
		AkUInt32 uNumReservedEntries = in_pHashTable->uNumReservedEntries;
		bool* pbSlotOccupied = in_pHashTable->pbSlotOccupied;
		AkUInt32 uSlot = 0;
		while (uSlot < uNumReservedEntries)
		{
			if (pbSlotOccupied[uSlot] && in_func(uSlot))
			{
				// if slot is occupied, and the function confirmed removal, remove this slot, but don't advance uSlot
				bool removeWrappedData = RemoveSlotValue(in_pHashTable, (AkInt32)uSlot);
				// if the removeSlot process moved data around the end of the table, then we need to lower numReservedEntries to compensate
				if (removeWrappedData)
				{
					uNumReservedEntries--;
				}
			}
			else
			{
				uSlot++;
			}
		}
	}
};

#define AK_GET_PLUGIN_SERVICE_HASHTABLE(plugin_ctx) static_cast<AK::IAkPluginServiceHashTable*>(plugin_ctx->GetPluginService(AK::PluginServiceType_HashTable))

/// A common hashtable for mapping audioobjectIds to a combination of audio buffers and objects
struct AkAudioObjectBuffer
{
	AkAudioObjectBuffer() : pMetadata(nullptr) {}

	AkAudioBuffer pBuffer;
	AkAudioObject* pMetadata;
};
typedef AK::AkHashTable<AkAudioObjectID, AkAudioObjectBuffer> AkAudioObjectBufferMap;

/// Common hash function for getting a unique hash for a channel on an audio object
AkForceInline AkUInt64 GetObjectChannelHash(AkUInt64 in_uAudioObjectId, AkUInt32 in_uChannelIdx)
{
	return AkHashMurMurMix64(((AkUInt64)in_uChannelIdx << AkAudioObject::kObjectKeyNumBits) | (in_uAudioObjectId & AkAudioObject::kObjectKeyMask));
}

} // namespace AK
