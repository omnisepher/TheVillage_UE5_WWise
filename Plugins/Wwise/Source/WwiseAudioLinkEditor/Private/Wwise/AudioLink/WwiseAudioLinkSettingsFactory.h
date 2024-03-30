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

#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#include "AssetToolsModule.h"

#include "WwiseAudioLinkSettingsFactory.generated.h"

class FAssetTypeActions_WwiseAudioLinkSettings : public FAssetTypeActions_Base
{
public:
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual uint32 GetCategories() override;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UWwiseAudioLinkSettingsFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;

	virtual uint32 GetMenuCategories() const override;
};

