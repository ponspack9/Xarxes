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
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];

	// TODO(jesus): select those sockets that have a read operation available
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


	for (auto s : sockets)
	{
		if (FD_ISSET(s, &readfds))
		{
			if (isListenSocket(s))
			{
				// accept
				sockaddr_in incomingAddr;
				int size = sizeof(incomingAddr);
				SOCKET acceptedSocket = accept(s, (struct sockaddr*)&incomingAddr, &size);
				onSocketConnected(acceptedSocket, incomingAddr);
				addSocket(acceptedSocket);
			}
			else
			{
				// recieve
				int bytes = recv(s, (char*)incomingDataBuffer, incomingDataBufferSize, 0);
				if (bytes == SOCKET_ERROR)
				{
					DLOG("Socket closed");
					onSocketDisconnected(s);
					closesocket(s);
					return true;
				}
				// Disconected
				else if (bytes == 0 || bytes == ECONNRESET)
				{
					if ( strlen((const char*)incomingDataBuffer) > 0)
					{
						// FIN packet
						onSocketDisconnected(s);
						shutdown(s, SD_BOTH);
						DLOG("Socket shutdown");
					}
				}
				else
				{ 
					std::string actual_msg = (char*)incomingDataBuffer;
					actual_msg = actual_msg.substr(0, bytes);

					DLOG("Recieved message '%s'[%d B]\n", actual_msg.c_str() , bytes);
					onSocketReceivedData(s, (byte*)actual_msg.c_str());
				}
			}
		}
	}

	
	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).
	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.

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
