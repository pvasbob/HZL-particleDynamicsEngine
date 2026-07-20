#pragma once

#include "simulation/Particle.h"

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

        bool integrate(
            std::vector<Particle>& particles,
            const glm::vec3& gravity,
            float damping,
            float simulationStep
        );

    private:
        bool ensureDeviceCapacity(std::size_t particleCount);
        void release();

        void* deviceParticles_ = nullptr;
        std::size_t deviceCapacity_ = 0;
    };
}
