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

#include "WaapiPicker/WwiseTreeItem.h"
#include "Materials/Material.h"
#if WITH_EDITOR
#include "Wwise/Ref/WwiseRefType.h"
#endif

/**
* Implements the visual style of Wwise Browser.
*/
class AKAUDIO_API FAkAudioStyle
{
public:
	static const ISlateStyle& Get();
	static void Initialize();
	static void Shutdown();

	static void DisplayEditorMessage(const FText& messageText, EWwiseItemType::Type wwiseItemType = EWwiseItemType::Type::None, float duration = 1.5f);

	static FName GetStyleSetName();

	static const FSlateBrush* GetWwiseIcon();
	static const FSlateBrush* GetBrush(EWwiseItemType::Type ItemType);
#if WITH_EDITOR
	static const FSlateBrush* GetBrush(EWwiseRefType WwiseRefType);
	static const FSlateBrush* GetBrush(UClass* Class);
#endif
	static const FSlateBrush* GetBrush(FName PropertyName, const ANSICHAR* Specifier = NULL);
	static const FSlateFontInfo GetFontStyle(FName PropertyName, const ANSICHAR* Specifier = NULL);
	static UMaterial* GetAkForegroundTextMaterial();
	/** returns a color from the WwiseUnrealColorPalette (taken from the dark theme in the Wwise Authoring tool). Use a colorIndex of -1 to use the 'default' color. */
	static FLinearColor GetWwiseObjectColor(int colorIndex);
private:
	static TSharedPtr< class FSlateStyleSet > StyleInstance;
	static UMaterial* TextMaterial;
};
