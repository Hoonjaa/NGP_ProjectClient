#pragma once
#include "Object.h"
class Obstacle : public Object
{
public:
    Obstacle();
    ~Obstacle() override = default;

    GLvoid draw(
        const GLuint& ShaderID,
        const glm::mat4& worldMatrix,
        const glm::mat4& viewProjMatrix
    ) override;

    GLvoid update() override;

private:
    static constexpr int Count = 9;

    // XZ는 밑면 중심, Y는 밑면 높이
    const std::array<glm::vec3, Count> Positions{ {
        {-8.1f,  0.1f, -14.5f},  // 왼쪽 위
        { 5.5f,  0.1f, -14.0f},  // 위쪽 가운데
        {15.2f,  0.1f,  -8.2f},  // 오른쪽 위 세로
        {-15.6f, 0.1f,  -2.9f},  // 왼쪽 가운데
        {-0.2f,  0.1f,   0.9f},  // 중앙
        {-13.8f, 0.1f,  10.0f},  // 왼쪽 아래 ㄱ자 가로
        {-9.3f,  0.1f,  13.5f},  // 왼쪽 아래 ㄱ자 세로
        { 6.3f,  0.1f,  15.2f},  // 오른쪽 아래 짧은 박스
        {10.5f,  0.1f,  10.1f},  // 오른쪽 아래 긴 박스
    } };

    // 회전하기 전 로컬 X, Z 방향 길이
    const std::array<glm::vec2, Count> Sizes{ {
        {8.0f,  2.0f},
        {6.0f,  4.0f},
        {3.0f, 12.0f},
        {4.0f,  4.0f},
        {4.0f,  4.0f},
        {6.0f,  3.0f},
        {3.0f, 10.0f},
        {6.0f,  2.0f},
        {7.0f,  2.0f},
    } };

    const std::array<float, Count> YawDegrees{ {
        30.0f, -15.0f, 0.0f, 30.0f, 0.0f,
        0.0f, 0.0f, 30.0f, 55.0f,
    } };
};

