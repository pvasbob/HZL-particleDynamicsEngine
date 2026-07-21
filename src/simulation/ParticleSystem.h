#pragma once

#include "simulation/CudaParticleBuffer.h"
#include "simulation/CudaParticleCollisionSolver.h"
#include "simulation/CudaParticleIntegrator.h"
#include "simulation/CudaUniformGrid.h"
#include "simulation/Particle.h"
#include "simulation/ParticleSystemSettings.h"
#include "simulation/UniformGrid.h"

#include <cuda_runtime_api.h>

#include <vector>

namespace hzl::simulation
{
    enum class ParticleIntegrationBackend
    {
        Cpu,
        Cuda
    };

    class ParticleSystem
    {
    public:
        ParticleSystem(
            std::vector<Particle> particles,
            ParticleSystemSettings settings,
            ParticleIntegrationBackend integrationBackend =
                ParticleIntegrationBackend::Cpu
        );
        ~ParticleSystem();

        void update(float simulationStep);

        const std::vector<Particle>& particles() const;
        const ParticleSystemSettings& settings() const;
        bool isUsingCuda() const;
        const CudaParticleBuffer& cudaParticleBuffer() const;
        cudaStream_t cudaStream() const;

    private:
        void updateParticleOnCpu(Particle& particle, float simulationStep);
        void resolveContainerCollisions(Particle& particle);
        void resolveParticleCollisions();
        void resolveParticleCollision(
            Particle& firstParticle,
            Particle& secondParticle
        );

        std::vector<Particle> particles_;
        ParticleSystemSettings settings_;
        ParticleIntegrationBackend integrationBackend_;
        bool cudaParticlesInitialized_ = false;
        cudaStream_t cudaStream_ = nullptr;
        CudaParticleBuffer cudaParticleBuffer_;
        CudaParticleIntegrator cudaParticleIntegrator_;
        CudaUniformGrid cudaCollisionGrid_;
        CudaParticleCollisionSolver cudaParticleCollisionSolver_;
        UniformGrid collisionGrid_;
    };
}
