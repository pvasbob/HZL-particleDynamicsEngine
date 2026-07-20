#include "simulation/ParticleFactory.h"

namespace hzl::simulation
{
    std::vector<Particle> createHeadOnCollisionDemo()
    {
        constexpr int particlesPerAxis = 3;
        constexpr float spacing = 0.18f;
        constexpr float leftStartX = -0.55f;
        constexpr float rightStartX = 0.55f;
        constexpr float velocityX = 1.00f;

        std::vector<Particle> particles;
        particles.reserve(particlesPerAxis * particlesPerAxis * 2);

        for (int yIndex = 0;
             yIndex < particlesPerAxis;
             ++yIndex)
        {
            for (int zIndex = 0;
                 zIndex < particlesPerAxis;
                 ++zIndex)
            {
                const float yPosition =
                    0.10f + static_cast<float>(yIndex) * spacing;
                const float zPosition =
                    0.52f + static_cast<float>(zIndex) * spacing;

                particles.push_back(
                    {
                        glm::vec3(leftStartX, yPosition, zPosition),
                        glm::vec3(velocityX, 0.0f, 0.0f)
                    }
                );
            }
        }

        for (int yIndex = 0;
             yIndex < particlesPerAxis;
             ++yIndex)
        {
            for (int zIndex = 0;
                 zIndex < particlesPerAxis;
                 ++zIndex)
            {
                const float yPosition =
                    0.10f + static_cast<float>(yIndex) * spacing;
                const float zPosition =
                    0.52f + static_cast<float>(zIndex) * spacing;

                particles.push_back(
                    {
                        glm::vec3(rightStartX, yPosition, zPosition),
                        glm::vec3(-velocityX, 0.0f, 0.0f)
                    }
                );
            }
        }

        return particles;
    }
}
