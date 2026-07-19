#include "renderer/OpenGLProgram.h"

#include <fstream>
#include <iostream>
#include <iterator>

namespace hzl::renderer
{
    OpenGLProgram::~OpenGLProgram()
    {
        destroy();
    }

    bool OpenGLProgram::loadFromFiles(
        const std::string& vertexShaderPath,
        const std::string& fragmentShaderPath
    )
    {
        destroy();

        const std::string vertexShaderSource = readTextFile(vertexShaderPath);
        const std::string fragmentShaderSource = readTextFile(fragmentShaderPath);

        if (vertexShaderSource.empty() || fragmentShaderSource.empty())
        {
            return false;
        }

        const GLuint vertexShader = compileShader(
            GL_VERTEX_SHADER,
            vertexShaderSource.c_str()
        );

        if (vertexShader == 0)
        {
            return false;
        }

        const GLuint fragmentShader = compileShader(
            GL_FRAGMENT_SHADER,
            fragmentShaderSource.c_str()
        );

        if (fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            return false;
        }

        program_ = linkShaderProgram(vertexShader, fragmentShader);

        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);

        return program_ != 0;
    }

    void OpenGLProgram::use() const
    {
        glUseProgram(program_);
    }

    void OpenGLProgram::setMatrix4(
        const char* uniformName,
        const float* values
    ) const
    {
        const GLint uniformLocation = glGetUniformLocation(program_, uniformName);
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, values);
    }

    std::string OpenGLProgram::readTextFile(const std::string& filePath)
    {
        std::ifstream file(filePath, std::ios::in | std::ios::binary);

        if (!file)
        {
            std::cerr << "Failed to open shader file: "
                      << filePath
                      << '\n';

            return {};
        }

        return std::string(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );
    }

    GLuint OpenGLProgram::compileShader(GLenum shaderType, const char* source)
    {
        const GLuint shader = glCreateShader(shaderType);

        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (compiled == GL_FALSE)
        {
            char log[1024];
            glGetShaderInfoLog(
                shader,
                static_cast<GLsizei>(sizeof(log)),
                nullptr,
                log
            );

            std::cerr << "Shader compilation failed:\n"
                      << log
                      << '\n';

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint OpenGLProgram::linkShaderProgram(
        GLuint vertexShader,
        GLuint fragmentShader
    )
    {
        const GLuint shaderProgram = glCreateProgram();

        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        GLint linked = GL_FALSE;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

        if (linked == GL_FALSE)
        {
            char log[1024];
            glGetProgramInfoLog(
                shaderProgram,
                static_cast<GLsizei>(sizeof(log)),
                nullptr,
                log
            );

            std::cerr << "Shader program linking failed:\n"
                      << log
                      << '\n';

            glDeleteProgram(shaderProgram);
            return 0;
        }

        return shaderProgram;
    }

    void OpenGLProgram::destroy()
    {
        if (program_ != 0)
        {
            glDeleteProgram(program_);
            program_ = 0;
        }
    }
}
