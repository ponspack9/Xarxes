#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define BUFFLEN 512

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Module.h"
#include <Macros.h>

#pragma comment(lib, "ws2_32.lib")

class ModuleUDP : public Module
{
public:

	ModuleUDP();
	~ModuleUDP();


	bool init() override;
	update_status update() override;
	bool cleanUp() override;

public:

	void printWSErrorAndExit(const char* msg);

public:
	u_short port = 9999;
	char buffer[BUFFLEN];

	const char* msgToSend = "PONG";
	const char* destIP = "127.0.0.1";

	sockaddr_in sourceAddr;
	sockaddr_in destAddr;

	SOCKET appSocket;
};

