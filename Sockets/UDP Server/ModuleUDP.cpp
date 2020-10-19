#include "ModuleUDP.h"

#include <iostream>
#include <stdlib.h>

ModuleUDP::ModuleUDP()
{
}

ModuleUDP::~ModuleUDP()
{
}

bool ModuleUDP::init()
{
	// Library initialization
	printf("Initializing WSA\n");
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printWSErrorAndExit("Error initializing");
		return false;
	}

	// Socket initialization
	appSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (appSocket == INVALID_SOCKET)
	{
		printWSErrorAndExit("Error creating appSocket");
		return false;
	}

	// Specifying addresses	
	// Local / source
	sourceAddr.sin_family = AF_INET; // IPv4 (AF_INET6 -> IPv6)
	sourceAddr.sin_port = htons(port);
	sourceAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	// Binding
	int enable = 1;
	int res = setsockopt(appSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error forcing reuse of address and port");
		return false;
	}

	res = bind(appSocket, (const struct sockaddr*)&sourceAddr, sizeof(sourceAddr));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error binding port");
		return false;
	}

	printf("Waiting for incoming messages ...\n");

	return true;
}

update_status ModuleUDP::update()
{

	if (GetAsyncKeyState(VK_ESCAPE))
		return update_status::UPDATE_STOP;


	memset(buffer, '\0', BUFFLEN);

	int size = sizeof(destAddr);

	printf("-----------------------------------\n");
	if (recvfrom(appSocket, buffer, BUFFLEN, 0, (struct sockaddr*)&destAddr, &size) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'recvform()'");
		return update_status::UPDATE_ERROR;
	}

	//printf("Recieved message '%s' from %s using port %d \n", buffer, destAddr.sin_addr.S_un.S_addr, ntohs(destAddr.sin_port));
	printf("Recieved message '%s'\n", buffer);
	Sleep(500);

#ifdef PING_PONG_EXERCISE
	msg_count++;
	sprintf_s(buffer, "PONG");
#endif

	// Sending back the message
	if (sendto(appSocket, buffer, BUFFLEN, 0, (struct sockaddr*)&destAddr, size) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'sendto()'");
		return update_status::UPDATE_ERROR;
	}
	printf("Sent acknowledge '%s' \n", buffer);


	return (msg_count < MSG_TO_SEND)? update_status::UPDATE_CONTINUE : update_status::UPDATE_STOP;
}

bool ModuleUDP::cleanUp()
{
	printf("Cleaning up WSA\n");
	int iResult = WSACleanup();
	if (iResult != NO_ERROR)
	{
		printWSErrorAndExit("Error cleaning up");
	}

	printf("Cleaning sockets \n");
	closesocket(appSocket);

	return true;

}


void ModuleUDP::printWSErrorAndExit(const char* msg)
{
	wchar_t* s = NULL;
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s,
		0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
}