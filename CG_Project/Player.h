#pragma once
#include "pch.h"
#include <string>
#include "TreeNode.h"
#include "Object_part.h"
#include "Player_body.h"
#include "Gun_body.h"
#include "Gun_cover.h"
#include "Gun_barrel.h"
#include "Gun_handle.h"
#include "Gun_magazine.h"
#include "Player_body_producer.h"
#include "Bullet.h"

#define MAX_PLAYER_HP 200

class Player
{
private:
	std::shared_ptr<TreeNode> root;
	std::shared_ptr<Object_Part> player_body;
	std::shared_ptr<Object_Part> player_head;
	std::shared_ptr<Object_Part> player_Larm;
	std::shared_ptr<Object_Part> player_Rarm;
	std::shared_ptr<Object_Part> player_Lleg;
	std::shared_ptr<Object_Part> player_Rleg;
	std::shared_ptr<Object_Part> gun_body;
	std::shared_ptr<Object_Part> gun_cover;
	std::shared_ptr<Object_Part> gun_barrel;
	std::shared_ptr<Object_Part> gun_handle;
	std::shared_ptr<Object_Part> gun_magazine;
	bool firstPersonMode = true; // 1인칭/3인칭 모드 전환용

	// 총 회전 업데이트를 위한 private 메서드
	GLvoid updateGunRotation();
	GLvoid setViewProjMatrixRecursive(std::shared_ptr<TreeNode> node, const glm::mat4& viewProjMatrix);

public: // 클라에만 있는 변수
	float hit_time = 0.0f; // 플레이어가 맞은 시간
	bool invincible = false; // 무적 상태 여부
	int cnt_death = 0, cnt_kill = 0;
public: // 서버에도 있는 변수
	// 총알 관련 변수
	std::vector<Bullet*> bullets;
	void drawBullet(const GLuint& ShaderID, const glm::mat4& viewProjMatrix)
	{
		for (auto bullet : bullets) {
			bullet->draw(ShaderID, glm::mat4(1.0f), viewProjMatrix);
		}
	}
	void addBullet(glm::vec3 Pos, glm::vec3 Color) { 
		Bullet *b = new Bullet(Pos, getFireDirection(), Color);
		bullets.push_back(b);
	}

	GLint bulletCount = 30;
	bool isAlive = true; // 렌더링 여부
	bool canFire = true;
	float HP = MAX_PLAYER_HP; // 플레이어 체력
	std::string playerName;
	short playerID = -1; // 플레이어 ID

public:
	// 플레이어 애니메이션 관련 변수& 함수
	bool is_Moving = false;
	glm::vec3 prevPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	GLfloat swingAngle = 0.0f;
	GLfloat swingSpeed = 1.5f;
	GLvoid Swing();

public:
	GLvoid setup(GLuint shader);
	GLvoid render(const glm::mat4& viewProjectionMatrix);
	// 이동 관련 함수
	GLvoid processMovement(bool w_pressed, bool a_pressed, bool s_pressed, bool d_pressed);
	glm::vec3 getPosition() const;
	void SetPosition(const glm::vec3& pos);
	void SetPlayerID(short id) { playerID = id; }

	// 총알 관련 변수
	glm::vec3 getFireDirection() const; //총알 발사 방향
	glm::vec3 getRightDirection() const; // 플레이어 오른쪽 방향
	glm::vec3 getMuzzlePosition() const; // 총구 위치
	// 재장전 애니메이션 관련 함수& 변수
	GLvoid reload();
	bool is_reloading = false; // 재장전 중인지 여부
	GLfloat gun_reload_angle = 0.0f; // 재장전 애니메이션 각도
	GLfloat gun_reload_speed = 1.0f; // 재장전 애니메이션 속도


	// 1인칭 카메라 관련 함수
	GLvoid handleMouseMovement(float deltaX, float deltaY);
	void setForwardDirection(float pitch, float yaw);
	glm::mat4 getFirstPersonViewMatrix() const;
	glm::mat4 getThirdPersonViewMatrix() const;
	GLvoid toggleViewMode(); // 1인칭/3인칭 전환
	bool isFirstPersonMode() const;
};

