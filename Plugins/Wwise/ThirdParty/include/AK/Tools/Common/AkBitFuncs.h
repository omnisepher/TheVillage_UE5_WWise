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

// AkBitFuncs.h

/// \file 
/// Functions for defining various bit-manipulation functions

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

#if defined (AK_XBOX)
#include <AK/Tools/XboxGC/AkBitFuncs.h>
#endif

// some platforms will define their own optimized bitscan functions
#if !defined(AK_BIT_SCAN_INSTRUCTIONS)
namespace AKPLATFORM
{
	// AkPopCount returns how many set bits there are in the provided value
	// e.g. 0b0000`1111`0000`0011 would return 6
#if __clang__ || defined __GNUC__
	AkForceInline AkUInt32 AkPopCount(AkUInt32 in_bits)
	{
		return __builtin_popcount(in_bits);
	}
#else
	AkForceInline AkUInt32 AkPopCount(AkUInt32 in_bits)
	{
		AkUInt32 num = 0;
		while (in_bits) { ++num; in_bits &= in_bits - 1; }
		return num;
	}
#endif

	// AkBitScanForward returns how many 0s there are until the least-significant-bit is set
	// (or the length of the param if the value is zero)
	// e.g. 0b0000`0000`0001`0000 would return 4
#if defined _MSC_VER && (defined AK_CPU_X86_64 || defined AK_CPU_ARM_64)
	AkForceInline AkUInt32 AkBitScanForward64(AkUInt64 in_bits)
	{
		unsigned long ret = 0;
		_BitScanForward64(&ret, in_bits);
		return in_bits ? ret : 64;
	}
#elif __clang__ || defined __GNUC__
	AkForceInline AkUInt32 AkBitScanForward64(AkUInt64 in_bits)
	{
		return in_bits ? __builtin_ctzll(in_bits) : 64;
	}
#else
	AkForceInline AkUInt32 AkBitScanForward64(AkUInt64 in_bits)
	{
		if (in_bits == 0) return 64;
		AkUInt32 ret = 0;
		if ((in_bits & 0x00000000FFFFFFFFULL) == 0) { ret += 32; in_bits >>= 32; }
		if ((in_bits & 0x000000000000FFFFULL) == 0) { ret += 16; in_bits >>= 16; }
		if ((in_bits & 0x00000000000000FFULL) == 0) { ret += 8;  in_bits >>= 8; }
		if ((in_bits & 0x000000000000000FULL) == 0) { ret += 4;  in_bits >>= 4; }
		if ((in_bits & 0x0000000000000003ULL) == 0) { ret += 2;  in_bits >>= 2; }
		if ((in_bits & 0x0000000000000001ULL) == 0) { ret += 1;  in_bits >>= 1; }
		return ret;
	}
#endif

#if defined _MSC_VER
	AkForceInline AkUInt32 AkBitScanForward(AkUInt32 in_bits)
	{
		unsigned long ret = 0;
		_BitScanForward(&ret, in_bits);
		return in_bits ? ret : 32;
	}

#elif __clang__ || defined __GNUC__
	AkForceInline AkUInt32 AkBitScanForward(AkUInt32 in_bits)
	{
		return in_bits ? __builtin_ctz(in_bits) : 32;
	}
#else
	AkForceInline AkUInt32 AkBitScanForward(AkUInt32 in_bits)
	{
		if (in_bits == 0) return 32;
		AkUInt32 ret = 0;
		if ((in_bits & 0x0000FFFFULL) == 0) { ret += 16; in_bits >>= 16; }
		if ((in_bits & 0x000000FFULL) == 0) { ret += 8;  in_bits >>= 8; }
		if ((in_bits & 0x0000000FULL) == 0) { ret += 4;  in_bits >>= 4; }
		if ((in_bits & 0x00000003ULL) == 0) { ret += 2;  in_bits >>= 2; }
		if ((in_bits & 0x00000001ULL) == 0) { ret += 1;  in_bits >>= 1; }
		return ret;
	}
#endif

	// AkBitScanReverse returns how many 0s there are after the most-significant-bit is set
	// (or the length of the param if the value is zero)
	// e.g. 0b0000`0000`0001`0000 would return 11
#if defined _MSC_VER && (defined AK_CPU_X86_64 || defined AK_CPU_ARM_64)
	AkForceInline AkUInt32 AkBitScanReverse64(AkUInt64 in_bits)
	{
		unsigned long ret = 0;
		_BitScanReverse64(&ret, in_bits);
		return in_bits ? 63 - ret : 64;
	}
#elif __clang__ || defined __GNUC__
	AkForceInline AkUInt32 AkBitScanReverse64(AkUInt64 in_bits)
	{
		return in_bits ? __builtin_clzll(in_bits) : 64;
	}
#else
	AkForceInline AkUInt32 AkBitScanReverse64(AkUInt64 in_bits)
	{
		if (in_bits == 0) return 64;
		int ret = 0;
		if ((in_bits & 0xFFFFFFFF00000000ULL) == 0) { ret += 32; in_bits <<= 32; }
		if ((in_bits & 0xFFFF000000000000ULL) == 0) { ret += 16; in_bits <<= 16; }
		if ((in_bits & 0xFF00000000000000ULL) == 0) { ret += 8;  in_bits <<= 8; }
		if ((in_bits & 0xF000000000000000ULL) == 0) { ret += 4;  in_bits <<= 4; }
		if ((in_bits & 0xC000000000000000ULL) == 0) { ret += 2;  in_bits <<= 2; }
		if ((in_bits & 0x8000000000000000ULL) == 0) { ret += 1;  in_bits <<= 1; }
		return ret;
	}
#endif

#if defined _MSC_VER
	AkForceInline AkUInt32 AkBitScanReverse(AkUInt32 in_bits)
	{
		unsigned long ret = 0;
		_BitScanReverse(&ret, in_bits);
		return in_bits ? 31 - ret : 32;
	}
#elif __clang__ || defined __GNUC__
	AkForceInline AkUInt32 AkBitScanReverse(AkUInt32 in_bits)
	{
		return in_bits ? __builtin_clz(in_bits) : 32;
	}
#else
	AkForceInline AkUInt32 AkBitScanReverse(AkUInt32 in_bits)
	{
		if (in_bits == 0) return 32;
		int ret = 0;
		if ((in_bits & 0xFFFF0000ULL) == 0) { ret += 16; in_bits <<= 16; }
		if ((in_bits & 0xFF000000ULL) == 0) { ret += 8;  in_bits <<= 8; }
		if ((in_bits & 0xF0000000ULL) == 0) { ret += 4;  in_bits <<= 4; }
		if ((in_bits & 0xC0000000ULL) == 0) { ret += 2;  in_bits <<= 2; }
		if ((in_bits & 0x80000000ULL) == 0) { ret += 1;  in_bits <<= 1; }
		return ret;
	}
#endif

}
#endif // defined(AK_BIT_SCAN_INSTRUCTIONS)

/// Utility functions
namespace AK
{
	/// Count non-zero bits, e.g. to count # of channels in a channelmask
	/// \return Number of channels.
	AkForceInline AkUInt32 GetNumNonZeroBits(AkUInt32 in_uWord)
	{
		return AKPLATFORM::AkPopCount(in_uWord);
	}

	/// Computes the next power of two given a value.
	/// \return next power of two.
	AkForceInline AkUInt32 GetNextPowerOfTwo(AkUInt32 in_uValue)
	{
		in_uValue--;
		in_uValue |= in_uValue >> 1;
		in_uValue |= in_uValue >> 2;
		in_uValue |= in_uValue >> 4;
		in_uValue |= in_uValue >> 8;
		in_uValue |= in_uValue >> 16;
		in_uValue++;
		return in_uValue;
	}

	AkForceInline AkUInt32 ROTL32(AkUInt32 x, AkUInt32 r)
	{
		return (x << r) | (x >> (32 - r));
	}

	AkForceInline AkUInt64 ROTL64(AkUInt64 x, AkUInt64 r)
	{
		return (x << r) | (x >> (64 - r));
	}
}
