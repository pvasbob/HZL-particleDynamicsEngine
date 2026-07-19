#include <cmath>
#include <numbers>
#include <vector>

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


    bool OpenGLMesh::createSphere(
    int latitudeSegments,
    int longitudeSegments
    )
    {
        std::vector<glm::vec3> vertices;
    
        for (int latitudeIndex = 0;
             latitudeIndex < latitudeSegments;
             ++latitudeIndex)
        {
            const float latitude0 =
                -std::numbers::pi_v<float> * 0.5f +
                std::numbers::pi_v<float> *
                static_cast<float>(latitudeIndex) /
                static_cast<float>(latitudeSegments);
        
            const float latitude1 =
                -std::numbers::pi_v<float> * 0.5f +
                std::numbers::pi_v<float> *
                static_cast<float>(latitudeIndex + 1) /
                static_cast<float>(latitudeSegments);
        
            for (int longitudeIndex = 0;
                 longitudeIndex < longitudeSegments;
                 ++longitudeIndex)
            {
                const float longitude0 =
                    2.0f * std::numbers::pi_v<float> *
                    static_cast<float>(longitudeIndex) /
                    static_cast<float>(longitudeSegments);
            
                const float longitude1 =
                    2.0f * std::numbers::pi_v<float> *
                    static_cast<float>(longitudeIndex + 1) /
                    static_cast<float>(longitudeSegments);
            
                const glm::vec3 topLeft(
                    std::cos(latitude1) * std::cos(longitude0),
                    std::sin(latitude1),
                    std::cos(latitude1) * std::sin(longitude0)
                );
            
                const glm::vec3 topRight(
                    std::cos(latitude1) * std::cos(longitude1),
                    std::sin(latitude1),
                    std::cos(latitude1) * std::sin(longitude1)
                );
            
                const glm::vec3 bottomLeft(
                    std::cos(latitude0) * std::cos(longitude0),
                    std::sin(latitude0),
                    std::cos(latitude0) * std::sin(longitude0)
                );
            
                const glm::vec3 bottomRight(
                    std::cos(latitude0) * std::cos(longitude1),
                    std::sin(latitude0),
                    std::cos(latitude0) * std::sin(longitude1)
                );
            
                vertices.push_back(topLeft);
                vertices.push_back(bottomLeft);
                vertices.push_back(topRight);
            
                vertices.push_back(topRight);
                vertices.push_back(bottomLeft);
                vertices.push_back(bottomRight);
            }
        }
    
        return create(
            vertices.data(),
            static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3)),
            static_cast<GLsizei>(vertices.size()),
            static_cast<GLsizei>(sizeof(glm::vec3)),
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
