#include "simulation/CudaParticleIntegrator.h"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>

namespace
{
    struct BenchmarkResult
    {
        double averageMilliseconds = 0.0;
        float positionChecksum = 0.0f;
    };

    std::vector<hzl::simulation::Particle> createParticles(
        std::size_t particleCount
    )
    {
        std::vector<hzl::simulation::Particle> particles;
        particles.reserve(particleCount);

        for (std::size_t particleIndex = 0;
             particleIndex < particleCount;
             ++particleIndex)
        {
            const float value = static_cast<float>(particleIndex);

            particles.push_back(
                {
                    glm::vec3(
                        value * 0.001f,
                        value * 0.002f,
                        value * 0.003f
                    ),
                    glm::vec3(0.25f, 0.50f, -0.25f)
                }
            );
        }

        return particles;
    }

    void integrateOnCpu(
        std::vector<hzl::simulation::Particle>& particles,
        const glm::vec3& gravity,
        float damping,
        float simulationStep
    )
    {
        for (hzl::simulation::Particle& particle : particles)
        {
            particle.acceleration = gravity;
            particle.velocity += particle.acceleration * simulationStep;
            particle.velocity *= damping;
            particle.position += particle.velocity * simulationStep;
        }
    }

    float calculatePositionChecksum(
        const std::vector<hzl::simulation::Particle>& particles
    )
    {
        float checksum = 0.0f;

        for (const hzl::simulation::Particle& particle : particles)
        {
            checksum +=
                particle.position.x +
                particle.position.y +
                particle.position.z;
        }

        return checksum;
    }

    BenchmarkResult benchmarkCpu(
        std::size_t particleCount,
        int updateCount
    )
    {
        std::vector<hzl::simulation::Particle> particles =
            createParticles(particleCount);

        const glm::vec3 gravity(0.0f, -0.6f, 0.0f);
        constexpr float damping = 0.999f;
        constexpr float simulationStep = 1.0f / 120.0f;

        const auto startTime = std::chrono::steady_clock::now();

        for (int updateIndex = 0;
             updateIndex < updateCount;
             ++updateIndex)
        {
            integrateOnCpu(particles, gravity, damping, simulationStep);
        }

        const auto endTime = std::chrono::steady_clock::now();

        const std::chrono::duration<double, std::milli> elapsedTime =
            endTime - startTime;

        return {
            elapsedTime.count() / static_cast<double>(updateCount),
            calculatePositionChecksum(particles)
        };
    }

    BenchmarkResult benchmarkCuda(
        std::size_t particleCount,
        int updateCount
    )
    {
        std::vector<hzl::simulation::Particle> particles =
            createParticles(particleCount);

        const glm::vec3 gravity(0.0f, -0.6f, 0.0f);
        constexpr float damping = 0.999f;
        constexpr float simulationStep = 1.0f / 120.0f;

        hzl::simulation::CudaParticleIntegrator integrator;

        const auto startTime = std::chrono::steady_clock::now();

        for (int updateIndex = 0;
             updateIndex < updateCount;
             ++updateIndex)
        {
            if (!integrator.integrate(
                    particles,
                    gravity,
                    damping,
                    simulationStep))
            {
                std::cerr << "CUDA integration failed.\n";
                return {};
            }
        }

        const auto endTime = std::chrono::steady_clock::now();

        const std::chrono::duration<double, std::milli> elapsedTime =
            endTime - startTime;

        return {
            elapsedTime.count() / static_cast<double>(updateCount),
            calculatePositionChecksum(particles)
        };
    }

    void runBenchmark(std::size_t particleCount, int updateCount)
    {
        const BenchmarkResult cpuResult =
            benchmarkCpu(particleCount, updateCount);

        const BenchmarkResult cudaResult =
            benchmarkCuda(particleCount, updateCount);

        std::cout
            << particleCount
            << " particles: CPU "
            << cpuResult.averageMilliseconds
            << " ms, CUDA "
            << cudaResult.averageMilliseconds
            << " ms\n";

        std::cout
            << "  checksums: CPU "
            << cpuResult.positionChecksum
            << ", CUDA "
            << cudaResult.positionChecksum
            << '\n';
    }
}

int main()
{
    constexpr int updateCount = 100;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Particle integration CPU versus CUDA benchmark\n";

    runBenchmark(1'000, updateCount);
    runBenchmark(10'000, updateCount);
    runBenchmark(100'000, updateCount);

    return 0;
}
