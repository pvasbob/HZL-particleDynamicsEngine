#include "simulation/CudaParticleBuffer.h"

#include <cuda_runtime.h>

#include <iostream>
#include <vector>

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

    CudaParticleBuffer::~CudaParticleBuffer()
    {
        release();
    }

    bool CudaParticleBuffer::upload(const std::vector<Particle>& particles)
    {
        if (particles.empty())
        {
            particleCount_ = 0;
            return true;
        }

        if (!ensureDeviceCapacity(particles.size()))
        {
            return false;
        }

        std::vector<CudaParticleState> hostParticles(particles.size());

        for (std::size_t particleIndex = 0;
             particleIndex < particles.size();
             ++particleIndex)
        {
            const Particle& particle = particles[particleIndex];
            CudaParticleState& deviceParticle = hostParticles[particleIndex];

            deviceParticle.positionX = particle.position.x;
            deviceParticle.positionY = particle.position.y;
            deviceParticle.positionZ = particle.position.z;

            deviceParticle.velocityX = particle.velocity.x;
            deviceParticle.velocityY = particle.velocity.y;
            deviceParticle.velocityZ = particle.velocity.z;

            deviceParticle.accelerationX = particle.acceleration.x;
            deviceParticle.accelerationY = particle.acceleration.y;
            deviceParticle.accelerationZ = particle.acceleration.z;
        }

        const std::size_t byteCount =
            hostParticles.size() * sizeof(CudaParticleState);

        if (!checkCuda(
                cudaMemcpy(
                    deviceParticles_,
                    hostParticles.data(),
                    byteCount,
                    cudaMemcpyHostToDevice
                ),
                "cudaMemcpy particle data to device"))
        {
            return false;
        }

        particleCount_ = particles.size();
        return true;
    }

    bool CudaParticleBuffer::download(std::vector<Particle>& particles)
    {
        if (particles.size() != particleCount_)
        {
            std::cerr << "Host and device particle counts do not match.\n";
            return false;
        }

        if (particles.empty())
        {
            return true;
        }

        std::vector<CudaParticleState> hostParticles(particles.size());

        const std::size_t byteCount =
            hostParticles.size() * sizeof(CudaParticleState);

        if (!checkCuda(
                cudaMemcpy(
                    hostParticles.data(),
                    deviceParticles_,
                    byteCount,
                    cudaMemcpyDeviceToHost
                ),
                "cudaMemcpy particle data to host"))
        {
            return false;
        }

        for (std::size_t particleIndex = 0;
             particleIndex < particles.size();
             ++particleIndex)
        {
            Particle& particle = particles[particleIndex];
            const CudaParticleState& deviceParticle = hostParticles[particleIndex];

            particle.position = glm::vec3(
                deviceParticle.positionX,
                deviceParticle.positionY,
                deviceParticle.positionZ
            );

            particle.velocity = glm::vec3(
                deviceParticle.velocityX,
                deviceParticle.velocityY,
                deviceParticle.velocityZ
            );

            particle.acceleration = glm::vec3(
                deviceParticle.accelerationX,
                deviceParticle.accelerationY,
                deviceParticle.accelerationZ
            );
        }

        return true;
    }

    CudaParticleState* CudaParticleBuffer::deviceData()
    {
        return deviceParticles_;
    }

    const CudaParticleState* CudaParticleBuffer::deviceData() const
    {
        return deviceParticles_;
    }

    std::size_t CudaParticleBuffer::particleCount() const
    {
        return particleCount_;
    }

    bool CudaParticleBuffer::ensureDeviceCapacity(
        std::size_t particleCount
    )
    {
        if (particleCount <= deviceCapacity_)
        {
            return true;
        }

        release();

        const std::size_t byteCount =
            particleCount * sizeof(CudaParticleState);

        if (!checkCuda(
                cudaMalloc(
                    reinterpret_cast<void**>(&deviceParticles_),
                    byteCount
                ),
                "cudaMalloc particle buffer"))
        {
            deviceParticles_ = nullptr;
            return false;
        }

        deviceCapacity_ = particleCount;
        return true;
    }

    void CudaParticleBuffer::release()
    {
        if (deviceParticles_ != nullptr)
        {
            cudaFree(deviceParticles_);
            deviceParticles_ = nullptr;
        }

        deviceCapacity_ = 0;
        particleCount_ = 0;
    }
}
