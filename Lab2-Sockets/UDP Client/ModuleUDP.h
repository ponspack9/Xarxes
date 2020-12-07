#pragma once

#include "Module.h"
#include <mutex>

class ModuleUDP : public Module
{
public:

	ModuleUDP();
	~ModuleUDP();


	bool init() override;

	update_status pingPong(const char* msg);

	void onTaskFinished(Task* task) override;

	update_status update() override;

	bool cleanUp() override;

public:

	void printWSErrorAndExit(const char* msg);

private:
	u_short port = 9999;
	char buffer[BUFFLEN];

	//char msgToSend[BUFFLEN] = "PING";
	const char* destIP = "127.0.0.1";

	sockaddr_in destAddr;
	SOCKET appSocket;


public:
	// PING PONG EXERCISE
	int msg_sent = 0;
	int msg_recieved = 0;

	std::mutex mtx;
};

