#pragma once

#include "renderer/OpenGLMesh.h"
#include "renderer/OpenGLProgram.h"

#include <glad/gl.h>

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
        ~OpenGLRenderer();

        bool initialize();

        void render(
            const Camera& camera,
            const hzl::scene::Container& container,
            GLuint particlePositionBuffer,
            GLsizei particleCount,
            int framebufferWidth,
            int framebufferHeight,
            float elapsedSeconds
        );

    private:
        void ensureParticleColorBuffer(GLsizei particleCount);
        void destroyParticleColorBuffer();

        OpenGLProgram program_;
        OpenGLMesh cubeMesh_;
        OpenGLMesh particleSphereMesh_;
        GLuint particleColorBuffer_ = 0;
        GLsizei particleColorCapacity_ = 0;
    };
}
