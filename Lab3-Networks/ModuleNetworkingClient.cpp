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
		// --- CLIENT WINDOW ---
		ImGui::Begin("Client Window");

		Texture* tex = App->modResources->client;
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

		// Tab Bar
		if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_FittingPolicyScroll))
		{
			flags = ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;

			// General
			if (new_message == true)
				ImGui::PushStyleColor(ImGuiCol_Text, yellow);
			if (ImGui::BeginTabItem("General"))
			{
				current_tab = "General";

				if (new_message == true)
					ImGui::PopStyleColor();
				new_message = false;

				ImGui::BeginChild("##Chat", ImVec2(0, ImGui::GetContentRegionAvail().y - 30), true);
				for (int i = 0; i < messages_list.size(); ++i)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, messages_list[i].second);
					ImGui::TextWrapped(messages_list[i].first.c_str());
					ImGui::PopStyleColor();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			else if (new_message == true)
				ImGui::PopStyleColor();

			// Whispers
			for (int i = 0; i < whispers_list.size(); ++i)
			{
				if (whispers_list[i].new_message == true)
					ImGui::PushStyleColor(ImGuiCol_Text, yellow);

				if (whispers_list[i].selected)
					flags |= ImGuiTabItemFlags_SetSelected;

				if (ImGui::BeginTabItem(whispers_list[i].username.c_str(), &whispers_list[i].opened, flags))
				{
					whispers_list[i].selected = false;
					current_tab = whispers_list[i].username;

					if (whispers_list[i].new_message == true)
						ImGui::PopStyleColor();
					whispers_list[i].new_message = false;

					ImGui::BeginChild(std::string("##" + whispers_list[i].username).c_str(), ImVec2(0, ImGui::GetContentRegionAvail().y - 30), true);
					for (int j = 0; j < whispers_list[i].messages.size(); ++j)
					{
						ImGui::TextWrapped(whispers_list[i].messages[j].c_str());
					}
					ImGui::EndChild();
					ImGui::EndTabItem();
				}
				else if (whispers_list[i].new_message == true)
					ImGui::PopStyleColor();
			}
			ImGui::EndTabBar();
		}

		char buffer[128] = "";
		bool is_command = false;
		if (ImGui::InputText("##InputText", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
		{
			OutputMemoryStream packet;
			std::string message = buffer;
			if (current_tab == "General")
			{
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
					else if (command == "listwindow")
					{
						packet << ClientMessage::ListWindow;
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
						message = message.substr(message.find_first_of(" ") + 1, message.back());
						std::string user = message;
						message = message.substr(message.find_first_of(" ") + 1, message.back());
						packet << ClientMessage::Whisper;
						packet << user;
						packet << message;
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
			}
			else
			{
				packet << ClientMessage::Whisper;
				packet << current_tab;
				packet << buffer;
			}
			sendPacket(packet, clientSocket);
		}
		ImGui::End();

		// --- USERS LIST WINDOW ---
		if (list_window)
		{
			ImGui::Begin("Connected Users", &list_window);
			static std::string selectedName = "";
			for (auto& user : users_list)
			{
				if (user == playerName)
					continue;

				ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
				ImGui::Selectable(user.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 30));
				ImGui::PopStyleVar();

				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsMouseDoubleClicked(0))
					{
						for (auto& whisper : whispers_list)
						{
							if (whisper.username == user)
							{
								current_tab = user;
								whisper.selected = true;
								if (!whisper.opened)
									whisper.opened = true;
								break;
							}
						}
					}
				}
				if (ImGui::OpenPopupOnItemClick("Options Menu"))
					selectedName = user;
				ImGui::Separator();
			}

			if (ImGui::BeginPopup("Options Menu"))
			{
				OutputMemoryStream packet;
				std::string message = "";

				if (ImGui::MenuItem("Mute"))
				{
					packet << ClientMessage::Mute;
					packet << selectedName;
					sendPacket(packet, clientSocket);
				}
				if (ImGui::MenuItem("Unmute"))
				{
					packet << ClientMessage::Unmute;
					packet << selectedName;
					sendPacket(packet, clientSocket);
				}
				if (ImGui::MenuItem("Block"))
				{
					packet << ClientMessage::Block;
					packet << selectedName;
					sendPacket(packet, clientSocket);
				}
				if (ImGui::MenuItem("Unblock"))
				{
					packet << ClientMessage::Unblock;
					packet << selectedName;
					sendPacket(packet, clientSocket);
				}
				if (ImGui::MenuItem("Kick"))
				{
					packet << ClientMessage::Kick;
					packet << selectedName;
					sendPacket(packet, clientSocket);
				}
				if (ImGui::MenuItem("Ban"))
				{
					packet << ClientMessage::Ban;
					packet << selectedName;
					sendPacket(packet, clientSocket);
				}
				ImGui::Separator();

				ImGui::Text("Whisper\n");
				char buffer[128] = "";
				if (ImGui::InputText("##InputText2", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
				{
					packet << ClientMessage::Whisper;
					packet << selectedName;
					packet << buffer;
					sendPacket(packet, clientSocket);
				}
				ImGui::EndPopup();
			}

			ImGui::End();
		}
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
		new_message = true;
		messages_list.push_back({ message, red });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Message) // Message
	{
		messages_list.push_back({ message, white });
		new_message = true;
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
		new_message = true;

		if (list_window)
		{
			std::string name = message.substr(3, message.find_first_of(" ") - 3);
			if (serverMessage == ServerMessage::UserConnected)
			{
				users_list.push_back(name);
			}
			else
			{
				for (int i = 0; i < users_list.size(); ++i)
				{
					if (users_list[i] == name)
					{
						users_list.erase(users_list.begin() + i);
						break;
					}
				}
			}
		}
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Help || 
		serverMessage == ServerMessage::List ||	serverMessage == ServerMessage::MuteList || 
		serverMessage == ServerMessage::BlockList || serverMessage == ServerMessage::BanList) // Help & Lists
	{
		messages_list.push_back({ message, yellow });
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::ListWindow) // List Window
	{
		int num;
		packet >> num;

		users_list.clear();
		for (int i = 0; i < num; ++i)
		{
			std::string user = message.substr(0, message.find_first_of("\n"));
			message = message.substr(message.find_first_of("\n") + 1, message.find_last_of("\n"));
			users_list.push_back(user);
		}

		list_window = true;
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::ChangeName) // ChangeName
	{
		for (auto& name : users_list)
		{
			if (name == playerName)
			{
				name = message;
				break;
			}
		}
		playerName = message;
		packet >> message;
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
		new_message = true;
		state = ClientState::Connected;
	}
	else if (serverMessage == ServerMessage::Whisper) // Whisper
	{
		std::string user;
		packet >> user;

		bool exists = false;
		if (!whispers_list.empty())
		{
			for (auto& whisper : whispers_list)
			{
				if (whisper.username == user)
				{
					exists = true;
					whisper.opened = true;
					whisper.messages.push_back(message);
					whisper.new_message = true;
					break;
				}
			}
		}
		if (!exists)
		{
			Whisper whisper;
			whisper.new_message = true;
			whisper.opened = true;
			whisper.username = user;
			whisper.messages.push_back(message);
			whispers_list.push_back(whisper);
		}
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


