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

#include "Wwise/Metadata/WwiseMetadataAcousticTexture.h"
#include "Wwise/Metadata/WwiseMetadataLoader.h"
#include "WwiseDefines.h"

FWwiseMetadataAcousticTexture::FWwiseMetadataAcousticTexture(FWwiseMetadataLoader& Loader) :
	FWwiseMetadataBasicReference(Loader)
{
	Loader.GetPropertyArray(this, FloatProperties);
	Loader.LogParsed(TEXT("AcousticTexture"), Id, Name);
}

const TMap<FName, size_t> FWwiseMetadataAcousticTexture::FloatProperties = FWwiseMetadataAcousticTexture::FillFloatProperties();
const TMap<FName, size_t> FWwiseMetadataAcousticTexture::FillFloatProperties()
{
	TMap<FName, size_t> Result;
	Result.Add(FName(TEXT("AbsorptionLow")), offsetof(FWwiseMetadataAcousticTexture, AbsorptionLow));
	Result.Add(FName(TEXT("AbsorptionMidLow")), offsetof(FWwiseMetadataAcousticTexture, AbsorptionMidLow));
	Result.Add(FName(TEXT("AbsorptionMidHigh")), offsetof(FWwiseMetadataAcousticTexture, AbsorptionMidHigh));
	Result.Add(FName(TEXT("AbsorptionHigh")), offsetof(FWwiseMetadataAcousticTexture, AbsorptionHigh));

#if WWISE_2023_1_OR_LATER
	Result.Add(FName(TEXT("Scattering")), offsetof(FWwiseMetadataAcousticTexture, Scattering));
#endif
	return Result;
}
