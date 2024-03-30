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

#include "Wwise/WwiseUnitTests.h"

#if WWISE_UNIT_TESTS
#include "Wwise/WwiseResourceLoaderImpl.h"

#include "Wwise/Mock/WwiseMockExternalSourceManager.h"
#include "Wwise/Mock/WwiseMockMediaManager.h"
#include "Wwise/Mock/WwiseMockSoundBankManager.h"

#include <array>
#include <atomic>

WWISE_TEST_CASE(ResourceLoader_Media, "Wwise::ResourceLoader::ResourceLoader_Media", "[ApplicationContextMask][SmokeFilter]")
{
	SECTION("Static")
	{
		static_assert(std::is_constructible<FWwiseResourceLoaderImpl>::value, "Resource Loader must be constructed without parameters");
		static_assert(!std::is_copy_constructible<FWwiseResourceLoaderImpl>::value, "Cannot copy a Resource Loader");
		static_assert(!std::is_copy_assignable<FWwiseResourceLoaderImpl>::value, "Cannot reassign a Resource Loader");
		static_assert(!std::is_move_constructible<FWwiseResourceLoaderImpl>::value, "Cannot move a Resource Loader");
	}

	SECTION("Instantiation")
	{
		FWwiseResourceLoaderImpl ResourceLoaderImpl;
	}

	SECTION("Singular Event")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseLocalizedEventCookedData CookedData;
		{
			FWwiseEventCookedData Data1;
			FWwiseSoundBankCookedData SoundBank1;
			SoundBank1.SoundBankId = 1;
			Data1.SoundBanks.Emplace(SoundBank1);

			FWwiseMediaCookedData Media1;
			Media1.MediaId = 1;
			Data1.Media.Emplace(MoveTemp(Media1));
			
			CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
		}

		// Creating Node
		auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

		CHECK(Node);
		if (UNLIKELY(!Node))
		{
			return;
		}

		// Loading Node
		FWwiseLoadedEventPromise LoadPromise;
		auto LoadFuture = LoadPromise.GetFuture();
		ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
		auto Loaded = LoadFuture.Get();		// Synchronously
		CHECK(Loaded);
		if (UNLIKELY(!Loaded))
		{
			return;
		}

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading Node
		FWwiseResourceUnloadPromise UnloadPromise;
		auto UnloadFuture = UnloadPromise.GetFuture();
		ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(Loaded));
		UnloadFuture.Get();		// Synchronously

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("Singular Event with required key")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseLocalizedEventCookedData CookedData;
		{
			FWwiseEventCookedData Data1;
			FWwiseSoundBankCookedData SoundBank1;
			SoundBank1.SoundBankId = 1;
			Data1.SoundBanks.Emplace(SoundBank1);

			FWwiseSwitchContainerLeafCookedData Leaf1;
			FWwiseMediaCookedData Leaf1Media;
			Leaf1Media.MediaId = 3;
			Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
			Leaf1.GroupValueSet.Emplace(GroupValue);
			Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));

			Data1.RequiredGroupValueSet.Add(GroupValue);
			
			CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
		}

		// Creating Node
		auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

		CHECK(Node);
		if (UNLIKELY(!Node))
		{
			return;
		}

		// Loading Node
		FWwiseLoadedEventPromise LoadPromise;
		auto LoadFuture = LoadPromise.GetFuture();
		ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
		auto Loaded = LoadFuture.Get();		// Synchronously
		CHECK(Loaded);
		if (UNLIKELY(!Loaded))
		{
			return;
		}

		CHECK(MediaManager.IsMediaLoaded(3));

		// Unloading Node
		FWwiseResourceUnloadPromise UnloadPromise;
		auto UnloadFuture = UnloadPromise.GetFuture();
		ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(Loaded));
		UnloadFuture.Get();		// Synchronously

		CHECK(!MediaManager.IsMediaLoaded(3));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("Another Event Loads the required Key for an Event")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseLocalizedEventCookedData CookedData;
		{
			FWwiseEventCookedData Data1;
			FWwiseSoundBankCookedData SoundBank1;
			SoundBank1.SoundBankId = 1;
			Data1.SoundBanks.Emplace(SoundBank1);

			Data1.RequiredGroupValueSet.Add(GroupValue);
			
			CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
		}
		
		// Creating Node
		auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

		CHECK(Node);
		if (UNLIKELY(!Node))
		{
			return;
		}

		FWwiseLocalizedEventCookedData CookedData2;
		{
			FWwiseEventCookedData Data2;
			FWwiseSoundBankCookedData SoundBank2;
			SoundBank2.SoundBankId = 2;
			Data2.SoundBanks.Emplace(SoundBank2);

			FWwiseSwitchContainerLeafCookedData Leaf1;
			FWwiseMediaCookedData Leaf1Media;
			Leaf1Media.MediaId = 3;
			Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
			Leaf1.GroupValueSet.Emplace(GroupValue);
			Data2.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
			
			CookedData2.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data2));
		}

		// Creating Node
		auto* Node2 = ResourceLoaderImpl.CreateEventNode(CookedData2, nullptr);

		CHECK(Node2);
		if (UNLIKELY(!Node2))
		{
			return;
		}

		CHECK(!MediaManager.IsMediaLoaded(3));

		// Loading Node2
		FWwiseLoadedEventPromise LoadPromise2;
		auto LoadFuture2 = LoadPromise2.GetFuture();
		ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise2), MoveTemp(Node2));
		auto Loaded2 = LoadFuture2.Get();		// Synchronously
		CHECK(Loaded2);
		if (UNLIKELY(!Loaded2))
		{
			return;
		}

		CHECK(!MediaManager.IsMediaLoaded(3));


		// Loading Node
		FWwiseLoadedEventPromise LoadPromise;
		auto LoadFuture = LoadPromise.GetFuture();
		ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
		auto Loaded = LoadFuture.Get();		// Synchronously
		CHECK(Loaded);
		if (UNLIKELY(!Loaded))
		{
			return;
		}

		CHECK(MediaManager.IsMediaLoaded(3));

		// Unloading Node
		FWwiseResourceUnloadPromise UnloadPromise;
		auto UnloadFuture = UnloadPromise.GetFuture();
		ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(Loaded));
		UnloadFuture.Get();		// Synchronously

		// Unloading Node
		FWwiseResourceUnloadPromise UnloadPromise2;
		auto UnloadFuture2 = UnloadPromise2.GetFuture();
		ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise2), MoveTemp(Loaded2));
		UnloadFuture2.Get();		// Synchronously

		CHECK(!MediaManager.IsMediaLoaded(3));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("Required Switch Container Not Loaded")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 3;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		check(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}
		
		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("Loading Event Before Switch Container")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading GroupValue
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("Loading Event After Switch Container")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};

		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		// Unloading GroupValue
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("Sync 2 SwitchContainer with same ID")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 1;
		GroupValue2.GroupId = 1;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue2{nullptr};

		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		// Load GroupValue2
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue2);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue2);
			if (UNLIKELY(!LoadedGroupValue2))
			{
				break;
			}
		}
		while(false);

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading GroupValue
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(MediaManager.IsMediaLoaded(1));

		if (LIKELY(LoadedGroupValue2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires first SwitchContainer)")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue2{nullptr};

		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		// Load GroupValue2
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue2);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue2);
			if (UNLIKELY(!LoadedGroupValue2))
			{
				break;
			}
		}
		while(false);

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading GroupValue
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		if (LIKELY(LoadedGroupValue2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires second SwitchContainer)")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue2{nullptr};

		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		// Load GroupValue2
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue2);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue2);
			if (UNLIKELY(!LoadedGroupValue2))
			{
				break;
			}
		}
		while(false);

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue2);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading GroupValue
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(MediaManager.IsMediaLoaded(1));

		if (LIKELY(LoadedGroupValue2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires both SwitchContainer)")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue2{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue2);
				Leaf1.GroupValueSet.Add(GroupValue);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		CHECK(!MediaManager.IsMediaLoaded(1));
		
		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load GroupValue2
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue2);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue2);
			if (UNLIKELY(!LoadedGroupValue2))
			{
				break;
			}
		}
		while(false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading GroupValue
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading GroupValue2
		if (LIKELY(LoadedGroupValue2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires both SwitchContainer). Switch Container 1 is Required key")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue2{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue2);
				Leaf1.GroupValueSet.Add(GroupValue);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));

				Data1.RequiredGroupValueSet.Add(GroupValue);
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);
		
		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load GroupValue2
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue2);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue2);
			if (UNLIKELY(!LoadedGroupValue2))
			{
				break;
			}
		}
		while(false);

		CHECK(MediaManager.IsMediaLoaded(1));

		//Unload GroupValue2
		if (LIKELY(LoadedGroupValue2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires both SwitchContainer). Switch Container 1 & 2 are Required key")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Leaf1.GroupValueSet.Add(GroupValue2);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));

				Data1.RequiredGroupValueSet.Add(GroupValue);
				Data1.RequiredGroupValueSet.Add(GroupValue2);
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);
		
		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires both SwitchContainer). Switch Container 1 Loaded Manually. Switch Container 2 is a Required Key on another Event.")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedEventPtr LoadedEvent2{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Leaf1.GroupValueSet.Add(GroupValue2);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));

				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);

		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		CHECK(!MediaManager.IsMediaLoaded(1));
 
		// Load Event 2 with Required Key
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;

				Data1.RequiredGroupValueSet.Add(GroupValue2);

				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent2);
			if (UNLIKELY(!LoadedEvent2))
			{
				break;
			}
		}
		while (false);
		
		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading Event 2
		if (LIKELY(LoadedEvent2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		//Unloading group value
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		
		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires both SwitchContainer). Switch Container 1 is a Required key on Event with the Media. Switch Container 2 is a Required Key on another Event.")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedEventPtr LoadedEvent2{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Leaf1.GroupValueSet.Add(GroupValue2);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));

				Data1.RequiredGroupValueSet.Add(GroupValue);
				
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);
		
		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load Event 2
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.RequiredGroupValueSet.Add(GroupValue2);
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent2);
			if (UNLIKELY(!LoadedEvent2))
			{
				break;
			}
		}
		while (false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event 2
		if (LIKELY(LoadedEvent2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("2 SwitchContainer with different ID (media requires both SwitchContainer). Switch Container 1 & 2 are a Required Key on another Event.")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedEventPtr LoadedEvent2{nullptr};
		FWwiseLoadedEventPtr LoadedEvent3{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Leaf1.GroupValueSet.Add(GroupValue2);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));

				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);
		
		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load Event 2
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.RequiredGroupValueSet.Add(GroupValue);
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent2);
			if (UNLIKELY(!LoadedEvent2))
			{
				break;
			}
		}
		while (false);

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load Event 3
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.RequiredGroupValueSet.Add(GroupValue2);
				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent3 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent3);
			if (UNLIKELY(!LoadedEvent3))
			{
				break;
			}
		}
		while (false);
		
		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading Event 3
		if (LIKELY(LoadedEvent3))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent3));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event 2
		if (LIKELY(2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent2));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}

	SECTION("3 SwitchContainer with different ID (media requires all 3 SwitchContainer).")
	{
		FWwiseMockExternalSourceManager ExternalSourceManager;
		FWwiseMockMediaManager MediaManager;
		FWwiseMockSoundBankManager SoundBankManager;
		FWwiseResourceLoaderImpl ResourceLoaderImpl(ExternalSourceManager, MediaManager, SoundBankManager);

		FWwiseGroupValueCookedData GroupValue;
		GroupValue.Id = 1;
		GroupValue.GroupId = 1;
		GroupValue.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue2;
		GroupValue2.Id = 2;
		GroupValue2.GroupId = 2;
		GroupValue2.Type = EWwiseGroupType::Switch;

		FWwiseGroupValueCookedData GroupValue3;
		GroupValue3.Id = 3;
		GroupValue3.GroupId = 3;
		GroupValue3.Type = EWwiseGroupType::Switch;

		FWwiseLoadedEventPtr LoadedEvent{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue2{nullptr};
		FWwiseLoadedGroupValuePtr LoadedGroupValue3{nullptr};

		// Load Event
		do
		{
			FWwiseLocalizedEventCookedData CookedData;
			{
				FWwiseEventCookedData Data1;
				Data1.Media.Emplace(FWwiseMediaCookedData{});
				Data1.SoundBanks.Emplace(FWwiseSoundBankCookedData{});

				FWwiseSwitchContainerLeafCookedData Leaf1;
				FWwiseMediaCookedData Leaf1Media;
				Leaf1Media.MediaId = 1;
				Leaf1.Media.Emplace(MoveTemp(Leaf1Media));
				Leaf1.GroupValueSet.Emplace(GroupValue);
				Leaf1.GroupValueSet.Add(GroupValue2);
				Leaf1.GroupValueSet.Add(GroupValue3);
				Data1.SwitchContainerLeaves.Emplace(MoveTemp(Leaf1));

				CookedData.EventLanguageMap.Emplace(FWwiseLanguageCookedData::Sfx, MoveTemp(Data1));
			}

			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateEventNode(CookedData, nullptr);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedEventPromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadEventAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedEvent = LoadFuture.Get();		// Synchronously
			CHECK(LoadedEvent);
			if (UNLIKELY(!LoadedEvent))
			{
				break;
			}
		}
		while (false);
		
		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load GroupValue
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue);
			if (UNLIKELY(!LoadedGroupValue))
			{
				break;
			}
		}
		while(false);

		
		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load GroupValue 2
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue2);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue2 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue2);
			if (UNLIKELY(!LoadedGroupValue2))
			{
				break;
			}
		}
		while(false);

		CHECK(!MediaManager.IsMediaLoaded(1));

		// Load GroupValue 3
		do
		{
			// Creating Node
			auto* Node = ResourceLoaderImpl.CreateGroupValueNode(GroupValue3);

			CHECK(Node);
			if (UNLIKELY(!Node))
			{
				break;
			}

			// Loading Node
			FWwiseLoadedGroupValuePromise LoadPromise;
			auto LoadFuture = LoadPromise.GetFuture();
			ResourceLoaderImpl.LoadGroupValueAsync(MoveTemp(LoadPromise), MoveTemp(Node));
			LoadedGroupValue3 = LoadFuture.Get();		// Synchronously
			CHECK(LoadedGroupValue3);
			if (UNLIKELY(!LoadedGroupValue3))
			{
				break;
			}
		}
		while(false);

		CHECK(MediaManager.IsMediaLoaded(1));

		// Unloading GroupValue3
		if (LIKELY(LoadedGroupValue3))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue3));
			UnloadFuture.Get();		// Synchronously
		}

		// Unloading GroupValue2
		if (LIKELY(LoadedGroupValue2))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue2));
			UnloadFuture.Get();		// Synchronously
		}

		// Unloading GroupValue1
		if (LIKELY(LoadedGroupValue))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadGroupValueAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedGroupValue));
			UnloadFuture.Get();		// Synchronously
		}

		// Unloading Event
		if (LIKELY(LoadedEvent))
		{
			FWwiseResourceUnloadPromise UnloadPromise;
			auto UnloadFuture = UnloadPromise.GetFuture();
			ResourceLoaderImpl.UnloadEventAsync(MoveTemp(UnloadPromise), MoveTemp(LoadedEvent));
			UnloadFuture.Get();		// Synchronously
		}

		CHECK(!MediaManager.IsMediaLoaded(1));

		CHECK(ExternalSourceManager.IsEmpty());
		CHECK(MediaManager.IsEmpty());
		CHECK(SoundBankManager.IsEmpty());
		CHECK(ResourceLoaderImpl.IsEmpty());
	}
}

#endif // WWISE_UNIT_TESTS