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
	appSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // IPPROTO_UDP
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


	return true;
}

update_status ModuleUDP::update()
{

	if (GetAsyncKeyState(VK_ESCAPE))
		return update_status::UPDATE_STOP;

	int len = sizeof(destAddr);

	if (sendto(appSocket, msgToSend, strlen(msgToSend), 0, (struct sockaddr*)&destAddr, len) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'sendto()'");
		return update_status::UPDATE_ERROR;
	}
	// breaks ??
	//printf("Sent message '%s' to %s using port %d \n", msgToSend, destAddr.sin_addr.S_un.S_addr, ntohs(destAddr.sin_port));
	printf("Sent message '%s' \n", msgToSend);


	memset(buffer, '\0', BUFFLEN);
	if (recvfrom(appSocket, buffer, BUFFLEN, 0, (struct sockaddr*)&destAddr, &len) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'recvform()'");
		return update_status::UPDATE_ERROR;
	}

	printf("Recieved message: '%s'\n", buffer);


	Sleep(5000);

	return update_status::UPDATE_CONTINUE;
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