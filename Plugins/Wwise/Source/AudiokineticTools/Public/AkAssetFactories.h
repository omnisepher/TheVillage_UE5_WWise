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
#include "AkAssetFactories.generated.h"

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	FGuid AssetID;
	uint32 ShortID;
	FString WwiseObjectName;
};

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkAcousticTextureFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkAcousticTextureFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
};

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkAudioEventFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkAudioEventFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
};

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkAuxBusFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkAuxBusFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
};

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkRtpcFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkRtpcFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
};

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkTriggerFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkTriggerFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
};

// mlarouche - For now Switch and State factory are only used in drag & drop
UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkStateValueFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkStateValueFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkSwitchValueFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkSwitchValueFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS(hidecategories = Object)
class AUDIOKINETICTOOLS_API UAkEffectShareSetFactory : public UAkAssetFactory
{
	GENERATED_BODY()

public:
	UAkEffectShareSetFactory(const class FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};