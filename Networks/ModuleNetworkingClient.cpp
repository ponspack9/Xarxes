#include "ModuleNetworkingClient.h"

// OSCAR PONS GALLART & DAVID TELLO PANEA

bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	color = white;
	playerName = pplayerName;
	DLOG("Starting client...");

	// - Create the socket
	DLOG("Creating client socket");
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		reportError("Error creating Client Socket");
		return false;
	}

	// - Create the remote address object
	sockaddr_in serverAddr;
	memset((char*)&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr);

	// - Connect to the remote address
	DLOG("Connecting to the server");
	if (connect(clientSocket, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		reportError("Error connecting to server");
		return false;
	}
	DLOG("Connection established");

	// - Add the created socket to the managed list of sockets using addSocket()
	addSocket(clientSocket);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, clientSocket))
		{
			DLOG("Sent '%s' to the server", playerName.c_str());
			state = ClientState::Logging;
		}
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}
	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("Hello %s! Welcome to the chat.", playerName.c_str());
		ImGui::SameLine();

		ImGui::Button("Logout");

		if (ImGui::IsItemClicked())
		{
			DLOG("Logging out");
			disconnect();
			state = ClientState::Stopped;
		}

		ImGui::BeginChild("##Chat", ImVec2(0, ImGui::GetContentRegionAvail().y - 30), true);
		{
			for (int i = 0; i < messages_list.size(); ++i)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, messages_list[i].second);
				ImGui::TextWrapped(messages_list[i].first.c_str());
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndChild();

		char buffer[128] = "";
		bool is_command = false;
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##InputText", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
		{
			OutputMemoryStream packet;
			std::string message = buffer;
			if (message.substr(0, 1) != "/")
			{
				message = playerName + ": " + buffer;
				packet << ClientMessage::Message;
				packet << message;
			}
			else
			{
				is_command = true;

				std::string command = message.substr(1, message.find_first_of(" ") - 1);
				if (command == "help")
				{
					packet << ClientMessage::Help;
				}
				else if (command == "list")
				{
					packet << ClientMessage::List;
				}
				else if (command == "blocklist")
				{
					packet << ClientMessage::BlockList;
				}
				else if (command == "mutelist")
				{
					packet << ClientMessage::MuteList;
				}
				else if (command == "changename")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::ChangeName;
					packet << message;
				}
				else if (command == "whisper")
				{
					packet << ClientMessage::Whisper;
					//***
				}
				else if (command == "block")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::Block;
					packet << message;
				}
				else if (command == "unblock")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::Unblock;
					packet << message;
				}
				else if (command == "kick")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::Kick;
					packet << message;
				}
				else if (command == "ban")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::Ban;
					packet << message;
				}
				else if (command == "unban")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::Unban;
					packet << message;
				}
				else if (command == "mute")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::Mute;
					packet << message;
				}
				else if (command == "unmute")
				{
					message = message.substr(message.find_first_of(" ") + 1, message.back());
					packet << ClientMessage::Unmute;
					packet << message;
				}
				else if (command == "muteall")
				{
					packet << ClientMessage::MuteAll;
				}
				else if (command == "unmuteall")
				{
					packet << ClientMessage::UnmuteAll;
				}
				else
				{
					message = "Command '" + message + "' doesn't exist. Type /help to see the available commands.";
					packet << ClientMessage::ErrorCommand;
					packet << message;
				}
			}
			sendPacket(packet, clientSocket);
		}
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	std::string message;
	packet >> message;

	if (serverMessage == ServerMessage::ErrorCommand) // Error
	{
		messages_list.push_back({ message, red });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Message) // Message
	{
		messages_list.push_back({ message, white });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Welcome) // Welcome
	{
		messages_list.push_back({ message, yellow });
		App->modScreen->screenMainMenu->name_used = false;
		App->modScreen->screenMainMenu->name_banned = false;
		App->modScreen->screenMainMenu->kicked = false;
		App->modScreen->screenMainMenu->banned = false;
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::UnableName) // Unable to connect (name already in use)
	{
		App->modScreen->screenMainMenu->name_used = true;
		disconnect();
		state = ClientState::Stopped;
	}
	else if (serverMessage == ServerMessage::UnableBan) // Unable to connect (name is banned)
	{
		App->modScreen->screenMainMenu->name_banned = true;
		disconnect();
		state = ClientState::Stopped;
	}
	else if (serverMessage == ServerMessage::UserKicked) // Kicked
	{
		App->modScreen->screenMainMenu->kicked = true;
		disconnect();
		state = ClientState::Stopped;
	}
	else if (serverMessage == ServerMessage::UserBanned) // Banned
	{
		App->modScreen->screenMainMenu->banned = true;
		disconnect();
		state = ClientState::Stopped;
	}
	else if (serverMessage == ServerMessage::UserConnected || serverMessage == ServerMessage::UserDisconnected) // User Connected/Disconnected
	{
		messages_list.push_back({ message, green });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Help || 
		serverMessage == ServerMessage::List ||	serverMessage == ServerMessage::MuteList || 
		serverMessage == ServerMessage::BlockList || serverMessage == ServerMessage::BanList) // Help & Lists
	{
		messages_list.push_back({ message, yellow });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::ChangeName) // ChangeName
	{
		playerName = message;
		messages_list.push_back({ message, blue });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Block || serverMessage == ServerMessage::Unblock || serverMessage == ServerMessage::Mute ||
		serverMessage == ServerMessage::Unmute || serverMessage == ServerMessage::MuteAll || serverMessage == ServerMessage::UnmuteAll) // Block/UnBlock, Mute/Unmute, MuteAll/UnMuteAll
	{
		messages_list.push_back({ message, blue });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Kick || serverMessage == ServerMessage::Ban || serverMessage == ServerMessage::Unban) // Kick, Ban/UnBan
	{
		messages_list.push_back({ message, red });
		state = ClientState::Connected;
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	closesocket(socket);

	for (unsigned int i = 0; i < sockets.size(); i++)
	{
		if ( sockets[i] == socket)
			sockets.erase(sockets.begin() + i);
	}
	state = ClientState::Stopped;
	DLOG("Server disconnected");
}


