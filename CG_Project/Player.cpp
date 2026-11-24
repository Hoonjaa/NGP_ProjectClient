#include "Player.h"

GLvoid Player::Swing()
{
	if (is_Moving) {
		swingAngle += swingSpeed;
		if (swingAngle >= 20.0f) {
			swingAngle = 20.0f;
			swingSpeed = -swingSpeed;
		}
		if (swingAngle <= -20.0f) {
			swingAngle = -20.0f;
			swingSpeed = -swingSpeed;
		}
	}
	else if (swingAngle != 0.0f) {
		const float returnStep = std::abs(swingSpeed);

		if (swingAngle > 0.0f) {
			swingAngle = glm::max(0.0f, swingAngle - returnStep);
		}
		else {
			swingAngle = glm::min(0.0f, swingAngle + returnStep);
		}
	}

	glm::vec3 currPosition = getPosition();
	if (prevPosition == currPosition) {
		is_Moving = false;
	}
	else {
		is_Moving = true;
	}
	prevPosition = currPosition;

	// 플레이어 걷기 애니메이션
	player_Larm->setTransform(glm::mat4(1.0f));
	player_Lleg->setTransform(glm::mat4(1.0f));
	player_Rleg->setTransform(glm::mat4(1.0f));

	player_Larm->translate(glm::vec3(0.375f, 0.1f, 0.0f));
	player_Larm->translate(glm::vec3(0.0f, 0.3f, 0.0f));
	player_Larm->rotate(glm::radians(swingAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	player_Larm->translate(glm::vec3(0.0f, -0.3f, 0.0f));

	player_Lleg->translate(glm::vec3(0.15f, -0.75f, 0.0f));
	player_Lleg->translate(glm::vec3(0.0f, 0.35f, 0.0f));
	player_Lleg->rotate(glm::radians(swingAngle), glm::vec3(-1.0f, 0.0f, 0.0f));
	player_Lleg->translate(glm::vec3(0.0f, -0.35f, 0.0f));

	player_Rleg->translate(glm::vec3(-0.15f, -0.75f, 0.0f));
	player_Rleg->translate(glm::vec3(0.0f, 0.35f, 0.0f));
	player_Rleg->rotate(glm::radians(swingAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	player_Rleg->translate(glm::vec3(0.0f, -0.35f, 0.0f));
}

GLvoid Player::reload() {
	if (is_reloading) {
		gun_reload_angle += gun_reload_speed;
		if (gun_reload_angle >= 20.0f) {
			gun_reload_angle = 20.0f;
			gun_reload_speed = -gun_reload_speed;
		}
		if (gun_reload_angle <= -0.01f) {
			gun_reload_angle = 0.0f;
			gun_reload_speed = -gun_reload_speed;
			canFire = true; // 재장전 완료 후 발사 가능
			bulletCount = 30; // 총알 채우기
			is_reloading = false; // 재장전 종료
		}
	}
}

glm::vec3 Player::getFireDirection() const
{
	if (player_body && player_body->getObject()) {
		auto playerBodyObj = std::dynamic_pointer_cast<Player_body>(player_body->getObject());
		if (playerBodyObj) {
			if (firstPersonMode) {
				// 1인칭 모드에서는 카메라가 바라보는 방향으로 발사
				return playerBodyObj->getFaceDirection();
			}
			else {
				// 3인칭 모드에서는 플레이어 앞쪽 방향으로 발사
				return playerBodyObj->getForwardDirection();
			}
		}
	}
	return glm::vec3(0.0f, 0.0f, -1.0f); // 기본값
}

glm::vec3 Player::getRightDirection() const
{
	if (player_body && player_body->getObject()) {
		auto playerBodyObj = std::dynamic_pointer_cast<Player_body>(player_body->getObject());
		if (playerBodyObj) {
			if (firstPersonMode) {
				// 1인칭 모드: 카메라의 face_dir를 기준으로 오른쪽 벡터 계산
				glm::vec3 faceDir = playerBodyObj->getFaceDirection();
				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				return glm::normalize(glm::cross(faceDir, up));
			}
			else {
				// 3인칭 모드: 플레이어의 forward를 기준으로 오른쪽 벡터 계산
				glm::vec3 forward = playerBodyObj->getForwardDirection();
				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				return glm::normalize(glm::cross(forward, up));
			}
		}
	}
	return glm::vec3(1.0f, 0.0f, 0.0f); // 기본값 (오른쪽)
}

glm::vec3 Player::getMuzzlePosition() const
{
	glm::vec3 playerPos = getPosition();
	glm::vec3 fireDir = getFireDirection();
	glm::vec3 rightDir = getRightDirection();

	// 총구 위치 계산
	glm::vec3 muzzlePos = playerPos;

	if (firstPersonMode) {
		// 1인칭 모드: 카메라(눈) 위치 기준으로 총구 위치 계산
		muzzlePos.y += 0.4f; // 눈 높이
		muzzlePos += rightDir * 0.55f; // 오른쪽으로 약간 이동 (총이 오른쪽에 있다고 가정)
		muzzlePos += fireDir * 2.0f; // 앞쪽으로 약간 이동 (총구가 앞에 있음)
	}
	else {
		// 3인칭 모드: 플레이어 몸체 기준으로 총구 위치 계산
		muzzlePos.y += 0.8f; // 어깨 높이
		muzzlePos += rightDir * 0.3f; // 오른쪽으로 이동
		muzzlePos += fireDir * 0.5f; // 앞쪽으로 이동
	}

	return muzzlePos;
}

GLvoid Player::setup(GLuint shader) {
	root = std::make_shared<TreeNode>();

	auto P_body = std::make_shared<Player_body>();
	player_body = std::make_shared<Object_Part>(P_body, shader);
	player_body->translate(glm::vec3(0.0f, 1.2f, 10.0f));
	root->addChild(player_body);

	auto P_head = std::make_shared<Player_body_producer>(0.4f, 0.4f, 0.4f, glm::vec3(1.0f, 1.0f, 0.0f));
	player_head = std::make_shared<Object_Part>(P_head, shader);
	player_head->translate(glm::vec3(0.0f, 0.6f, 0.0f));
	player_body->addChild(player_head);

	auto P_Larm = std::make_shared<Player_body_producer>(0.15f, 0.6f, 0.15f, glm::vec3(1.0f, 0.752f, 0.796f));
	player_Larm = std::make_shared<Object_Part>(P_Larm, shader);
	player_Larm->translate(glm::vec3(0.375f, 0.1f, 0.0f));
	player_body->addChild(player_Larm);

	auto P_Rarm = std::make_shared<Player_body_producer>(0.15f, 0.15f, 0.6f, glm::vec3(1.0f, 0.752f, 0.796f));
	player_Rarm = std::make_shared<Object_Part>(P_Rarm, shader);
	player_Rarm->translate(glm::vec3(-0.375f, 0.375f, 0.25f));
	player_body->addChild(player_Rarm);

	auto P_Lleg = std::make_shared<Player_body_producer>(0.2f, 0.7f, 0.2f, glm::vec3(0.0f, 1.0f, 0.5f));
	player_Lleg = std::make_shared<Object_Part>(P_Lleg, shader);
	player_Lleg->translate(glm::vec3(0.15f, -0.75f, 0.0f));
	player_body->addChild(player_Lleg);

	auto P_Rleg = std::make_shared<Player_body_producer>(0.2f, 0.7f, 0.2f, glm::vec3(0.0f, 1.0f, 0.5f));
	player_Rleg = std::make_shared<Object_Part>(P_Rleg, shader);
	player_Rleg->translate(glm::vec3(-0.15f, -0.75f, 0.0f));
	player_body->addChild(player_Rleg);

	auto G_body = std::make_shared<Gun_body>();
	gun_body = std::make_shared<Object_Part>(G_body, shader);
	gun_body->translate(glm::vec3(0.0f, 0.15f, 0.5f));
	player_Rarm->addChild(gun_body);

	auto G_cover = std::make_shared<Gun_cover>();
	gun_cover = std::make_shared<Object_Part>(G_cover, shader);
	gun_cover->translate(glm::vec3(0.0f, 0.0f, 0.125f));
	gun_body->addChild(gun_cover);

	auto G_barrel = std::make_shared<Gun_barrel>();
	gun_barrel = std::make_shared<Object_Part>(G_barrel, shader);
	gun_barrel->translate(glm::vec3(0.0f, 0.0f, 0.35f));
	gun_body->addChild(gun_barrel);

	auto G_handle = std::make_shared<Gun_handle>();
	gun_handle = std::make_shared<Object_Part>(G_handle, shader);
	gun_handle->translate(glm::vec3(0.0f, -0.125f, -0.225f));
	gun_body->addChild(gun_handle);

	auto G_magazine = std::make_shared<Gun_magazine>();
	gun_magazine = std::make_shared<Object_Part>(G_magazine, shader);
	gun_magazine->translate(glm::vec3(0.0f, -0.15f, -0.0125f));
	gun_body->addChild(gun_magazine);
}

GLvoid Player::render(const glm::mat4& viewProjectionMatrix) {
	if (firstPersonMode && gun_body && player_body && player_body->getObject()) {
		updateGunRotation();
	}

	if (root) {
		setViewProjMatrixRecursive(root, viewProjectionMatrix);
		root->render(glm::mat4(1.0f));
		
	}
}

GLvoid Player::setViewProjMatrixRecursive(std::shared_ptr<TreeNode> node, const glm::mat4& viewProjMatrix) {
	// Object_Part로 캐스팅 시도
	auto objPart = std::dynamic_pointer_cast<Object_Part>(node);
	if (objPart) {
		objPart->setViewProjMatrix(viewProjMatrix);
	}

	// 모든 자식 노드에 대해 재귀 호출
	for (const auto& child : node->getChildren()) {
		setViewProjMatrixRecursive(child, viewProjMatrix);
	}
}

GLvoid Player::updateGunRotation() {
	auto playerBodyObj = std::dynamic_pointer_cast<Player_body>(player_body->getObject());
	if (playerBodyObj) {
		// 현재 yaw와 pitch 값을 가져와서 총 회전 적용
		float currentYaw = playerBodyObj->yaw;
		float currentPitch = playerBodyObj->pitch;

		// 총의 로컬 변환 초기화 (이전 회전 제거)
		player_body->setTransform(glm::mat4(1.0f));
		player_Rarm->setTransform(glm::mat4(1.0f));
		gun_magazine->setTransform(glm::mat4(1.0f));

		// Yaw 회전 (Y축 중심으로 좌우 회전)
		player_body->translate(glm::vec3(0.0f, 1.2f, 10.0f));
		player_body->rotate(currentYaw - glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));

		// Pitch 회전 (X축 중심으로 상하 회전)
		player_Rarm->translate(glm::vec3(-0.375f, 0.375f, 0.25f));
		player_Rarm->translate(glm::vec3(0.0f, 0.0f, -0.3f));
		player_Rarm->rotate(currentPitch + glm::radians(gun_reload_angle), glm::vec3(-1.0f, 0.0f, 0.0f));
		player_Rarm->translate(glm::vec3(0.0f, 0.0f, 0.3f));

		gun_magazine->translate(glm::vec3(0.0f, -0.15f, -0.0125f));
		gun_magazine->translate(glm::vec3(0.0f, 0.0f, -0.6f));
		gun_magazine->rotate(glm::radians(gun_reload_angle) * 2, glm::vec3(1.0f, 0.0f, 0.0f));
		gun_magazine->translate(glm::vec3(0.0f, 0.0f, 0.6f));
	}
}

GLvoid Player::processMovement(bool w_pressed, bool a_pressed, bool s_pressed, bool d_pressed)
{
	if (player_body && player_body->getObject()) {
		auto playerBodyObj = std::dynamic_pointer_cast<Player_body>(player_body->getObject());
		if (playerBodyObj) {
			// 이동 전 현재 위치 저장
			glm::vec3 oldPos = getPosition();
			
			// TreeNode의 translate를 직접 사용하여 이동
			if (w_pressed) {
				root->translate(playerBodyObj->getForwardMovement());
			}
			if (s_pressed) {
				root->translate(playerBodyObj->getBackwardMovement());
			}
			if (a_pressed) {
				root->translate(playerBodyObj->getLeftMovement());
			}
			if (d_pressed) {
				root->translate(playerBodyObj->getRightMovement());
			}
			
			// 이동 후 새 위치 확인
			glm::vec3 newPos = getPosition();
			
			// 범위 제한: x, z는 -19 ~ 19
			const float MIN_BOUNDARY = -25.0f;
			const float MAX_BOUNDARY = 25.0f;
			
			// 범위를 벗어났는지 확인
			bool outOfBounds = false;
			glm::vec3 correction(0.0f);
			
			if (newPos.x < MIN_BOUNDARY) {
				correction.x = MIN_BOUNDARY - newPos.x;
				outOfBounds = true;
			}
			else if (newPos.x > MAX_BOUNDARY) {
				correction.x = MAX_BOUNDARY - newPos.x;
				outOfBounds = true;
			}
			
			if (newPos.z < MIN_BOUNDARY) {
				correction.z = MIN_BOUNDARY - newPos.z;
				outOfBounds = true;
			}
			else if (newPos.z > MAX_BOUNDARY) {
				correction.z = MAX_BOUNDARY - newPos.z;
				outOfBounds = true;
			}
			
			// 범위를 벗어난 경우 보정
			if (outOfBounds) {
				root->translate(correction);
			}
		}
	}
}

glm::vec3 Player::getPosition() const
{
	if (player_body) {
		// TreeNode의 월드 변환에서 위치 추출
		glm::mat4 worldTransform = player_body->getWorldTransform();
		return glm::vec3(worldTransform[3][0], worldTransform[3][1], worldTransform[3][2]);
	}
	return glm::vec3(0.0f);
}

void Player::SetPosition(const glm::vec3& pos)
{
	if (player_body) {
		// 현재 위치를 가져와서 이동할 벡터 계산
		glm::vec3 currentPos = getPosition();
		glm::vec3 translation = pos - currentPos;
		root->translate(translation);
	}
}

GLvoid Player::handleMouseMovement(float deltaX, float deltaY)
{
	if (player_body && player_body->getObject()) {
		auto playerBodyObj = std::dynamic_pointer_cast<Player_body>(player_body->getObject());
		if (playerBodyObj) {
			// 마우스 이동을 face_dir 업데이트에 사용
			playerBodyObj->updateFaceDirection(deltaX, -deltaY); // Y축 반전
		}
	}
}

void Player::setForwardDirection(float pitch, float yaw)
{
	if (!player_body || !gun_body || !gun_magazine) return;

	auto body = std::dynamic_pointer_cast<Player_body>(player_body->getObject());
	if (!body) return;

	body->pitch = pitch;
	body->yaw = yaw;
	body->updateFaceDirection(0.0f, 0.0f);
}

glm::mat4 Player::getFirstPersonViewMatrix() const
{
	if (player_body && player_body->getObject()) {
		auto playerBodyObj = std::dynamic_pointer_cast<Player_body>(player_body->getObject());
		if (playerBodyObj) {
			glm::vec3 playerPos = getPosition();
			glm::vec3 cameraPos = playerBodyObj->getCameraPosition(playerPos);
			glm::vec3 faceDir = playerBodyObj->getFaceDirection();
			glm::vec3 upDir = glm::vec3(0.0f, 1.0f, 0.0f);

			return glm::lookAt(cameraPos, cameraPos + faceDir, upDir);
		}
	}
	return glm::mat4(1.0f);
}

glm::mat4 Player::getThirdPersonViewMatrix() const
{
	glm::vec3 eye = glm::vec3(0.0f, 10.0f, 20.0f);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	return glm::lookAt(eye, center, up);
}

GLvoid Player::toggleViewMode()
{
	firstPersonMode = !firstPersonMode;
}

bool Player::isFirstPersonMode() const
{
	return firstPersonMode;
}