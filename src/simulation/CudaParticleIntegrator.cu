#include "simulation/CudaParticleIntegrator.h"

#include <cuda_runtime.h>

#include <iostream>
#include <vector>

namespace hzl::simulation
{
    namespace
    {
        struct DeviceParticle
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

        __global__ void integrateParticles(
            DeviceParticle* particles,
            std::size_t particleCount,
            float gravityX,
            float gravityY,
            float gravityZ,
            float damping,
            float simulationStep
        )
        {
            const std::size_t particleIndex =
                static_cast<std::size_t>(blockIdx.x) * blockDim.x +
                threadIdx.x;

            if (particleIndex >= particleCount)
            {
                return;
            }

            DeviceParticle& particle = particles[particleIndex];

            particle.accelerationX = gravityX;
            particle.accelerationY = gravityY;
            particle.accelerationZ = gravityZ;

            particle.velocityX += particle.accelerationX * simulationStep;
            particle.velocityY += particle.accelerationY * simulationStep;
            particle.velocityZ += particle.accelerationZ * simulationStep;

            particle.velocityX *= damping;
            particle.velocityY *= damping;
            particle.velocityZ *= damping;

            particle.positionX += particle.velocityX * simulationStep;
            particle.positionY += particle.velocityY * simulationStep;
            particle.positionZ += particle.velocityZ * simulationStep;
        }
    }

    CudaParticleIntegrator::~CudaParticleIntegrator()
    {
        release();
    }

    bool CudaParticleIntegrator::integrate(
        std::vector<Particle>& particles,
        const glm::vec3& gravity,
        float damping,
        float simulationStep
    )
    {
        if (particles.empty())
        {
            return true;
        }

        if (!ensureDeviceCapacity(particles.size()))
        {
            return false;
        }

        std::vector<DeviceParticle> hostParticles(particles.size());

        for (std::size_t particleIndex = 0;
             particleIndex < particles.size();
             ++particleIndex)
        {
            const Particle& particle = particles[particleIndex];
            DeviceParticle& deviceParticle = hostParticles[particleIndex];

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
            hostParticles.size() * sizeof(DeviceParticle);

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

        constexpr int threadsPerBlock = 256;
        const int blockCount = static_cast<int>(
            (particles.size() + threadsPerBlock - 1) /
            threadsPerBlock
        );

        integrateParticles<<<blockCount, threadsPerBlock>>>(
            static_cast<DeviceParticle*>(deviceParticles_),
            particles.size(),
            gravity.x,
            gravity.y,
            gravity.z,
            damping,
            simulationStep
        );

        if (!checkCuda(cudaGetLastError(), "integrateParticles launch") ||
            !checkCuda(cudaDeviceSynchronize(), "integrateParticles execution"))
        {
            return false;
        }

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
            const DeviceParticle& deviceParticle = hostParticles[particleIndex];

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

    bool CudaParticleIntegrator::ensureDeviceCapacity(
        std::size_t particleCount
    )
    {
        if (particleCount <= deviceCapacity_)
        {
            return true;
        }

        release();

        const std::size_t byteCount =
            particleCount * sizeof(DeviceParticle);

        if (!checkCuda(
                cudaMalloc(&deviceParticles_, byteCount),
                "cudaMalloc particle buffer"))
        {
            deviceParticles_ = nullptr;
            return false;
        }

        deviceCapacity_ = particleCount;
        return true;
    }

    void CudaParticleIntegrator::release()
    {
        if (deviceParticles_ != nullptr)
        {
            cudaFree(deviceParticles_);
            deviceParticles_ = nullptr;
        }

        deviceCapacity_ = 0;
    }
}
