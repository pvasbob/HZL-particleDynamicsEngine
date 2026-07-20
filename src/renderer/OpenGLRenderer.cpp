#include "renderer/OpenGLRenderer.h"

#include "renderer/Camera.h"
#include "scene/Container.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace hzl::renderer
{
    OpenGLRenderer::~OpenGLRenderer()
    {
        destroyParticleColorBuffer();
    }

    bool OpenGLRenderer::initialize()
    {
        const int version = gladLoadGL(glfwGetProcAddress);

        if (version == 0)
        {
            return false;
        }

        glEnable(GL_DEPTH_TEST);

        if (!program_.loadFromFiles(
                "assets/shaders/particle.vert",
                "assets/shaders/particle.frag"))
        {
            return false;
        }

        if (!cubeMesh_.createCube())
        {
            return false;
        }

        return particleSphereMesh_.createSphere(12, 18);
    }

    void OpenGLRenderer::render(
        const Camera& camera,
        const hzl::scene::Container& container,
        GLuint particlePositionBuffer,
        GLsizei particleCount,
        int framebufferWidth,
        int framebufferHeight,
        float elapsedSeconds
    )
    {
        if (framebufferWidth == 0 || framebufferHeight == 0)
        {
            return;
        }

        glClearColor(0.04f, 0.07f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program_.use();
        program_.setBoolean("uUseInstancePositions", false);
        program_.setBoolean("uUseInstanceColors", false);

        const glm::mat4 modelMatrix = glm::rotate(
            glm::mat4(1.0f),
            elapsedSeconds,
            glm::vec3(1.0f, 1.0f, 0.5f)
        );

        const glm::mat4 viewMatrix = camera.viewMatrix();
        const float aspectRatio =
            static_cast<float>(framebufferWidth) /
            static_cast<float>(framebufferHeight);

        const glm::mat4 projectionMatrix = glm::perspective(
            glm::radians(45.0f),
            aspectRatio,
            0.1f,
            100.0f
        );

        // rotating cube
        program_.setMatrix4(
            "uModel",
            glm::value_ptr(modelMatrix)
        );

        program_.setMatrix4(
            "uMvp",
            glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix)
        );

        program_.setVector3(
            "uMaterialColor",
            glm::vec3(0.95f, 0.40f, 0.15f)
        );
        cubeMesh_.draw(GL_TRIANGLES);

        // Floor
        program_.setMatrix4(
            "uModel",
            glm::value_ptr(container.floorModelMatrix())
        );
        program_.setMatrix4(
            "uMvp",
            glm::value_ptr(
                projectionMatrix * viewMatrix * container.floorModelMatrix()
            )
        );

        program_.setVector3(
            "uMaterialColor",
            glm::vec3(0.18f, 0.20f, 0.25f)
        );
        cubeMesh_.draw(GL_TRIANGLES);

        // leftWall
        program_.setMatrix4(
            "uModel",
            glm::value_ptr(container.leftWallModelMatrix())
        );
        program_.setMatrix4(
            "uMvp",
            glm::value_ptr(
                projectionMatrix * viewMatrix * container.leftWallModelMatrix()
            )
        );
        program_.setVector3(
            "uMaterialColor",
            glm::vec3(0.16f, 0.32f, 0.45f)
        );
        
        cubeMesh_.draw(GL_TRIANGLES);
        
        // rightWall
        program_.setMatrix4(
            "uModel",
            glm::value_ptr(container.rightWallModelMatrix())
        );
        program_.setMatrix4(
            "uMvp",
            glm::value_ptr(
                projectionMatrix * viewMatrix * container.rightWallModelMatrix()
            )
        );
        
        
        program_.setVector3(
            "uMaterialColor",
            glm::vec3(0.16f, 0.32f, 0.45f)
        );
        
        cubeMesh_.draw(GL_TRIANGLES);
        
        // backWall
        program_.setMatrix4(
            "uModel",
            glm::value_ptr(container.backWallModelMatrix())
        );
        program_.setMatrix4(
            "uMvp",
            glm::value_ptr(
                projectionMatrix * viewMatrix * container.backWallModelMatrix()
            )
        );

        program_.setVector3(
            "uMaterialColor",
            glm::vec3(0.16f, 0.32f, 0.45f)
        );

        cubeMesh_.draw(GL_TRIANGLES);


        const float particleRadius =
            container.physicsSettings().particleRadius;

        ensureParticleColorBuffer(particleCount);

        program_.setBoolean("uUseInstancePositions", true);
        program_.setBoolean("uUseInstanceColors", true);
        program_.setFloat("uParticleRadius", particleRadius);
        program_.setMatrix4(
            "uModel",
            glm::value_ptr(glm::mat4(1.0f))
        );
        program_.setMatrix4(
            "uMvp",
            glm::value_ptr(projectionMatrix * viewMatrix)
        );
        program_.setVector3(
            "uMaterialColor",
            glm::vec3(0.20f, 0.75f, 1.00f)
        );

        particleSphereMesh_.setInstancePositionBuffer(particlePositionBuffer);
        particleSphereMesh_.setInstanceColorBuffer(particleColorBuffer_);
        particleSphereMesh_.drawInstanced(GL_TRIANGLES, particleCount);
    }

    void OpenGLRenderer::ensureParticleColorBuffer(GLsizei particleCount)
    {
        if (particleCount <= particleColorCapacity_)
        {
            return;
        }

        std::vector<glm::vec3> colors;
        colors.reserve(particleCount);

        for (GLsizei particleIndex = 0;
             particleIndex < particleCount;
             ++particleIndex)
        {
            const bool belongsToLeftGroup =
                particleIndex < particleCount / 2;

            colors.emplace_back(
                belongsToLeftGroup
                    ? glm::vec3(1.0f, 0.20f, 0.25f)
                    : glm::vec3(0.10f, 0.35f, 1.0f)
            );
        }

        if (particleColorBuffer_ == 0)
        {
            glGenBuffers(1, &particleColorBuffer_);
        }

        glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer_);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(colors.size() * sizeof(glm::vec3)),
            colors.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        particleColorCapacity_ = particleCount;
    }

    void OpenGLRenderer::destroyParticleColorBuffer()
    {
        if (particleColorBuffer_ != 0)
        {
            glDeleteBuffers(1, &particleColorBuffer_);
            particleColorBuffer_ = 0;
        }

        particleColorCapacity_ = 0;
    }

}
