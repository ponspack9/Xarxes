#include "ModuleNetworkingServer.h"




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
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
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

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, byte * data)
{
	// Set the player name of the corresponding connected socket proxy
	for (auto &connectedSocket : connectedSockets)
	{
		if (connectedSocket.socket == socket)
		{
			connectedSocket.playerName = (const char *)data;
		}
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
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
}

