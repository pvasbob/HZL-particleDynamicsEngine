#include "simulation/CudaParticleCollisionSolver.h"
#include "simulation/CudaParticleIntegrator.h"
#include "simulation/CudaUniformGrid.h"

#include <cuda_runtime.h>

#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>

namespace
{
    struct BenchmarkResult
    {
        float averageMilliseconds = 0.0f;
        float positionChecksum = 0.0f;
    };

    bool checkCuda(cudaError_t result, const char* operation)
    {
        if (result == cudaSuccess)
        {
            return true;
        }

        std::cerr
            << operation
            << " failed: "
            << cudaGetErrorString(result)
            << '\n';

        return false;
    }

    std::vector<hzl::simulation::Particle> createBenchmarkParticles(
        std::size_t particleCount
    )
    {
        const int particlesPerAxis = static_cast<int>(
            std::ceil(std::cbrt(static_cast<double>(particleCount)))
        );

        constexpr float spacing = 0.038f;
        const float centerOffset =
            static_cast<float>(particlesPerAxis - 1) * spacing * 0.5f;

        std::vector<hzl::simulation::Particle> particles;
        particles.reserve(particleCount);

        for (std::size_t particleIndex = 0;
             particleIndex < particleCount;
             ++particleIndex)
        {
            const int xIndex = static_cast<int>(
                particleIndex % static_cast<std::size_t>(particlesPerAxis)
            );
            const int yIndex = static_cast<int>(
                (particleIndex /
                 static_cast<std::size_t>(particlesPerAxis)) %
                static_cast<std::size_t>(particlesPerAxis)
            );
            const int zIndex = static_cast<int>(
                particleIndex /
                static_cast<std::size_t>(
                    particlesPerAxis * particlesPerAxis
                )
            );

            const float velocityDirection =
                (particleIndex % 2 == 0) ? 1.0f : -1.0f;

            particles.push_back(
                {
                    glm::vec3(
                        static_cast<float>(xIndex) * spacing - centerOffset,
                        static_cast<float>(yIndex) * spacing - centerOffset,
                        static_cast<float>(zIndex) * spacing - centerOffset
                    ),
                    glm::vec3(velocityDirection * 0.05f, 0.0f, 0.0f)
                }
            );
        }

        return particles;
    }

    hzl::simulation::ParticleSystemSettings createBenchmarkSettings(
        int collisionSolverIterations
    )
    {
        hzl::simulation::ParticleSystemSettings settings;

        settings.gravity = glm::vec3(0.0f);
        settings.damping = 1.0f;
        settings.floorY = -1'000'000.0f;
        settings.leftWallX = -1'000'000.0f;
        settings.rightWallX = 1'000'000.0f;
        settings.backWallZ = -1'000'000.0f;
        settings.frontWallZ = 1'000'000.0f;
        settings.particleRadius = 0.02f;
        settings.particleRestitution = 0.75f;
        settings.collisionSolverIterations = collisionSolverIterations;

        return settings;
    }

    bool updateOnCuda(
        hzl::simulation::CudaParticleBuffer& particleBuffer,
        hzl::simulation::CudaParticleIntegrator& integrator,
        hzl::simulation::CudaUniformGrid& grid,
        hzl::simulation::CudaParticleCollisionSolver& collisionSolver,
        const hzl::simulation::ParticleSystemSettings& settings,
        float simulationStep
    )
    {
        if (!integrator.integrateOnDevice(
                particleBuffer,
                settings,
                simulationStep))
        {
            return false;
        }

        for (int iteration = 0;
             iteration < settings.collisionSolverIterations;
             ++iteration)
        {
            if (!grid.build(
                    particleBuffer,
                    2.0f * settings.particleRadius) ||
                !collisionSolver.resolve(particleBuffer, grid, settings))
            {
                return false;
            }
        }

        return true;
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

    BenchmarkResult benchmarkCudaParticleSystem(
        std::size_t particleCount,
        int collisionSolverIterations,
        int updateCount
    )
    {
        std::vector<hzl::simulation::Particle> particles =
            createBenchmarkParticles(particleCount);

        const hzl::simulation::ParticleSystemSettings settings =
            createBenchmarkSettings(collisionSolverIterations);

        hzl::simulation::CudaParticleBuffer particleBuffer;
        hzl::simulation::CudaParticleIntegrator integrator;
        hzl::simulation::CudaUniformGrid grid;
        hzl::simulation::CudaParticleCollisionSolver collisionSolver;

        constexpr float simulationStep = 1.0f / 120.0f;

        if (!particleBuffer.upload(particles) ||
            !updateOnCuda(
                particleBuffer,
                integrator,
                grid,
                collisionSolver,
                settings,
                simulationStep))
        {
            std::cerr << "CUDA benchmark warm-up failed.\n";
            return {};
        }

        cudaEvent_t startEvent = nullptr;
        cudaEvent_t stopEvent = nullptr;

        if (!checkCuda(cudaEventCreate(&startEvent), "cudaEventCreate start") ||
            !checkCuda(cudaEventCreate(&stopEvent), "cudaEventCreate stop"))
        {
            if (startEvent != nullptr)
            {
                cudaEventDestroy(startEvent);
            }

            return {};
        }

        if (!checkCuda(cudaEventRecord(startEvent), "cudaEventRecord start"))
        {
            cudaEventDestroy(stopEvent);
            cudaEventDestroy(startEvent);
            return {};
        }

        bool succeeded = true;

        for (int updateIndex = 0;
             updateIndex < updateCount;
             ++updateIndex)
        {
            if (!updateOnCuda(
                    particleBuffer,
                    integrator,
                    grid,
                    collisionSolver,
                    settings,
                    simulationStep))
            {
                std::cerr << "CUDA particle-system update failed.\n";
                succeeded = false;
                break;
            }
        }

        float elapsedMilliseconds = 0.0f;

        succeeded = succeeded &&
            checkCuda(cudaEventRecord(stopEvent), "cudaEventRecord stop") &&
            checkCuda(cudaEventSynchronize(stopEvent), "cudaEventSynchronize stop") &&
            checkCuda(
                cudaEventElapsedTime(
                    &elapsedMilliseconds,
                    startEvent,
                    stopEvent
                ),
                "cudaEventElapsedTime"
            );

        cudaEventDestroy(stopEvent);
        cudaEventDestroy(startEvent);

        if (!succeeded || !particleBuffer.download(particles))
        {
            return {};
        }

        return {
            elapsedMilliseconds / static_cast<float>(updateCount),
            calculatePositionChecksum(particles)
        };
    }

    void runBenchmark(std::size_t particleCount, int updateCount)
    {
        const BenchmarkResult oneIterationResult =
            benchmarkCudaParticleSystem(particleCount, 1, updateCount);

        const BenchmarkResult fourIterationResult =
            benchmarkCudaParticleSystem(particleCount, 4, updateCount);

        std::cout
            << particleCount
            << " particles: 1 iteration "
            << oneIterationResult.averageMilliseconds
            << " ms, 4 iterations "
            << fourIterationResult.averageMilliseconds
            << " ms\n";

        std::cout
            << "  checksums: 1 iteration "
            << oneIterationResult.positionChecksum
            << ", 4 iterations "
            << fourIterationResult.positionChecksum
            << '\n';
    }
}

int main()
{
    constexpr int updateCount = 60;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "GPU particle-system benchmark\n";

    runBenchmark(1'000, updateCount);
    runBenchmark(10'000, updateCount);
    runBenchmark(100'000, updateCount);

    return 0;
}
