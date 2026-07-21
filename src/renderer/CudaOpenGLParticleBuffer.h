#pragma once

#include "simulation/CudaParticleBuffer.h"

#include <cuda_runtime_api.h>
#include <glad/gl.h>

#include <cstddef>
#include <vector>

struct cudaGraphicsResource;

namespace hzl::renderer
{
    class CudaOpenGLParticleBuffer
    {
    public:
        CudaOpenGLParticleBuffer() = default;
        ~CudaOpenGLParticleBuffer();

        CudaOpenGLParticleBuffer(const CudaOpenGLParticleBuffer&) = delete;
        CudaOpenGLParticleBuffer& operator=(const CudaOpenGLParticleBuffer&) = delete;

        bool create(std::size_t particleCapacity);
        bool uploadFromCpu(
            const std::vector<hzl::simulation::Particle>& particles
        );
        bool copyPositionsFromCuda(
            const hzl::simulation::CudaParticleBuffer& particleBuffer,
            cudaStream_t stream
        );

        GLuint vertexBuffer() const;
        GLsizei particleCount() const;

    private:
        bool ensureCapacity(std::size_t particleCapacity);
        void destroy();

        GLuint vertexBuffer_ = 0;
        cudaGraphicsResource* cudaResource_ = nullptr;
        std::size_t capacity_ = 0;
        GLsizei particleCount_ = 0;
    };
}
