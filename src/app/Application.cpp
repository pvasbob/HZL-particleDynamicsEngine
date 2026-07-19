#include "app/Application.h"

#include "platform/GlfwWindow.h"
#include "renderer/Camera.h"
#include "renderer/OpenGLRenderer.h"
#include "scene/Container.h"
#include "simulation/ParticleFactory.h"
#include "simulation/ParticleSystem.h"

#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include <cstdlib>
#include <utility>
#include <vector>

namespace hzl::app
{
    namespace
    {
        void processInput(
            hzl::platform::GlfwWindow& window,
            hzl::renderer::Camera& camera,
            float deltaTime
        )
        {
            if (window.isKeyPressed(GLFW_KEY_ESCAPE))
            {
                window.requestClose();
            }

            constexpr float cameraSpeed = 2.0f;

            if (window.isKeyPressed(GLFW_KEY_W))
            {
                camera.moveForward(cameraSpeed * deltaTime);
            }

            if (window.isKeyPressed(GLFW_KEY_S))
            {
                camera.moveForward(-cameraSpeed * deltaTime);
            }

            if (window.isKeyPressed(GLFW_KEY_A))
            {
                camera.strafe(-cameraSpeed * deltaTime);
            }

            if (window.isKeyPressed(GLFW_KEY_D))
            {
                camera.strafe(cameraSpeed * deltaTime);
            }
        }
    }

    int Application::run()
    {
        hzl::platform::GlfwWindow window(
            1280,
            720,
            "HZL Particle Dynamics Engine"
        );

        if (!window.isValid())
        {
            return EXIT_FAILURE;
        }

        std::vector<hzl::simulation::Particle> particles =
            hzl::simulation::createInitialParticleGrid();

        hzl::renderer::OpenGLRenderer renderer;

        if (!renderer.initialize())
        {
            return EXIT_FAILURE;
        }

        const hzl::scene::Container container;

        hzl::simulation::ParticleSystem particleSystem(
            std::move(particles),
            container.physicsSettings()
        );

        hzl::renderer::Camera camera(
            glm::vec3(0.0f, 0.0f, 2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        float lastFrameTime = window.timeSeconds();

        constexpr float simulationStep = 1.0f / 120.0f;
        float simulationAccumulator = 0.0f;

        while (!window.shouldClose())
        {
            window.pollEvents();

            const float currentFrameTime = window.timeSeconds();
            const float deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;

            processInput(window, camera, deltaTime);

            simulationAccumulator += deltaTime;

            while (simulationAccumulator >= simulationStep)
            {
                particleSystem.update(simulationStep);
                simulationAccumulator -= simulationStep;
            }

            int framebufferWidth = 0;
            int framebufferHeight = 0;
            window.framebufferSize(framebufferWidth, framebufferHeight);

            renderer.render(
                camera,
                container,
                particleSystem.particles(),
                framebufferWidth,
                framebufferHeight,
                currentFrameTime
            );

            window.swapBuffers();
        }

        return EXIT_SUCCESS;
    }
}
