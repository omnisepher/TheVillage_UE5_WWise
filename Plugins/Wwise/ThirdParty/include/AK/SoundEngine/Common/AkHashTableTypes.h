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

#include <AK/SoundEngine/Common/AkTypes.h>

// Basic types for an open-addressed hash array (or, otherwise, keyState-value array).
// Exposing enough data and structures to used for forward declarations of types.
// Helper and inlineable functions, especially for use by IAkPluginServiceHashTable,
// are available in AkHashTableFuncs.h

// While the AkHashTableBase structures are exposed here, it is strongly recommended to use either the
// AkHashTable namespace functions to modify it, or the IAkPluginHashTable plug-in service, instead.

namespace AK
{
	// template instantiations available, in AkHashTable.cpp, are
	// template<AkUInt32>
	// template<AkUInt64> 
	template<typename KeyType>
	struct AkHashTableBase {
		AkHashTableBase()
			: pKeys(nullptr)
			, pbSlotOccupied(nullptr)
			, pValues(nullptr)
			, uNumReservedEntries(0)
			, uNumUsedEntries(0)
			, uMaxLoadFactor(kDefaultMaxLoadFactor)
		{
		}
		KeyType* pKeys;
		bool* pbSlotOccupied;
		void* pValues;

		AkUInt32 uNumReservedEntries;
		AkUInt32 uNumUsedEntries;
		AkUInt16 uValueElementSize;
		AkUInt16 uValueElementAlign;
		AkUInt32 uMaxLoadFactor;

		static const AkUInt32 kDefaultMaxLoadFactor = 28; // divided by 32 = 87% load. Robin-hood-style storage ensures that most probe lengths should be low even at this ratio

	};

	// main class to use; trivially convertible to AkHashTableBase
	// This allows for static disambiguation of hashtables for different value types; it's just syntactic sugar
	template<typename KeyType, typename ValueType>
	struct AkHashTable : public AkHashTableBase<KeyType>
	{
		ValueType* ValueData() { return (ValueType*)AkHashTableBase<KeyType>::pValues; }
		const ValueType* ValueData() const { return (const ValueType*)AkHashTableBase<KeyType>::pValues; }

		ValueType& ValueAt(AkInt32 uSlot) { return ((ValueType*)AkHashTableBase<KeyType>::pValues)[uSlot]; }
		const ValueType& ValueAt(AkInt32 uSlot) const { return ((ValueType*)AkHashTableBase<KeyType>::pValues)[uSlot]; }
	};

} // namespace AK
