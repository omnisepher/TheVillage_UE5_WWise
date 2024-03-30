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

#pragma once

#include "Containers/Map.h"
#include "HAL/UnrealMemory.h"
#include "Serialization/MemoryLayout.h"

namespace WwiseUnrealHelper
{
	WWISEUTILS_API void SetHelperFunctions(
		FString(*GetWwisePluginDirectoryImpl)(),
		FString(*GetWwiseProjectPathImpl)(),
		FString(*GetSoundBankDirectoryImpl)(),
		FString(*GetStagePathImpl)()
		);

	WWISEUTILS_API FString GetWwisePluginDirectory();
	WWISEUTILS_API FString GetWwiseProjectPath();
	WWISEUTILS_API FString GetSoundBankDirectory();

	WWISEUTILS_API void TrimPath(FString& Path);

	WWISEUTILS_API FString GetProjectDirectory();
	WWISEUTILS_API FString GetThirdPartyDirectory();
	WWISEUTILS_API FString GetContentDirectory();
	WWISEUTILS_API FString GetExternalSourceDirectory();

	WWISEUTILS_API FString GetWwiseProjectDirectoryPath();
	WWISEUTILS_API FString GetWwiseSoundBankInfoCachePath();
	WWISEUTILS_API FString FormatFolderPath(FString folderPath);
	WWISEUTILS_API bool MakePathRelativeToWwiseProject(FString& AbsolutePath);

	extern WWISEUTILS_API const TCHAR* MediaFolderName;

	extern WWISEUTILS_API const FGuid InitBankID;

#if WITH_EDITOR
	WWISEUTILS_API FString GuidToBankName(const FGuid& Guid);
	WWISEUTILS_API FGuid BankNameToGuid(const FString& BankName);
#endif

	template <typename T>
struct TMallocDelete
	{
		DECLARE_INLINE_TYPE_LAYOUT(TMallocDelete, NonVirtual);

		TMallocDelete() = default;
		TMallocDelete(const TMallocDelete&) = default;
		TMallocDelete& operator=(const TMallocDelete&) = default;
		~TMallocDelete() = default;

		template <
			typename U,
			typename = decltype(ImplicitConv<T*>((U*)nullptr))
		>
			TMallocDelete(const TMallocDelete<U>&)
		{
		}

		template <
			typename U,
			typename = decltype(ImplicitConv<T*>((U*)nullptr))
		>
			TMallocDelete& operator=(const TMallocDelete<U>&)
		{
			return *this;
		}

		void operator()(T* Ptr) const
		{
			FMemory::Free(Ptr);
		}
	};
}
