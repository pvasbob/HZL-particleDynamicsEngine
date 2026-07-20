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

    hzl::simulation::ParticleSystemSettings settings;
    settings.gravity = glm::vec3(0.0f, -10.0f, 0.0f);
    settings.damping = 1.0f;
    settings.floorY = -100.0f;

    REQUIRE(integrator.integrate(
        particles,
        settings,
        0.1f
    ));

    REQUIRE(std::abs(particles[0].velocity.y - (-1.0f)) < 0.0001f);
    REQUIRE(std::abs(particles[0].position.y - (-0.1f)) < 0.0001f);
}

TEST_CASE("CUDA integrator resolves floor collision")
{
    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(0.0f, -0.97f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)
        }
    };

    hzl::simulation::ParticleSystemSettings settings;
    settings.gravity = glm::vec3(0.0f);
    settings.damping = 1.0f;
    settings.floorY = -1.0f;
    settings.particleRadius = 0.02f;
    settings.restitution = 0.75f;
    settings.restingSpeed = 0.02f;

    hzl::simulation::CudaParticleIntegrator integrator;

    REQUIRE(integrator.integrate(particles, settings, 0.1f));
    REQUIRE(std::abs(particles[0].position.y - (-0.98f)) < 0.0001f);
    REQUIRE(std::abs(particles[0].velocity.y - 0.75f) < 0.0001f);
}
