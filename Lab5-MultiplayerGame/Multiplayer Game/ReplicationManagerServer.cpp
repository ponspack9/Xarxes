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
void ReplicationManagerServer::write(OutputMemoryStream& packet, Delivery* delivery)
{
	// Maybe to be optimized :
	//		- put a flag to send packets only if changes are made
	//		- traverse the array til ModuleLinkingContext.networkGameObjectsCount;
	//			App->modLinkingContext->getNetworkGameObjectsCount()
	

	for (int i = 0; i < MAX_NETWORK_OBJECTS; i++)
	{
		// Replication Action
		ReplicationAction action = savedActions[i].action;
		if (action == ReplicationAction::None)
			continue;

		// Network ID
		packet << savedActions[i].networkId;
		delivery->networkIds.push_back(savedActions[i].networkId);

		// Serializing the action
		packet << action;
		delivery->actions.push_back(action);

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
		{
			if (action == ReplicationAction::Create)
			{
				SerializeCreate(packet,obj);
			}
			else if (action == ReplicationAction::Update)
			{
				SerializeUpdate(packet,obj);
			}
		}
		else
			ELOG("Error getting network gameobject");

		// Reseting the state
		savedActions[i].action = ReplicationAction::None;
		savedActions[i].networkId = 0;
	}
}

void ReplicationManagerServer::SerializeUpdate(OutputMemoryStream& packet, GameObject* gameObject) const
{
	packet << gameObject->position.x;
	packet << gameObject->position.y;
	packet << gameObject->size.x;
	packet << gameObject->size.y;
	packet << gameObject->angle;
	packet << int(gameObject->state);
	
	if (gameObject->behaviour)
	{
		if (gameObject->behaviour->type() == BehaviourType::Spaceship)
		{
			Spaceship* spaceship = (Spaceship*)gameObject->behaviour;
			packet << spaceship->hitPoints;
		}
		else if (gameObject->behaviour->type() == BehaviourType::Laser)
		{
			Laser* laser = (Laser*)gameObject->behaviour;
			packet << laser->gameObject->id;
		}
	}
}

void ReplicationManagerServer::SerializeCreate(OutputMemoryStream& packet, GameObject* gameObject) const
{
	packet << gameObject->position.x;
	packet << gameObject->position.y;
	packet << gameObject->size.x;
	packet << gameObject->size.y;
	packet << gameObject->angle;

	std::string textureFileName = gameObject->sprite->texture->filename;
	packet << textureFileName;
	packet << gameObject->sprite->order;

	packet << int(gameObject->state);

	if (gameObject->behaviour)
		packet << gameObject->behaviour->type();
	else
		packet << BehaviourType::None;
}