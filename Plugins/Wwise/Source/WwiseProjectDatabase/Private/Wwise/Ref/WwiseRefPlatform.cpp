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

#include "Wwise/Ref/WwiseRefPlatform.h"
#include "Wwise/WwiseProjectDatabaseModule.h"
#include "Wwise/Stats/ProjectDatabase.h"
#include "Wwise/Metadata/WwiseMetadataPlatform.h"
#include "Wwise/Metadata/WwiseMetadataPlatformInfo.h"
#include "Wwise/Metadata/WwiseMetadataProjectInfo.h"

const TCHAR* const FWwiseRefPlatform::NAME = TEXT("Platform");

void FWwiseRefPlatform::Merge(FWwiseRefPlatform&& InOtherPlatform)
{
	if (UNLIKELY(InOtherPlatform.ProjectInfo.IsValid() && InOtherPlatform.IsValid()))
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("FWwiseRefPlatform::Merge: Merging with a complete OtherPlatform."));
	}
	if (UNLIKELY(ProjectInfo.IsValid() && IsValid()))
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("FWwiseRefPlatform::Merge: Merging with a complete self."));
	}

	if (InOtherPlatform.IsValid())
	{
		if (UNLIKELY(IsValid()))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("FWwiseRefPlatform::Merge: Already have a PlatformInfo. Overriding."));
		}
		RootFileRef = MoveTemp(InOtherPlatform.RootFileRef);
		JsonFilePath = MoveTemp(InOtherPlatform.JsonFilePath);
	}
	if (InOtherPlatform.ProjectInfo.IsValid())
	{
		if (UNLIKELY(ProjectInfo.IsValid()))
		{
			UE_LOG(LogWwiseProjectDatabase, Error, TEXT("FWwiseRefPlatform::Merge: Already have a ProjectInfo. Overriding."));
		}
		ProjectInfo = MoveTemp(InOtherPlatform.ProjectInfo);
		ProjectInfoPlatformReferenceIndex = MoveTemp(InOtherPlatform.ProjectInfoPlatformReferenceIndex);
	}
}

const FWwiseMetadataPlatform* FWwiseRefPlatform::GetPlatform() const
{
	const auto* PlatformInfo = GetPlatformInfo();
	if (UNLIKELY(!PlatformInfo))
	{
		return nullptr;
	}
	return &PlatformInfo->Platform;
}

const FWwiseMetadataPlatformReference* FWwiseRefPlatform::GetPlatformReference() const
{
	const auto* GetProjectInfo = ProjectInfo.GetProjectInfo();
	if (UNLIKELY(!GetProjectInfo))
	{
		return nullptr;
	}
	const auto& Platforms = GetProjectInfo->Platforms;
	if (Platforms.IsValidIndex(ProjectInfoPlatformReferenceIndex))
	{
		return &Platforms[ProjectInfoPlatformReferenceIndex];
	}
	else
	{
		UE_LOG(LogWwiseProjectDatabase, Error, TEXT("Could not get Platform Reference index #%zu"), ProjectInfoPlatformReferenceIndex);
		return nullptr;
	}
}

FGuid FWwiseRefPlatform::PlatformGuid() const
{
	const auto* PlatformReference = GetPlatformReference();
	if (UNLIKELY(!PlatformReference))
	{
		return {};
	}
	return PlatformReference->GUID;
}

FName FWwiseRefPlatform::PlatformName() const
{
	const auto* PlatformReference = GetPlatformReference();
	if (UNLIKELY(!PlatformReference))
	{
		return {};
	}
	return PlatformReference->Name;
}

FGuid FWwiseRefPlatform::BasePlatformGuid() const
{
	const auto* PlatformReference = GetPlatformReference();
	if (UNLIKELY(!PlatformReference))
	{
		return {};
	}
	return PlatformReference->BasePlatformGUID;
}

FName FWwiseRefPlatform::BasePlatformName() const
{
	const auto* PlatformReference = GetPlatformReference();
	if (UNLIKELY(!PlatformReference))
	{
		return {};
	}
	return PlatformReference->BasePlatform;
}
uint32 FWwiseRefPlatform::Hash() const
{
	auto Result = FWwiseRefPlatformInfo::Hash();
	Result = HashCombine(Result, ProjectInfo.Hash());
	Result = HashCombine(Result, GetTypeHash(ProjectInfoPlatformReferenceIndex));
	return Result;
}
