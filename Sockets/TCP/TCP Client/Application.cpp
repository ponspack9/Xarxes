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
	appSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // IPPROTO_TCP
	if (appSocket == INVALID_SOCKET)
	{
		printWSErrorAndExit("Error creating appSocket");
		return false;
	}

	// Destination
	memset((char*)&destAddr, 0, sizeof(destAddr));
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(port);
	inet_pton(AF_INET, destIP, &destAddr.sin_addr);

	// Connect
	int res = connect(appSocket, (const struct sockaddr*) & destAddr, sizeof(destAddr));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error connecting to server");
		return false;
	}

	return true;
}

update_status Application::Update()
{
	if (GetAsyncKeyState(VK_ESCAPE))
		return UPDATE_STOP;

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