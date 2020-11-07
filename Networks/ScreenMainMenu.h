#pragma once

class ScreenMainMenu : public Screen
{
	void enable() override;

	void gui() override;

public:
	bool name_used = false;
	bool name_banned = false;
	bool banned = false;
	bool kicked = false;
};
