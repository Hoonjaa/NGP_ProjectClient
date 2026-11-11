#pragma once

class Player;

class UIMgr
{
public:
	UIMgr(GLint WW = 800, GLint WH = 800, Player* p) 
		: WindowWidth(WW), WindowHeight(WH), player(p) {}
	void Draw();
	Player* player;
private:
	void DrawAmmoCount();
	void DrawPlayerHP();
	void DrawKillLog();
	void DrawEnermyHP();
	void DrawChat();

	GLint WindowWidth, WindowHeight;
};