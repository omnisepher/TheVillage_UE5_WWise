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
	AkComponentVisualizer.cpp:
=============================================================================*/
#include "AkComponentVisualizer.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "SceneView.h"
#include "SceneManagement.h"

#include "AkWaapiClient.h"
#include "AkWaapiUtils.h"

namespace FAkComponentVisualizer_Helper
{
	float GetRadius(const UAkComponent* AkComponent)
	{
		if (auto waapiClient = FAkWaapiClient::Get())
		{
			FString AkAudioEventName = {};
			if(AkComponent->AkAudioEvent)
			{
				AkAudioEventName = AkComponent->AkAudioEvent->GetName();
			}
			if (!AkAudioEventName.IsEmpty())
			{
				TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
				{
					TSharedPtr<FJsonObject> from = MakeShared<FJsonObject>();
					from->SetArrayField(WwiseWaapiHelper::NAME, TArray<TSharedPtr<FJsonValue>>
					{
						MakeShared<FJsonValueString>(FString::Printf(TEXT("Event:%s"), *AkAudioEventName))
					});
					args->SetObjectField(WwiseWaapiHelper::FROM, from);
				}

				TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
				options->SetArrayField(WwiseWaapiHelper::RETURN, TArray<TSharedPtr<FJsonValue>>
				{
					MakeShared<FJsonValueString>(WwiseWaapiHelper::MAX_RADIUS_ATTENUATION)
				});

#if AK_SUPPORT_WAAPI
				TSharedPtr<FJsonObject> result;
				if (waapiClient->Call(ak::wwise::core::object::get, args, options, result, 500, true))
				{
					TArray<TSharedPtr<FJsonValue>> ReturnArray = result->GetArrayField(WwiseWaapiHelper::RETURN);
					if (ReturnArray.Num() > 0)
					{
						const TSharedPtr<FJsonObject>* AttenuationObjectPtr = nullptr;
						if (ReturnArray[0]->AsObject()->TryGetObjectField(WwiseWaapiHelper::MAX_RADIUS_ATTENUATION, AttenuationObjectPtr))
						{
							return (*AttenuationObjectPtr)->GetNumberField(WwiseWaapiHelper::RADIUS);
						}
					}
				}
#endif
			}
		}

		return AkComponent->GetAttenuationRadius();
	}
}

void FAkComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAkComponent* AkComponent = Cast<const UAkComponent>(Component);
	if (!AkComponent)
		return;

	if (AkComponent->outerRadius != 0.f)
	{
		FColor RadialEmitterColor(255, 255, 0);
		DrawWireSphereAutoSides(PDI, AkComponent->GetComponentLocation(), RadialEmitterColor, AkComponent->outerRadius, SDPG_World);
	}
	if (AkComponent->innerRadius != 0.f)
	{
		FColor RadialEmitterColor(204, 204, 0);
		DrawWireSphereAutoSides(PDI, AkComponent->GetComponentLocation(), RadialEmitterColor, AkComponent->innerRadius, SDPG_World);
	}

	if (!View->Family->EngineShowFlags.AudioRadius)
		return;

	auto radius = FAkComponentVisualizer_Helper::GetRadius(AkComponent);
	if (radius <= 0.0f)
		return;

	FColor AudioOuterRadiusColor(255, 153, 0);
	DrawWireSphereAutoSides(PDI, AkComponent->GetComponentLocation(), AudioOuterRadiusColor, radius, SDPG_World);
}
