#include "ModuleNetworkingClient.h"

// OSCAR PONS GALLART & DAVID TELLO PANEA

bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
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
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());

		if (ImGui::Button("Log out"))
		{
			DLOG("Logging out");
			shutdown(clientSocket, SD_SEND);
		}
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	if (serverMessage == ServerMessage::Welcome)
	{
		std::string message;
		packet >> message;

		DLOG(message.c_str());
		state = ClientState::Connected;
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	closesocket(socket);

	for (int i = 0; i < sockets.size(); i++)
	{
		if ( sockets[i] == socket)
			sockets.erase(sockets.begin() + i);
	}
	state = ClientState::Stopped;
	DLOG("Server disconnected");
}


