#include "simulation/ParticleSystem.h"

#include <glm/geometric.hpp>

#include <cuda_runtime.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <utility>

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
    }

    ParticleSystem::ParticleSystem(
        std::vector<Particle> particles,
        ParticleSystemSettings settings,
        ParticleIntegrationBackend integrationBackend
    )
        : particles_(std::move(particles)),
          settings_(settings),
          integrationBackend_(integrationBackend),
          collisionGrid_(2.0f * settings_.particleRadius)
    {
    }

    ParticleSystem::~ParticleSystem()
    {
        if (cudaStream_ != nullptr)
        {
            cudaStreamSynchronize(cudaStream_);
            cudaStreamDestroy(cudaStream_);
            cudaStream_ = nullptr;
        }
    }

    void ParticleSystem::update(float simulationStep)
    {
        bool usedCuda = false;

        if (integrationBackend_ == ParticleIntegrationBackend::Cuda)
        {
            if (cudaStream_ == nullptr &&
                !checkCuda(
                    cudaStreamCreateWithFlags(
                        &cudaStream_,
                        cudaStreamNonBlocking
                    ),
                    "cudaStreamCreateWithFlags"))
            {
                cudaStream_ = nullptr;
            }

            if (!cudaParticlesInitialized_)
            {
                cudaParticlesInitialized_ =
                    cudaStream_ != nullptr &&
                    cudaParticleBuffer_.upload(particles_);
            }

            if (cudaParticlesInitialized_)
            {
                usedCuda =
                    cudaParticleIntegrator_.integrateOnDevice(
                        cudaParticleBuffer_,
                        settings_,
                        simulationStep,
                        cudaStream_
                    );

                const int collisionIterations = std::max(
                    1,
                    settings_.collisionSolverIterations
                );

                for (int iteration = 0;
                     usedCuda && iteration < collisionIterations;
                     ++iteration)
                {
                    usedCuda =
                        cudaCollisionGrid_.build(
                            cudaParticleBuffer_,
                            2.0f * settings_.particleRadius,
                            cudaStream_
                        ) &&
                        cudaParticleCollisionSolver_.resolve(
                            cudaParticleBuffer_,
                            cudaCollisionGrid_,
                            settings_,
                            cudaStream_
                        );
                }
            }
        }

        if (!usedCuda)
        {
            if (cudaParticlesInitialized_)
            {
                cudaStreamSynchronize(cudaStream_);
                cudaParticleBuffer_.download(particles_);
                cudaParticlesInitialized_ = false;
            }

            for (Particle& particle : particles_)
            {
                updateParticleOnCpu(particle, simulationStep);
            }

            const int collisionIterations = std::max(
                1,
                settings_.collisionSolverIterations
            );

            for (int iteration = 0;
                 iteration < collisionIterations;
                 ++iteration)
            {
                resolveParticleCollisions();
            }
        }
    }

    const std::vector<Particle>& ParticleSystem::particles() const
    {
        return particles_;
    }

    const ParticleSystemSettings& ParticleSystem::settings() const
    {
        return settings_;
    }

    bool ParticleSystem::isUsingCuda() const
    {
        return cudaParticlesInitialized_ && cudaStream_ != nullptr;
    }

    const CudaParticleBuffer& ParticleSystem::cudaParticleBuffer() const
    {
        return cudaParticleBuffer_;
    }

    cudaStream_t ParticleSystem::cudaStream() const
    {
        return cudaStream_;
    }

    void ParticleSystem::updateParticleOnCpu(
        Particle& particle,
        float simulationStep
    )
    {
        particle.acceleration = settings_.gravity;
        particle.velocity += particle.acceleration * simulationStep;
        particle.velocity *= settings_.damping;
        particle.position += particle.velocity * simulationStep;

        resolveContainerCollisions(particle);
    }

    void ParticleSystem::resolveContainerCollisions(Particle& particle)
    {
        if (particle.position.y <= settings_.floorY + settings_.particleRadius)
        {
            particle.position.y = settings_.floorY + settings_.particleRadius;

            if (particle.velocity.y < 0.0f)
            {
                particle.velocity.y = -particle.velocity.y * settings_.restitution;
            }

            if (particle.velocity.y <= settings_.restingSpeed)
            {
                particle.velocity.y = 0.0f;
                particle.velocity.x *= settings_.groundFriction;
                particle.velocity.z *= settings_.groundFriction;

                if (particle.velocity.x > -settings_.horizontalRestingSpeed &&
                    particle.velocity.x < settings_.horizontalRestingSpeed)
                {
                    particle.velocity.x = 0.0f;
                }

                if (particle.velocity.z > -settings_.horizontalRestingSpeed &&
                    particle.velocity.z < settings_.horizontalRestingSpeed)
                {
                    particle.velocity.z = 0.0f;
                }
            }
        }

        if (particle.position.x <= settings_.leftWallX + settings_.particleRadius)
        {
            particle.position.x = settings_.leftWallX + settings_.particleRadius;

            if (particle.velocity.x < 0.0f)
            {
                particle.velocity.x = -particle.velocity.x * settings_.restitution;
            }
        }
        else if (particle.position.x >= settings_.rightWallX - settings_.particleRadius)
        {
            particle.position.x = settings_.rightWallX - settings_.particleRadius;

            if (particle.velocity.x > 0.0f)
            {
                particle.velocity.x = -particle.velocity.x * settings_.restitution;
            }
        }

        if (particle.position.z <= settings_.backWallZ + settings_.particleRadius)
        {
            particle.position.z = settings_.backWallZ + settings_.particleRadius;

            if (particle.velocity.z < 0.0f)
            {
                particle.velocity.z = -particle.velocity.z * settings_.restitution;
            }
        }
        else if (particle.position.z >= settings_.frontWallZ - settings_.particleRadius)
        {
            particle.position.z = settings_.frontWallZ - settings_.particleRadius;

            if (particle.velocity.z > 0.0f)
            {
                particle.velocity.z = -particle.velocity.z * settings_.restitution;
            }
        }
    }

    void ParticleSystem::resolveParticleCollisions()
    {
        collisionGrid_.rebuild(particles_);

        for (std::size_t firstIndex = 0;
             firstIndex < particles_.size();
             ++firstIndex)
        {
            Particle& firstParticle = particles_[firstIndex];

            int cellX = 0;
            int cellY = 0;
            int cellZ = 0;

            collisionGrid_.positionToCell(
                firstParticle.position,
                cellX,
                cellY,
                cellZ
            );

            for (int zOffset = -1; zOffset <= 1; ++zOffset)
            {
                for (int yOffset = -1; yOffset <= 1; ++yOffset)
                {
                    for (int xOffset = -1; xOffset <= 1; ++xOffset)
                    {
                        const std::vector<std::size_t>* candidateIndices =
                            collisionGrid_.particleIndicesInCell(
                                cellX + xOffset,
                                cellY + yOffset,
                                cellZ + zOffset
                            );

                        if (candidateIndices == nullptr)
                        {
                            continue;
                        }

                        for (const std::size_t secondIndex : *candidateIndices)
                        {
                            if (secondIndex <= firstIndex)
                            {
                                continue;
                            }

                            resolveParticleCollision(
                                firstParticle,
                                particles_[secondIndex]
                            );
                        }
                    }
                }
            }
        }
    }

    void ParticleSystem::resolveParticleCollision(
        Particle& firstParticle,
        Particle& secondParticle
    )
    {
        const glm::vec3 separation =
            secondParticle.position - firstParticle.position;

        const float distance = glm::length(separation);
        const float minimumDistance =
            2.0f * settings_.particleRadius;

        if (distance >= minimumDistance)
        {
            return;
        }

        glm::vec3 collisionNormal(1.0f, 0.0f, 0.0f);

        if (distance > 0.0001f)
        {
            collisionNormal = separation / distance;
        }

        const float overlap = minimumDistance - distance;

        firstParticle.position -= collisionNormal * (overlap * 0.5f);
        secondParticle.position += collisionNormal * (overlap * 0.5f);

        const glm::vec3 relativeVelocity =
            secondParticle.velocity - firstParticle.velocity;

        const float velocityAlongNormal =
            glm::dot(relativeVelocity, collisionNormal);

        if (velocityAlongNormal < 0.0f)
        {
            const float impulseMagnitude =
                -(1.0f + settings_.particleRestitution) *
                velocityAlongNormal *
                0.5f;

            const glm::vec3 impulse =
                impulseMagnitude * collisionNormal;

            firstParticle.velocity -= impulse;
            secondParticle.velocity += impulse;
        }
    }
}
