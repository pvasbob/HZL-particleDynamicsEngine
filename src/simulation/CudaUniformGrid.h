#pragma once

#include "simulation/CudaParticleBuffer.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace hzl::simulation
{
    using CudaCellKey = std::uint64_t;

    class CudaUniformGrid
    {
    public:
        CudaUniformGrid() = default;
        ~CudaUniformGrid();

        CudaUniformGrid(const CudaUniformGrid&) = delete;
        CudaUniformGrid& operator=(const CudaUniformGrid&) = delete;

        bool build(
            const CudaParticleBuffer& particleBuffer,
            float cellSize
        );

        bool download(
            std::vector<CudaCellKey>& cellKeys,
            std::vector<unsigned int>& particleIndices
        ) const;

        const CudaCellKey* deviceCellKeys() const;
        const unsigned int* deviceParticleIndices() const;
        std::size_t particleCount() const;

    private:
        bool ensureDeviceCapacity(std::size_t particleCount);
        void release();

        CudaCellKey* deviceCellKeys_ = nullptr;
        unsigned int* deviceParticleIndices_ = nullptr;
        std::size_t deviceCapacity_ = 0;
        std::size_t particleCount_ = 0;
    };
}
