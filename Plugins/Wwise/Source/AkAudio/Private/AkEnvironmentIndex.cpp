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

#include "AkEnvironmentIndex.h"
#include "AkAudioDevice.h"

void FAkEnvironmentOctreeSemantics::SetElementId(AK_OCTREE_TYPE<FAkEnvironmentOctreeElement, FAkEnvironmentOctreeSemantics>& OctreeOwner, const FAkEnvironmentOctreeElement& Element, AK_OCTREE_ELEMENT_ID Id)
{
	static_cast<UAkEnvironmentOctree&>(OctreeOwner).ObjectToOctreeId.Add(Element.Component->GetUniqueID(), Id);
}

void FAkEnvironmentIndex::Add(USceneComponent* EnvironmentToAdd)
{
	UWorld* CurrentWorld = EnvironmentToAdd->GetWorld();
	TUniquePtr<UAkEnvironmentOctree>& Octree = Map.FindOrAdd(CurrentWorld);
	
	if (Octree == nullptr)
	{
		Octree = MakeUnique<UAkEnvironmentOctree>();
	}

	if (Octree != nullptr)
	{
		FAkEnvironmentOctreeElement Element(EnvironmentToAdd);
		Octree->AddElement(Element);
	}
}

bool FAkEnvironmentIndex::Remove(USceneComponent* EnvironmentToRemove)
{
	UWorld* CurrentWorld = EnvironmentToRemove->GetWorld();
	TUniquePtr<UAkEnvironmentOctree>* Octree = Map.Find(CurrentWorld);

	if (Octree != nullptr && EnvironmentToRemove != nullptr)
	{
		AK_OCTREE_ELEMENT_ID* Id = (*Octree)->ObjectToOctreeId.Find(EnvironmentToRemove->GetUniqueID());
		if (Id != nullptr && (*Octree)->IsValidElementId(*Id))
		{
			(*Octree)->RemoveElement(*Id);
		}

		(*Octree)->ObjectToOctreeId.Remove(EnvironmentToRemove->GetUniqueID());
		return true;
	}

	return false;
}

void FAkEnvironmentIndex::Update(USceneComponent* Environment)
{
	Remove(Environment);
	Add(Environment);
}

void FAkEnvironmentIndex::Clear(const UWorld* World)
{
	Map.Remove(World);
}

bool FAkEnvironmentIndex::IsEmpty(const UWorld* World)
{
	TUniquePtr<UAkEnvironmentOctree>* Octree = Map.Find(World);
	return Octree == nullptr;
}
