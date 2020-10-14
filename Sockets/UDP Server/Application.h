#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

enum update_status
{
	UPDATE_CONTINUE,
	UPDATE_ERROR,
	UPDATE_STOP
};

class Application
{
public:

	Application();
	~Application();


	bool Init();
	update_status Update();
	bool CleanUp();

public:

	void printWSErrorAndExit(const char* msg);

public:
	u_short port = 9999;
	const char* destAddr = "127.0.0.1";

	SOCKET appSocket;
	sockaddr_in bindAddr;


	// temp
	int count = 0;
};

