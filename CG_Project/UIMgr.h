#pragma once

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
	Player* player;
private:
	void DrawAmmoCount();
	void DrawPlayerHP();
	void DrawKillLog();
	void DrawEnermyHP();
	void DrawChat();

	GLuint* shaderProgramID;
	GLint WindowWidth, WindowHeight;
	std::vector<std::string> chatMessages{
		"01 Alice: Hello!",
		"02 Bob: Can you see this?",
		"03 Carol: Yes.",
		"04 Alice: Ready?",
		"05 Bob: Ready.",
		"06 Carol: Go!",
		"07 Bob: Reloading.",
		"08 Alice: Behind you!",
		"09 Carol: Nice shot.",
		"10 Bob: Thanks!",
		"11 Alice: One more round?",
		"12 Carol: Let's go."
	};

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