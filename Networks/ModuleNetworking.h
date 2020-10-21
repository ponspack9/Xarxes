#pragma once

#define PORT 9998
#define SERVER_ADDRESS "127.0.0.1"

class ModuleNetworking : public Module
{
private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool init() override;

	bool preUpdate() override;

	bool cleanUp() override;



	//////////////////////////////////////////////////////////////////////
	// Socket event callbacks
	//////////////////////////////////////////////////////////////////////

	virtual bool isListenSocket(SOCKET socket) const { return false; }

	virtual void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) { }

	virtual void onSocketReceivedData(SOCKET s, byte * data) = 0;

	virtual void onSocketDisconnected(SOCKET s) = 0;



protected:

	std::vector<SOCKET> sockets;

	void addSocket(SOCKET socket);

	void disconnect();

	static void reportError(const char *message);

	bool is_server = false;
};

