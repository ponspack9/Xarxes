#pragma once

// TODO(Oscar): World state replication 
class ReplicationManagerClient
{
public:

	void read(const InputMemoryStream& packet);
	void DeserializeUpdate(const InputMemoryStream& packet, GameObject* gameObject);
	void DeserializeCreate(const InputMemoryStream& packet, GameObject* gameObject);
};