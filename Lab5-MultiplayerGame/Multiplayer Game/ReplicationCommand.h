#pragma once

// TODO(Oscar): World state replication 
// Maybe there's no need to keep this struct and can
// be deleted if used static size arrays using only the action

enum class ReplicationAction
{
	None,
	Create,
	Update,
	Destroy
};

struct ReplicationCommand
{
	uint32 networkId;
	ReplicationAction action;

	ReplicationCommand() {
		networkId = 0;
		action = ReplicationAction::None;
	}
};