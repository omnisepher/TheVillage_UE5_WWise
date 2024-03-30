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

#include "Wwise/Ref/WwiseRefPlatformInfo.h"
#include "Wwise/Ref/WwiseRefProjectInfo.h"

class WWISEPROJECTDATABASE_API FWwiseRefPlatform : public FWwiseRefPlatformInfo
{
public:
	static const TCHAR* const NAME;
	static constexpr EWwiseRefType TYPE = EWwiseRefType::Platform;

	// The reference does contain supplemental information, such as Path.
	FWwiseRefProjectInfo ProjectInfo;
	WwiseRefIndexType ProjectInfoPlatformReferenceIndex;

	FWwiseRefPlatform() :
		ProjectInfo(),
		ProjectInfoPlatformReferenceIndex(INDEX_NONE)
	{}
	FWwiseRefPlatform(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath,
		const WwiseMetadataSharedRootFileConstPtr& InProjectInfoRootMediaRef, const FName& InProjectInfoJsonFilePath,
		WwiseRefIndexType InProjectInfoPlatformReferenceIndex) :
		FWwiseRefPlatformInfo(InRootMediaRef, InJsonFilePath),
		ProjectInfo(InProjectInfoRootMediaRef, InProjectInfoJsonFilePath),
		ProjectInfoPlatformReferenceIndex(InProjectInfoPlatformReferenceIndex)
	{}
	FWwiseRefPlatform(const WwiseMetadataSharedRootFileConstPtr& InRootMediaRef, const FName& InJsonFilePath) :
		FWwiseRefPlatformInfo(InRootMediaRef, InJsonFilePath),
		ProjectInfo(),
		ProjectInfoPlatformReferenceIndex()
	{}
	FWwiseRefPlatform(const WwiseMetadataSharedRootFileConstPtr& InProjectInfoRootMediaRef, const FName& InProjectInfoJsonFilePath,
		WwiseRefIndexType InProjectInfoPlatformReferenceIndex) :
		FWwiseRefPlatformInfo(),
		ProjectInfo(InProjectInfoRootMediaRef, InProjectInfoJsonFilePath),
		ProjectInfoPlatformReferenceIndex(InProjectInfoPlatformReferenceIndex)
	{}
	void Merge(FWwiseRefPlatform&& InOtherPlatform);

	const FWwiseMetadataPlatform* GetPlatform() const;
	const FWwiseMetadataPlatformReference* GetPlatformReference() const;

	FGuid PlatformGuid() const;
	FName PlatformName() const;
	FGuid BasePlatformGuid() const;
	FName BasePlatformName() const;

	uint32 Hash() const override;
	EWwiseRefType Type() const override { return TYPE; }
};
