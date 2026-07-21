#pragma once

#include "simulation/CudaParticleBuffer.h"
#include "simulation/ParticleSystemSettings.h"

#include <cuda_runtime_api.h>

namespace hzl::simulation
{
    class CudaParticleIntegrator
    {
    public:
        bool integrateOnDevice(
            CudaParticleBuffer& particleBuffer,
            const ParticleSystemSettings& settings,
            float simulationStep,
            cudaStream_t stream = nullptr
        );
    };
}
