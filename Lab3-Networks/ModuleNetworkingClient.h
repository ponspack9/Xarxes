#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	bool start(const char *serverAddress, int serverPort, const char *playerName);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging,
		Connected
	};

	struct Whisper
	{
		bool opened = false;
		bool selected = false;
		bool new_message = false;
		std::string username;
		std::vector<std::string> messages;
	};

	ClientState state = ClientState::Stopped;

	sockaddr_in serverAddress = {};
	SOCKET clientSocket = INVALID_SOCKET;

	std::string playerName;

	bool list_window = false;
	bool new_message = false;

	std::string current_tab = "General";
	std::vector<std::string> users_list;
	std::vector<std::pair<std::string, ImVec4>> messages_list;
	std::vector<Whisper> whispers_list;

	const ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	const ImVec4 blue = ImVec4(0.25f, 0.75f, 1.0f, 1.0f);
	const ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 color = white;

	ImGuiTabItemFlags flags = ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
};

