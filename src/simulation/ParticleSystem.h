#pragma once

#include "simulation/CudaParticleBuffer.h"
#include "simulation/CudaParticleIntegrator.h"
#include "simulation/Particle.h"
#include "simulation/ParticleSystemSettings.h"
#include "simulation/UniformGrid.h"

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

        void update(float simulationStep);

        const std::vector<Particle>& particles() const;
        const ParticleSystemSettings& settings() const;

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
        CudaParticleBuffer cudaParticleBuffer_;
        CudaParticleIntegrator cudaParticleIntegrator_;
        UniformGrid collisionGrid_;
    };
}
