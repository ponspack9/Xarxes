#include "ModuleNetworkingServer.h"


// OSCAR PONS GALLART & DAVID TELLO PANEA


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	DLOG("Starting server...");
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	DLOG("Creating listening socket");
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		reportError("Error creating appSocket");
		return false;
	}
	// - Set address reuse
	DLOG("Setting reusable address");
	int enable = 1;
	int res = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (res == SOCKET_ERROR)
	{
		reportError("Error forcing reuse of address and port");
		return false;
	}

	// - Bind the socket to a local interface
	DLOG("Binding socket");
	sockaddr_in sourceAddr;
	sourceAddr.sin_family = AF_INET; // IPv4 (AF_INET6 -> IPv6)
	sourceAddr.sin_port = htons(port);
	sourceAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	res = bind(listenSocket, (const struct sockaddr*)&sourceAddr, sizeof(sourceAddr));
	if (res == SOCKET_ERROR)
	{
		reportError("Error binding port");
		return false;
	}

	// - Enter in listen mode
	DLOG("Entering listening mode");
	res = listen(listenSocket, 1);

	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);

	DLOG("Server started successfully");
	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("User name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream &packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	OutputMemoryStream new_packet;
	std::string message = "";

	if (clientMessage == ClientMessage::Hello) // Hello
	{
		std::string playerName;
		packet >> playerName;

		bool name_used = false;
		bool name_banned = false;

		for (auto& banned : banList)
		{
			if (banned == playerName)
			{
				name_banned = true;
				break;
			}
		}
		if (name_banned) //name is banned
		{
			new_packet << ServerMessage::UnableBan;
			new_packet << message;

			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.socket == socket)
				{
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
		else
		{
			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.playerName == playerName)
				{
					name_used = true;
					break;
				}
			}

			if (name_used) //name is used
			{
				new_packet << ServerMessage::UnableName;
				new_packet << message;

				for (auto& connectedSocket : connectedSockets)
				{
					if (connectedSocket.socket == socket)
					{
						sendPacket(new_packet, connectedSocket.socket);
						break;
					}
				}
			}
			else
			{
				for (auto& connectedSocket : connectedSockets)
				{
					if (connectedSocket.socket == socket)
					{
						message = "***************************************************\n"
							"               WELCOME TO THE CHAT!\n"
							"  Please type /help to see the available commands\n"
							"***************************************************\n\n";
						new_packet.Clear();
						new_packet << ServerMessage::Welcome;
						new_packet << message;

						DLOG("User '%s' logged in", playerName.c_str());
						connectedSocket.playerName = playerName;
						sendPacket(new_packet, connectedSocket.socket);
					}
					else
					{
						message = "***" + playerName + " joined the chat***";
						new_packet.Clear();
						new_packet << ServerMessage::UserConnected;
						new_packet << message;

						sendPacket(new_packet, connectedSocket.socket);
					}
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::ErrorCommand) // Error Command
	{
		packet >> message;
		new_packet << ServerMessage::ErrorCommand;
		new_packet << message;
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::Message) // Message
	{
		packet >> message;
		new_packet << ServerMessage::Message;
		new_packet << message;

		std::string playerName = message.substr(0, message.find_first_of(":"));
		for (auto& connectedSocket : connectedSockets)
		{
			bool is_blocked = false;
			for (auto& blocked : connectedSocket.blockList) //blocked
			{
				if (blocked == playerName)
				{
					is_blocked = true;
					break;
				}
			}
			if (!is_blocked)
			{
				for (auto& blocked : connectedSocket.muteList) //muted
				{
					if (blocked == playerName)
					{
						is_blocked = true;
						break;
					}
				}
			}

			if (!is_blocked)
				sendPacket(new_packet, connectedSocket.socket);
		}
	}
	else if (clientMessage == ClientMessage::Help) // Help
	{
		message = "*** List of Commands ***\n"
			"/help\n"
			"/list\n"
			"/mutelist\n"
			"/blocklist\n"
			"/banlist\n"
			"/changename 'new_name'\n"
			"/whisper 'username' 'message'\n"
			"/block 'username'\n"
			"/unblock 'username'\n"
			"/kick 'username'\n"
			"/ban 'username'\n"
			"/unban 'username'\n"
			"/mute 'username'\n"
			"/unmute 'username'\n"
			"/muteall\n";
			"/unmuteall\n\n";

		new_packet << ServerMessage::Help;
		new_packet << message;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::List) // List
	{
		message = "*** List of Users ***\n";
		for (auto& connectedSocket : connectedSockets)
			message.append(connectedSocket.playerName + "\n");
		message.append("\n");

		new_packet << ServerMessage::List;
		new_packet << message;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::MuteList) // Mute List
	{
		message = "*** List of Muted Users ***\n";
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				for (auto& user : connectedSocket.muteList)
					message.append(user + "\n");
				message.append("\n");
				break;
			}
		}

		new_packet << ServerMessage::MuteList;
		new_packet << message;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::BlockList) // Block List
	{
		message = "*** List of Blocked Users ***\n";
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				for (auto& user : connectedSocket.blockList)
					message.append(user + "\n");
				message.append("\n");
				break;
			}
		}

		new_packet << ServerMessage::BlockList;
		new_packet << message;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::BanList) // Ban List
	{
		message = "*** List of Banned Users ***\n";
		for (auto& user : banList)
			message.append(user + "\n");
		message.append("\n");

		new_packet << ServerMessage::BanList;
		new_packet << message;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::ChangeName) // Change Name
	{
		bool exists = false;
		bool banned = false;
		std::string playerName;
		packet >> playerName;

		// check if name is banned
		for (auto& connectedSocket : connectedSockets)
		{
			for (auto& user : banList)
			{
				if (connectedSocket.playerName == user)
				{
					banned = true;
					message = "Name is banned";
					new_packet << ServerMessage::ErrorCommand;
					break;
				}
			}
		}

		if (!banned)
		{
			// check if name already exists
			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.playerName == playerName)
				{
					exists = true;
					message = "Name is already in use";
					new_packet << ServerMessage::ErrorCommand;
					break;
				}
			}
			if (!exists)
			{
				message = "*** Name changed to '" + playerName + "' ***";
				new_packet << ServerMessage::ChangeName;
			}
		}

		new_packet << message;
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				sendPacket(new_packet, connectedSocket.socket);

				if (!exists && !banned)
					connectedSocket.playerName = playerName;
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::Whisper) // Whisper
	{
	}
	else if (clientMessage == ClientMessage::Block) // Block
	{
		std::string playerName;
		packet >> playerName;

		bool error = true;
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				error = false;
				break;
			}
		}

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				if (error)
				{
					message = "'" + playerName + "'" + " is not connected";
					new_packet << ServerMessage::ErrorCommand;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
				else
				{
					bool exists = false;
					for (auto& blocked : connectedSocket.blockList)
					{
						if (blocked == playerName)
						{
							exists = true;
							message = playerName + " is already blocked";
							new_packet << ServerMessage::ErrorCommand;
							break;
						}
					}
					if (!exists)
					{
						connectedSocket.blockList.push_back(playerName);
						message = "You blocked " + playerName;
						new_packet << ServerMessage::Block;
					}
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::Unblock) // UnBlock
	{
		std::string playerName;
		packet >> playerName;

		bool error = true;
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				error = false;
				break;
			}
		}

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				if (error)
				{
					message = "'" + playerName + "'" + " is not connected";
					new_packet << ServerMessage::ErrorCommand;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
				else
				{
					bool exists = false;
					for (unsigned int i = 0; i < connectedSocket.blockList.size(); ++i)
					{
						if (connectedSocket.blockList[i] == playerName)
						{
							connectedSocket.blockList.erase(connectedSocket.blockList.begin() + i);
							exists = true;
							message = "You unblocked " + playerName;
							new_packet << ServerMessage::Unblock;
							break;
						}
					}
					if (!exists)
					{
						message = playerName + " is not blocked";
						new_packet << ServerMessage::ErrorCommand;
					}
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::Kick) // Kick
	{
		std::string playerName;
		packet >> playerName;

		bool exists = false;
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				exists = true;
				break;
			}
		}

		for (auto& connectedSocket : connectedSockets)
		{
			if (exists)
			{
				if (connectedSocket.playerName == playerName)
				{
					new_packet.Clear();
					new_packet << ServerMessage::UserKicked;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
				}
				else
				{
					new_packet.Clear();
					message = "***" + playerName + " was kicked from server***";
					new_packet << ServerMessage::Kick;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
				}
			}
			else
			{
				if (connectedSocket.socket == socket)
				{
					message = "'" + playerName + "'" + " is not connected";
					new_packet << ServerMessage::ErrorCommand;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::Ban) // Ban
	{
		std::string playerName;
		packet >> playerName;

		bool exists = false;
		for (auto& banned : banList)
		{
			if (banned == playerName)
			{
				exists = true;
				break;
			}
		}

		if (exists)
		{
			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.socket == socket)
				{
					message = playerName + " is already banned";
					new_packet << ServerMessage::ErrorCommand;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
		else
		{
			banList.push_back(playerName);
			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.playerName == playerName)
				{
					new_packet.Clear();
					new_packet << ServerMessage::UserBanned;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
				}
				else
				{
					new_packet.Clear();
					message = "***" + playerName + " was banned from server***";
					new_packet << ServerMessage::Ban;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::Unban) // UnBan
	{
		std::string playerName;
		packet >> playerName;

		bool exists = false;
		for (unsigned int i = 0; i < banList.size(); ++i)
		{
			if (banList[i] == playerName)
			{
				banList.erase(banList.begin() + i);
				exists = true;
				message = "***" + playerName + " was unbanned from server***";
				new_packet << ServerMessage::Unban;
				break;
			}
		}

		if (exists)
		{
			for (auto& connectedSocket : connectedSockets)
			{
				new_packet << message;
				sendPacket(new_packet, connectedSocket.socket);
			}
		}
		else
		{
			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.socket == socket)
				{
					message = playerName + " is not banned";
					new_packet << ServerMessage::ErrorCommand;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::Mute) // Mute
	{
		std::string playerName;
		packet >> playerName;

		bool error = true;
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				error = false;
				break;
			}
		}

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				if (error)
				{
					message = "'" + playerName + "'" + " is not connected";
					new_packet << ServerMessage::ErrorCommand;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
				else
				{
					bool exists = false;
					for (auto& blocked : connectedSocket.muteList)
					{
						if (blocked == playerName)
						{
							exists = true;
							break;
						}
					}
					if (exists)
					{
						message = playerName + " is already muted";
						new_packet << ServerMessage::ErrorCommand;
					}
					else
					{
						connectedSocket.muteList.push_back(playerName);
						message = "You muted " + playerName;
						new_packet << ServerMessage::Mute;
					}
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::Unmute) // UnMute
	{
		std::string playerName;
		packet >> playerName;

		bool error = true;
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				error = false;
				break;
			}
		}

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				if (error)
				{
					message = "'" + playerName + "'" + " is not connected";
					new_packet << ServerMessage::ErrorCommand;
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
				else
				{
					bool exists = false;
					for (unsigned int i = 0; i < connectedSocket.muteList.size(); ++i)
					{
						if (connectedSocket.muteList[i] == playerName)
						{
							connectedSocket.muteList.erase(connectedSocket.muteList.begin() + i);
							exists = true;
							break;
						}
					}
					if (exists)
					{
						message = "You unmuted " + playerName;
						new_packet << ServerMessage::Unmute;
					}
					else
					{
						message = playerName + " is not muted";
						new_packet << ServerMessage::ErrorCommand;
					}
					new_packet << message;
					sendPacket(new_packet, connectedSocket.socket);
					break;
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::MuteAll) // MuteAll
	{
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				connectedSocket.muteList.clear();
				for (auto& muted : connectedSockets)
				{
					if (muted.socket == socket)
						continue;
					else
						connectedSocket.muteList.push_back(muted.playerName);
				}

				message = "You muted all users";
				new_packet << ServerMessage::MuteAll;
				new_packet << message;
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::UnmuteAll) // UnmuteAll
	{
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				connectedSocket.muteList.clear();
				message = "You unmuted all users";
				new_packet << ServerMessage::UnmuteAll;
				new_packet << message;
				sendPacket(new_packet, connectedSocket.socket);
				break;
			}
		}
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	std::string message = "";
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto& connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			if (connectedSocket.playerName != "")
				message = "***" + connectedSocket.playerName + " left the chat***";
			DLOG("%s logged out", (*it).playerName.c_str());
			connectedSockets.erase(it);
			break;
		}
	}

	// Remove the socket from the main list
	for (auto it = sockets.begin(); it != sockets.end(); ++it)
	{
		auto& connectedSocket = *it;
		if (connectedSocket == socket)
		{
			sockets.erase(it);
			break;
		}
	}

	// Notify other users
	if (message != "")
	{
		OutputMemoryStream packet;
		for (auto& connectedSocket : connectedSockets)
		{
			packet << ServerMessage::UserDisconnected;
			packet << message;
			sendPacket(packet, connectedSocket.socket);
		}
	}
}