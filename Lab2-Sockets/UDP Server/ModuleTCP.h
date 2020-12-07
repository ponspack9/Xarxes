#pragma once
#include "Module.h"

class ModuleTCP : public Module
{
public:

	ModuleTCP();
	~ModuleTCP();

	bool init() override;

	update_status update() override;

	bool cleanUp() override;

	void printWSErrorAndExit(const char* msg);

public:

	u_short port = 9998;
	char buffer[BUFFLEN];

	const char* msgToSend = "PONG";
	const char* destIP = "127.0.0.1";

	sockaddr_in sourceAddr;
	sockaddr_in destAddr;

	SOCKET appSocket;

};

