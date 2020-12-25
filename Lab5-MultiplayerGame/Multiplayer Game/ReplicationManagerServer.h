#pragma once
// TODO(Oscar): World state replication lab session
#include "ReplicationCommand.h"


class ReplicationManagerServer
{
public:
	void create(uint32 networkId);
	void update(uint32 networkId);
	void destroy(uint32 networkId);

	// Generate a packet with the data of all the commands you have used
	void write(OutputMemoryStream& packet);

	// Saves all the commands used in a list/map...
	//std::vector<ReplicationCommand> savedCommands;
	ReplicationAction savedActions[MAX_NETWORK_OBJECTS] = { ReplicationAction::None};
};