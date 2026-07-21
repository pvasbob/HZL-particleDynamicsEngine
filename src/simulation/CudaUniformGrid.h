#pragma once

#include "simulation/CudaGridCell.h"
#include "simulation/CudaParticleBuffer.h"

#include <cuda_runtime_api.h>

#include <cstddef>
#include <vector>

namespace hzl::simulation
{
    class CudaUniformGrid
    {
    public:
        CudaUniformGrid() = default;
        ~CudaUniformGrid();

        CudaUniformGrid(const CudaUniformGrid&) = delete;
        CudaUniformGrid& operator=(const CudaUniformGrid&) = delete;

        bool build(
            const CudaParticleBuffer& particleBuffer,
            float cellSize,
            cudaStream_t stream = nullptr
        );

        bool download(
            std::vector<CudaCellKey>& cellKeys,
            std::vector<unsigned int>& particleIndices
        ) const;

        const CudaCellKey* deviceCellKeys() const;
        const unsigned int* deviceParticleIndices() const;
        std::size_t particleCount() const;
        float cellSize() const;

    private:
        bool ensureDeviceCapacity(std::size_t particleCount);
        void release();

        CudaCellKey* deviceCellKeys_ = nullptr;
        unsigned int* deviceParticleIndices_ = nullptr;
        std::size_t deviceCapacity_ = 0;
        std::size_t particleCount_ = 0;
        float cellSize_ = 0.0001f;
    };
}
