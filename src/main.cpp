#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>

#include <iostream>


void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    std::cout << "width: " << width << "  " <<  "height: " << height << '\n';
}


constexpr const char* vertexShaderSource = R"(
    #version 330 core

    layout (location = 0) in vec3 aPosition;
    uniform mat4 uMvp;

    void main() 
    {
        gl_Position = uMvp * vec4(aPosition, 1.0);
    }

)";

constexpr const char* fragmentShaderSource = R"(
    #version 330 core

    out vec4 fragmentColor;

    void main()
    {
        fragmentColor = vec4(0.20, 0.75, 1.00, 1.0);
    }

)";


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


int main() 
{
    if (glfwInit() == GLFW_FALSE)
    {
        std::cerr << "Failed to initialize GLFW. \n";
        return EXIT_FAILURE; 
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "HZL Particle Dynamics Engine",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window. \n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to load OpenGL functions. \n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "Loaded OpenGL "
                << GLAD_VERSION_MAJOR(version) 
                << "."
                << GLAD_VERSION_MINOR(version) 
                << "\n";            

    glEnable(GL_DEPTH_TEST);


    constexpr float triangleVertices[] = {
         0.0f,  0.6f, 0.0f,
        -0.6f, -0.5f, 0.0f,
         0.6f, -0.5f, 0.0f
    };


    GLuint vertexBuffer = 0;

    GLuint vertexArray = 0;
    glGenVertexArrays(1, &vertexArray);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(triangleVertices),
        triangleVertices,
        GL_STATIC_DRAW
    );


    glBindVertexArray(vertexArray);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3*sizeof(float),
        nullptr
    );
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);

    if (vertexShader == 0)
    {
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

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
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    const GLint mvpLocation = glGetUniformLocation(shaderProgram, "uMvp");
    // const glm::mat4 mvp(1.0f);

    while(glfwWindowShouldClose(window) == GLFW_FALSE)
    {
        glfwPollEvents();

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth , &framebufferHeight);

        if (framebufferWidth == 0 || framebufferHeight == 0)
        {
            continue;
        }


        glClearColor(0.04f, 0.07f, 0.12f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        const float elapsedSeconds = static_cast<float>(glfwGetTime());


        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::rotate(
            modelMatrix,
            elapsedSeconds,
            glm::vec3(1.0f, 1.0f, 1.0f)
        );

        glm::mat4 viewMatrix = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.0f, 0.0f, -2.0f)
        );

        const float aspectRatio = 
        static_cast<float>(framebufferWidth)/ 
        static_cast<float>(framebufferHeight);

        glm::mat4 projectionMatrix = glm::perspective(
            glm::radians(45.0f),
            aspectRatio,
            0.1f,
            100.0f
        );

        glm::mat4 mvp =   
            projectionMatrix *
            viewMatrix *
            modelMatrix;

        glUniformMatrix4fv(
            mvpLocation,
            1, 
            GL_FALSE,
            glm::value_ptr(mvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(shaderProgram);

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArray);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}