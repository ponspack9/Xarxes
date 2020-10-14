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
	appSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (appSocket == INVALID_SOCKET)
	{
		printWSErrorAndExit("Error creating appSocket");
		return false;
	}

	// Specifying addresses	
	remoteAddr.sin_family = AF_INET; // IPv4 (AF_INET6 -> IPv6)
	remoteAddr.sin_port = htons(port);
	// Converting string to ip address as well as putin it inside the sockaddr_in
	inet_pton(AF_INET, serverAddr, &remoteAddr.sin_addr);

	// Binding
	int enable = 1;
	if (setsockopt(appSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error forcing reuse of address and port");
		return false;
	}

	int result = bind(appSocket, (const struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
	

	return true;
}

update_status Application::Update()
{

	if (GetAsyncKeyState(VK_ESCAPE))
		return UPDATE_STOP;

	char msg[256] = "PING TO SERVER";
	//const char* msg = "PING TO SERVER";

	printf("Sending message '%s' to %s using port %d \n", msg, serverAddr, port);
	int len = strlen(msg);
	sendto(appSocket, msg,256*sizeof(char) , 0, (const sockaddr*)&remoteAddr, sizeof(remoteAddr));
	Sleep(5000);

	return UPDATE_CONTINUE;
}

bool Application::CleanUp()
{
	printf("Cleaning up WSA\n");
	int iResult = WSACleanup();
	if (iResult != NO_ERROR)
	{
		printWSErrorAndExit("Error cleaning up");
		return false;
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