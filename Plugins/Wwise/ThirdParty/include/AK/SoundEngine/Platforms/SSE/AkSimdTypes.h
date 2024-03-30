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
/// AKSIMD - SSE types definitions

#pragma once

#include <xmmintrin.h>
#include <emmintrin.h>

////////////////////////////////////////////////////////////////////////
/// @name Platform specific defines for prefetching
//@{

#define AKSIMD_ARCHCACHELINESIZE	(64)				///< Assumed cache line width for architectures on this platform
#define AKSIMD_ARCHMAXPREFETCHSIZE	(512) 				///< Use this to control how much prefetching maximum is desirable (assuming 8-way cache)		
/// Cross-platform memory prefetch of effective address assuming non-temporal data
#define AKSIMD_PREFETCHMEMORY( __offset__, __add__ ) _mm_prefetch(((char *)(__add__))+(__offset__), _MM_HINT_NTA ) 

//@}
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
/// @name AKSIMD types
//@{

typedef float	AKSIMD_F32;		///< 32-bit float
typedef __m128	AKSIMD_V4F32;	///< Vector of 4 32-bit floats
typedef AKSIMD_V4F32 AKSIMD_V4COND;	 ///< Vector of 4 comparison results
typedef AKSIMD_V4F32 AKSIMD_V4FCOND;	 ///< Vector of 4 comparison results

typedef __m128i	AKSIMD_V4I32;	///< Vector of 4 32-bit signed integers

struct AKSIMD_V4I32X2 {			///< Pair of 4 32-bit signed integers
	AKSIMD_V4I32 val[2];
};

struct AKSIMD_V4I32X4 {			///< Quartet of 4 32-bit signed integers
	AKSIMD_V4I32 val[4];
};

typedef AKSIMD_V4I32 AKSIMD_V4ICOND;

//@}
////////////////////////////////////////////////////////////////////////

