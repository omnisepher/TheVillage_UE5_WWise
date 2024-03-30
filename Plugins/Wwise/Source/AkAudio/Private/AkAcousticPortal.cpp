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
	AAkAcousticPortal.cpp:
=============================================================================*/

#include "AkAcousticPortal.h"
#include "AkAudioDevice.h"
#include "AkComponentHelpers.h"
#include "AkSpatialAudioHelper.h"
#include "AkSpatialAudioDrawUtils.h"
#include "AkRoomComponent.h"
#include "AkComponent.h"
#include "AkCustomVersion.h"
#include "AkSpatialAudioVolume.h"
#include "AkReverbZone.h"
#include "AkSettingsPerUser.h"
#include "WwiseUnrealDefines.h"
#include "WwiseUnrealObjectHelper.h"
#include "WwiseUnrealEngineHelper.h"

#include "Components/BrushComponent.h"
#include "Model.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"

// A standard AAkAcousticPortal is based on a cube brush with verts at [+/-]100 X,Y,Z. 
static const float kDefaultBrushExtents = 100.f;

// min portal size, in cm. For raycasts
static const float kMinPortalSize = 10.0f; 

#if WITH_EDITOR
#include "AkDrawPortalComponent.h"
#include "AkAudioStyle.h"
#include "LevelEditorViewport.h"
#endif

UAkPortalComponent::UAkPortalComponent(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	ObstructionRefreshInterval = 0.f;

	PortalState = InitialState;
	PortalNeedsUpdate = true;

	PortalOcclusion = InitialOcclusion;
	PortalOcclusionChanged = true;

	bUseAttachParentBound = true;

	FrontRoom = TWeakObjectPtr<UAkRoomComponent>();
	BackRoom = TWeakObjectPtr<UAkRoomComponent>();

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bTickInEditor = true;

#if WITH_EDITOR
	bWantsOnUpdateTransform = true;
	bWantsInitializeComponent = true;
#else
	bWantsOnUpdateTransform = false;
#endif

#if WITH_EDITOR
	if (AkSpatialAudioHelper::GetObjectReplacedEvent())
	{
		AkSpatialAudioHelper::GetObjectReplacedEvent()->AddUObject(this, &UAkPortalComponent::HandleObjectsReplaced);
	}
#endif
}

void UAkPortalComponent::SetDynamic(bool bInDynamic)
{
	bDynamic = bInDynamic;
#if WITH_EDITOR
	bWantsOnUpdateTransform = true;

	// If we're PIE, or somehow otherwise in a game world in editor, simulate the bDynamic behaviour.
	UWorld* world = GetWorld();
	if (world != nullptr && (world->WorldType == EWorldType::Type::Game || world->WorldType == EWorldType::Type::PIE))
	{
		bWantsOnUpdateTransform = bDynamic;
	}
#else
	bWantsOnUpdateTransform = bDynamic;
#endif
}

void UAkPortalComponent::OnRegister()
{
	Super::OnRegister();

#if WITH_EDITOR
	bWantsOnUpdateTransform = true;

	// If we're PIE, or somehow otherwise in a game world in editor, simulate the bDynamic behaviour.
	UWorld* world = GetWorld();
	if (world != nullptr && (world->WorldType == EWorldType::Type::Game || world->WorldType == EWorldType::Type::PIE))
	{
		bWantsOnUpdateTransform = bDynamic;
	}
#else
	bWantsOnUpdateTransform = bDynamic;
#endif

	SetRelativeTransform(FTransform::Identity);
	InitializeParent();
	UpdateConnectedRooms(true);

	PortalNeedsUpdate = true;

#if WITH_EDITOR
	if (GetDefault<UAkSettingsPerUser>()->VisualizeRoomsAndPortals)
	{
		InitializeDrawComponent();
	}
#endif
}

void UAkPortalComponent::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (!HasAnyFlags(RF_Transient))
	{
		DestroyTextVisualizers();
	}
#endif
	FAkAudioDevice * Dev = FAkAudioDevice::Get();
	if (Dev != nullptr)
	{
		RemovePortalConnections();
		Dev->RemoveSpatialAudioPortal(this);
	}
}

#if WITH_EDITOR
void UAkPortalComponent::BeginDestroy()
{
	Super::BeginDestroy();
	if (AkSpatialAudioHelper::GetObjectReplacedEvent())
	{
		AkSpatialAudioHelper::GetObjectReplacedEvent()->RemoveAll(this);
	}
}

void UAkPortalComponent::HandleObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	if (ReplacementMap.Contains(Parent.Get()))
	{
		InitializeParent();
	}
	if (ReplacementMap.Contains(FrontRoom.Get()) || ReplacementMap.Contains(BackRoom.Get()))
	{
		PortalRoomsNeedUpdate = true;
	}
}

void UAkPortalComponent::InitializeComponent()
{
	Super::InitializeComponent();
	RegisterVisEnabledCallback();
}

void UAkPortalComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	RegisterVisEnabledCallback();
}

void UAkPortalComponent::PostLoad()
{
	Super::PostLoad();
	RegisterVisEnabledCallback();
}

void UAkPortalComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();
	AkSettingsPerUser->OnShowRoomsPortalsChanged.Remove(ShowPortalsChangedHandle);
	ShowPortalsChangedHandle.Reset();
	DestroyDrawComponent();
}
#endif // WITH_EDITOR

bool UAkPortalComponent::MoveComponentImpl(
	const FVector & Delta,
	const FQuat & NewRotation,
	bool bSweep,
	FHitResult * Hit,
	EMoveComponentFlags MoveFlags,
	ETeleportType Teleport)
{
	if (AkComponentHelpers::DoesMovementRecenterChild(this, Parent.Get(), Delta))
		Super::MoveComponentImpl(Delta, NewRotation, bSweep, Hit, MoveFlags, Teleport);

	return false;
}

void UAkPortalComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	PortalRoomsNeedUpdate = true;
	PortalNeedsUpdate = true;
}


#if WITH_EDITOR
void UAkPortalComponent::RegisterVisEnabledCallback()
{
	if (!ShowPortalsChangedHandle.IsValid())
	{
		UAkSettingsPerUser* AkSettingsPerUser = GetMutableDefault<UAkSettingsPerUser>();
		ShowPortalsChangedHandle = AkSettingsPerUser->OnShowRoomsPortalsChanged.AddLambda([this, AkSettingsPerUser]()
		{
			if (AkSettingsPerUser->VisualizeRoomsAndPortals)
			{
				InitializeDrawComponent();
			}
			else
			{
				DestroyDrawComponent();
			}

			UpdateTextVisibility();
		});
	}
}


void UAkPortalComponent::InitializeDrawComponent()
{
	if (AActor* Owner = GetOwner())
	{
		if (DrawPortalComponent == nullptr)
		{
			DrawPortalComponent = NewObject<UDrawPortalComponent>(Owner, NAME_None, RF_Transactional | RF_TextExportTransient);
			DrawPortalComponent->SetupAttachment(this);
			DrawPortalComponent->SetIsVisualizationComponent(true);
			DrawPortalComponent->CreationMethod = CreationMethod;
			DrawPortalComponent->RegisterComponentWithWorld(GetWorld());
			DrawPortalComponent->MarkRenderStateDirty();
		}
	}
}

void UAkPortalComponent::DestroyDrawComponent()
{
	if (DrawPortalComponent != nullptr)
	{
		DrawPortalComponent->DestroyComponent();
		DrawPortalComponent = nullptr;
	}
}
#endif 

void UAkPortalComponent::InitializeParent()
{
	USceneComponent* SceneParent = GetAttachParent();
	if (SceneParent != nullptr)
	{
		Parent = Cast<UPrimitiveComponent>(SceneParent);
		if (!Parent.IsValid())
		{
			AkComponentHelpers::LogAttachmentError(this, SceneParent, "UPrimitiveComponent");
		}
#if WITH_EDITOR
		DestroyTextVisualizers();
		InitTextVisualizers();
		UpdateRoomNames();
		UpdateTextLocRotVis();
#endif
	}
}

void UAkPortalComponent::SetSpatialAudioPortal()
{
	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (AkAudioDevice != nullptr)
	{
		AkAudioDevice->SetSpatialAudioPortal(this);
		PortalNeedsUpdate = false;
	}
}

void UAkPortalComponent::EnablePortal()
{
	if (PortalState == AkAcousticPortalState::Closed)
	{
		PortalState = AkAcousticPortalState::Open;
		PortalNeedsUpdate = true;
	}
}

void UAkPortalComponent::DisablePortal()
{
	if (PortalState == AkAcousticPortalState::Open)
	{
		PortalState = AkAcousticPortalState::Closed;
		PortalNeedsUpdate = true;
	}
}

AkAcousticPortalState UAkPortalComponent::GetCurrentState() const
{
	return PortalState;
}

float UAkPortalComponent::GetPortalOcclusion() const
{
	return PortalOcclusion;
}

void UAkPortalComponent::SetPortalOcclusion(float InPortalOcclusion)
{
	if (InPortalOcclusion < 0.f)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkPortalComponent %s called SetPortalOcclusion with an occlusion value lower than 0 (%.6g). It was clamped to 0."), *GetPortalName(), InPortalOcclusion);
		InPortalOcclusion = 0.f;
	}

	if (InPortalOcclusion > 1.f)
	{
		UE_LOG(LogAkAudio, Warning, TEXT("UAkPortalComponent %s called SetPortalOcclusion with an occlusion value higher than 1 (%.6g). It was clamped to 1."), *GetPortalName(), InPortalOcclusion);
		InPortalOcclusion = 1.f;
	}

	if (PortalOcclusion != InPortalOcclusion)
	{
		PortalOcclusion = InPortalOcclusion;
		PortalOcclusionChanged = true;
	}
}

void UAkPortalComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetPortalState();
	ResetPortalOcclusion();

	UWorld* World = GetWorld();
	auto portalID = GetPortalID();
	ObstructionServiceFrontRoom.Init(portalID, World, ObstructionRefreshInterval);
	ObstructionServiceBackRoom.Init(portalID, World, ObstructionRefreshInterval);

	PortalRoomsNeedUpdate = true;
}

void UAkPortalComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	if (PortalRoomsNeedUpdate)
	{
		UpdateConnectedRooms();
	}

	if (PortalNeedsUpdate)
	{
		SetSpatialAudioPortal();
	}

	UWorld* World = GetWorld();
#if WITH_EDITOR
	if (World && (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::PIE))
	{
		// Only show the text renderer for selected actors.
		if (GetOwner()->IsSelected() && !bWasSelected)
		{
			bWasSelected = true;
			UpdateTextVisibility();
		}
		if (!GetOwner()->IsSelected() && bWasSelected)
		{
			bWasSelected = false;
			UpdateTextVisibility();
		}
	}
#endif

	if (!PortalPlacementValid())
	{
		return;
	}

	FAkAudioDevice* AkAudioDevice = FAkAudioDevice::Get();
	if (AkAudioDevice)
	{
		if (PortalOcclusionChanged)
		{
			AkAudioDevice->SetPortalObstructionAndOcclusion(this, 0.f, PortalOcclusion);
			PortalOcclusionChanged = false;
		}

		if (World && AkAudioDevice->ShouldNotifySoundEngine(World->WorldType))
		{
			AkObstructionAndOcclusionService::ListenerMap Listeners;
			UAkComponent* Listener = AkAudioDevice->GetSpatialAudioListener();
			if (Listener != nullptr)
			{
				AkObstructionAndOcclusionService::FListenerInfo ListenerInfo(Listener->GetPosition(), Listener->GetSpatialAudioRoomID());
				Listeners.Add(Listener->GetAkGameObjectID(), ListenerInfo);
			}

			AkObstructionAndOcclusionService::PortalMap FrontPortals;
			AkObstructionAndOcclusionService::PortalMap BackPortals;

			AkAudioDevice->GetObsOccServicePortalMap(FrontRoom.Get(), GetWorld(), FrontPortals);
			AkAudioDevice->GetObsOccServicePortalMap(BackRoom.Get(), GetWorld(), BackPortals);

			ObstructionServiceFrontRoom.Tick(Listeners, FrontPortals, GetOwner()->GetActorLocation(), GetOwner(), GetFrontRoomID(), ObstructionCollisionChannel, DeltaTime, ObstructionRefreshInterval);
			ObstructionServiceBackRoom.Tick(Listeners, BackPortals, GetOwner()->GetActorLocation(), GetOwner(), GetBackRoomID(), ObstructionCollisionChannel, DeltaTime, ObstructionRefreshInterval);
		}
	}
}

void UAkPortalComponent::ResetPortalState()
{
	PortalState = InitialState;
	PortalNeedsUpdate = true;
}

void UAkPortalComponent::ResetPortalOcclusion()
{
	PortalOcclusion = InitialOcclusion;
	PortalOcclusionChanged = true;
}

bool UAkPortalComponent::UpdateConnectedRooms(bool in_bForceUpdate/* = false*/)
{
	FAkAudioDevice* Dev = FAkAudioDevice::Get();
	if (UNLIKELY(!Dev || !GetWorld()))
	{
		return false;
	}

	/* Keep note of the rooms and validity before the update. */
	TWeakObjectPtr<UAkRoomComponent> pPreviousFront = FrontRoom;
	TWeakObjectPtr<UAkRoomComponent> pPreviousBack = BackRoom;
	AkRoomID previousFrontID = GetFrontRoomID();
	AkRoomID previousBackID = GetBackRoomID();
	/* Update the room connections */
	FrontRoom = TWeakObjectPtr<UAkRoomComponent>();
	BackRoom = TWeakObjectPtr<UAkRoomComponent>();
	FindConnectedComponents(Dev->GetRoomIndex(), FrontRoom, BackRoom);
	LastRoomsUpdate = GetWorld()->GetTimeSeconds();
	PreviousLocation = GetComponentLocation();
	PreviousRotation = GetComponentRotation();

	bool bRoomsChanged = false;
	bool PortalIsValid = PortalPlacementValid();

	if (in_bForceUpdate || GetFrontRoomID() != previousFrontID)
	{
		bRoomsChanged = true;

		if (pPreviousFront.IsValid())
		{
			pPreviousFront->RemovePortalConnection(GetPortalID());
		}
		else
		{
			Dev->RemovePortalConnectionToOutdoors(GetWorld(), GetPortalID());
		}

		if (PortalIsValid)
		{
			if (FrontRoom.IsValid())
			{
				FrontRoom->AddPortalConnection(this);
			}
			else
			{
				Dev->AddPortalConnectionToOutdoors(GetWorld(), this);
			}
		}
	}

	if (in_bForceUpdate || GetBackRoomID() != previousBackID)
	{
		bRoomsChanged = true;

		// Make sure we are not removing connections we just added in the front room condition above.
		if (pPreviousBack != FrontRoom)
		{
			if (pPreviousBack.IsValid())
			{
				pPreviousBack->RemovePortalConnection(GetPortalID());
			}
			else
			{
				Dev->RemovePortalConnectionToOutdoors(GetWorld(), GetPortalID());
			}
		}

		if (PortalIsValid)
		{
			if (BackRoom.IsValid())
			{
				BackRoom->AddPortalConnection(this);
			}
			else
			{
				Dev->AddPortalConnectionToOutdoors(GetWorld(), this);
			}
		}
	}

	if (bRoomsChanged)
	{
		PortalNeedsUpdate = true;
#if WITH_EDITOR
		UpdateRoomNames();
#endif
	}

#if WITH_EDITOR
	UpdateTextLocRotVis();
#endif

	PortalRoomsNeedUpdate = false;

	/* Return true if any room connection has changed. */
	return bRoomsChanged;
}

void UAkPortalComponent::RemovePortalConnections()
{
	FAkAudioDevice* Dev = FAkAudioDevice::Get();

	if (FrontRoom.IsValid())
	{
		FrontRoom->RemovePortalConnection(GetPortalID());
	}
	else if (Dev != nullptr)
	{
		Dev->RemovePortalConnectionToOutdoors(GetWorld(), GetPortalID());
	}

	if (BackRoom != FrontRoom)
	{
		if (BackRoom.IsValid())
		{
			BackRoom->RemovePortalConnection(GetPortalID());
		}
		else if (Dev != nullptr)
		{
			Dev->RemovePortalConnectionToOutdoors(GetWorld(), GetPortalID());
		}
	}
}

UPrimitiveComponent* UAkPortalComponent::GetPrimitiveParent() const
{
	return Parent.IsValid() ? Parent.Get() : nullptr;
}

bool UAkPortalComponent::PortalPlacementValid() const
{
	// Front and Back Rooms cannot be the same Room.
	bool bIsPortalPlacementValid = GetFrontRoomID() != GetBackRoomID();

	// Front and Back Rooms cannot be in the same Reverb Zone hierarchy
	if (bIsPortalPlacementValid)
	{
		auto FrontRootID = FrontRoom.IsValid() ? FrontRoom->GetRootID() : AK::SpatialAudio::kOutdoorRoomID;
		auto BackRootID = BackRoom.IsValid() ? BackRoom->GetRootID() : AK::SpatialAudio::kOutdoorRoomID;
		bIsPortalPlacementValid = FrontRootID != BackRootID;
	}

	return bIsPortalPlacementValid;
}

FVector UAkPortalComponent::GetExtent() const
{
	FBoxSphereBounds ComponentBounds = Bounds;
	if (Parent.IsValid())
	{
		FTransform Transform (Parent->GetComponentTransform());
		Transform.SetRotation(FQuat::Identity);
		ComponentBounds = Parent->CalcBounds(Transform);
	}
	return ComponentBounds.BoxExtent;
}

AkRoomID UAkPortalComponent::GetFrontRoomID() const { return FrontRoom.IsValid() ? FrontRoom->GetRoomID() : AkRoomID(); }
AkRoomID UAkPortalComponent::GetBackRoomID() const { return BackRoom.IsValid() ? BackRoom->GetRoomID() : AkRoomID(); }

void UAkPortalComponent::FindConnectedComponents(FAkEnvironmentIndex& RoomIndex, TWeakObjectPtr<UAkRoomComponent>& out_pFront, TWeakObjectPtr<UAkRoomComponent>& out_pBack)
{
	out_pFront = TWeakObjectPtr<UAkRoomComponent>();
	out_pBack = TWeakObjectPtr<UAkRoomComponent>();

	FAkAudioDevice* pAudioDevice = FAkAudioDevice::Get();
	if (pAudioDevice != nullptr && Parent.IsValid())
	{
		float x = GetExtent().X;
		FVector frontVector(x, 0.f, 0.f);

		FTransform toWorld = Parent->GetComponentTransform();
		toWorld.SetScale3D(FVector(1.0f));

		FVector frontPoint = toWorld.TransformPosition(frontVector);
		FVector backPoint = toWorld.TransformPosition(-1 * frontVector);

		TArray<UAkRoomComponent*> front = RoomIndex.Query<UAkRoomComponent>(frontPoint, GetWorld());
		if (front.Num() > 0)
			out_pFront = front[0];

		TArray<UAkRoomComponent*> back = RoomIndex.Query<UAkRoomComponent>(backPoint, GetWorld());
		if (back.Num() > 0)
			out_pBack = back[0];
	}
}

FString UAkPortalComponent::GetPortalName()
{
	FString nameStr = UObject::GetName();

	AActor* owner = GetOwner();
	if (owner != nullptr)
	{
#if WITH_EDITOR
		nameStr = owner->GetActorLabel();
#else
		nameStr = owner->GetName();
#endif
		if (Parent.IsValid())
		{
			TInlineComponentArray<UAkPortalComponent*> PortalComponents;
			owner->GetComponents(PortalComponents);
			if (PortalComponents.Num() > 1)
				nameStr.Append(FString("_").Append(Parent->GetName()));
		}
	}

	return nameStr;
}

#if WITH_EDITOR
bool UAkPortalComponent::AreTextVisualizersInitialized() const
{
	return FrontRoomText != nullptr || BackRoomText != nullptr;
}

void UAkPortalComponent::InitTextVisualizers()
{
	if (!HasAnyFlags(RF_Transient))
	{
		FString NamePrefix = GetOwner()->GetName() + GetName();
		UMaterialInterface* mat = Cast<UMaterialInterface>(FAkAudioStyle::GetAkForegroundTextMaterial());
		FrontRoomText = NewObject<UTextRenderComponent>(GetOuter(), *(NamePrefix + "_FrontRoomName"));
		BackRoomText = NewObject<UTextRenderComponent>(GetOuter(), *(NamePrefix + "_BackRoomName"));
		FrontRoomText->SetText(FText::FromString(""));
		BackRoomText->SetText(FText::FromString(""));
		TArray<UTextRenderComponent*> TextComponents{ FrontRoomText, BackRoomText };
		for (UTextRenderComponent* Text : TextComponents)
		{
			if (mat != nullptr)
				Text->SetTextMaterial(mat);
			Text->RegisterComponentWithWorld(GetWorld());
			Text->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
			Text->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			Text->bIsEditorOnly = true;
			Text->bSelectable = true;
			Text->bAlwaysRenderAsText = true;
			Text->SetHorizontalAlignment(EHTA_Center);
			Text->SetWorldScale3D(FVector(1.0f));
		}
		FrontRoomText->SetVerticalAlignment(EVRTA_TextTop);
		BackRoomText->SetVerticalAlignment(EVRTA_TextBottom);
	}
}

void UAkPortalComponent::DestroyTextVisualizers()
{
	if (FrontRoomText != nullptr)
	{
		FrontRoomText->DestroyComponent();
		FrontRoomText = nullptr;
	}
	if (BackRoomText != nullptr)
	{
		BackRoomText->DestroyComponent();
		BackRoomText = nullptr;
	}
}

void UAkPortalComponent::UpdateRoomNames()
{
	if (!Parent.IsValid() || HasAnyFlags(RF_Transient) || !AreTextVisualizersInitialized())
		return;

	if (FrontRoomText != nullptr)
	{		
		FrontRoomText->SetText(FText::FromString(""));
		if (FrontRoom != nullptr)
			FrontRoomText->SetText(FText::FromString(FrontRoom->GetRoomName()));
	}
	if (BackRoomText != nullptr)
	{
		BackRoomText->SetText(FText::FromString(""));
		if (BackRoom != nullptr)
			BackRoomText->SetText(FText::FromString(BackRoom->GetRoomName()));
	}
}

void UAkPortalComponent::UpdateTextRotations() const
{
	if (!Parent.IsValid() || !AreTextVisualizersInitialized())
		return;
	FVector BoxExtent = Parent->CalcBounds(FTransform()).BoxExtent;
	const FTransform T = Parent->GetComponentTransform();
	AkDrawBounds DrawBounds(T, BoxExtent);
	// Setup the font normal to orient the text.
	FVector Front = DrawBounds.FLD() - DrawBounds.BLD();
	Front.Normalize();
	FVector Up = DrawBounds.FLU() - DrawBounds.FLD();
	Up.Normalize();
	if (FrontRoomText != nullptr)
		FrontRoomText->SetVerticalAlignment(EVRTA_TextTop);
	if (BackRoomText != nullptr)
		BackRoomText->SetVerticalAlignment(EVRTA_TextBottom);
	if (FVector::DotProduct(FVector::UpVector, Up) < 0.0f)
	{
		if (FrontRoomText != nullptr)
			FrontRoomText->SetVerticalAlignment(EVRTA_TextBottom);
		if (BackRoomText != nullptr)
			BackRoomText->SetVerticalAlignment(EVRTA_TextTop);
		Up *= -1.0f;
	}
	// Choose to point both text components towards the front or back of the portal, depending on the position of the camera, so that they are both always readable.
	FVector CamToCentre;
	if (GCurrentLevelEditingViewportClient != nullptr)
	{
		CamToCentre = GCurrentLevelEditingViewportClient->GetViewLocation() - Parent->Bounds.Origin;
		if (FVector::DotProduct(CamToCentre, Front) < 0.0f)
			Front *= -1.0f;
	}
	if (FrontRoomText != nullptr)
		FrontRoomText->SetWorldRotation(UKismetMathLibrary::MakeRotFromXZ(Front, Up));
	if (BackRoomText != nullptr)
		BackRoomText->SetWorldRotation(UKismetMathLibrary::MakeRotFromXZ(Front, Up));
}

void UAkPortalComponent::UpdateTextVisibility()
{
	if (!AreTextVisualizersInitialized()) return;
	if (GetOwner() == nullptr) return;

	bool Visible = false;

	if (GetDefault<UAkSettingsPerUser>()->VisualizeRoomsAndPortals)
	{
		Visible = true;
	}
	else
	{
		if (GetWorld() != nullptr)
		{
			EWorldType::Type WorldType = GetWorld()->WorldType;
			if (WorldType == EWorldType::Editor)
			{
				Visible = GetOwner()->IsSelected();
			}
			else if (WorldType == EWorldType::EditorPreview)
			{
				Visible = true;
			}
		}
	}

	if (BackRoomText != nullptr)
		BackRoomText->SetVisibility(Visible);
	if (FrontRoomText != nullptr)
		FrontRoomText->SetVisibility(Visible);
}

void UAkPortalComponent::UpdateTextLocRotVis()
{
	if (!Parent.IsValid() || !AreTextVisualizersInitialized())
		return;
	FVector BoxExtent = Parent->CalcBounds(FTransform()).BoxExtent;
	const FTransform T = Parent->GetComponentTransform();
	AkDrawBounds DrawBounds(T, BoxExtent);
	float PortalWidth = 0.0f;
	FVector Right;
	(DrawBounds.FRD() - DrawBounds.FLD()).ToDirectionAndLength(Right, PortalWidth);
	// Setup the font normal to orient the text.
	FVector Front = DrawBounds.FLD() - DrawBounds.BLD();
	FVector Up = DrawBounds.FLU() - DrawBounds.FLD();

	if (FrontRoomText != nullptr)
	{
		FrontRoomText->SetWorldScale3D(FVector(1.0f));
		// Get a point at the top center of the local axis-aligned bounds, to position the front room text.
		//FVector Top = Parent->Bounds.Origin + FVector(0.0f, 0.0f, Parent->Bounds.BoxExtent.Z);
		FVector Top = Parent->Bounds.Origin;// +Front + Up;
		// Add a depth offset so that the text sits out at the top front of the portal
		Top += Front * 0.5f;
		Top += Up * 0.5f;
		FrontRoomText->SetWorldLocation(Top);
		const FVector FrontTextWorldSize = FrontRoomText->GetTextWorldSize();
		const float TextWidth = FrontTextWorldSize.GetAbsMax();
		if (TextWidth > PortalWidth && PortalWidth > 0.0f)
			FrontRoomText->SetWorldScale3D(FVector(PortalWidth / TextWidth));
	}
	if (BackRoomText != nullptr)
	{
		BackRoomText->SetWorldScale3D(FVector(1.0f));
		// Get a point at the bottom center of the local axis-aligned bounds, to position the back room text.
		//FVector Bottom = Parent->Bounds.Origin - FVector(0.0f, 0.0f, Parent->Bounds.BoxExtent.Z);
		FVector Bottom = Parent->Bounds.Origin;
		// Add a depth offset so that the text sits out at the bottom back of the portal
		Bottom -= Front * 0.5f;
		Bottom -= Up * 0.5f;
		BackRoomText->SetWorldLocation(Bottom);
		const FVector BackTextWorldSize = BackRoomText->GetTextWorldSize();
		const float TextWidth = BackTextWorldSize.GetAbsMax();
		if (TextWidth > PortalWidth && PortalWidth > 0.0f)
			BackRoomText->SetWorldScale3D(FVector(PortalWidth / TextWidth));
	}
	UpdateTextRotations();
	UpdateTextVisibility();
}

#endif
/*------------------------------------------------------------------------------------
	AAkAcousticPortal
------------------------------------------------------------------------------------*/

AAkAcousticPortal::AAkAcousticPortal(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// Property initialization
	static const FName CollisionProfileName(TEXT("OverlapAll"));
	GetBrushComponent()->SetCollisionProfileName(CollisionProfileName);

	bColored = true;
	BrushColor = FColor(255, 196, 137, 255);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	static const FName PortalComponentName = TEXT("PortalComponent");
	Portal = ObjectInitializer.CreateDefaultSubobject<UAkPortalComponent>(this, PortalComponentName);
	Portal->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

#if WITH_EDITOR
	CollisionChannel = EAkCollisionChannel::EAKCC_UseIntegrationSettingsDefault;
#endif
}

void AAkAcousticPortal::EnablePortal()
{
	if (Portal != nullptr)
	{
		Portal->EnablePortal();
	}
	else
	{
		UE_LOG(LogAkAudio, Warning, TEXT("AAkAcousticPortal %s called EnablePortal with uninitialized portal component."), *GetName());
	}
}

void AAkAcousticPortal::DisablePortal()
{
	if (Portal != nullptr)
	{
		Portal->DisablePortal();
	}
	else
	{
		UE_LOG(LogAkAudio, Warning, TEXT("AAkAcousticPortal %s called DisablePortal with uninitialized portal component."), *GetName());
	}
}

AkAcousticPortalState AAkAcousticPortal::GetCurrentState() const
{
	if (Portal != nullptr)
		return Portal->GetCurrentState();
	UE_LOG(LogAkAudio, Warning, TEXT("AAkAcousticPortal %s called GetCurrentState with uninitialized portal component."), *GetName());
	return AkAcousticPortalState::Closed;
}

void AAkAcousticPortal::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (bRequiresStateMigration)
	{
		if (Portal != nullptr)
		{
			Portal->InitialState = InitialState;
			bRequiresStateMigration = false;
		}
	}
	
	if (bRequiresTransformMigration)
	{
		FVector right = FVector(0.0f, 1.0f, 0.0f);
		FVector left = FVector(0.0f, -1.0f, 0.0f);
		FTransform actorTransform = GetActorTransform();
		/* get the local 'front' (with respect to Y). */
		FVector localYFront = (actorTransform.TransformPosition(right) - actorTransform.TransformPosition(left));
		localYFront.Normalize();
		FVector scale = GetActorScale3D();
		SetActorScale3D(FVector(scale.Y, scale.X, scale.Z));
		/* get the local front, using Unreal coordinate orientation. */
		FVector localXFront = GetActorForwardVector();
		/* get the local up vector around which to rotate. */
		FVector localUp = FVector::CrossProduct(localYFront, localXFront);
		/* rotate the local front vector around the local up, such that it points along the 'true' local front, in Unreal terms. */
		localXFront = localXFront.RotateAngleAxis(-90.0f, localUp);
		/* Set up new local axes such that localUp remains constant, local front is changed to localXFront, and the local right is calculated from these two. */
		SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(localXFront, localUp));

		bRequiresTransformMigration = false;
	}
}

void AAkAcousticPortal::PostLoad()
{
	Super::PostLoad();
	const int32 AkVersion = GetLinkerCustomVersion(FAkCustomVersion::GUID);

	if (AkVersion < FAkCustomVersion::SpatialAudioExtentAPIChange)
	{
		bRequiresTransformMigration = true;
	}

	if (AkVersion < FAkCustomVersion::SpatialAudioComponentisation)
	{
		bRequiresStateMigration = true;
	}
}

void AAkAcousticPortal::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FAkCustomVersion::GUID);
	Super::Serialize(Ar);
}

#if WITH_EDITOR
ECollisionChannel AAkAcousticPortal::GetCollisionChannel()
{
	return UAkSettings::ConvertFitToGeomCollisionChannel(CollisionChannel.GetValue());
}

void AAkAcousticPortal::FitRaycast()
{
	static const FName NAME_SAV_Fit = TEXT("AAkAcousticPortalRaycast");

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
	if (!World)
		return;

	TArray<TTuple<float, FVector, FVector>> hits;

	// Ray length - DetectionRadius X current scale. 
	float RayLength = GetDetectionRadius();

	FCollisionQueryParams CollisionParams(NAME_SAV_Fit, true, this);

	FVector RaycastOrigin = bUseSavedRaycastOrigin ? SavedRaycastOrigin : GetActorLocation();

	float Offset = 2.f / kNumRaycasts;
	float Increment = PI * (3.f - sqrtf(5.f));

	TArray< FHitResult > OutHits;

	for (int i = 0; i < kNumRaycasts; ++i)
	{
		float x = ((i * Offset) - 1) + (Offset / 2);
		float r = sqrtf(1.f - powf(x, 2.f));

		float phi = ((i + 1) % kNumRaycasts) * Increment;

		float y = cosf(phi) * r;
		float z = sinf(phi) * r;

		FVector to = RaycastOrigin + FVector(x, y, z) * RayLength;

		OutHits.Empty();
		World->LineTraceMultiByObjectType(OutHits, RaycastOrigin, to, (int)GetCollisionChannel(), CollisionParams);

		if (OutHits.Num() > 0)
		{
			bool bHit = false;
			FVector ImpactPoint0;
			FVector ImpactNormal0;

			for (auto& res : OutHits)
			{
				if (res.IsValidBlockingHit() &&
					!AkSpatialAudioHelper::IsAkSpatialAudioActorClass(WwiseUnrealHelper::GetActorFromHitResult(res)))
				{
					bHit = true;
					ImpactPoint0 = res.ImpactPoint;
					ImpactNormal0 = res.ImpactNormal;
					break;
				}
			}

			if (bHit)
			{
				OutHits.Empty();
				World->LineTraceMultiByObjectType(OutHits, ImpactPoint0, ImpactPoint0 + ImpactNormal0 * RayLength, (int)GetCollisionChannel(), CollisionParams);

				bHit = false;
				FVector ImpactPoint1;

				for (auto& res : OutHits)
				{
					if (res.IsValidBlockingHit() &&
						res.Distance > kMinPortalSize &&
						!AkSpatialAudioHelper::IsAkSpatialAudioActorClass(WwiseUnrealHelper::GetActorFromHitResult(res)))
					{
						bHit = true;
						ImpactPoint1 = res.ImpactPoint;
						break;
					}
				}

				if (bHit)
				{
					float distance = (ImpactPoint0 - ImpactPoint1).Size();
					hits.Emplace(MakeTuple(distance, ImpactPoint0, ImpactPoint1));
				}
				
			}
		}
	}

	auto SortPredicate = [](TTuple<float, FVector, FVector>& A, TTuple<float, FVector, FVector>& B) {	return A.Get<0>() < B.Get<0>(); };

	Algo::Sort(hits, SortPredicate);
	
	static const float kDotEpsilon = 0.1f;
	static const float kLineIntersectThresh = 2.0f;

	float minDist = FLT_MAX;
	int Best0 = INT_MAX;
	int Best1 = INT_MAX;
	bool bIntersects = false;
	for (int i = 0; i < hits.Num() && !bIntersects; ++i)
	{
		FVector& pti = hits[i].Get<1>();
		FVector vi = hits[i].Get<2>() - hits[i].Get<1>();
		FVector diri;
		float leni;
		vi.ToDirectionAndLength(diri, leni);

		for (int j = i + 1; j < hits.Num() && !bIntersects; ++j)
		{
			FVector& ptj = hits[j].Get<1>();
			FVector vj = hits[j].Get<2>() - hits[j].Get<1>();
			FVector dirj;
			float lenj;
			vj.ToDirectionAndLength(dirj, lenj);

			if (FMath::Abs(FVector::DotProduct(diri, dirj)) < kDotEpsilon)
			{
				float proj_ji = FVector::DotProduct((ptj - pti), diri);
				if (proj_ji > 0.f && proj_ji < leni)
				{
					float proj_ij = FVector::DotProduct((pti - ptj), dirj);
					if (proj_ij > 0.f && proj_ij < lenj)
					{
						FVector p0 = pti + proj_ji * diri;
						FVector p1 = ptj + proj_ij * dirj;

						float dist = (p0 - p1).Size();
						if (dist < minDist)
						{
							minDist = dist;
							Best0 = i;
							Best1 = j;
							
							if (dist < kLineIntersectThresh)
							{
								//Assuming here we found a pretty good result, bail out so as to favor smaller portals over bigger ones.
								bIntersects = true;
							}
						}
					}
				}
			}
		}
	}

	if (bIntersects)
	{
		BestFit[0] = hits[Best0].Get<1>();
		BestFit[1] = hits[Best0].Get<2>();
		BestFit[2] = hits[Best1].Get<1>();
		BestFit[3] = hits[Best1].Get<2>();

		BestFitValid = true;
	}
	else
	{
		// We will hold on to the best fit points, as long as they are within the detection radius.
		BestFitValid = FVector::DistSquared(RaycastOrigin, (BestFit[0] + BestFit[1] + BestFit[2] + BestFit[3]) / 4.f) < DetectionRadius * DetectionRadius;
	}
}

void AAkAcousticPortal::FitPortal()
{
	if (!BestFitValid)
		return;

	FVector center;
	FVector front;
	FVector side;
	FVector up;
	FVector scale;
	
	FVector& pti = BestFit[0];
	FVector vi = BestFit[1] - BestFit[0];
	FVector diri;
	float leni;
	vi.ToDirectionAndLength(diri, leni);

	FVector& ptj = BestFit[2];
	FVector vj = BestFit[3] - BestFit[2];
	FVector dirj;
	float lenj;
	vj.ToDirectionAndLength(dirj, lenj);

	float proj_ji = FVector::DotProduct((ptj - pti), diri);
	if (proj_ji > 0.f && proj_ji < leni)
	{
		float proj_ij = FVector::DotProduct((pti - ptj), dirj);
		if (proj_ij > 0.f && proj_ij < lenj)
		{
			FVector p0 = pti + proj_ji * diri;
			FVector p1 = ptj + proj_ij * dirj;

			center = pti - proj_ij * dirj;
			center += diri * leni / 2.f + dirj * lenj / 2.f;

			front = FVector::CrossProduct(diri, dirj);
			side = diri;
			up = dirj;
			scale.Y = leni / 2.f;
			scale.Z = lenj / 2.f;

			scale /= kDefaultBrushExtents;

			scale.X = GetActorScale3D().X;

			auto* RC = GetRootComponent();
			if (RC)
			{
				RC->SetWorldLocation(center);
				FRotator rotation = FRotationMatrix::MakeFromXZ(front, up).Rotator();
				RC->SetWorldRotation(rotation);
				RC->SetWorldScale3D(scale);
			}
		}
	}
}

void AAkAcousticPortal::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (FitToGeometry)
	{
		FitRaycast();
		
		IsDragging = !bFinished;
		
		if (bFinished)
		{
			bUseSavedRaycastOrigin = false;
	
			FitPortal();
		}
	}
}

void AAkAcousticPortal::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AAkAcousticPortal, FitToGeometry))
		{
			ClearBestFit();

			if (FitToGeometry)
			{
				FitRaycast();
				FitPortal();
			}
		}

		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AAkAcousticPortal, DetectionRadius))
		{
			if (FitToGeometry)
			{
				if (!bUseSavedRaycastOrigin)
				{
					// Cache the actor position to get consistant results over multiple updates, since FitPortal() changes the actor location.
					SavedRaycastOrigin = GetActorLocation();
					bUseSavedRaycastOrigin = true;
				}

				FitRaycast();
				FitPortal();
			}
		}
	}
}

void AAkAcousticPortal::ClearBestFit()
{
	BestFit[0] = FVector::ZeroVector;
	BestFit[1] = FVector::ZeroVector;
	BestFit[2] = FVector::ZeroVector;
	BestFit[3] = FVector::ZeroVector;
	BestFitValid = false;
}

#endif // WITH_EDITOR

