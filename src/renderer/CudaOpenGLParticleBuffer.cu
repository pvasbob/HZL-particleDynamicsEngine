#include "renderer/CudaOpenGLParticleBuffer.h"

#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

#include <iostream>
#include <limits>

namespace hzl::renderer
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

        __global__ void copyParticlePositions(
            const hzl::simulation::CudaParticleState* particles,
            std::size_t particleCount,
            float* positions
        )
        {
            const std::size_t particleIndex =
                static_cast<std::size_t>(blockIdx.x) * blockDim.x +
                threadIdx.x;

            if (particleIndex >= particleCount)
            {
                return;
            }

            const hzl::simulation::CudaParticleState& particle =
                particles[particleIndex];

            const std::size_t positionIndex = particleIndex * 3;

            positions[positionIndex] = particle.positionX;
            positions[positionIndex + 1] = particle.positionY;
            positions[positionIndex + 2] = particle.positionZ;
        }
    }

    CudaOpenGLParticleBuffer::~CudaOpenGLParticleBuffer()
    {
        destroy();
    }

    bool CudaOpenGLParticleBuffer::create(std::size_t particleCapacity)
    {
        destroy();
        return ensureCapacity(particleCapacity);
    }

    bool CudaOpenGLParticleBuffer::uploadFromCpu(
        const std::vector<hzl::simulation::Particle>& particles
    )
    {
        if (!ensureCapacity(particles.size()))
        {
            return false;
        }

        std::vector<float> positions;
        positions.reserve(particles.size() * 3);

        for (const hzl::simulation::Particle& particle : particles)
        {
            positions.push_back(particle.position.x);
            positions.push_back(particle.position.y);
            positions.push_back(particle.position.z);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            static_cast<GLsizeiptr>(positions.size() * sizeof(float)),
            positions.data()
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        particleCount_ = static_cast<GLsizei>(particles.size());
        return true;
    }

    bool CudaOpenGLParticleBuffer::copyPositionsFromCuda(
        const hzl::simulation::CudaParticleBuffer& particleBuffer
    )
    {
        const std::size_t particleCount = particleBuffer.particleCount();

        if (!ensureCapacity(particleCount))
        {
            return false;
        }

        if (particleCount == 0)
        {
            particleCount_ = 0;
            return true;
        }

        if (!checkCuda(
                cudaGraphicsMapResources(1, &cudaResource_, 0),
                "cudaGraphicsMapResources particle positions"))
        {
            return false;
        }

        void* mappedPointer = nullptr;
        std::size_t mappedByteCount = 0;

        const bool mapped = checkCuda(
            cudaGraphicsResourceGetMappedPointer(
                &mappedPointer,
                &mappedByteCount,
                cudaResource_
            ),
            "cudaGraphicsResourceGetMappedPointer particle positions"
        );

        const std::size_t requiredByteCount =
            particleCount * 3 * sizeof(float);

        if (!mapped || mappedByteCount < requiredByteCount)
        {
            cudaGraphicsUnmapResources(1, &cudaResource_, 0);
            return false;
        }

        constexpr int threadsPerBlock = 256;
        const int blockCount = static_cast<int>(
            (particleCount + threadsPerBlock - 1) / threadsPerBlock
        );

        copyParticlePositions<<<blockCount, threadsPerBlock>>>(
            particleBuffer.deviceData(),
            particleCount,
            static_cast<float*>(mappedPointer)
        );

        const bool copied = checkCuda(
            cudaGetLastError(),
            "copyParticlePositions launch"
        );

        const bool unmapped = checkCuda(
            cudaGraphicsUnmapResources(1, &cudaResource_, 0),
            "cudaGraphicsUnmapResources particle positions"
        );

        if (!copied || !unmapped)
        {
            return false;
        }

        particleCount_ = static_cast<GLsizei>(particleCount);
        return true;
    }

    GLuint CudaOpenGLParticleBuffer::vertexBuffer() const
    {
        return vertexBuffer_;
    }

    GLsizei CudaOpenGLParticleBuffer::particleCount() const
    {
        return particleCount_;
    }

    bool CudaOpenGLParticleBuffer::ensureCapacity(
        std::size_t particleCapacity
    )
    {
        if (particleCapacity <= capacity_ && vertexBuffer_ != 0)
        {
            return true;
        }

        destroy();

        if (particleCapacity == 0 ||
            particleCapacity >
                static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()))
        {
            return particleCapacity == 0;
        }

        glGenBuffers(1, &vertexBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(particleCapacity * 3 * sizeof(float)),
            nullptr,
            GL_DYNAMIC_DRAW
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (!checkCuda(
                cudaGraphicsGLRegisterBuffer(
                    &cudaResource_,
                    vertexBuffer_,
                    cudaGraphicsRegisterFlagsWriteDiscard
                ),
                "cudaGraphicsGLRegisterBuffer particle positions"))
        {
            glDeleteBuffers(1, &vertexBuffer_);
            vertexBuffer_ = 0;
            return false;
        }

        capacity_ = particleCapacity;
        return true;
    }

    void CudaOpenGLParticleBuffer::destroy()
    {
        if (cudaResource_ != nullptr)
        {
            cudaGraphicsUnregisterResource(cudaResource_);
            cudaResource_ = nullptr;
        }

        if (vertexBuffer_ != 0)
        {
            glDeleteBuffers(1, &vertexBuffer_);
            vertexBuffer_ = 0;
        }

        capacity_ = 0;
        particleCount_ = 0;
    }
}
