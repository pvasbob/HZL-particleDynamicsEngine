#pragma once

#include "simulation/CudaParticleBuffer.h"
#include "simulation/CudaUniformGrid.h"
#include "simulation/ParticleSystemSettings.h"

#include <cuda_runtime_api.h>

#include <cstddef>

namespace hzl::simulation
{
    class CudaParticleCollisionSolver
    {
    public:
        CudaParticleCollisionSolver() = default;
        ~CudaParticleCollisionSolver();

        CudaParticleCollisionSolver(const CudaParticleCollisionSolver&) = delete;
        CudaParticleCollisionSolver& operator=(const CudaParticleCollisionSolver&) = delete;

        bool resolve(
            CudaParticleBuffer& particleBuffer,
            const CudaUniformGrid& grid,
            const ParticleSystemSettings& settings,
            cudaStream_t stream = nullptr
        );

    private:
        bool ensureDeviceCapacity(std::size_t particleCount);
        void release();

        CudaParticleState* deviceResolvedParticles_ = nullptr;
        std::size_t deviceCapacity_ = 0;
    };
}
