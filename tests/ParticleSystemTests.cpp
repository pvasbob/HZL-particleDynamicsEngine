#include <catch2/catch_test_macros.hpp>

#include "simulation/ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <vector>

TEST_CASE("Gravity decreases a particle's vertical velocity")
{
    hzl::simulation::ParticleSystemSettings settings;
    settings.gravity = glm::vec3(0.0f, -10.0f, 0.0f);
    settings.damping = 1.0f;

    settings.floorY = -100.f;

    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
        }
    };

    hzl::simulation::ParticleSystem particleSystem(
        particles,
        settings
    );

    particleSystem.update(0.1f);

    const hzl::simulation::Particle& particle = 
        particleSystem.particles().front();

    REQUIRE(std::abs(particle.velocity.y - (-1.0f)) < 0.0001f);
    REQUIRE(std::abs(particle.position.y - (-0.1f)) < 0.0001f);

}

TEST_CASE("Floor collision clamps and bounces a particle")
{
    hzl::simulation::ParticleSystemSettings settings;

    settings.gravity = glm::vec3(0.0f);
    settings.damping = 1.0f;

    settings.floorY = -1.0f;
    settings.particleRadius = 0.02f;
    settings.restitution = 0.75f;
    settings.restingSpeed = 0.02f;

    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(0.0f, -0.97f, 0.0f),
            glm::vec3(0.0f, -1.00f, 0.0f)
        }
    };

    hzl::simulation::ParticleSystem particleSystem(
        particles,
        settings
    );

    particleSystem.update(0.1f);

    const hzl::simulation::Particle& particle = 
        particleSystem.particles().front();
    
    REQUIRE(
        std::abs(
            particle.position.y - (settings.floorY + settings.particleRadius)
        ) < 0.0001f
    );
    REQUIRE(std::abs(particle.velocity.y - 0.75f) < 0.0001f);

}

TEST_CASE("Right wall collision clamps and bounces a particle")
{
    hzl::simulation::ParticleSystemSettings settings;

    settings.gravity = glm::vec3(0.0f);
    settings.damping = 1.0f;

    settings.rightWallX = 1.5f;
    settings.particleRadius = 0.02f;
    settings.restitution = 0.75f;

    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(1.47f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
        }
    };

    hzl::simulation::ParticleSystem particleSystem(
        particles,
        settings
    );

    particleSystem.update(0.1f);

    const hzl::simulation::Particle& particle =
        particleSystem.particles().front();

    REQUIRE(
        std::abs(
            particle.position.x - (settings.rightWallX - settings.particleRadius)
        ) < 0.0001f
    );
    REQUIRE(std::abs(particle.velocity.x - (-0.75f)) < 0.0001f);
}



TEST_CASE("Head-on particle collision reverses equal velocities")
{
    hzl::simulation::ParticleSystemSettings settings;

    settings.gravity = glm::vec3(0.0f);
    settings.damping = 1.0f;

    settings.floorY = -100.0f;
    settings.particleRadius = 0.02f;
    settings.particleRestitution = 1.0f;

    std::vector<hzl::simulation::Particle> particles
    {
        {
            glm::vec3(-0.03f, 0.0f, 0.0f),
            glm::vec3( 0.20f, 0.0f, 0.0f)
        },
        {
            glm::vec3( 0.03f, 0.0f, 0.0f),
            glm::vec3(-0.20f, 0.0f, 0.0f)
        }
    };

    hzl::simulation::ParticleSystem particleSystem(
        particles,
        settings
    );

    particleSystem.update(0.1f);

    const std::vector<hzl::simulation::Particle>& updatedParticles =
        particleSystem.particles();

    REQUIRE(std::abs(updatedParticles[0].position.x - (-0.02f)) < 0.0001f);
    REQUIRE(std::abs(updatedParticles[1].position.x -   0.02f) < 0.0001f);

    REQUIRE(std::abs(updatedParticles[0].velocity.x - (-0.20f)) < 0.0001f);
    REQUIRE(std::abs(updatedParticles[1].velocity.x -   0.20f) < 0.0001f);
}

TEST_CASE("Additional collision iterations reduce dense overlap")
{
    hzl::simulation::ParticleSystemSettings settings;
    settings.gravity = glm::vec3(0.0f);
    settings.damping = 1.0f;
    settings.floorY = -100.0f;
    settings.particleRadius = 0.02f;
    settings.particleRestitution = 0.0f;

    const std::vector<hzl::simulation::Particle> particles
    {
        { glm::vec3(-0.015f, 0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.000f, 0.0f, 0.0f), glm::vec3(0.0f) },
        { glm::vec3( 0.015f, 0.0f, 0.0f), glm::vec3(0.0f) }
    };

    settings.collisionSolverIterations = 1;
    hzl::simulation::ParticleSystem oneIterationSystem(particles, settings);
    oneIterationSystem.update(0.0f);

    settings.collisionSolverIterations = 4;
    hzl::simulation::ParticleSystem fourIterationSystem(particles, settings);
    fourIterationSystem.update(0.0f);

    const std::vector<hzl::simulation::Particle>& oneIterationParticles =
        oneIterationSystem.particles();
    const std::vector<hzl::simulation::Particle>& fourIterationParticles =
        fourIterationSystem.particles();

    const float oneIterationMinimumDistance = std::min(
        oneIterationParticles[1].position.x - oneIterationParticles[0].position.x,
        oneIterationParticles[2].position.x - oneIterationParticles[1].position.x
    );

    const float fourIterationMinimumDistance = std::min(
        fourIterationParticles[1].position.x - fourIterationParticles[0].position.x,
        fourIterationParticles[2].position.x - fourIterationParticles[1].position.x
    );

    REQUIRE(fourIterationMinimumDistance > oneIterationMinimumDistance);
    REQUIRE(fourIterationMinimumDistance > 0.039f);
}
