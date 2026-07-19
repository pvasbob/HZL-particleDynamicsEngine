#pragma once

#include <glad/gl.h>

#include <string>

namespace hzl::renderer
{
    class OpenGLProgram
    {
    public:
        OpenGLProgram() = default;
        ~OpenGLProgram();

        OpenGLProgram(const OpenGLProgram&) = delete;
        OpenGLProgram& operator=(const OpenGLProgram&) = delete;

        bool loadFromFiles(
            const std::string& vertexShaderPath,
            const std::string& fragmentShaderPath
        );

        void use() const;
        void setMatrix4(const char* uniformName, const float* values) const;

    private:
        static std::string readTextFile(const std::string& filePath);
        static GLuint compileShader(GLenum shaderType, const char* source);
        static GLuint linkShaderProgram(GLuint vertexShader, GLuint fragmentShader);

        void destroy();

        GLuint program_ = 0;
    };
}
