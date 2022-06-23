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

  Version: v2021.1.9  Build: 7847
  Copyright (c) 2006-2022 Audiokinetic Inc.
*******************************************************************************/

/// \file

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <pad.h>

// Do a bitwise-OR with the deviceID, when adding the output, to utilize the scePad Haptics ("Advanced") functionality
// If unset, the device will use scePad Rumble ("Compatible") functionality
#define AKMOTION_SCEPAD_HAPTICS_MODE 0x80000000

namespace AK
{
	typedef int(*_akmotionPadGetHandle)(int userId, int type, int index);
	typedef int(*_akmotionPadGetContainerIdInformation)(int handle, void* pInfo);
	typedef int(*_akmotionPadGetControllerType)(int handle, void* pControllerType);
	typedef int(*_akmotionPadSetVibrationMode)(int handle, ScePadVibrationMode mode);
	typedef int(*_akmotionPadSetVibration)(int handle, const void* pParam);
}

// Helper functions to facilitate "loose" linkage with scePad library.
// If AkMotion is statically linked into your program, call AKMOTION_STATIC_LINK_SCEPAD_FUNCTIONS anywhere (even pre-init)
// If AkMotion is dynamically linked into your program, call AKMOTION_DYNAMIC_LINK_SCEPAD_FUNCTIONS after Init.bnk has been loaded
#define AKMOTIONSINK_STATIC_LINK_SCEPAD_FUNCTIONS \
	struct _AkMotionInitializeScePadFunctionsHelper \
	{ \
		_AkMotionInitializeScePadFunctionsHelper() \
		{ \
			AkMotionInitializeScePadFunctions(\
				(AK::_akmotionPadGetHandle)scePadGetHandle, \
				(AK::_akmotionPadGetContainerIdInformation)scePadGetContainerIdInformation, \
				(AK::_akmotionPadGetControllerType)scePadGetControllerType, \
				(AK::_akmotionPadSetVibrationMode)scePadSetVibrationMode, \
				(AK::_akmotionPadSetVibration)scePadSetVibration); \
		} \
	} AkMotionInitializeScePadFunctionsHelper; 

#define AKMOTIONSINK_DYNAMIC_LINK_SCEPAD_FUNCTIONS \
	{ \
		HMODULE _akmotion_Hmod; \
		if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, "AkMotion", &_akmotion_Hmod)) \
		{ \
			typedef int(*_akmotionPadInitFunc)( \
				AK::_akmotionPadGetHandle in_pPadGetHandle, \
				AK::_akmotionPadGetContainerIdInformation in_pPadGetContainerIdInformation, \
				AK::_akmotionPadGetControllerType in_pPadGetControllerType, \
				AK::_akmotionPadSetVibrationMode in_pPadSetVibrationMode, \
				AK::_akmotionPadSetVibration in_pPadSetVibration); \
			_akmotionPadInitFunc _akmotion_pInitFn = reinterpret_cast<_akmotionPadInitFunc>( reinterpret_cast<void*>( \
					GetProcAddress(_akmotion_Hmod, "AkMotionInitializeScePadFunctions") \
				)); \
			_akmotion_pInitFn( \
				(AK::_akmotionPadGetHandle)scePadGetHandle, \
				(AK::_akmotionPadGetContainerIdInformation)scePadGetContainerIdInformation, \
				(AK::_akmotionPadGetControllerType)scePadGetControllerType, \
				(AK::_akmotionPadSetVibrationMode)scePadSetVibrationMode, \
				(AK::_akmotionPadSetVibration)scePadSetVibration); \
		} \
	} \


#ifdef __cplusplus
extern "C" {
#endif
	AK_DLLEXPORT void AkMotionInitializeScePadFunctions(
		AK::_akmotionPadGetHandle in_pPadGetHandle,
		AK::_akmotionPadGetContainerIdInformation in_pPadGetContainerIdInformation,
		AK::_akmotionPadGetControllerType in_pPadGetControllerType,
		AK::_akmotionPadSetVibrationMode in_pPadSetVibrationMode,
		AK::_akmotionPadSetVibration in_pPadSetVibration);
#ifdef __cplusplus
}
#endif
