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

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <memoryapi.h>

#define AK_VM_PAGE_SIZE      (64*1024)
#define AK_VM_HUGE_PAGE_SIZE (2*1024*1024)

namespace AKPLATFORM
{
	AkForceInline void* AllocVM(size_t size, size_t* /*extra*/)
	{
		return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	}

	AkForceInline void FreeVM(void* address, size_t size, size_t /*extra*/, size_t release)
	{
		VirtualFree(address, release ? 0 : size, release ? MEM_RELEASE : MEM_DECOMMIT);
	}

}