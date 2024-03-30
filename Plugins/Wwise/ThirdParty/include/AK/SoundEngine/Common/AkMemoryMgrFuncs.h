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

// AkMemoryMgrFuncs.h

/// \file 
/// Platform-dependent function definitions for memory mgr

#pragma once 

#include <AK/SoundEngine/Common/AkTypes.h>

#if defined(AK_WIN)
#include <AK/SoundEngine/Platforms/Windows/AkMemoryMgrFuncs.h>

#elif defined (AK_XBOX)
#include <AK/SoundEngine/Platforms/XboxGC/AkMemoryMgrFuncs.h>

#elif defined (AK_PS4)
#include <AK/SoundEngine/Platforms/PS4/AkMemoryMgrFuncs.h>

#elif defined (AK_PS5)
#include <AK/SoundEngine/Platforms/PS5/AkMemoryMgrFuncs.h>

#elif defined (AK_NX)
#include <AK/SoundEngine/Platforms/NX/AkMemoryMgrFuncs.h>

#elif defined (AK_APPLE) || defined (AK_LINUX) || defined(AK_QNX) | defined( AK_ANDROID ) || defined ( AK_LINUX_AOSP ) || defined(AK_EMSCRIPTEN)
#include <AK/SoundEngine/Platforms/POSIX/AkMemoryMgrFuncs.h>

#else
#error AkMemoryMgrFuncs.h: Undefined platform
#endif

