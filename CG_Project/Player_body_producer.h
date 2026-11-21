#pragma once
#include "Object.h"
class Player_body_producer : public Object
{
public:
	Player_body_producer(GLfloat Width, GLfloat Height, GLfloat Depth, glm::vec3 color);
	~Player_body_producer();

	GLvoid draw(const GLuint& ShaderID, const glm::mat4& worldMatrix, const glm::mat4& viewProjMatrix) override;
	GLvoid update() override;
private:
    GLfloat vertices[9 * 24];

    GLuint indexes[6 * 6] = {
        // ¾Õ¸é (normal: 0, 0, 1)
        0,1,2,    0,2,3,
        // ¿ÞÂÊ¸é (normal: -1, 0, 0)
        4,5,6,    4,6,7,
        // µÞ¸é (normal: 0, 0, -1)
        8,9,10,   8,10,11,
        // ¿À¸¥ÂÊ¸é (normal: 1, 0, 0)
        12,13,14, 12,14,15,
        // À­¸é (normal: 0, 1, 0)
        16,17,18, 16,18,19,
        // ¾Æ·§¸é (normal: 0, -1, 0)
        20,21,22, 20,22,23
    };
};

