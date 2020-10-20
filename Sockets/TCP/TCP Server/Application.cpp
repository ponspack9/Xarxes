#include "Application.h"

#include <iostream>
#include <stdlib.h>

Application::Application()
{
}

Application::~Application()
{
}

bool Application::Init()
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
	appSocket = socket(AF_INET, SOCK_STREAM, 0);
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

	res = bind(appSocket, (const struct sockaddr*) & sourceAddr, sizeof(sourceAddr));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error binding port");
		return false;
	}

	// Listen
	res = listen(appSocket, MAX_CONNECTIONS_SAMETIME);
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error setting socket as listen mode");
		return false;
	}

	// Waiting for incoming connections
	printf("Ready to accept connections\n");
	int size = sizeof(destAddr);
	acceptedSocket = accept(appSocket, (struct sockaddr*) & destAddr, &size);

	printf("Connected, waiting for incoming messages ...\n");
	return true;
}

update_status Application::Update()
{
	if (GetAsyncKeyState(VK_ESCAPE))
		return UPDATE_STOP;

	int bytes = 0;
	memset(buffer, '\0', BUFFLEN);

	// Recieving
	bytes = recv(acceptedSocket, buffer, BUFFLEN, 0);
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
	bytes = send(acceptedSocket, buffer, BUFFLEN, 0);
	if (bytes == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'send()'");
		return update_status::UPDATE_ERROR;
	}
	printf("Sent message '%s'[%d B]\n", buffer, bytes);

	return UPDATE_CONTINUE;
}

bool Application::CleanUp()
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

void Application::printWSErrorAndExit(const char* msg)
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