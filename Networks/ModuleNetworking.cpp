#include "Networks.h"
#include "ModuleNetworking.h"

static uint8 NumModulesUsingWinsock = 0;

// OSCAR PONS GALLART & DAVID TELLO PANEA

void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}
	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}
	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) 
		return true;

	fd_set readfds;
	FD_ZERO(&readfds);
	for (auto s : sockets)
	{
		FD_SET(s, &readfds);
	}
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int res = select(0, &readfds, nullptr, nullptr, &timeout);
	if (res == SOCKET_ERROR)
	{
		reportError("Error in select socket");
		return false;
	}

	InputMemoryStream packet;
	for (auto s : sockets)
	{
		if (FD_ISSET(s, &readfds))
		{
			if (isListenSocket(s)) // Accept
			{
				sockaddr_in incomingAddr;
				int size = sizeof(incomingAddr);
				SOCKET acceptedSocket = accept(s, (struct sockaddr*)&incomingAddr, &size);
				onSocketConnected(acceptedSocket, incomingAddr);
				addSocket(acceptedSocket);
			}
			else
			{
				int bytes = recv(s, packet.GetBufferPtr(), packet.GetCapacity(), 0); // Receive
				if (bytes == SOCKET_ERROR)
				{
					onSocketDisconnected(s);
					closesocket(s);
					return true;
				}
				else if (bytes == 0 || bytes == ECONNRESET) // Disconected
				{
					if (strlen(packet.GetBufferPtr()) > 0) // FIN packet
					{
						onSocketDisconnected(s);
						shutdown(s, SD_BOTH);
					}
				}
				else
				{ 
					packet.SetSize((uint32)bytes);
					onSocketReceivedData(s, packet);
				}
			}
		}
	}
	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{
		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}
	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	if (send(socket, packet.GetBufferPtr(), packet.GetSize(), 0) == SOCKET_ERROR)
	{
		reportError("Error sending packet: ");
		return false;
	}
	return true;
}