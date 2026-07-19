#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <utility>
#include <string>

#include <iostream>
#include <vector>
#include <cstddef>

#include "renderer/OpenGLShader.h"
#include "renderer/Camera.h"
#include "scene/Container.h"
#include "simulation/Particle.h"
#include "simulation/ParticleSystem.h"

using hzl::simulation::Particle;


void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    std::cout << "width: " << width << "  " <<  "height: " << height << '\n';
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


    // constexpr float triangleVertices[] = {
    //      0.0f,  0.6f, 0.0f,
    //     -0.6f, -0.5f, 0.0f,
    //      0.6f, -0.5f, 0.0f
    // };


    constexpr float cubeVertices[]
    {
        // Back face
        -0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        // Front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        // Left face
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        // Right face
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,

        // Bottom face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        // Top face
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f
    };


    GLuint vertexBuffer = 0;

    GLuint vertexArray = 0;
    glGenVertexArrays(1, &vertexArray);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(cubeVertices),
        cubeVertices,
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

    const std::string vertexShaderSource = 
        hzl::renderer::readTextFile(
            "assets/shaders/particle.vert"
        );

    const std::string fragmentShaderSource = 
        hzl::renderer::readTextFile(
            "assets/shaders/particle.frag"
        );


    const GLuint vertexShader = hzl::renderer::compileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());

    if (vertexShader == 0)
    {
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

    const GLuint fragmentShader = hzl::renderer::compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());

    if (fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

    const GLuint shaderProgram = hzl::renderer::linkShaderProgram(
        vertexShader,
        fragmentShader
    );

    if (shaderProgram == 0)
    {
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


    hzl::renderer::Camera camera(
        glm::vec3(0.0f, 0.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    
    // std::vector<Particle> particles
    // {
        // {
            // glm::vec3(-0.25f, 0.35f, 0.80f),
            // glm::vec3( 0.20f, 0.00f, 0.00f)
        // },
        // {
            // glm::vec3( 0.25f, 0.35f, 0.80f),
            // glm::vec3(-0.20f, 0.00f, 0.00f)
        // },
        // {
            // glm::vec3(0.00f, 0.75f, 0.80f),
            // glm::vec3(0.00f, 0.00f, 0.00f)
        // }
    // };

    std::vector<Particle> particles;
    // 
    constexpr int particleCountX = 5;         
    constexpr int particleCountY = 4;     
    constexpr int particleCountZ = 3;     
    constexpr float initialSpacing = 0.23f;    

    particles.reserve(
        particleCountX * 
        particleCountY *
        particleCountZ
    );

    for (int yIndex = 0; yIndex < particleCountY; ++yIndex)
    {
        for (int zIndex = 0; zIndex < particleCountZ; ++zIndex)
        {
            for (int xIndex = 0; xIndex < particleCountX; ++xIndex)
            {
                const glm::vec3 position(
                    (xIndex - 2) * initialSpacing,
                    0.20f + yIndex * initialSpacing,
                    0.50f + zIndex * initialSpacing
                );

                const glm::vec3 velocity(
                    -position.x * 1.5f,
                    0.0f,
                    -(position.z - 0.70f) * 0.40f
                );

                particles.push_back({position, velocity});
            }
        }
    }

    std::vector<glm::vec3> particlePositions;
    particlePositions.reserve(particles.size());

    for (const Particle& particle: particles)
    {
        particlePositions.push_back(particle.position);
    }

    GLuint particleVertexArray = 0;
    GLuint particleVertexBuffer = 0;

    glGenVertexArrays(1, &particleVertexArray);
    glGenBuffers(1, &particleVertexBuffer);

    glBindVertexArray(particleVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            particlePositions.size() * sizeof(glm::vec3)
        ),
        particlePositions.data(),
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0, 
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        nullptr
    );
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float lastFrameTime = static_cast<float>(glfwGetTime());

    constexpr float simulationStep = 1.0f/120.0f;
    float simulationAccumulator = 0.0f;

    const hzl::scene::Container container;

    hzl::simulation::ParticleSystem particleSystem(
        std::move(particles),
        container.physicsSettings()
    );


    while(glfwWindowShouldClose(window) == GLFW_FALSE)
    {
        glfwPollEvents();

        const float currentFrameTime = static_cast<float>(glfwGetTime());
        const float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        const float cameraSpeed = 2.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.moveForward(cameraSpeed * deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.moveForward(-cameraSpeed * deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.strafe(-cameraSpeed * deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.strafe(cameraSpeed * deltaTime);
        }

        simulationAccumulator += deltaTime;

        while (simulationAccumulator >= simulationStep)
        {
            particleSystem.update(simulationStep);

            simulationAccumulator -= simulationStep;
        }

        particlePositions.clear();

        for (const Particle& particle : particleSystem.particles())
        {
            particlePositions.push_back(particle.position);
        }

        glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            static_cast<GLsizeiptr>(particlePositions.size() * sizeof(glm::vec3)),
            particlePositions.data()
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);



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

        const float elapsedSeconds = currentFrameTime;

        glm::mat4 modelMatrix(1.0f);

        modelMatrix = glm::rotate(
            modelMatrix,
            elapsedSeconds,
            glm::vec3(1.0f, 1.0f, 0.5f)
        );

        const glm::mat4 viewMatrix = camera.viewMatrix();

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
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render floor
        const glm::mat4 floorMvp =
            projectionMatrix * viewMatrix * container.floorModelMatrix();
        glUniformMatrix4fv(
            mvpLocation,
            1, GL_FALSE,
            glm::value_ptr(floorMvp)
        );
        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render leftWall
        const glm::mat4 leftWallMvp = 
            projectionMatrix * viewMatrix * container.leftWallModelMatrix();
        
        glUniformMatrix4fv(
            mvpLocation,
            1,
            GL_FALSE,
            glm::value_ptr(leftWallMvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render rightWall
        const glm::mat4 rightWallMvp = 
            projectionMatrix * viewMatrix * container.rightWallModelMatrix();
        
        glUniformMatrix4fv(
            mvpLocation,
            1, 
            GL_FALSE,
            glm::value_ptr(rightWallMvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render bakcWall
        const glm::mat4 backWallMvp = 
            projectionMatrix * viewMatrix * container.backWallModelMatrix();

        glUniformMatrix4fv(
            mvpLocation,
            1,
            GL_FALSE,
            glm::value_ptr(backWallMvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render particles.
        const glm::mat4 particleMvp = projectionMatrix * viewMatrix;

        glUniformMatrix4fv(
            mvpLocation,
            1,
            GL_FALSE,
            glm::value_ptr(particleMvp)
        );

        glPointSize(12.0f);
        glBindVertexArray(particleVertexArray);
        glDrawArrays(
            GL_POINTS,
            0,
            static_cast<GLsizei>(particleSystem.particles().size())
        );

        glfwSwapBuffers(window);
    }

    glDeleteProgram(shaderProgram);

    glDeleteBuffers(1, &particleVertexBuffer);
    glDeleteVertexArrays(1, &particleVertexArray);

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArray);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
