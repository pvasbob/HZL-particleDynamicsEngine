#pragma once

#include "renderer/OpenGLMesh.h"
#include "renderer/OpenGLProgram.h"
#include "simulation/Particle.h"

#include <glm/vec3.hpp>

#include <vector>

namespace hzl::renderer
{
    class Camera;
}

namespace hzl::scene
{
    class Container;
}

namespace hzl::renderer
{
    class OpenGLRenderer
    {
    public:
        bool initialize();

        void render(
            const Camera& camera,
            const hzl::scene::Container& container,
            const std::vector<hzl::simulation::Particle>& particles,
            int framebufferWidth,
            int framebufferHeight,
            float elapsedSeconds
        );

    private:
        OpenGLProgram program_;
        OpenGLMesh cubeMesh_;
        OpenGLMesh particleSphereMesh_;
    };
}
