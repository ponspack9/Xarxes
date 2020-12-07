#include "Networks.h"
#include "ScreenMainMenu.h"

void ScreenMainMenu::enable()
{
	/*LOG("Example Info log entry...");
	DLOG("Example Debug log entry...");
	WLOG("Example Warning log entry...");
	ELOG("Example Error log entry...");*/
}

void ScreenMainMenu::gui()
{
	ImGui::Begin("Main Menu");
	
	Texture *banner = App->modResources->banner;
	ImVec2 bannerSize(400.0f, 400.0f * banner->height / banner->width);
	ImGui::Image(banner->shaderResource, bannerSize);

	ImGui::Spacing();

	ImGui::Text("Server");

	static int localServerPort = PORT;
	ImGui::InputInt("Server port", &localServerPort);

	if (ImGui::Button("Start server"))
	{
		App->modScreen->screenGame->isServer = true;
		App->modScreen->screenGame->serverPort = localServerPort;
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenGame);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Client");

	static char serverAddressStr[64] = "127.0.0.1";
	ImGui::InputText("Server address", serverAddressStr, sizeof(serverAddressStr));

	static int remoteServerPort = PORT;
	ImGui::InputInt("Server port", &remoteServerPort);

	static char playerNameStr[64] = "playername";
	ImGui::InputText("Player name", playerNameStr, sizeof(playerNameStr));

	if (ImGui::IsItemClicked())
		name_used = name_banned = banned = kicked = false;

	if (name_used)
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Name already in use");
	else if (name_banned)
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Name is banned");
	else if (banned)
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "You were banned from server");
	else if (kicked)
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "You were kicked from server");

	if (ImGui::Button("Connect to server"))
	{
		App->modScreen->screenGame->isServer = false;
		App->modScreen->screenGame->serverPort = remoteServerPort;
		App->modScreen->screenGame->serverAddress = serverAddressStr;
		App->modScreen->screenGame->playerName = playerNameStr;
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenGame);
	}

	ImGui::End();
}
