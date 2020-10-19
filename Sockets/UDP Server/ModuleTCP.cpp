#include "ModuleTCP.h"

#include <iostream>
#include <stdlib.h>

ModuleTCP::ModuleTCP(){}

ModuleTCP::~ModuleTCP(){}


bool ModuleTCP::init()
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
	appSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

	// Reuse of port
	int enable = 1;
	int res = setsockopt(appSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error forcing reuse of address and port");
		return false;
	}

	// Binding
	res = bind(appSocket, (const struct sockaddr*)&sourceAddr, sizeof(sourceAddr));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error binding port");
		return false;
	}

	// Switching to listening mode
	printf("Binding done, switching to listen mode\n");
	res = listen(appSocket, 1);

	// Waiting for incoming connections
	printf("Ready to accept connections\n");
	int size = sizeof(destAddr);
	appSocket = accept(appSocket, (struct sockaddr*)&destAddr, &size);

	printf("Connected, waiting for incoming messages ...\n");

	return true;
}


update_status ModuleTCP::update()
{
	int bytes = 0;
	memset(buffer, '\0', BUFFLEN);

	// Recieving
	bytes = recv(appSocket, buffer, BUFFLEN, 0);
	if (bytes == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'recv()'");
		return update_status::UPDATE_ERROR;
	}
	printf("Recieved message '%s'[%d B]\n", buffer, bytes);
	if (bytes == 0)
		printf("BYTES IS 0\n");
	Sleep(500);

	// Sending
	bytes = send(appSocket, buffer, BUFFLEN, 0);
	if (bytes == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'send()'");
		return update_status::UPDATE_ERROR;
	}
	printf("Sent message '%s'[%d B]\n", buffer, bytes);




	return update_status::UPDATE_CONTINUE;
}

bool ModuleTCP::cleanUp()
{
	return true;
}

void ModuleTCP::printWSErrorAndExit(const char* msg)
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
	fprintf(stderr, "%s:\n %S\n", msg, s);
	LocalFree(s);
}
