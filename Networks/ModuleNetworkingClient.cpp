#include "ModuleNetworkingClient.h"


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
		// TODO(jesus): Send the player name to the server
		int bytes = send(clientSocket, playerName.c_str(), playerName.length(), 0);

		if (bytes == SOCKET_ERROR)
		{
			reportError("Failed sending player name");
			state = ClientState::Stopped;
			return false;
		}
		DLOG("Sent '%s' to the server", playerName.c_str());
		//shutdown(clientSocket, SD_SEND);
		state = ClientState::Logging;
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

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, byte * data)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	DLOG("Server disconnected");
	closesocket(socket);

	for (int i = 0; i < sockets.size(); i++)
	{
		if ( sockets[i] == socket)
			sockets.erase(sockets.begin() + i);
	}
	state = ClientState::Stopped;
}


