#include <catch2/catch_test_macros.hpp>

#include "simulation/CudaUniformGrid.h"

#include <vector>

TEST_CASE("CUDA uniform grid groups particles by cell")
{
    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(0.01f, 0.01f, 0.01f),
            glm::vec3(0.0f)
        },
        {
            glm::vec3(0.03f, 0.02f, 0.03f),
            glm::vec3(0.0f)
        },
        {
            glm::vec3(0.09f, 0.01f, 0.01f),
            glm::vec3(0.0f)
        }
    };

    hzl::simulation::CudaParticleBuffer particleBuffer;
    hzl::simulation::CudaUniformGrid grid;

    REQUIRE(particleBuffer.upload(particles));
    REQUIRE(grid.build(particleBuffer, 0.04f));

    std::vector<hzl::simulation::CudaCellKey> cellKeys;
    std::vector<unsigned int> particleIndices;

    REQUIRE(grid.download(cellKeys, particleIndices));
    REQUIRE(cellKeys.size() == 3);
    REQUIRE(particleIndices.size() == 3);

    REQUIRE(cellKeys[0] == cellKeys[1]);
    REQUIRE(cellKeys[1] < cellKeys[2]);

    REQUIRE(
        (particleIndices[0] == 0 || particleIndices[0] == 1)
    );
    REQUIRE(
        (particleIndices[1] == 0 || particleIndices[1] == 1)
    );
    REQUIRE(particleIndices[0] != particleIndices[1]);
    REQUIRE(particleIndices[2] == 2);
}
