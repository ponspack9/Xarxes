#include "ModuleTCP.h"

#include <iostream>
#include <stdlib.h>

ModuleTCP::ModuleTCP() {}

ModuleTCP::~ModuleTCP() {}

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

	// Destination
	memset((char*)&destAddr, 0, sizeof(destAddr));
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(port);
	inet_pton(AF_INET, destIP, &destAddr.sin_addr);

	// Connecting to server
	if (connect(appSocket, (const struct sockaddr*)&destAddr, sizeof(destAddr)) == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error connecting to server");
		return false;
	}

	return true;
}


update_status ModuleTCP::pingPong(const char* msg)
{

	int bytes = send(appSocket, msg, BUFFLEN, 0);

	if (bytes == SOCKET_ERROR)
	{
		printWSErrorAndExit("Failed 'send()'");
		return update_status::UPDATE_ERROR;
	}

	printf("Sent message '%s'[%d B]\n", msg, bytes);

	// Recieving
	memset(buffer, '\0', BUFFLEN);
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

	return update_status::UPDATE_CONTINUE;
}

update_status ModuleTCP::update()
{

	update_status status = pingPong("PINGTCP");
	if (status != update_status::UPDATE_CONTINUE) return status;

	msg_sent++;

	if (msg_sent >= MSG_TO_SEND) return update_status::UPDATE_STOP;

	Sleep(500);

	return update_status::UPDATE_CONTINUE;
}

bool ModuleTCP::cleanUp()
{
	// Clean sockets
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
