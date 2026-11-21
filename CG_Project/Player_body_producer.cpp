#include "Player_body_producer.h"

Player_body_producer::Player_body_producer(GLfloat Width, GLfloat Height, GLfloat Depth, glm::vec3 color)
{
	GLfloat x = Width * 0.5f, y = Height * 0.5f, z = Depth * 0.5f;

	GLfloat temp[9 * 24] = {
        // 앞면 (z = +0.5, normal = 0, 0, 1)
            vPos.x + x, vPos.y + y, vPos.z + z,  color.r, color.g, color.b,  0.0f, 0.0f, 1.0f,
            vPos.x - x, vPos.y + y, vPos.z + z,  color.r, color.g, color.b,  0.0f, 0.0f, 1.0f,
            vPos.x - x, vPos.y - y, vPos.z + z,  color.r, color.g, color.b,  0.0f, 0.0f, 1.0f,
            vPos.x + x, vPos.y - y, vPos.z + z,  color.r, color.g, color.b,  0.0f, 0.0f, 1.0f,

            // 왼쪽면 (x = -0.5, normal = -1, 0, 0)
            vPos.x - x, vPos.y + y, vPos.z + z,  color.r, color.g, color.b,  -1.0f, 0.0f, 0.0f,
            vPos.x - x, vPos.y + y, vPos.z - z,  color.r, color.g, color.b,  -1.0f, 0.0f, 0.0f,
            vPos.x - x, vPos.y - y, vPos.z - z,  color.r, color.g, color.b,  -1.0f, 0.0f, 0.0f,
            vPos.x - x, vPos.y - y, vPos.z + z,  color.r, color.g, color.b,  -1.0f, 0.0f, 0.0f,

            // 뒷면 (z = -0.5, normal = 0, 0, -1)
            vPos.x - x, vPos.y + y, vPos.z - z,  color.r, color.g, color.b,  0.0f, 0.0f, -1.0f,
            vPos.x + x, vPos.y + y, vPos.z - z,  color.r, color.g, color.b,  0.0f, 0.0f, -1.0f,
            vPos.x + x, vPos.y - y, vPos.z - z,  color.r, color.g, color.b,  0.0f, 0.0f, -1.0f,
            vPos.x - x, vPos.y - y, vPos.z - z,  color.r, color.g, color.b,  0.0f, 0.0f, -1.0f,

            // 오른쪽면 (x = +0.5, normal = 1, 0, 0)
            vPos.x + x, vPos.y + y, vPos.z - z,  color.r, color.g, color.b,  1.0f, 0.0f, 0.0f,
            vPos.x + x, vPos.y + y, vPos.z + z,  color.r, color.g, color.b,  1.0f, 0.0f, 0.0f,
            vPos.x + x, vPos.y - y, vPos.z + z,  color.r, color.g, color.b,  1.0f, 0.0f, 0.0f,
            vPos.x + x, vPos.y - y, vPos.z - z,  color.r, color.g, color.b,  1.0f, 0.0f, 0.0f,

            // 윗면 (y = +0.5, normal = 0, 1, 0)
            vPos.x + x, vPos.y + y, vPos.z - z,  color.r, color.g, color.b,  0.0f, 1.0f, 0.0f,
            vPos.x - x, vPos.y + y, vPos.z - z,  color.r, color.g, color.b,  0.0f, 1.0f, 0.0f,
            vPos.x - x, vPos.y + y, vPos.z + z,  color.r, color.g, color.b,  0.0f, 1.0f, 0.0f,
            vPos.x + x, vPos.y + y, vPos.z + z,  color.r, color.g, color.b,  0.0f, 1.0f, 0.0f,

            // 아랫면 (y = -0.5, normal = 0, -1, 0)
            vPos.x + x, vPos.y - y, vPos.z + z,  color.r, color.g, color.b,  0.0f, -1.0f, 0.0f,
            vPos.x - x, vPos.y - y, vPos.z + z,  color.r, color.g, color.b,  0.0f, -1.0f, 0.0f,
            vPos.x - x, vPos.y - y, vPos.z - z,  color.r, color.g, color.b,  0.0f, -1.0f, 0.0f,
            vPos.x + x, vPos.y - y, vPos.z - z,  color.r, color.g, color.b,  0.0f, -1.0f, 0.0f,
	};
	memcpy(vertices, temp, sizeof(temp));

    // VBO: 정점 데이터
    allocate(sizeof(vertices), GL_STATIC_DRAW);
    upload(vertices, sizeof(vertices));
    // 위치(0), 색상(1), 법선(2)
    setVertexAttrib(0, 3, GL_FLOAT, 9 * sizeof(GLfloat), (void*)0);
    setVertexAttrib(1, 3, GL_FLOAT, 9 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    setVertexAttrib(2, 3, GL_FLOAT, 9 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    // EBO: 인덱스 데이터
    allocateIndex(sizeof(indexes), GL_STATIC_DRAW);
    uploadIndex(indexes, sizeof(indexes));
    vertexCount = 36;
    setDrawMode(GL_TRIANGLES);
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