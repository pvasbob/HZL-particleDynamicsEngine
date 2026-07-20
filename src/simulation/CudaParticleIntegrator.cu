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

        struct DeviceParticleSystemSettings
        {
            float gravityX;
            float gravityY;
            float gravityZ;

            float floorY;
            float leftWallX;
            float rightWallX;
            float backWallZ;
            float frontWallZ;

            float restitution;
            float damping;
            float restingSpeed;
            float groundFriction;
            float horizontalRestingSpeed;
            float particleRadius;
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
            DeviceParticleSystemSettings settings,
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

            particle.accelerationX = settings.gravityX;
            particle.accelerationY = settings.gravityY;
            particle.accelerationZ = settings.gravityZ;

            particle.velocityX += particle.accelerationX * simulationStep;
            particle.velocityY += particle.accelerationY * simulationStep;
            particle.velocityZ += particle.accelerationZ * simulationStep;

            particle.velocityX *= settings.damping;
            particle.velocityY *= settings.damping;
            particle.velocityZ *= settings.damping;

            particle.positionX += particle.velocityX * simulationStep;
            particle.positionY += particle.velocityY * simulationStep;
            particle.positionZ += particle.velocityZ * simulationStep;

            if (particle.positionY <= settings.floorY + settings.particleRadius)
            {
                particle.positionY = settings.floorY + settings.particleRadius;

                if (particle.velocityY < 0.0f)
                {
                    particle.velocityY =
                        -particle.velocityY * settings.restitution;
                }

                if (particle.velocityY <= settings.restingSpeed)
                {
                    particle.velocityY = 0.0f;
                    particle.velocityX *= settings.groundFriction;
                    particle.velocityZ *= settings.groundFriction;

                    if (particle.velocityX > -settings.horizontalRestingSpeed &&
                        particle.velocityX < settings.horizontalRestingSpeed)
                    {
                        particle.velocityX = 0.0f;
                    }

                    if (particle.velocityZ > -settings.horizontalRestingSpeed &&
                        particle.velocityZ < settings.horizontalRestingSpeed)
                    {
                        particle.velocityZ = 0.0f;
                    }
                }
            }

            if (particle.positionX <= settings.leftWallX + settings.particleRadius)
            {
                particle.positionX = settings.leftWallX + settings.particleRadius;

                if (particle.velocityX < 0.0f)
                {
                    particle.velocityX =
                        -particle.velocityX * settings.restitution;
                }
            }
            else if (particle.positionX >= settings.rightWallX - settings.particleRadius)
            {
                particle.positionX = settings.rightWallX - settings.particleRadius;

                if (particle.velocityX > 0.0f)
                {
                    particle.velocityX =
                        -particle.velocityX * settings.restitution;
                }
            }

            if (particle.positionZ <= settings.backWallZ + settings.particleRadius)
            {
                particle.positionZ = settings.backWallZ + settings.particleRadius;

                if (particle.velocityZ < 0.0f)
                {
                    particle.velocityZ =
                        -particle.velocityZ * settings.restitution;
                }
            }
            else if (particle.positionZ >= settings.frontWallZ - settings.particleRadius)
            {
                particle.positionZ = settings.frontWallZ - settings.particleRadius;

                if (particle.velocityZ > 0.0f)
                {
                    particle.velocityZ =
                        -particle.velocityZ * settings.restitution;
                }
            }
        }

        DeviceParticleSystemSettings toDeviceSettings(
            const ParticleSystemSettings& settings
        )
        {
            return {
                settings.gravity.x,
                settings.gravity.y,
                settings.gravity.z,
                settings.floorY,
                settings.leftWallX,
                settings.rightWallX,
                settings.backWallZ,
                settings.frontWallZ,
                settings.restitution,
                settings.damping,
                settings.restingSpeed,
                settings.groundFriction,
                settings.horizontalRestingSpeed,
                settings.particleRadius
            };
        }
    }

    CudaParticleIntegrator::~CudaParticleIntegrator()
    {
        release();
    }

    bool CudaParticleIntegrator::integrate(
        std::vector<Particle>& particles,
        const ParticleSystemSettings& settings,
        float simulationStep
    )
    {
        return upload(particles) &&
               integrateOnDevice(settings, simulationStep) &&
               download(particles);
    }

    bool CudaParticleIntegrator::upload(
        const std::vector<Particle>& particles
    )
    {
        if (particles.empty())
        {
            deviceParticleCount_ = 0;
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

        deviceParticleCount_ = particles.size();
        return true;
    }

    bool CudaParticleIntegrator::integrateOnDevice(
        const ParticleSystemSettings& settings,
        float simulationStep
    )
    {
        if (deviceParticleCount_ == 0)
        {
            return true;
        }

        constexpr int threadsPerBlock = 256;
        const int blockCount = static_cast<int>(
            (deviceParticleCount_ + threadsPerBlock - 1) /
            threadsPerBlock
        );

        integrateParticles<<<blockCount, threadsPerBlock>>>(
            static_cast<DeviceParticle*>(deviceParticles_),
            deviceParticleCount_,
            toDeviceSettings(settings),
            simulationStep
        );

        if (!checkCuda(cudaGetLastError(), "integrateParticles launch") ||
            !checkCuda(cudaDeviceSynchronize(), "integrateParticles execution"))
        {
            return false;
        }

        return true;
    }

    bool CudaParticleIntegrator::download(std::vector<Particle>& particles)
    {
        if (particles.size() != deviceParticleCount_)
        {
            std::cerr << "Host and device particle counts do not match.\n";
            return false;
        }

        if (particles.empty())
        {
            return true;
        }

        std::vector<DeviceParticle> hostParticles(particles.size());

        const std::size_t byteCount =
            hostParticles.size() * sizeof(DeviceParticle);

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
        deviceParticleCount_ = 0;
    }
}
