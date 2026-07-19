#include "simulation/ParticleFactory.h"

namespace hzl::simulation
{
    std::vector<Particle> createInitialParticleGrid()
    {
        constexpr int particleCountX = 5;
        constexpr int particleCountY = 4;
        constexpr int particleCountZ = 3;
        constexpr float initialSpacing = 0.23f;

        std::vector<Particle> particles;
        particles.reserve(particleCountX * particleCountY * particleCountZ);

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

                    particles.push_back({ position, velocity });
                }
            }
        }

        return particles;
    }
}
