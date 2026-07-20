#include <cmath>
#include <numbers>
#include <vector>
#include <cstddef>
#include <vector>

#include "renderer/OpenGLMesh.h"

namespace hzl::renderer
{
    namespace
    {

        struct MeshVertex
        {
            glm::vec3 position;
            glm::vec3 normal;
        };


        void addTriangle(
            std::vector<MeshVertex>& vertices,
            const glm::vec3& normal,
            const glm::vec3& first,
            const glm::vec3& second,
            const glm::vec3& third

        )
        {
            vertices.push_back({first, normal});
            vertices.push_back({second, normal});
            vertices.push_back({third, normal});
        }

    }

    OpenGLMesh::~OpenGLMesh()
    {
        destroy();
    }

    bool OpenGLMesh::createCube() 
    {
        constexpr float halfExtent = 0.5f;

        std::vector<MeshVertex> vertices;
        vertices.reserve(36);

        // Back face: negative Z.
        addTriangle(
            vertices, glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(-halfExtent, -halfExtent, -halfExtent),
            glm::vec3( halfExtent,  halfExtent, -halfExtent),
            glm::vec3( halfExtent, -halfExtent, -halfExtent)
        );
        addTriangle(
            vertices, glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3( halfExtent,  halfExtent, -halfExtent),
            glm::vec3(-halfExtent, -halfExtent, -halfExtent),
            glm::vec3(-halfExtent,  halfExtent, -halfExtent)
        );
    
        // Front face: positive Z.
        addTriangle(
            vertices, glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(-halfExtent, -halfExtent, halfExtent),
            glm::vec3( halfExtent, -halfExtent, halfExtent),
            glm::vec3( halfExtent,  halfExtent, halfExtent)
        );
        addTriangle(
            vertices, glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3( halfExtent,  halfExtent, halfExtent),
            glm::vec3(-halfExtent,  halfExtent, halfExtent),
            glm::vec3(-halfExtent, -halfExtent, halfExtent)
        );
    
        // Left face: negative X.
        addTriangle(
            vertices, glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-halfExtent,  halfExtent,  halfExtent),
            glm::vec3(-halfExtent,  halfExtent, -halfExtent),
            glm::vec3(-halfExtent, -halfExtent, -halfExtent)
        );
        addTriangle(
            vertices, glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-halfExtent, -halfExtent, -halfExtent),
            glm::vec3(-halfExtent, -halfExtent,  halfExtent),
            glm::vec3(-halfExtent,  halfExtent,  halfExtent)
        );
    
        // Right face: positive X.
        addTriangle(
            vertices, glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(halfExtent,  halfExtent,  halfExtent),
            glm::vec3(halfExtent, -halfExtent, -halfExtent),
            glm::vec3(halfExtent,  halfExtent, -halfExtent)
        );
        addTriangle(
            vertices, glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(halfExtent, -halfExtent, -halfExtent),
            glm::vec3(halfExtent,  halfExtent,  halfExtent),
            glm::vec3(halfExtent, -halfExtent,  halfExtent)
        );
    
        // Bottom face: negative Y.
        addTriangle(
            vertices, glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(-halfExtent, -halfExtent, -halfExtent),
            glm::vec3( halfExtent, -halfExtent, -halfExtent),
            glm::vec3( halfExtent, -halfExtent,  halfExtent)
        );
        addTriangle(
            vertices, glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3( halfExtent, -halfExtent,  halfExtent),
            glm::vec3(-halfExtent, -halfExtent,  halfExtent),
            glm::vec3(-halfExtent, -halfExtent, -halfExtent)
        );
    
        // Top face: positive Y.
        addTriangle(
            vertices, glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(-halfExtent, halfExtent, -halfExtent),
            glm::vec3( halfExtent, halfExtent,  halfExtent),
            glm::vec3( halfExtent, halfExtent, -halfExtent)
        );
        addTriangle(
            vertices, glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3( halfExtent, halfExtent,  halfExtent),
            glm::vec3(-halfExtent, halfExtent, -halfExtent),
            glm::vec3(-halfExtent, halfExtent,  halfExtent)
        );

        return create(
            vertices.data(),
            static_cast<GLsizeiptr>(vertices.size() * sizeof(MeshVertex)),
            static_cast<GLsizei>(vertices.size()),
            static_cast<GLsizei>(sizeof(MeshVertex)),
            GL_STATIC_DRAW
        );
        
    }


    bool OpenGLMesh::createSphere(
    int latitudeSegments,
    int longitudeSegments
    )
    {
        std::vector<MeshVertex> vertices;
    
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
            
                vertices.push_back({topLeft, topLeft});
                vertices.push_back({bottomLeft, bottomLeft});
                vertices.push_back({topRight, topRight});
            
                vertices.push_back({topRight, topRight});
                vertices.push_back({bottomLeft, bottomLeft});
                vertices.push_back({bottomRight, bottomRight});
            }
        }
    
        return create(
            vertices.data(),
            static_cast<GLsizeiptr>(vertices.size() * sizeof(MeshVertex)),
            static_cast<GLsizei>(vertices.size()),
            static_cast<GLsizei>(sizeof(MeshVertex)),
            GL_STATIC_DRAW
        );
    }
    

    void OpenGLMesh::draw(GLenum primitive) const
    {
        glBindVertexArray(vertexArray_);
        glDrawArrays(primitive, 0, vertexCount_);
    }

    void OpenGLMesh::setInstancePositionBuffer(GLuint positionBuffer)
    {
        glBindVertexArray(vertexArray_);
        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);

        glVertexAttribPointer(
            2,
            3,
            GL_FLOAT,
            GL_FALSE,
            3 * sizeof(float),
            nullptr
        );
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void OpenGLMesh::setInstanceColorBuffer(GLuint colorBuffer)
    {
        glBindVertexArray(vertexArray_);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);

        glVertexAttribPointer(
            3,
            3,
            GL_FLOAT,
            GL_FALSE,
            3 * sizeof(float),
            nullptr
        );
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void OpenGLMesh::drawInstanced(
        GLenum primitive,
        GLsizei instanceCount
    ) const
    {
        glBindVertexArray(vertexArray_);
        glDrawArraysInstanced(primitive, 0, vertexCount_, instanceCount);
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
            reinterpret_cast<void*>(offsetof(MeshVertex, position))
        );
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<void*>(offsetof(MeshVertex, normal))
        );
        glEnableVertexAttribArray(1);


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
