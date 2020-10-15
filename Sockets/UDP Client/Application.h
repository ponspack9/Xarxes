#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define BUFFLEN 512

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
	char buffer[BUFFLEN];

	char msgToSend[BUFFLEN] = "PING";
	const char* sourceIP = "192.168.1.129";
	const char* destIP = "127.0.0.1";

	sockaddr_in sourceAddr;
	sockaddr_in destAddr;


	SOCKET appSocket;


	// temp
	int count = 0;
};
