#include "simulation/CudaUniformGrid.h"

#include <cuda_runtime.h>

#include <thrust/device_ptr.h>
#include <thrust/execution_policy.h>
#include <thrust/sort.h>
#include <thrust/system_error.h>

#include <algorithm>
#include <cmath>
#include <iostream>

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

        __global__ void buildCellKeys(
            const CudaParticleState* particles,
            std::size_t particleCount,
            float cellSize,
            CudaCellKey* cellKeys,
            unsigned int* particleIndices
        )
        {
            const std::size_t particleIndex =
                static_cast<std::size_t>(blockIdx.x) * blockDim.x +
                threadIdx.x;

            if (particleIndex >= particleCount)
            {
                return;
            }

            const CudaParticleState& particle = particles[particleIndex];

            const int cellX = static_cast<int>(
                floorf(particle.positionX / cellSize)
            );
            const int cellY = static_cast<int>(
                floorf(particle.positionY / cellSize)
            );
            const int cellZ = static_cast<int>(
                floorf(particle.positionZ / cellSize)
            );

            cellKeys[particleIndex] =
                encodeCudaCellCoordinates(cellX, cellY, cellZ);

            particleIndices[particleIndex] =
                static_cast<unsigned int>(particleIndex);
        }
    }

    CudaUniformGrid::~CudaUniformGrid()
    {
        release();
    }

    bool CudaUniformGrid::build(
        const CudaParticleBuffer& particleBuffer,
        float cellSize,
        cudaStream_t stream
    )
    {
        const std::size_t newParticleCount = particleBuffer.particleCount();

        if (newParticleCount == 0)
        {
            particleCount_ = 0;
            return true;
        }

        if (!ensureDeviceCapacity(newParticleCount))
        {
            return false;
        }

        const float safeCellSize = std::max(cellSize, 0.0001f);

        constexpr int threadsPerBlock = 256;
        const int blockCount = static_cast<int>(
            (newParticleCount + threadsPerBlock - 1) /
            threadsPerBlock
        );

        buildCellKeys<<<blockCount, threadsPerBlock, 0, stream>>>(
            particleBuffer.deviceData(),
            newParticleCount,
            safeCellSize,
            deviceCellKeys_,
            deviceParticleIndices_
        );

        if (!checkCuda(cudaGetLastError(), "buildCellKeys launch"))
        {
            return false;
        }

        try
        {
            thrust::device_ptr<CudaCellKey> keys(deviceCellKeys_);
            thrust::device_ptr<unsigned int> indices(deviceParticleIndices_);

            thrust::sort_by_key(
                thrust::cuda::par.on(stream),
                keys,
                keys + newParticleCount,
                indices
            );
        }
        catch (const thrust::system_error& error)
        {
            std::cerr << "CUDA grid sort failed: " << error.what() << '\n';
            return false;
        }

        particleCount_ = newParticleCount;
        cellSize_ = safeCellSize;
        return true;
    }

    bool CudaUniformGrid::download(
        std::vector<CudaCellKey>& cellKeys,
        std::vector<unsigned int>& particleIndices
    ) const
    {
        cellKeys.resize(particleCount_);
        particleIndices.resize(particleCount_);

        if (particleCount_ == 0)
        {
            return true;
        }

        const std::size_t keyByteCount =
            particleCount_ * sizeof(CudaCellKey);

        const std::size_t indexByteCount =
            particleCount_ * sizeof(unsigned int);

        return checkCuda(
                   cudaMemcpy(
                       cellKeys.data(),
                       deviceCellKeys_,
                       keyByteCount,
                       cudaMemcpyDeviceToHost
                   ),
                   "cudaMemcpy grid keys to host"
               ) &&
               checkCuda(
                   cudaMemcpy(
                       particleIndices.data(),
                       deviceParticleIndices_,
                       indexByteCount,
                       cudaMemcpyDeviceToHost
                   ),
                   "cudaMemcpy grid indices to host"
               );
    }

    const CudaCellKey* CudaUniformGrid::deviceCellKeys() const
    {
        return deviceCellKeys_;
    }

    const unsigned int* CudaUniformGrid::deviceParticleIndices() const
    {
        return deviceParticleIndices_;
    }

    std::size_t CudaUniformGrid::particleCount() const
    {
        return particleCount_;
    }

    float CudaUniformGrid::cellSize() const
    {
        return cellSize_;
    }

    bool CudaUniformGrid::ensureDeviceCapacity(
        std::size_t particleCount
    )
    {
        if (particleCount <= deviceCapacity_)
        {
            return true;
        }

        release();

        if (!checkCuda(
                cudaMalloc(
                    reinterpret_cast<void**>(&deviceCellKeys_),
                    particleCount * sizeof(CudaCellKey)
                ),
                "cudaMalloc grid keys"))
        {
            deviceCellKeys_ = nullptr;
            return false;
        }

        if (!checkCuda(
                cudaMalloc(
                    reinterpret_cast<void**>(&deviceParticleIndices_),
                    particleCount * sizeof(unsigned int)
                ),
                "cudaMalloc grid indices"))
        {
            cudaFree(deviceCellKeys_);
            deviceCellKeys_ = nullptr;
            deviceParticleIndices_ = nullptr;
            return false;
        }

        deviceCapacity_ = particleCount;
        return true;
    }

    void CudaUniformGrid::release()
    {
        if (deviceCellKeys_ != nullptr)
        {
            cudaFree(deviceCellKeys_);
            deviceCellKeys_ = nullptr;
        }

        if (deviceParticleIndices_ != nullptr)
        {
            cudaFree(deviceParticleIndices_);
            deviceParticleIndices_ = nullptr;
        }

        deviceCapacity_ = 0;
        particleCount_ = 0;
    }
}
