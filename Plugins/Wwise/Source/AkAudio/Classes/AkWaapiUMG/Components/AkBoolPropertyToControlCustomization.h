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

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

class SButton;
class SComboButton;
class IMenu;

class AKAUDIO_API FAkBoolPropertyToControlCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader( TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;

	virtual void CustomizeChildren( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;              

private:

	/** Delegate used to display the Waapi Picker */
	FReply OnPickContent(TSharedRef<IPropertyHandle> PropertyHandle) ;

	/** Handler for tree view selection changes */
	void PropertySelectionChanged(TSharedPtr< FString > ItemProperty, ESelectInfo::Type SelectInfo);

private:

	/** The pick button widget */
	TSharedPtr<SButton> PickerButton;

	TSharedPtr<IPropertyHandle> ItemPropertyHandle;

	TSharedPtr<SWindow> Window;
};

#endif//WITH_EDITOR
