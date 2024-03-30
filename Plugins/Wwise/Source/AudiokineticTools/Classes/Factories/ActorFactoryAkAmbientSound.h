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

/*=============================================================================
	ActorFactoryAkAmbientSound.h:
=============================================================================*/
#pragma once

#include "ActorFactories/ActorFactory.h"
#include "ActorFactoryAkAmbientSound.generated.h"

/*------------------------------------------------------------------------------------
	UActorFactoryAkAmbientSound
------------------------------------------------------------------------------------*/
UCLASS(config=Editor, collapsecategories, hidecategories=Object, MinimalAPI)
class UActorFactoryAkAmbientSound : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactoryAkAmbientSound(const class FObjectInitializer& ObjectInitializer);

	// Begin UActorFactory Interface
	virtual void PostSpawnActor( UObject* Asset, AActor* NewActor ) override;
	virtual void PostCreateBlueprint( UObject* Asset, AActor* CDO ) override;
	virtual bool CanCreateActorFrom( const FAssetData& AssetData, FText& OutErrorMsg ) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	// End UActorFactory Interface
};



