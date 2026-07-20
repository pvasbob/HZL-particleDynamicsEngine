#include "simulation/UniformGrid.h"

#include <algorithm>
#include <cmath>
#include <functional>

namespace hzl::simulation
{
    UniformGrid::UniformGrid(float cellSize)
        : cellSize_(std::max(cellSize, 0.0001f))
    {
    }

    void UniformGrid::rebuild(const std::vector<Particle>& particles)
    {
        cells_.clear();
        cells_.reserve(particles.size());

        for (std::size_t particleIndex = 0;
             particleIndex < particles.size();
             ++particleIndex)
        {
            int cellX = 0;
            int cellY = 0;
            int cellZ = 0;

            positionToCell(
                particles[particleIndex].position,
                cellX,
                cellY,
                cellZ
            );

            cells_[Cell{ cellX, cellY, cellZ }].push_back(particleIndex);
        }
    }

    void UniformGrid::positionToCell(
        const glm::vec3& position,
        int& cellX,
        int& cellY,
        int& cellZ
    ) const
    {
        cellX = static_cast<int>(std::floor(position.x / cellSize_));
        cellY = static_cast<int>(std::floor(position.y / cellSize_));
        cellZ = static_cast<int>(std::floor(position.z / cellSize_));
    }

    const std::vector<std::size_t>* UniformGrid::particleIndicesInCell(
        int cellX,
        int cellY,
        int cellZ
    ) const
    {
        const auto cellIterator = cells_.find(Cell{ cellX, cellY, cellZ });

        if (cellIterator == cells_.end())
        {
            return nullptr;
        }

        return &cellIterator->second;
    }

    std::size_t UniformGrid::CellHash::operator()(const Cell& cell) const
    {
        const std::size_t xHash = std::hash<int>{}(cell.x);
        const std::size_t yHash = std::hash<int>{}(cell.y);
        const std::size_t zHash = std::hash<int>{}(cell.z);

        return xHash ^
               (yHash + 0x9e3779b9U + (xHash << 6U) + (xHash >> 2U)) ^
               (zHash + 0x9e3779b9U + (yHash << 6U) + (yHash >> 2U));
    }
}
