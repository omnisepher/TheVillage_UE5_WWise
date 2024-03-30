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

/*------------------------------------------------------------------------------------
 includes.
------------------------------------------------------------------------------------*/
#include "AkWaapiUtils.h"
#include "AkAudioDevice.h"
#include "AkWaapiClient.h"

/*------------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------------*/
#define LOCTEXT_NAMESPACE "AkAudio"

/*------------------------------------------------------------------------------------
Statics and Globals
------------------------------------------------------------------------------------*/
const FString WwiseWaapiHelper::ACTION                       = TEXT("action");
const FString WwiseWaapiHelper::ANCESTORS                    = TEXT("ancestors"); 
const FString WwiseWaapiHelper::AT                           = TEXT("@");
const FString WwiseWaapiHelper::AUX_BUSSES                   = TEXT("auxBusses");
const FString WwiseWaapiHelper::BACK_SLASH                   = TEXT("\\");
const FString WwiseWaapiHelper::BANK_DATA                    = TEXT("bankData");
const FString WwiseWaapiHelper::BANK_INFO                    = TEXT("bankInfo");
const FString WwiseWaapiHelper::CHILD                        = TEXT("child");
const FString WwiseWaapiHelper::CHILDREN                     = TEXT("children");
const FString WwiseWaapiHelper::CHILDREN_COUNT               = TEXT("childrenCount");
const FString WwiseWaapiHelper::CLASSID                      = TEXT("classId");
const FString WwiseWaapiHelper::COMMAND                      = TEXT("command");
const FString WwiseWaapiHelper::DATA                         = TEXT("data");
const FString WwiseWaapiHelper::DELETE_ITEMS                 = TEXT("Delete Items");
const FString WwiseWaapiHelper::DESCENDANTS                  = TEXT("descendants"); 
const FString WwiseWaapiHelper::DISPLAY_NAME                 = TEXT("displayName");
const FString WwiseWaapiHelper::DRAG_DROP_ITEMS              = TEXT("Drag Drop Items");
const FString WwiseWaapiHelper::EVENT                        = TEXT("event");
const FString WwiseWaapiHelper::EVENTS                       = TEXT("events");
const FString WwiseWaapiHelper::FILEPATH                     = TEXT("filePath");
const FString WwiseWaapiHelper::FILTER	                     = TEXT("filter");
const FString WwiseWaapiHelper::FIND_IN_PROJECT_EXPLORER     = TEXT("FindInProjectExplorerSelectionChannel1");
const FString WwiseWaapiHelper::FOLDER                       = TEXT("Folder");
const FString WwiseWaapiHelper::FROM                         = TEXT("from");
const FString WwiseWaapiHelper::ID                           = TEXT("id");
const FString WwiseWaapiHelper::INCLUSIONS                   = TEXT("inclusions");
const FString WwiseWaapiHelper::INFO_FILE                    = TEXT("infoFile");
const FString WwiseWaapiHelper::IS_CONNECTED                 = TEXT("isConnected");
const FString WwiseWaapiHelper::LANGUAGE                     = TEXT("language");
const FString WwiseWaapiHelper::LANGUAGES                    = TEXT("languages");
const FString WwiseWaapiHelper::MAX                          = TEXT("max");
const FString WwiseWaapiHelper::MAX_RADIUS_ATTENUATION       = TEXT("audioSource:maxRadiusAttenuation");
const FString WwiseWaapiHelper::MESSSAGE                     = TEXT("message");
const FString WwiseWaapiHelper::MIN                          = TEXT("min");
const FString WwiseWaapiHelper::NAME                         = TEXT("name");
const FString WwiseWaapiHelper::NAMECONTAINS                 = TEXT("name:contains");
const FString WwiseWaapiHelper::NEW                          = TEXT("new");
const FString WwiseWaapiHelper::NEW_NAME                     = TEXT("newName");
const FString WwiseWaapiHelper::NOTES                        = TEXT("notes");
const FString WwiseWaapiHelper::OBJECT                       = TEXT("object");
const FString WwiseWaapiHelper::OBJECTS                      = TEXT("objects");
const FString WwiseWaapiHelper::OF_TYPE                      = TEXT("ofType");
const FString WwiseWaapiHelper::OLD_NAME                     = TEXT("oldName");
const FString WwiseWaapiHelper::ON_NAME_CONFLICT             = TEXT("onNameConflict");
const FString WwiseWaapiHelper::OPERATION					 = TEXT("operation");
const FString WwiseWaapiHelper::PARENT                       = TEXT("parent");
const FString WwiseWaapiHelper::PATH                         = TEXT("path");
const FString WwiseWaapiHelper::PHYSICAL_FOLDER              = TEXT("PhysicalFolder");
const FString WwiseWaapiHelper::PLATFORM                     = TEXT("platform");
const FString WwiseWaapiHelper::PLATFORMS                    = TEXT("platforms");
const FString WwiseWaapiHelper::PLAY                         = TEXT("play");
const FString WwiseWaapiHelper::PLAYING                      = TEXT("playing");
const FString WwiseWaapiHelper::PLAYSTOP                     = TEXT("playStop");
const FString WwiseWaapiHelper::PLUGININFO_OPTIONS           = TEXT("pluginInfo");
const FString WwiseWaapiHelper::PLUGININFO_RESPONSE          = TEXT("PluginInfo");
const FString WwiseWaapiHelper::PROJECT                      = TEXT("Project");
const FString WwiseWaapiHelper::PROPERTY                     = TEXT("property");
const FString WwiseWaapiHelper::RADIUS                       = TEXT("radius");
const FString WwiseWaapiHelper::RANGE                        = TEXT("range");
const FString WwiseWaapiHelper::REBUILD                      = TEXT("rebuild");
const FString WwiseWaapiHelper::REBUILD_INIT_BANK            = TEXT("rebuildInitBank");
const FString WwiseWaapiHelper::REDO                         = TEXT("Redo");
const FString WwiseWaapiHelper::RENAME                       = TEXT("rename");
const FString WwiseWaapiHelper::RESTRICTION                  = TEXT("restriction");
const FString WwiseWaapiHelper::RETURN                       = TEXT("return");
const FString WwiseWaapiHelper::SEARCH                       = TEXT("search");
const FString WwiseWaapiHelper::SELECT                       = TEXT("select");
const FString WwiseWaapiHelper::SIZE                         = TEXT("size");
const FString WwiseWaapiHelper::SKIP_LANGUAGES               = TEXT("skipLanguages");
const FString WwiseWaapiHelper::SOUNDBANK_TYPE               = TEXT("SoundBank");
const FString WwiseWaapiHelper::SOUNDBANK_FIELD              = TEXT("soundbank");
const FString WwiseWaapiHelper::SOUNDBANKS                   = TEXT("soundbanks");
const FString WwiseWaapiHelper::STATE                        = TEXT("state");
const FString WwiseWaapiHelper::STOP                         = TEXT("stop");
const FString WwiseWaapiHelper::STOPPED                      = TEXT("stopped");
const FString WwiseWaapiHelper::STRUCTURE                    = TEXT("structure");
const FString WwiseWaapiHelper::TRANSFORM                    = TEXT("transform");
const FString WwiseWaapiHelper::TRANSPORT                    = TEXT("transport");
const FString WwiseWaapiHelper::TYPE                         = TEXT("type");
const FString WwiseWaapiHelper::UI                           = TEXT("ui");
const FString WwiseWaapiHelper::UNDO                         = TEXT("Undo");
const FString WwiseWaapiHelper::VALUE                        = TEXT("value");
const FString WwiseWaapiHelper::VOLUME                       = TEXT("Volume");
const FString WwiseWaapiHelper::WHERE                        = TEXT("where");
const FString WwiseWaapiHelper::WORKUNIT_TYPE                = TEXT("workunit:type");
const FString WwiseWaapiHelper::WRITE_TO_DISK				 = TEXT("writeToDisk");


/*------------------------------------------------------------------------------------
 Methods
------------------------------------------------------------------------------------*/

bool CallWappiGetPropertySate(const FString& ItemID, const FString& ItemProperty, TSharedPtr<FJsonObject>& ItemInfoResult)
{
	auto* waapiClient = FAkWaapiClient::Get();
	if (!waapiClient)
		return false;

	TSharedRef<FJsonObject> args = MakeShareable(new FJsonObject());
	{
		TSharedPtr<FJsonObject> from = MakeShareable(new FJsonObject());
		from->SetArrayField(WwiseWaapiHelper::ID, TArray<TSharedPtr<FJsonValue>> { MakeShareable(new FJsonValueString(ItemID)) });
		args->SetObjectField(WwiseWaapiHelper::FROM, from);
	}

	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
	options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>> { MakeShareable(new FJsonValueString(WwiseWaapiHelper::AT + ItemProperty)) });

#if AK_SUPPORT_WAAPI
	TSharedPtr<FJsonObject> outJsonResult;
	if (!waapiClient->Call(ak::wwise::core::object::get, args, options, outJsonResult))
		return false;

	ItemInfoResult = outJsonResult->GetArrayField(WwiseWaapiHelper::RETURN)[0]->AsObject();
#endif
	return true;
}

bool SubscribeToPropertyStateChange(const FString& ItemID, const FString& ItemProperty, WampEventCallback CallBack, uint64& SubscriptionId, TSharedPtr<FJsonObject>& outJsonResult)
{
	auto* waapiClient = FAkWaapiClient::Get();
	if (!waapiClient)
		return false;

#if AK_SUPPORT_WAAPI
	TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
	options->SetStringField(WwiseWaapiHelper::OBJECT, ItemID);
	options->SetStringField(WwiseWaapiHelper::PROPERTY, ItemProperty);

	return waapiClient->Subscribe(ak::wwise::core::object::propertyChanged, options, CallBack, SubscriptionId, outJsonResult);
#endif
	return false;
}

#undef LOCTEXT_NAMESPACE
