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
	appSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // IPPROTO_UDP
	if (appSocket == INVALID_SOCKET)
	{
		printWSErrorAndExit("Error creating appSocket");
		return false;
	}

	// Specifying addresses	
	// Local / source
	//sourceAddr.sin_family = AF_INET; // IPv4 (AF_INET6 -> IPv6)
	//sourceAddr.sin_port = htons(port);
	//sourceAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	//// Binding
	//int enable = 1;
	//int res = setsockopt(appSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	//if (res == SOCKET_ERROR) 
	//{
	//	printWSErrorAndExit("Error forcing reuse of address and port");
	//	return false;
	//}

	//int result = bind(appSocket, (const struct sockaddr*)&sourceAddr, sizeof(sourceAddr));

	// Destination
	memset((char*)&destAddr, 0, sizeof(destAddr));
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(port);
	//destAddr.sin_addr.S_un.S_addr = inet_addr(destIP);
	inet_pton(AF_INET, destIP, &destAddr.sin_addr);
	

	return true;
}

update_status Application::Update()
{

	if (GetAsyncKeyState(VK_ESCAPE))
		return UPDATE_STOP;

	int len = sizeof(destAddr);

	if (sendto(appSocket, msgToSend, strlen(msgToSend), 0, (struct sockaddr*)&destAddr, len) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'sendto()'");
		return UPDATE_ERROR;
	}
	//printf("Sent message '%s' to %s using port %d \n", msgToSend, destAddr.sin_addr.S_un.S_addr, ntohs(destAddr.sin_port));
	printf("Sent message '%s' \n", msgToSend);


	memset(buffer, '\0', BUFFLEN);
	if (recvfrom(appSocket,buffer,BUFFLEN,0, (struct sockaddr*)&destAddr, &len) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'recvform()'");
		return UPDATE_ERROR;
	}

	printf("Recieved message: '%s'\n", buffer);


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