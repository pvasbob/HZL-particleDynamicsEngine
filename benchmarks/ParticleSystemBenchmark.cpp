#include "simulation/ParticleSystem.h"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

namespace
{
    std::vector<hzl::simulation::Particle> createBenchmarkParticles(
        std::size_t particleCount
    )
    {
        const int particlesPerSide = static_cast<int>(
            std::ceil(
                std::cbrt(static_cast<double>(particleCount))
            )
        );

        constexpr float spacing = 0.05f;

        const float centerOffset =
            static_cast<float>(particlesPerSide - 1) *
            spacing *
            0.5f;

        std::vector<hzl::simulation::Particle> particles;
        particles.reserve(particleCount);

        for (std::size_t particleIndex = 0;
             particleIndex < particleCount;
             ++particleIndex)
        {
            const int xIndex = static_cast<int>(
                particleIndex %
                static_cast<std::size_t>(particlesPerSide)
            );

            const int yIndex = static_cast<int>(
                (particleIndex /
                 static_cast<std::size_t>(particlesPerSide)) %
                static_cast<std::size_t>(particlesPerSide)
            );

            const int zIndex = static_cast<int>(
                particleIndex /
                static_cast<std::size_t>(
                    particlesPerSide * particlesPerSide
                )
            );

            particles.push_back(
                {
                    glm::vec3(
                        static_cast<float>(xIndex) * spacing - centerOffset,
                        static_cast<float>(yIndex) * spacing - centerOffset,
                        static_cast<float>(zIndex) * spacing - centerOffset
                    ),
                    glm::vec3(0.0f)
                }
            );
        }

        return particles;
    }

    hzl::simulation::ParticleSystemSettings createBenchmarkSettings()
    {
        hzl::simulation::ParticleSystemSettings settings;

        settings.gravity = glm::vec3(0.0f);
        settings.damping = 1.0f;

        settings.floorY = -100.0f;
        settings.leftWallX = -100.0f;
        settings.rightWallX = 100.0f;
        settings.backWallZ = -100.0f;
        settings.frontWallZ = 100.0f;

        settings.particleRadius = 0.02f;

        return settings;
    }

    void runBenchmark(
        std::size_t particleCount,
        int updateCount
    )
    {
        hzl::simulation::ParticleSystem particleSystem(
            createBenchmarkParticles(particleCount),
            createBenchmarkSettings()
        );

        constexpr float simulationStep = 1.0f / 120.0f;

        const auto startTime =
            std::chrono::steady_clock::now();

        for (int updateIndex = 0;
             updateIndex < updateCount;
             ++updateIndex)
        {
            particleSystem.update(simulationStep);
        }

        const auto endTime =
            std::chrono::steady_clock::now();

        const std::chrono::duration<double, std::milli> elapsedTime =
            endTime - startTime;

        const double averageMilliseconds =
            elapsedTime.count() /
            static_cast<double>(updateCount);

        std::cout
            << particleCount
            << " particles: "
            << averageMilliseconds
            << " ms per update\n";
    }
}

int main()
{
    constexpr int updateCount = 60;

    std::cout << "ParticleSystem CPU benchmark\n";

    runBenchmark(250, updateCount);
    runBenchmark(500, updateCount);
    runBenchmark(1000, updateCount);
    runBenchmark(2000, updateCount);
    runBenchmark(4000, updateCount);

    return 0;
}