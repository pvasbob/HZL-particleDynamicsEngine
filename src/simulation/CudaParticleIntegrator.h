#pragma once

#include "simulation/CudaParticleBuffer.h"
#include "simulation/ParticleSystemSettings.h"

namespace hzl::simulation
{
    class CudaParticleIntegrator
    {
    public:
        bool integrateOnDevice(
            CudaParticleBuffer& particleBuffer,
            const ParticleSystemSettings& settings,
            float simulationStep
        );
    };
}
