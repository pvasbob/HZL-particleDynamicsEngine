#include "renderer/OpenGLShader.h"

#include <iostream>
#include <fstream>
#include <iterator>

namespace hzl::renderer{

    GLuint compileShader(GLenum shaderType, const char* source)
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

    GLuint linkShaderProgram(GLuint vertexShader, GLuint fragmentShader)
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

    std::string readTextFile(const std::string& filePath)
    {
        std::ifstream file(
            filePath,
            std::ios::in | std::ios::binary
        );

        if(!file)
        {
            std::cerr 
                << "Failed to open shader file: "
                << filePath 
                << '\n';
            
            return {};
        }

        return std::string(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );
    }
}


