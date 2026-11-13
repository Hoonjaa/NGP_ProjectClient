#include "pch.h"
#include "UIMgr.h"
#include "Player.h"
#include <algorithm>

void UIMgr::BeginChat()
{
	isChatting = true;
	chatInput.clear();
}

void UIMgr::HandleChatKey(unsigned char key)
{
	if (!isChatting) return;

	if (key == 13) {
		if (chatInput.find_first_not_of(' ') != std::string::npos) {
			if (!onChatSubmit || !onChatSubmit(chatInput)) return;
			// The server broadcasts to other clients only, so keep a local echo.
			AddChatMessage("You: " + chatInput);
		}
		chatInput.clear();
		isChatting = false;
	}
	else if (key == 27) {
		chatInput.clear();
		isChatting = false;
	}
	else if (key == 8) {
		if (!chatInput.empty()) chatInput.pop_back();
	}
	else if (key >= 32 && key <= 126 && chatInput.size() < MAX_CHAT_MSG_LEN - 1) {
		// GLUT bitmap fonts and keyboard callbacks support printable ASCII here.
		chatInput.push_back(static_cast<char>(key));
	}
}

void UIMgr::AddChatMessage(const std::string& message)
{
	if (message.empty()) return;
	constexpr std::size_t maxHistory = 100;
	if (chatMessages.size() >= maxHistory) {
		chatMessages.erase(chatMessages.begin());
	}
	chatMessages.push_back(message);
}

void UIMgr::Resize(GLint width, GLint height)
{
	WindowWidth = std::max(1, width);
	WindowHeight = std::max(1, height);
}

void UIMgr::Draw()
{
	// ============= UI 렌더링 시작 =============
	// 셰이더 프로그램 비활성화
	glUseProgram(0);

	// 투영 및 모델뷰 행렬을 2D 모드로 전환
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WindowWidth, 0, WindowHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// 깊이 테스트 비활성화
	glDisable(GL_DEPTH_TEST);

	DrawAmmoCount();
	DrawPlayerHP();
	DrawEnermyHP();
	DrawChat();
	DrawKillLog();

	// 깊이 테스트 다시 활성화
	glEnable(GL_DEPTH_TEST);

	// 행렬 복원
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// 셰이더 프로그램 다시 활성화
	glUseProgram(*shaderProgramID);
}

void UIMgr::DrawAmmoCount()
{
	// 탄약 정보 배경 박스
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0.0f, 0.0f, 0.0f, 0.7f); // 반투명 검은색 배경
	glBegin(GL_QUADS);
	glVertex2f(WindowWidth - 170, 15);
	glVertex2f(WindowWidth - 10, 15);
	glVertex2f(WindowWidth - 10, player && player->is_reloading ? 85 : 55);
	glVertex2f(WindowWidth - 170, player && player->is_reloading ? 85 : 55);
	glEnd();

	glDisable(GL_BLEND);

	// 탄약 정보 표시
	if (player) {
		char ammoText[50];
		sprintf_s(ammoText, "Ammo: %d / 30", player->bulletCount);

		// 텍스트 색상 (밝은 녹색)
		glColor3f(0.0f, 1.0f, 0.0f);

		glRasterPos2f(WindowWidth - 155, 30);
		for (char* c = ammoText; *c != '\0'; c++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
		}

		// 재장전 중이면 표시
		if (player->is_reloading) {
			glColor3f(1.0f, 1.0f, 0.0f); // 노란색
			glRasterPos2f(WindowWidth - 155, 60);
			const char* reloadText = "Reloading...";
			for (const char* c = reloadText; *c != '\0'; c++) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
			}
		}
	}
}

void UIMgr::DrawPlayerHP()
{
	if (!player) return;

	constexpr float left = 10.0f;
	constexpr float right = 170.0f;
	const float hpRatio = std::min(1.0f,std::max(player->HP / MAX_PLAYER_HP, 0.0f));
	const float hpRight = left + (right - left) * hpRatio;

	glBegin(GL_QUADS);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2f(left, 15.0f);
	glVertex2f(right, 15.0f);
	glVertex2f(right, 55.0f);
	glVertex2f(left, 55.0f);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(left, 20.0f);
	glVertex2f(hpRight, 20.0f);
	glVertex2f(hpRight, 50.0f);
	glVertex2f(left, 50.0f);

	glEnd();
}

void UIMgr::DrawEnermyHP()
{

}

void UIMgr::DrawChat()
{
	constexpr int left = 10;
	constexpr int bottom = 90;
	constexpr int x = left + 6;
	constexpr int lineHeight = 20;
	const int width = std::min(500, WindowWidth - 2 * left);
	const int visibleLines = std::max(0, std::min(8, (WindowHeight - 160) / lineHeight));
	if (width < 80 || WindowHeight < 160) return;
	const int textWidth = width - 12;
	void* font = GLUT_BITMAP_HELVETICA_18;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
	glBegin(GL_QUADS);
	glVertex2i(left, bottom);
	glVertex2i(left + width, bottom);
	glVertex2i(left + width, bottom + 58 + visibleLines * lineHeight);
	glVertex2i(left, bottom + 58 + visibleLines * lineHeight);
	glEnd();

	// Wrap history to the panel width, then show its most recent lines.
	std::vector<std::string> lines;
	for (const auto& message : chatMessages) {
		std::string line;
		int lineWidth = 0;
		for (unsigned char c : message) {
			const int charWidth = glutBitmapWidth(font, c);
			if (!line.empty() && lineWidth + charWidth > textWidth) {
				lines.push_back(line);
				line.clear();
				lineWidth = 0;
			}
			line.push_back(static_cast<char>(c));
			lineWidth += charWidth;
		}
		if (!line.empty()) lines.push_back(line);
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	int y = bottom + 48;
	int count = 0;
	for (auto it = lines.rbegin(); it != lines.rend() && count < visibleLines; ++it, ++count) {
		glWindowPos2i(x, y);
		glutBitmapString(font, reinterpret_cast<const unsigned char*>(it->c_str()));
		y += lineHeight;
	}

	std::string inputLine = isChatting ? "> " + chatInput + "_" : "Enter: chat";
	// Keep the end of a long input visible while typing.
	while (inputLine.size() > 3 &&
		glutBitmapLength(font, reinterpret_cast<const unsigned char*>(inputLine.c_str())) > textWidth) {
		inputLine.erase(2, 1);
	}
	glColor3f(1.0f, 1.0f, 0.4f);
	glWindowPos2i(x, bottom + 8);
	glutBitmapString(font, reinterpret_cast<const unsigned char*>(inputLine.c_str()));
	if (isChatting) {
		glColor3f(0.8f, 0.8f, 0.8f);
		glWindowPos2i(x, bottom + 28);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12,
			reinterpret_cast<const unsigned char*>("Enter: send | Esc: cancel"));
	}

	glPopAttrib();
}

void UIMgr::DrawKillLog()
{
	constexpr int x = 800 / 2;
	constexpr int lineHeight = 20;
	int y = 600;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 1.0f);

	std::size_t count = 0;
	for (auto it = killLogMessages.rbegin(); it != killLogMessages.rend() && count < 10; ++it, ++count)
	{
		glWindowPos2i(x- it->message.size()*2, y);
		glutBitmapString(
			GLUT_BITMAP_HELVETICA_18,
			reinterpret_cast<const unsigned char*>(it->message.c_str())
		);
		y -= lineHeight;
	}

	glPopAttrib();
}