#include "Obstacle.h"

namespace
{
    constexpr GLfloat Height = 3.0f;
    constexpr int FloatsPerVertex = 9;   // 위치 3 + 색상 3 + 법선 3
    constexpr int VerticesPerBox = 24;
    constexpr int IndicesPerBox = 36;

    struct Face
    {
        int corners[4];
        glm::vec3 normal;
    };

    // 각 면을 바깥에서 봤을 때 반시계 방향
    // 면마다 노말 설정하는거 때문에 있는겨
    const Face Faces[] = {
        {{6, 7, 3, 2}, { 0.0f,  0.0f,  1.0f}},  // +Z
        {{7, 4, 0, 3}, {-1.0f,  0.0f,  0.0f}},  // -X
        {{4, 5, 1, 0}, { 0.0f,  0.0f, -1.0f}},  // -Z
        {{5, 6, 2, 1}, { 1.0f,  0.0f,  0.0f}},  // +X
        {{5, 4, 7, 6}, { 0.0f,  1.0f,  0.0f}},  // +Y
        {{2, 3, 0, 1}, { 0.0f, -1.0f,  0.0f}},  // -Y
    };

    constexpr GLuint FaceIndices[] = { 0, 1, 2, 0, 2, 3 };
}

Obstacle::Obstacle()
{
    GLfloat vertices[Count * VerticesPerBox * FloatsPerVertex]; // 9 * 24 * 36 짜리 정점 배열 만들기 <- 전체 장애물 다 넣는거임
    GLuint indices[Count * IndicesPerBox]; // 인덱스도 정점 배열처럼 전체 장애물 다 넣음

    vColor = glm::vec3(0.45f, 0.85f, 0.85f);
    GLuint vertexCount = 0;
    int indexCount = 0;

    // 장애물 개수 9번 만큼 반복
    for (int i = 0; i < Count; ++i)
    {
        // 가로 세로 크기
        const glm::vec2 halfSize = Sizes[i] * 0.5f;
        const GLfloat x = halfSize.x;
        const GLfloat z = halfSize.y;

        // 모서리 정점
        const glm::vec3 corners[] = {
            {-x, 0.0f, -z}, { x, 0.0f, -z},
            { x, 0.0f,  z}, {-x, 0.0f,  z},
            {-x, Height, -z}, { x, Height, -z},
            { x, Height,  z}, {-x, Height,  z},
        };

        // 장애물 회전값
        const glm::mat3 rotation(glm::rotate(
            glm::mat4(1.0f),
            glm::radians(YawDegrees[i]),
            glm::vec3(0.0f, 1.0f, 0.0f)
        ));

        for (const Face& face : Faces)
        {
            const GLuint baseVertex = vertexCount;
            const glm::vec3 normal = rotation * face.normal; // 법선 회전

            for (int corner : face.corners)
            {
                const glm::vec3 position = Positions[i] + rotation * corners[corner]; // 모서리들 회전 먼저하고 배치
                const GLuint offset = vertexCount * FloatsPerVertex; // 점 하나가 배열칸 9개를 차지함 그래서 offset으로 하나씩 넣어줌. 

                vertices[offset + 0] = position.x;
                vertices[offset + 1] = position.y;
                vertices[offset + 2] = position.z;
                vertices[offset + 3] = vColor.r;
                vertices[offset + 4] = vColor.g;
                vertices[offset + 5] = vColor.b;
                vertices[offset + 6] = normal.x;
                vertices[offset + 7] = normal.y;
                vertices[offset + 8] = normal.z;

                ++vertexCount;
            }

            // baseVertex 기준으로 삼각형 두개씩 해서 인덱스 만드는거
            // 
            for (GLuint index : FaceIndices)
            {
                indices[indexCount++] = baseVertex + index;
            }
        }
    }

    allocate(sizeof(vertices), GL_STATIC_DRAW);
    upload(vertices, sizeof(vertices));

    constexpr GLsizei stride = FloatsPerVertex * sizeof(GLfloat);
    setVertexAttrib(0, 3, GL_FLOAT, stride, nullptr);
    setVertexAttrib(1, 3, GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat)));
    setVertexAttrib(2, 3, GL_FLOAT, stride, (void*)(6 * sizeof(GLfloat)));

    allocateIndex(sizeof(indices), GL_STATIC_DRAW);
    uploadIndex(indices, sizeof(indices));

    setDrawMode(GL_TRIANGLES);
}

GLvoid Obstacle::draw(
    const GLuint& ShaderID,
    const glm::mat4& worldMatrix,
    const glm::mat4& viewProjMatrix)
{
    glBindVertexArray(VAO);

    const glm::mat4 model = worldMatrix * modelMatrix;
    const glm::mat4 transform = viewProjMatrix * model;

    glUniformMatrix4fv(
        glGetUniformLocation(ShaderID, "model"),
        1, GL_FALSE, glm::value_ptr(model)
    );
    glUniformMatrix4fv(
        glGetUniformLocation(ShaderID, "Transform"),
        1, GL_FALSE, glm::value_ptr(transform)
    );

    glDrawElements(DrawMode, Count * IndicesPerBox, GL_UNSIGNED_INT, nullptr);
}

GLvoid Obstacle::update()
{
}