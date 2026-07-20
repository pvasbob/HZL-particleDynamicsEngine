#include <catch2/catch_test_macros.hpp>

#include "simulation/CudaParticleIntegrator.h"

#include <cmath>
#include <vector>

TEST_CASE("CUDA integrator updates particle motion")
{
    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
        }
    };

    hzl::simulation::CudaParticleIntegrator integrator;

    REQUIRE(integrator.integrate(
        particles,
        glm::vec3(0.0f, -10.0f, 0.0f),
        1.0f,
        0.1f
    ));

    REQUIRE(std::abs(particles[0].velocity.y - (-1.0f)) < 0.0001f);
    REQUIRE(std::abs(particles[0].position.y - (-0.1f)) < 0.0001f);
}
