#pragma once

#include "Module.h"

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

private:
	// PING_PONG_EXERCISE
	int msg_count = 0;
};

