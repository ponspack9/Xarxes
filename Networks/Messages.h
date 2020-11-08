#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Message,
	Help,
	List,
	ListWindow,
	MuteList,
	BlockList,
	BanList,
	ChangeName,
	Whisper,
	Block,
	Unblock,
	Kick,
	Ban,
	Unban,
	Mute,
	Unmute,
	MuteAll,
	UnmuteAll,
	ErrorCommand
};

enum class ServerMessage
{
	Welcome,
	UnableBan,
	UnableName,
	UserBanned,
	UserKicked,
	UserConnected,
	UserDisconnected,
	Message,
	Help,
	List,
	ListWindow,
	MuteList,
	BlockList,
	BanList,
	ChangeName,
	Whisper,
	Block,
	Unblock,
	Kick,
	Ban,
	Unban,
	Mute,
	Unmute,
	MuteAll,
	UnmuteAll,
	ErrorCommand
};

