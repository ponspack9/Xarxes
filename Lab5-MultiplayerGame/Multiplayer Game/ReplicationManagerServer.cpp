#include "Networks.h"
#include "ReplicationManagerServer.h"
#include "ModuleLinkingContext.h"

// TODO(Oscar): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	uint16 arrayIndex = networkId & 0xffff;
	ASSERT(arrayIndex < MAX_NETWORK_OBJECTS);
	savedActions[arrayIndex].networkId = networkId;
	savedActions[arrayIndex].action = ReplicationAction::Create;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	uint16 arrayIndex = networkId & 0xffff;
	ASSERT(arrayIndex < MAX_NETWORK_OBJECTS);
	savedActions[arrayIndex].networkId = networkId;
	savedActions[arrayIndex].action = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	uint16 arrayIndex = networkId & 0xffff;
	ASSERT(arrayIndex < MAX_NETWORK_OBJECTS);
	savedActions[arrayIndex].networkId = networkId;
	savedActions[arrayIndex].action = ReplicationAction::Destroy;
}

// Prepares the packet to be send with all the actions needed
// Does NOT send the packet
void ReplicationManagerServer::write(OutputMemoryStream& packet)
{
	// Maybe to be optimized :
	//		- put a flag to send packets only if changes are made
	//		- traverse the array til ModuleLinkingContext.networkGameObjectsCount;
	//			App->modLinkingContext->getNetworkGameObjectsCount()
	packet << REPLICATION_ID;
	for (int i = 0; i < MAX_NETWORK_OBJECTS; i++)
	{
		// Replication Action
		ReplicationAction action = savedActions[i].action;
		if (action == ReplicationAction::None)
			continue;

		// Network ID
		packet << savedActions[i].networkId;

		// Serializing the action
		packet << action;

		if (action == ReplicationAction::Destroy)
		{
			// Reseting the state
			savedActions[i].action = ReplicationAction::None;
			savedActions[i].networkId = 0;
			continue;
		}

		// Serializing GameObject info
		GameObject* obj = App->modLinkingContext->getNetworkGameObject(savedActions[i].networkId);
		if (obj != nullptr)
			obj->Serialize(packet);
		else
			ELOG("Error getting network gameobject");

		// Reseting the state
		savedActions[i].action = ReplicationAction::None;
		savedActions[i].networkId = 0;
	}
}
