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

// AkSimdTypes.h

/// \file 
/// AKSIMD - arm_neon types definition

#pragma once

#if defined _MSC_VER && defined _M_ARM64
	#include <arm64_neon.h>
#else
	#include <arm_neon.h>
#endif
#include <AK/SoundEngine/Common/AkTypes.h>

// Platform specific defines for prefetching
#define AKSIMD_ARCHMAXPREFETCHSIZE	(512) 				///< Use this to control how much prefetching maximum is desirable (assuming 8-way cache)		
#define AKSIMD_ARCHCACHELINESIZE	(64)				///< Assumed cache line width for architectures on this platform
#if defined __clang__ || defined __GNUC__
#define AKSIMD_PREFETCHMEMORY( __offset__, __add__ ) __builtin_prefetch(((char *)(__add__))+(__offset__)) 
#else
#define AKSIMD_PREFETCHMEMORY( __offset__, __add__ )
#endif

////////////////////////////////////////////////////////////////////////
/// @name AKSIMD types
//@{

typedef int32x4_t		AKSIMD_V4I32;		///< Vector of 4 32-bit signed integers
typedef int16x8_t		AKSIMD_V8I16;		///< Vector of 8 16-bit signed integers
typedef int16x4_t		AKSIMD_V4I16;		///< Vector of 4 16-bit signed integers
typedef uint32x4_t		AKSIMD_V4UI32;		///< Vector of 4 32-bit unsigned signed integers
typedef uint32x2_t		AKSIMD_V2UI32;		///< Vector of 2 32-bit unsigned signed integers
typedef int32x2_t		AKSIMD_V2I32;		///< Vector of 2 32-bit signed integers
typedef float32_t		AKSIMD_F32;			///< 32-bit float
typedef float32x2_t		AKSIMD_V2F32;		///< Vector of 2 32-bit floats
typedef float32x4_t		AKSIMD_V4F32;		///< Vector of 4 32-bit floats

typedef uint32x4_t		AKSIMD_V4COND;		///< Vector of 4 comparison results
typedef uint32x4_t		AKSIMD_V4ICOND;		///< Vector of 4 comparison results
typedef uint32x4_t		AKSIMD_V4FCOND;		///< Vector of 4 comparison results

#if defined(AK_CPU_ARM_NEON)
typedef float32x2x2_t	AKSIMD_V2F32X2;
typedef float32x4x2_t	AKSIMD_V4F32X2;
typedef float32x4x4_t	AKSIMD_V4F32X4;

typedef int32x4x2_t		AKSIMD_V4I32X2;
typedef int32x4x4_t		AKSIMD_V4I32X4;
#endif

//@}
////////////////////////////////////////////////////////////////////////

