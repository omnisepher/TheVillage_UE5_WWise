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
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class WaapiPlaybackTransport
{
	struct TransportInfo
	{
		int32 TransportID;
		uint64 SubscriptionID;

		TransportInfo(int32 transID, uint64 subsID) : TransportID(transID), SubscriptionID(subsID) {}
	};

public:
	~WaapiPlaybackTransport();

	int32 FindOrAdd(const FGuid& InItemID);
	void Remove(const FGuid& InItemID);
	void TogglePlay(int32 InTransportID);
	void Stop(int32 InTransportID);
	bool IsPlaying(const FGuid& InItemID);
	void StopAndDestroyAll();

private:
	uint64 SubscribeToStateChanged(int32 TransportId);

	void OnStateChanged(TSharedPtr<FJsonObject> InUEJsonObject);

	/** Remember the played items. Useful to play/stop and event. */
	TMap<FGuid, TransportInfo> TransportItems;
	FCriticalSection TransportItemsLock;

};
