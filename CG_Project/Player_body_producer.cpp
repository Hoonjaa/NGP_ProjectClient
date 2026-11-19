#include "Player_body_producer.h"

Player_body_producer::Player_body_producer(float Width, float Height, glm::vec3 color)
{


	GLfloat temp[9 * 24] = {

	};

	memcpy(vertices, temp, sizeof(temp));
}

Player_body_producer::~Player_body_producer()
{
}

GLvoid Player_body_producer::draw(const GLuint& ShaderID, const glm::mat4& worldMatrix, const glm::mat4& viewProjMatrix)
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 와이어프레임 모드
	glBindVertexArray(VAO);

	// modelTransform 행렬을 여기에서 변환




	glm::mat4 finalWorldMatrix = worldMatrix * modelMatrix;
	unsigned int modelLocation = glGetUniformLocation(ShaderID, "model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(finalWorldMatrix));

	glm::mat4 finalMatrix = viewProjMatrix * finalWorldMatrix;
	unsigned int transformLocation = glGetUniformLocation(ShaderID, "Transform");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

	glDrawElements(DrawMode, vertexCount, GL_UNSIGNED_INT, 0);
}

GLvoid Player_body_producer::update()
{
	return GLvoid();
}