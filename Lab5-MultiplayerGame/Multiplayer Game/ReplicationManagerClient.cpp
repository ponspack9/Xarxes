#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(Oscar): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	// At this point the packet has been read the REPLICATION_ID
	// Next comes for all the gameobjects afected (loop):
	// NetworkID
	// Action
	// Serialized GameObject
	while (packet.RemainingByteCount() > 0)
	{
		// Read network object 
		uint32 networkId;
		packet >> networkId;

		// Read the action to be performed
		ReplicationAction action;
		packet >> action;

		if (action == ReplicationAction::Destroy)
		{
			GameObject* obj = App->modLinkingContext->getNetworkGameObject(networkId);
			if (obj != nullptr)
				App->modLinkingContext->unregisterNetworkGameObject(obj);
			Destroy(obj);
		}
		else if (action == ReplicationAction::Create)
		{
			GameObject* obj = Instantiate();
			App->modLinkingContext->registerNetworkGameObject(obj);
			obj->Deserialize(packet);
		}
		else if (action == ReplicationAction::Update)
		{
			GameObject* obj = App->modLinkingContext->getNetworkGameObject(networkId);
			if (obj != nullptr)
				obj->Deserialize(packet);
		}
	}
}
