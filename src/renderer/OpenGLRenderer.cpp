#include "renderer/OpenGLRenderer.h"

#include "renderer/Camera.h"
#include "scene/Container.h"
#include "simulation/Particle.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace hzl::renderer
{
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
        const std::vector<hzl::simulation::Particle>& particles,
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
        cubeMesh_.draw(GL_TRIANGLES);


        const float particleRadius = 
            container.physicsSettings().particleRadius;
        
        for (const hzl::simulation::Particle& particle: particles)
        {
            const glm::mat4 particleModelMatrix = glm::scale(
                glm::translate(
                    glm::mat4(1.0f),
                    particle.position
                ),
                glm::vec3(particleRadius)
            );

            program_.setMatrix4(
                "uModel",
                glm::value_ptr(particleModelMatrix)
            );

            program_.setMatrix4(
                "uMvp",
                glm::value_ptr(
                    projectionMatrix *
                    viewMatrix*
                    particleModelMatrix
                )
            );

            particleSphereMesh_.draw(GL_TRIANGLES);
        }
    }

}
