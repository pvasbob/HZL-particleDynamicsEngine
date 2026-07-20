#pragma once

#include "simulation/Particle.h"

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace hzl::simulation
{
    class UniformGrid
    {
    public:
        explicit UniformGrid(float cellSize);

        void rebuild(const std::vector<Particle>& particles);

        void positionToCell(
            const glm::vec3& position,
            int& cellX,
            int& cellY,
            int& cellZ
        ) const;

        const std::vector<std::size_t>* particleIndicesInCell(
            int cellX,
            int cellY,
            int cellZ
        ) const;

    private:
        struct Cell
        {
            int x = 0;
            int y = 0;
            int z = 0;

            bool operator==(const Cell&) const = default;
        };

        struct CellHash
        {
            std::size_t operator()(const Cell& cell) const;
        };

        float cellSize_;
        std::unordered_map<
            Cell,
            std::vector<std::size_t>,
            CellHash
        > cells_;
    };
}
