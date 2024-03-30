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
	AkJsonFactory.h:
=============================================================================*/
#pragma once

#include "Factories/Factory.h"
#include "AkJsonFactory.generated.h"

/*------------------------------------------------------------------------------------
	UAkJsonFactory
------------------------------------------------------------------------------------*/
UCLASS(hidecategories=Object)
class UAkJsonFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAkJsonFactory(const class FObjectInitializer& ObjectInitializer);

#if CPP
	/*------------------------------------------------------------------------------------
		UFactory Interface
	------------------------------------------------------------------------------------*/
	/**
	 * Create a new instance
	 *
	 * @param Class		The type of class to create
	 * @param InParent	The parent class
	 * @param Name		The name of the new instance
	 * @param Flags		Creation flags
	 * @param Context	Creation context
	 * @param Warn		Warnings
	 * @return The new object if creation was successful, otherwise false 
	 */
	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
#endif

	/**
	 * @return	true if this factory can deal with the file sent in.
	 */
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual bool ShouldShowInNewMenu() const override;

};



