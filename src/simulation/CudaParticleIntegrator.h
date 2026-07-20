#pragma once

#include "simulation/Particle.h"
#include "simulation/ParticleSystemSettings.h"

#include <cstddef>
#include <vector>

namespace hzl::simulation
{
    class CudaParticleIntegrator
    {
    public:
        CudaParticleIntegrator() = default;
        ~CudaParticleIntegrator();

        CudaParticleIntegrator(const CudaParticleIntegrator&) = delete;
        CudaParticleIntegrator& operator=(const CudaParticleIntegrator&) = delete;

        bool upload(const std::vector<Particle>& particles);

        bool integrateOnDevice(
            const ParticleSystemSettings& settings,
            float simulationStep
        );

        bool download(std::vector<Particle>& particles);

        bool integrate(
            std::vector<Particle>& particles,
            const ParticleSystemSettings& settings,
            float simulationStep
        );

    private:
        bool ensureDeviceCapacity(std::size_t particleCount);
        void release();

        void* deviceParticles_ = nullptr;
        std::size_t deviceCapacity_ = 0;
        std::size_t deviceParticleCount_ = 0;
    };
}
