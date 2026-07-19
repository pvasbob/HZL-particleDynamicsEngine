#include "renderer/OpenGLMesh.h"

namespace hzl::renderer
{
    namespace
    {
        constexpr float cubeVertices[]
        {
            -0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f
        };
    }

    OpenGLMesh::~OpenGLMesh()
    {
        destroy();
    }

    bool OpenGLMesh::createCube()
    {
        return create(
            cubeVertices,
            static_cast<GLsizeiptr>(sizeof(cubeVertices)),
            36,
            3 * static_cast<GLsizei>(sizeof(float)),
            GL_STATIC_DRAW
        );
    }

    bool OpenGLMesh::createDynamicPoints(const std::vector<glm::vec3>& positions)
    {
        return create(
            positions.data(),
            static_cast<GLsizeiptr>(positions.size() * sizeof(glm::vec3)),
            static_cast<GLsizei>(positions.size()),
            static_cast<GLsizei>(sizeof(glm::vec3)),
            GL_DYNAMIC_DRAW
        );
    }

    void OpenGLMesh::updatePoints(const std::vector<glm::vec3>& positions)
    {
        vertexCount_ = static_cast<GLsizei>(positions.size());

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            static_cast<GLsizeiptr>(positions.size() * sizeof(glm::vec3)),
            positions.data()
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void OpenGLMesh::draw(GLenum primitive) const
    {
        glBindVertexArray(vertexArray_);
        glDrawArrays(primitive, 0, vertexCount_);
    }

    bool OpenGLMesh::create(
        const void* vertexData,
        GLsizeiptr byteSize,
        GLsizei vertexCount,
        GLsizei stride,
        GLenum usage
    )
    {
        destroy();

        glGenVertexArrays(1, &vertexArray_);
        glGenBuffers(1, &vertexBuffer_);

        glBindVertexArray(vertexArray_);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);

        glBufferData(GL_ARRAY_BUFFER, byteSize, vertexData, usage);

        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            stride,
            nullptr
        );
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        vertexCount_ = vertexCount;
        return true;
    }

    void OpenGLMesh::destroy()
    {
        if (vertexBuffer_ != 0)
        {
            glDeleteBuffers(1, &vertexBuffer_);
            vertexBuffer_ = 0;
        }

        if (vertexArray_ != 0)
        {
            glDeleteVertexArrays(1, &vertexArray_);
            vertexArray_ = 0;
        }

        vertexCount_ = 0;
    }
}
