#include <catch2/catch_test_macros.hpp>

#include "simulation/CudaParticleCollisionSolver.h"

#include <cmath>
#include <vector>

TEST_CASE("CUDA collision solver resolves a head-on particle collision")
{
    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(-0.01f, 0.0f, 0.0f),
            glm::vec3(0.20f, 0.0f, 0.0f)
        },
        {
            glm::vec3(0.01f, 0.0f, 0.0f),
            glm::vec3(-0.20f, 0.0f, 0.0f)
        }
    };

    hzl::simulation::ParticleSystemSettings settings;
    settings.particleRadius = 0.02f;
    settings.particleRestitution = 1.0f;

    hzl::simulation::CudaParticleBuffer particleBuffer;
    hzl::simulation::CudaUniformGrid grid;
    hzl::simulation::CudaParticleCollisionSolver solver;

    REQUIRE(particleBuffer.upload(particles));
    REQUIRE(grid.build(particleBuffer, 2.0f * settings.particleRadius));
    REQUIRE(solver.resolve(particleBuffer, grid, settings));
    REQUIRE(particleBuffer.download(particles));

    REQUIRE(std::abs(particles[0].position.x - (-0.02f)) < 0.0001f);
    REQUIRE(std::abs(particles[1].position.x - 0.02f) < 0.0001f);
    REQUIRE(std::abs(particles[0].velocity.x - (-0.20f)) < 0.0001f);
    REQUIRE(std::abs(particles[1].velocity.x - 0.20f) < 0.0001f);
}
