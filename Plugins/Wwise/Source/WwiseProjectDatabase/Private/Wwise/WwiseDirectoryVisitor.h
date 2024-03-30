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

#include "Wwise/WwiseGeneratedFiles.h"

#include "Async/Future.h"
#include "CoreTypes.h"
#include "Containers/UnrealString.h"
#include "GenericPlatform/GenericPlatformFile.h"

class WWISEPROJECTDATABASE_API FWwiseDirectoryVisitor : public IPlatformFile::FDirectoryVisitor
{
public:
	FWwiseDirectoryVisitor(IPlatformFile& InFileInterface,
						   const FName* InPlatformName = nullptr,
						   const FGuid* InPlatformGuid = nullptr) :
		FileInterface(InFileInterface),
		PlatformName(InPlatformName),
		PlatformGuid(InPlatformGuid)
	{}

	FWwiseGeneratedFiles& Get();

protected:
	bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override;

private:
	IPlatformFile& FileInterface;
	FWwiseGeneratedFiles GeneratedDirectory;

	class IGettableVisitor
	{
	public:
		virtual FWwiseGeneratedFiles::FPlatformFiles& Get() = 0;
		virtual ~IGettableVisitor() {}
	};
	class FPlatformRootDirectoryVisitor;

	TArray<TFuture<FPlatformRootDirectoryVisitor*>> Futures;

	const FName* PlatformName;
	const FGuid* PlatformGuid;

	class FSoundBankVisitor;
	class FMediaVisitor;

	FWwiseDirectoryVisitor& operator=(const FWwiseDirectoryVisitor& Rhs) = delete;
	FWwiseDirectoryVisitor(const FWwiseDirectoryVisitor& Rhs) = delete;
};
