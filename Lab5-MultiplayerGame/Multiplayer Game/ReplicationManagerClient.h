#pragma once

// TODO(Oscar): World state replication 
class ReplicationManagerClient
{
public:

	void read(const InputMemoryStream& packet);
};