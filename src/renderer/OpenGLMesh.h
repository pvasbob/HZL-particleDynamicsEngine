#pragma once

#include <glad/gl.h>

#include <glm/vec3.hpp>

#include <vector>

namespace hzl::renderer
{
    class OpenGLMesh
    {
    public:
        OpenGLMesh() = default;
        ~OpenGLMesh();

        OpenGLMesh(const OpenGLMesh&) = delete;
        OpenGLMesh& operator=(const OpenGLMesh&) = delete;

        bool createCube();
        bool createSphere(int latitudeSegments, int longitudeSegments);
        void draw(GLenum primitive) const;

    private:
        bool create(
            const void* vertexData,
            GLsizeiptr byteSize,
            GLsizei vertexCount,
            GLsizei stride,
            GLenum usage
        );

        void destroy();

        GLuint vertexArray_ = 0;
        GLuint vertexBuffer_ = 0;
        GLsizei vertexCount_ = 0;
    };
}
