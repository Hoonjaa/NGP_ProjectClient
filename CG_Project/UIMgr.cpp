#include "pch.h"
#include "UIMgr.h"
#include "Player.h"

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
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f); // 반투명 검은색 배경
	glBegin(GL_QUADS);
	glVertex2f(170, 15);
	glVertex2f(10, 15);
	glVertex2f(10,  55);
	glVertex2f(170, 55);


	//glColor4f(1.0f, 0.0f, 0.0f, 1);
	//glVertex2f(player->HP, 20);
	//glVertex2f(10, 20);
	//glVertex2f(10, 50);
	//glVertex2f(player->HP, 50);
	//glEnd();
}

void UIMgr::DrawEnermyHP()
{

}

void UIMgr::DrawChat()
{
	constexpr int x = 16;
	constexpr int lineHeight = 20;
	int y = 24;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 1.0f);

	std::size_t count = 0;
	for (auto it = chatMessages.rbegin(); it != chatMessages.rend() && count < 10; ++it, ++count)
	{
		glWindowPos2i(x, y);
		glutBitmapString(
			GLUT_BITMAP_HELVETICA_18,
			reinterpret_cast<const unsigned char*>(it->c_str())
		);
		y += lineHeight;
	}

	glPopAttrib();
}

void UIMgr::DrawKillLog()
{
	constexpr int x = 16;
	constexpr int lineHeight = 20;
	int y = 24;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 1.0f);

	std::size_t count = 0;
	for (auto it = killLogMessages.rbegin(); it != killLogMessages.rend() && count < 10; ++it, ++count)
	{
		glWindowPos2i(x, y);
		glutBitmapString(
			GLUT_BITMAP_HELVETICA_18,
			reinterpret_cast<const unsigned char*>(it->c_str())
		);
		y += lineHeight;
	}

	glPopAttrib();
}