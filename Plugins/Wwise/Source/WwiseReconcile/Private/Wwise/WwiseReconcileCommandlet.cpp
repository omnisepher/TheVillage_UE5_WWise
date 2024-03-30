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

#include "Wwise/WwiseReconcileCommandlet.h"

#include "AkAudioEvent.h"
#include "AkAudioType.h"
#include "AkInitBank.h"
#include "AkUnrealAssetDataHelper.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "Wwise/Stats/Reconcile.h"
#include "Wwise/WwiseReconcile.h"
#include "Wwise/WwiseProjectDatabase.h"

static constexpr auto ModesParam = TEXT("modes");
static constexpr auto CreateOption = TEXT("create");
static constexpr auto UpdateOption = TEXT("update");
static constexpr auto DeleteOption = TEXT("delete");
static constexpr auto AllOption = TEXT("all");
static constexpr auto HelpOption = TEXT("help");

UWwiseReconcileCommandlet::UWwiseReconcileCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;

	HelpDescription = TEXT("Commandlet to reconcile UAssets with the Generated SoundBanks.");

	HelpParamNames.Add(ModesParam);
	HelpParamDescriptions.Add(FString::Format(TEXT("Comma separated list of operations to perform on assets.\n"
		"{0}: Create Unreal assets from the Generated SoundBanks\n"
		"{1}: Update existing Unreal assets. This updates the asset name as well as its metadata.\n"
		"{2}: Delete Unreal assets that no longer exist in the Generated SoundBanks\n"
		"{3}: Fully reconcile Unreal assets"),
		{CreateOption, UpdateOption, DeleteOption, AllOption}));

	HelpParamNames.Add("?, help");
	HelpParamDescriptions.Add(TEXT("Display help"));

	HelpUsage = FString::Format(TEXT("<Editor.exe> <path_to_uproject> -run=WwiseReconcileCommandlet -modes={0},{1},{2},{3}"), {CreateOption, DeleteOption, UpdateOption, AllOption});
}

int32 UWwiseReconcileCommandlet::Main(const FString& Params)
{
	int32 Result = 0;

	TMap<FString, FString> ParsedParams;
	ParseCommandLine(*Params, CmdTokens, CmdSwitches, ParsedParams);

	if( Params.Contains(TEXT("?")) || Params.Contains(HelpOption) )
	{
		PrintHelp();
		return Result;
	}

	EWwiseReconcileOperationFlags ReconcileOperationFlags = EWwiseReconcileOperationFlags::None;

	if (ParsedParams.Contains(ModesParam))
	{
		FString ModeStr = ParsedParams.FindRef("modes");

		TArray<FString> Modes;
		if (ModeStr.Contains(TEXT(",")))
		{
			ModeStr.ParseIntoArray(Modes, TEXT(","), true);
		}
		else
		{
			Modes.Add(ModeStr);
		}

		if (Modes.Num() == 0)
		{
			PrintHelp();
			Result = -1;
			return Result;
		}

		if (Modes.Contains(CreateOption))
		{
			ReconcileOperationFlags |= EWwiseReconcileOperationFlags::Create;
		}
		if (Modes.Contains(UpdateOption))
		{
			ReconcileOperationFlags |= EWwiseReconcileOperationFlags::UpdateExisting;
		}
		if (Modes.Contains(DeleteOption))
		{
			ReconcileOperationFlags |= EWwiseReconcileOperationFlags::Delete;
		}
		if (Modes.Contains(AllOption))
		{
			ReconcileOperationFlags |= EWwiseReconcileOperationFlags::All;
		}
	}

	if (ReconcileOperationFlags == EWwiseReconcileOperationFlags::None)
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("No Reconcile mode specified"));
		PrintHelp();
		Result = -1;
		return Result;
	}

	if (auto WwiseReconcile = IWwiseReconcile::Get())
	{
		WwiseReconcile->GetAllAssets(ReconcileItems);
		WwiseReconcile->GetAssetChanges(ReconcileItems, ReconcileOperationFlags);

		int TotalNumberOfAssets = WwiseReconcile->GetNumberOfAssets();

		if (TotalNumberOfAssets > 0)
		{
			UE_LOG(LogWwiseReconcile, Display, TEXT("Reconciling %d Wwise Asset(s)..."), TotalNumberOfAssets)

				if (!WwiseReconcile->ReconcileAssets())
				{
					Result = -1;
					UE_LOG(LogWwiseReconcile, Error, TEXT("Failed to reconcile assets. Check the log for details"));
				}
		}
		else
		{
			UE_LOG(LogWwiseReconcile, Display, TEXT("No Wwise Assets to Reconcile..."));
		}

		UE_LOG(LogWwiseReconcile, Display, TEXT("Finished reconciling Wwise Assets..."));
	}
	else
	{
		UE_LOG(LogWwiseReconcile, Error, TEXT("Failed to get Wwise Reconcile implementation."));
	}

	return Result;
}

void UWwiseReconcileCommandlet::PrintHelp()
{
	UE_LOG(LogWwiseReconcile, Display, TEXT("%s"), *HelpDescription);
	UE_LOG(LogWwiseReconcile, Display, TEXT("Usage: %s"), *HelpUsage);
	UE_LOG(LogWwiseReconcile, Display, TEXT("Parameters:"));
	for (int32 i = 0; i < HelpParamNames.Num(); ++i)
	{
		UE_LOG(LogWwiseReconcile, Display, TEXT("\t- %s: %s"), *HelpParamNames[i], *HelpParamDescriptions[i]);
	}
	UE_LOG(LogWwiseReconcile, Display, TEXT("For more information, see %s"), *HelpWebLink);
}