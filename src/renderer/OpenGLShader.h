#pragma once
#include <glad/gl.h>
#include <string>

namespace hzl::renderer 
{
    GLuint compileShader(GLenum shaderType, const char* source);
    GLuint linkShaderProgram(GLuint vertexShader, GLuint fragmentShader);

    std::string readTextFile(const std::string& filePath);
}
