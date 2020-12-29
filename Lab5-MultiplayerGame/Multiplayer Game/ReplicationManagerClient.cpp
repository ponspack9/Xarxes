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
			{
				App->modLinkingContext->unregisterNetworkGameObject(obj);
				Destroy(obj);
			}
			else
			{
				ELOG("Could not replicate action DESTROY for network object %d", networkId);
			}
		}
		else if (action == ReplicationAction::Create)
		{
			GameObject* obj = Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(obj,networkId);
			DeserializeCreate(packet,obj);
		}
		else if (action == ReplicationAction::Update)
		{
			GameObject* obj = App->modLinkingContext->getNetworkGameObject(networkId);
			if (obj != nullptr)
				DeserializeUpdate(packet,obj);
		}
	}
}

void ReplicationManagerClient::DeserializeUpdate(const InputMemoryStream& packet, GameObject* gameObject)
{
	packet >> gameObject->position.x;
	packet >> gameObject->position.y;
	packet >> gameObject->size.x;
	packet >> gameObject->size.y;
	packet >> gameObject->angle;
	packet >> gameObject->state;
}

void ReplicationManagerClient::DeserializeCreate(const InputMemoryStream& packet, GameObject* gameObject)
{
	packet >> gameObject->position.x;
	packet >> gameObject->position.y;
	packet >> gameObject->size.x;
	packet >> gameObject->size.y;
	packet >> gameObject->angle;

	// sprite, animation...
	std::string texture;
	packet >> texture;
	gameObject->sprite = App->modRender->addSprite(gameObject);
	packet >> gameObject->sprite->order;

	if (gameObject->sprite != nullptr)
	{
		if (texture == "spacecraft1.png")
			gameObject->sprite->texture = App->modResources->spacecraft1;
		else if (texture == "spacecraft2.png")
			gameObject->sprite->texture = App->modResources->spacecraft1;
		else if (texture == "spacecraft3.png")
			gameObject->sprite->texture = App->modResources->spacecraft1;
		else if (texture == "laser.png")
			gameObject->sprite->texture = App->modResources->laser;
		else if (texture == "explosion1.png")
		{
			gameObject->sprite->texture = App->modResources->explosion1;
			gameObject->animation->clip = App->modResources->explosionClip;
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
		}
	}

	packet >> gameObject->state;
}