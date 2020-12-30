#pragma once

struct Spaceship;
struct Laser;

// TODO(Oscar): World state replication 
class ReplicationManagerClient
{
public:

	void read(const InputMemoryStream& packet);
	void DeserializeUpdate(const InputMemoryStream& packet, GameObject* gameObject, Spaceship* spaceship = nullptr, Laser* laser = nullptr);
	void DeserializeCreate(const InputMemoryStream& packet, GameObject* gameObject);
};