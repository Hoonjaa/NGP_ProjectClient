#pragma once

#include "pch.h"
#include "Protocol.h"
#include <string>
#include <functional>

struct killLogEntry {
	std::string message;
	float displayTime;
};

class Player;

class UIMgr
{
public:
	UIMgr(GLuint* shaderProgramID, GLint WW = 800, GLint WH = 800, Player* p = nullptr)
		: shaderProgramID(shaderProgramID), WindowWidth(WW), WindowHeight(WH), player(p) {}
	void Draw();
	void BeginChat();
	void HandleChatKey(unsigned char key);
	// Return false to keep the input open for retry.
	std::function<bool(const std::string&)> onChatSubmit;
	bool IsChatting() const { return isChatting; }
	void AddChatMessage(const std::string& message);
	void Resize(GLint width, GLint height);
	Player* player;
private:
	void DrawAmmoCount();
	void DrawPlayerHP();
	void DrawKillLog();
	void DrawEnermyHP();
	void DrawChat();

	GLuint* shaderProgramID;
	GLint WindowWidth, WindowHeight;
	bool isChatting = false;
	std::string chatInput;
	std::vector<std::string> chatMessages;

	std::vector<killLogEntry> killLogMessages{
		{"Alice eliminated Bob",5},
		{"Bob eliminated Carol",4},
		{"Carol eliminated Alice",3},
		{"Alice eliminated Carol",2},
		{"Bob eliminated Alice",1},
	};
	
	
	
	
	
	//std::vector<std::string> chatMessages;
	//std::vector<std::string> killLogMessages;
	std::queue<std::string> killMessages;
};
