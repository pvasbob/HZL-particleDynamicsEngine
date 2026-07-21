#include "simulation/CudaParticleIntegrator.h"

#include <cuda_runtime.h>

#include <iostream>

namespace hzl::simulation
{
    namespace
    {
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
            CudaParticleState* particles,
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

            CudaParticleState& particle = particles[particleIndex];

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

    bool CudaParticleIntegrator::integrateOnDevice(
        CudaParticleBuffer& particleBuffer,
        const ParticleSystemSettings& settings,
        float simulationStep,
        cudaStream_t stream
    )
    {
        if (particleBuffer.particleCount() == 0)
        {
            return true;
        }

        constexpr int threadsPerBlock = 256;
        const int blockCount = static_cast<int>(
            (particleBuffer.particleCount() + threadsPerBlock - 1) /
            threadsPerBlock
        );

        integrateParticles<<<blockCount, threadsPerBlock, 0, stream>>>(
            particleBuffer.deviceData(),
            particleBuffer.particleCount(),
            toDeviceSettings(settings),
            simulationStep
        );

        return checkCuda(cudaGetLastError(), "integrateParticles launch");
    }
}
