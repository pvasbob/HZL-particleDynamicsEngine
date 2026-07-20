#pragma once

#include "simulation/Particle.h"

#include <cstddef>
#include <vector>

namespace hzl::simulation
{
    struct CudaParticleState
    {
        float positionX;
        float positionY;
        float positionZ;

        float velocityX;
        float velocityY;
        float velocityZ;

        float accelerationX;
        float accelerationY;
        float accelerationZ;
    };

    class CudaParticleBuffer
    {
    public:
        CudaParticleBuffer() = default;
        ~CudaParticleBuffer();

        CudaParticleBuffer(const CudaParticleBuffer&) = delete;
        CudaParticleBuffer& operator=(const CudaParticleBuffer&) = delete;

        bool upload(const std::vector<Particle>& particles);
        bool download(std::vector<Particle>& particles);

        CudaParticleState* deviceData();
        const CudaParticleState* deviceData() const;
        std::size_t particleCount() const;

    private:
        bool ensureDeviceCapacity(std::size_t particleCount);
        void release();

        CudaParticleState* deviceParticles_ = nullptr;
        std::size_t deviceCapacity_ = 0;
        std::size_t particleCount_ = 0;
    };
}
