#include "simulation/CudaParticleCollisionSolver.h"

#include <cuda_runtime.h>

#include <iostream>

namespace hzl::simulation
{
    namespace
    {
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

        __device__ std::size_t lowerBound(
            const CudaCellKey* sortedKeys,
            std::size_t keyCount,
            CudaCellKey targetKey
        )
        {
            std::size_t first = 0;
            std::size_t count = keyCount;

            while (count > 0)
            {
                const std::size_t step = count / 2;
                const std::size_t middle = first + step;

                if (sortedKeys[middle] < targetKey)
                {
                    first = middle + 1;
                    count -= step + 1;
                }
                else
                {
                    count = step;
                }
            }

            return first;
        }

        __global__ void resolveParticleCollisions(
            const CudaParticleState* particles,
            std::size_t particleCount,
            const CudaCellKey* sortedCellKeys,
            const unsigned int* sortedParticleIndices,
            float cellSize,
            float particleRadius,
            float particleRestitution,
            CudaParticleState* resolvedParticles
        )
        {
            const std::size_t particleIndex =
                static_cast<std::size_t>(blockIdx.x) * blockDim.x +
                threadIdx.x;

            if (particleIndex >= particleCount)
            {
                return;
            }

            const CudaParticleState particle = particles[particleIndex];

            const int cellX = static_cast<int>(
                floorf(particle.positionX / cellSize)
            );
            const int cellY = static_cast<int>(
                floorf(particle.positionY / cellSize)
            );
            const int cellZ = static_cast<int>(
                floorf(particle.positionZ / cellSize)
            );

            float positionCorrectionX = 0.0f;
            float positionCorrectionY = 0.0f;
            float positionCorrectionZ = 0.0f;

            float velocityChangeX = 0.0f;
            float velocityChangeY = 0.0f;
            float velocityChangeZ = 0.0f;

            const float minimumDistance = 2.0f * particleRadius;

            for (int zOffset = -1; zOffset <= 1; ++zOffset)
            {
                for (int yOffset = -1; yOffset <= 1; ++yOffset)
                {
                    for (int xOffset = -1; xOffset <= 1; ++xOffset)
                    {
                        const CudaCellKey neighborKey =
                            encodeCudaCellCoordinates(
                                cellX + xOffset,
                                cellY + yOffset,
                                cellZ + zOffset
                            );

                        const std::size_t firstCandidate = lowerBound(
                            sortedCellKeys,
                            particleCount,
                            neighborKey
                        );

                        for (std::size_t sortedIndex = firstCandidate;
                             sortedIndex < particleCount &&
                             sortedCellKeys[sortedIndex] == neighborKey;
                             ++sortedIndex)
                        {
                            const std::size_t candidateIndex =
                                sortedParticleIndices[sortedIndex];

                            if (candidateIndex == particleIndex)
                            {
                                continue;
                            }

                            const CudaParticleState candidate =
                                particles[candidateIndex];

                            const float separationX =
                                candidate.positionX - particle.positionX;
                            const float separationY =
                                candidate.positionY - particle.positionY;
                            const float separationZ =
                                candidate.positionZ - particle.positionZ;

                            const float distanceSquared =
                                separationX * separationX +
                                separationY * separationY +
                                separationZ * separationZ;

                            if (distanceSquared >=
                                minimumDistance * minimumDistance)
                            {
                                continue;
                            }

                            const float distance = sqrtf(distanceSquared);

                            float normalX = 1.0f;
                            float normalY = 0.0f;
                            float normalZ = 0.0f;

                            if (distance > 0.0001f)
                            {
                                normalX = separationX / distance;
                                normalY = separationY / distance;
                                normalZ = separationZ / distance;
                            }
                            else if (particleIndex > candidateIndex)
                            {
                                normalX = -1.0f;
                            }

                            const float overlap = minimumDistance - distance;

                            positionCorrectionX -= normalX * overlap * 0.5f;
                            positionCorrectionY -= normalY * overlap * 0.5f;
                            positionCorrectionZ -= normalZ * overlap * 0.5f;

                            const float relativeVelocityX =
                                candidate.velocityX - particle.velocityX;
                            const float relativeVelocityY =
                                candidate.velocityY - particle.velocityY;
                            const float relativeVelocityZ =
                                candidate.velocityZ - particle.velocityZ;

                            const float velocityAlongNormal =
                                relativeVelocityX * normalX +
                                relativeVelocityY * normalY +
                                relativeVelocityZ * normalZ;

                            if (velocityAlongNormal < 0.0f)
                            {
                                const float impulseMagnitude =
                                    -(1.0f + particleRestitution) *
                                    velocityAlongNormal *
                                    0.5f;

                                velocityChangeX -= impulseMagnitude * normalX;
                                velocityChangeY -= impulseMagnitude * normalY;
                                velocityChangeZ -= impulseMagnitude * normalZ;
                            }
                        }
                    }
                }
            }

            CudaParticleState resolvedParticle = particle;

            resolvedParticle.positionX += positionCorrectionX;
            resolvedParticle.positionY += positionCorrectionY;
            resolvedParticle.positionZ += positionCorrectionZ;

            resolvedParticle.velocityX += velocityChangeX;
            resolvedParticle.velocityY += velocityChangeY;
            resolvedParticle.velocityZ += velocityChangeZ;

            resolvedParticles[particleIndex] = resolvedParticle;
        }
    }

    CudaParticleCollisionSolver::~CudaParticleCollisionSolver()
    {
        release();
    }

    bool CudaParticleCollisionSolver::resolve(
        CudaParticleBuffer& particleBuffer,
        const CudaUniformGrid& grid,
        const ParticleSystemSettings& settings,
        cudaStream_t stream
    )
    {
        const std::size_t particleCount = particleBuffer.particleCount();

        if (particleCount == 0)
        {
            return true;
        }

        if (particleCount != grid.particleCount())
        {
            std::cerr << "CUDA particle and grid counts do not match.\n";
            return false;
        }

        if (!ensureDeviceCapacity(particleCount))
        {
            return false;
        }

        constexpr int threadsPerBlock = 256;
        const int blockCount = static_cast<int>(
            (particleCount + threadsPerBlock - 1) / threadsPerBlock
        );

        resolveParticleCollisions<<<blockCount, threadsPerBlock, 0, stream>>>(
            particleBuffer.deviceData(),
            particleCount,
            grid.deviceCellKeys(),
            grid.deviceParticleIndices(),
            grid.cellSize(),
            settings.particleRadius,
            settings.particleRestitution,
            deviceResolvedParticles_
        );

        if (!checkCuda(
                cudaGetLastError(),
                "resolveParticleCollisions launch"))
        {
            return false;
        }

        return checkCuda(
            cudaMemcpyAsync(
                particleBuffer.deviceData(),
                deviceResolvedParticles_,
                particleCount * sizeof(CudaParticleState),
                cudaMemcpyDeviceToDevice,
                stream
            ),
            "cudaMemcpyAsync resolved particles to simulation buffer"
        );
    }

    bool CudaParticleCollisionSolver::ensureDeviceCapacity(
        std::size_t particleCount
    )
    {
        if (particleCount <= deviceCapacity_)
        {
            return true;
        }

        release();

        if (!checkCuda(
                cudaMalloc(
                    reinterpret_cast<void**>(&deviceResolvedParticles_),
                    particleCount * sizeof(CudaParticleState)
                ),
                "cudaMalloc resolved particle buffer"))
        {
            deviceResolvedParticles_ = nullptr;
            return false;
        }

        deviceCapacity_ = particleCount;
        return true;
    }

    void CudaParticleCollisionSolver::release()
    {
        if (deviceResolvedParticles_ != nullptr)
        {
            cudaFree(deviceResolvedParticles_);
            deviceResolvedParticles_ = nullptr;
        }

        deviceCapacity_ = 0;
    }
}
